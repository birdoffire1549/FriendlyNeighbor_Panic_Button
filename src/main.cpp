#include <Arduino.h>
#include <Settings.h>
#include <IpUtils.h>
#include <Utils.h>
#include <ParseUtils.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServerSecure.h>
#include <Adafruit_SSD1306.h>
#include <DisplayWrapper.h>
#include <ArduinoJson.h>

#include <ESP_Mail_Client.h>

#include "ExampleSecrets.h"
#include "Secrets.h"
#include "HtmlContent.h"

#define FIRMWARE_VERSION "1.3.6"
#define PANIC_BTN_PIN 12
#define CANCEL_BTN_PIN 13
#define LED_PIN 16

enum MessageType {
  MT_ALERT,
  MT_PARTIAL,
  MT_CANCEL
};

void resetOrLoadSettings();
void initNetwork();
void initDisplay();
void initWeb();
void sendMessage(enum MessageType messageType);
void dumpDeviceInfo();
bool isConnectionGood();
String getSettingsAsJson();
void fileUploadHandler();
void notFoundHandler();
void endpointHandlerAdmin();
void endpointHandlerUpdate();
void endpointHandlerRoot();

void activateApMode();
void connectToNetwork();

void sendHtmlPageUsingTemplate(int code, String title, String heading, String &content);

void doVerifyDeviceStatus();
void doHandleButtons();

Settings settings = Settings();

Adafruit_SSD1306 disp(128/*ScreenWidth*/, 32/*ScreenHeight*/, &Wire/*WireReference*/, -1/*OledReset*/);
DisplayWrapper display(&disp, LED_PIN);
BearSSL::ESP8266WebServerSecure webServer(/*Port*/443);
BearSSL::ServerSessions serverCache(/*Sessions*/4);

String deviceId = "";
bool lastAlertSendError = false; // TODO: Might use to flag periodic retries???
bool deviceInFaultStatus = false;
unsigned long lastInternetVerify = 0UL;
unsigned long lastInternetVerifySkip = 0UL;

/**
 * ==============================================================================
 * MAIN SETUP
 * ==============================================================================
 * This is the main setup function of the software, as such it runs first when
 * the software starts and only runs the one time. After this function the entire
 * runtime is controlled by the Loop function.
*/
void setup() {
  /* Initialize IOs */
  pinMode(PANIC_BTN_PIN, INPUT);
  pinMode(CANCEL_BTN_PIN, INPUT);

  /* Initialize Serial */
  Serial.begin(115200);
  yield();
  delay(50);

  Serial.println(F("\nInitializing device..."));

  /* Generate Device ID Based On MAC Address */
  deviceId = Utils::genDeviceIdFromMacAddr(WiFi.macAddress());

  /* Perform Device Initializations */
  initDisplay();
  resetOrLoadSettings();
  initNetwork();
  initWeb();

  Serial.println(F("Initialization complete."));

  /* Dump Device Information */
  dumpDeviceInfo();

  Serial.println(F("Device entering normal operating mode."));
  yield();
}

/**
 * =========================================================================
 * MAIN LOOP
 * =========================================================================
 * This is the main looping part of the software after setup has completed
 * this loop drive the remainder of the software's functionality for as long
 * as the device remains operational.
*/
void loop() {
  doVerifyDeviceStatus();
  webServer.handleClient();
  display.run();
  doHandleButtons();

  yield();
}

/**
 * This function does the handling of any and all button presses by the user
 * which take place durning the post setup normal running of the applicaiton.
 * This function DOES NOT handle the factory reset feature, as that is handled 
 * by the 'resetOrLoadSettings' function.
*/
void doHandleButtons() {
  if (!deviceInFaultStatus) {
    /* Check For IP Signal Request */
    if (
      digitalRead(CANCEL_BTN_PIN) == HIGH 
      && !settings.getInPanicMode() 
      && digitalRead(PANIC_BTN_PIN) == HIGH
    ) { // Tigger Special Action... 
      bool isShown = false;
      String ip = ((WiFi.getMode() == WIFI_AP) ? WiFi.softAPIP().toString() : WiFi.localIP().toString());
      while (digitalRead(CANCEL_BTN_PIN) == HIGH && digitalRead(PANIC_BTN_PIN) == HIGH) {
        if (!isShown) {
          display.show("IP: " + ip);
        }
        yield();
      }
    }

    /* Check For Panic Button */
    if (
      digitalRead(PANIC_BTN_PIN) == HIGH 
      && !settings.getInPanicMode() 
      && digitalRead(CANCEL_BTN_PIN) == LOW
    ) { // Prepare to trigger Panic Mode...
      int countDown = 3;
      while (digitalRead(PANIC_BTN_PIN) == HIGH && countDown != -1) {
        display.show("Panic in... " + String(countDown));
        countDown --;
        delay(1000);
      }
      if (countDown == -1) {
          display.show(F("Panic In Progress..."));
          display.ledFlash();
          settings.setInPanicMode(true);
          sendMessage(MessageType::MT_ALERT);
          while (digitalRead(PANIC_BTN_PIN) == HIGH) {
            yield();
          }
      } else {
        display.show(F("Panic Aborted."));
        display.ledOff();
        yield();
        delay(5000);
      }
    }

    /* Check for Panic Cancel */
    if (
      digitalRead(CANCEL_BTN_PIN) == HIGH 
      && settings.getInPanicMode() 
      && digitalRead(PANIC_BTN_PIN) == LOW
    ) { // Prepare to cancel Panic Mode...
      int countDown = 3;
      while (digitalRead(CANCEL_BTN_PIN) == HIGH && countDown != -1) {
        display.show("Cancel in... " + String(countDown));
        countDown --;
        delay(1000);
      }
      if (countDown == -1) {
          display.show(F("Panic Canceled."));
          display.ledOff();
          settings.setInPanicMode(false);
          sendMessage(MessageType::MT_CANCEL);
          yield();
          delay(5000);
          while (digitalRead(CANCEL_BTN_PIN) == HIGH) {
            yield();
          }
      } else {
        display.show(F("Cancel Aborted."));
        display.ledOff();
        yield();
        delay(5000);
      }
    }
  }
}

/**
 * This function does the verification of the device's operational status.
 * As such it is its job to notify the user of the device's status when 
 * not in an alert condition.
*/
void doVerifyDeviceStatus() {
  if (!settings.getInPanicMode()) {
    /* Device in AP Mode triggers a fault */
    if (!deviceInFaultStatus && WiFi.getMode() == WIFI_AP) {
      display.show(F("Setup Required!"));
      display.ledOn();
      deviceInFaultStatus = true;
    } 
    /* Periodic SMTP Host Checks */
    if (WiFi.getMode() == WIFI_STA) {
      if (millis() - lastInternetVerify > 120000UL) {
        if (isConnectionGood()) {
          display.show(F("System Ready."));
          display.ledOff();
          deviceInFaultStatus = false;
        } else if (!deviceInFaultStatus) {
          display.show(F("Internet Down?"));
          display.ledOn();
          deviceInFaultStatus = true;
        }

        lastInternetVerify = millis();
      } else if (!deviceInFaultStatus && millis() - lastInternetVerifySkip > 3000UL) {
        display.show(F("System Ready."));
        display.ledOff();

        lastInternetVerifySkip = millis();
      }
    }
  }
}

/**
 * This function verifies if the device can make a successful connection
 * to the SMTP Server. It returns true if the connection can be sucessfuly
 * made otherwise false indicates an issue between the device and server.
 * Issue could be network related, server related or credential related.
 * 
 * @return Returns true if connection is good, otherwise false as bool.
*/
bool isConnectionGood() {
  Session_Config config;
  config.server.host_name = settings.getSmtpHost();
  config.server.port = settings.getSmtpPort();
  config.login.email = settings.getSmtpUser();
  config.login.password = settings.getSmtpPwd();

  SMTPSession smtp;
  smtp.connect(&config);
  bool isConn = smtp.connected();
  smtp.closeSession();

  return isConn;
}

 /**
 * Detects and reacts to a reqest for factory reset
 * during the boot-up. Also loads settings from 
 * EEPROM if there are saved settings.
 */
void resetOrLoadSettings() {
  if (digitalRead(CANCEL_BTN_PIN) == HIGH) { // Restore button pressed on boot...
    Serial.println(F("Factory Reset?"));
    display.show(F("Factory Reset?"));
    int cntdwn = 3;
    unsigned long lastCount = millis();
    while (digitalRead(CANCEL_BTN_PIN) == HIGH && cntdwn >= 0) {
      yield();
      if (millis() - lastCount > 2000UL) {
        display.show("Factory Reset? " + String(cntdwn));
        Serial.println("Factory Reset? " + String(cntdwn));
        cntdwn --;
        lastCount = millis();
      }
    }

    if (cntdwn == -1) {
      Serial.println(F("\nPerforming Factory Reset..."));
      settings.factoryDefault();
      Serial.println(F("Factory reset complete."));
      display.show(F("Reset Complete!"));
      yield();
      delay(2000);

      while(digitalRead(CANCEL_BTN_PIN) == HIGH) { // Wait for pin to be released to continue...
        yield();
      }

      return;
    } 
    
    Serial.println(F("Factory reset aborted."));
    display.show(F("Reset Aborted."));
    yield();
    delay(3000);
    
    display.show(F("Initializing..."));
  }

  settings.loadSettings();
}

/**
 * Initializes the device's display.
 * 
*/
void initDisplay() {
  display.begin();
  display.show(F("Initializing..."));
  display.ledOn();
}

void sendMessage(enum MessageType msgType) {
  Session_Config config;
  config.server.host_name = settings.getSmtpHost();
  config.server.port = settings.getSmtpPort();
  config.login.email = settings.getSmtpUser();
  config.login.password = settings.getSmtpPwd();

  /* 
    Set the NTP config time
    For times east of the Prime Meridian use 0-12
    For times west of the Prime Meridian add 12 to the offset.
    Ex. American/Denver GMT would be -6. 6 + 12 = 18
    See https://en.wikipedia.org/wiki/Time_zone for a list of the GMT/UTC timezone offsets
  */
  config.time.ntp_server = "pool.ntp.org,time.nist.gov";
  config.time.gmt_offset = 18;
  config.time.day_light_offset = 0;

  SMTP_Message msg;
  msg.sender.name = settings.getFromName();
  msg.sender.email = settings.getFromEmail();
  String panicLevel = F("TEST");
  switch(settings.getPanicLevel()) {
    case 1:
      panicLevel = F("TEST");
    break;
    case 2:
      panicLevel = F("INFORMATIONAL");
    break;
    case 3:
      panicLevel = F("WARNING");
    break;
    case 4:
      panicLevel = F("CRITICAL");
    break;
    case 5:
      panicLevel = F("EMERGENCY");
    break;
  } 

  String recips = settings.getRecipients();
  unsigned int addrCnt = ParseUtils::occurrences(recips, ";");
  addrCnt ++; // because ';' only needed if more than 1 address
  String addresses[addrCnt];
  ParseUtils::split(recips, ';', addresses, sizeof(addresses));
  for (String addr : addresses) {
    addr.trim();
    if (!addr.isEmpty()) {
      msg.addRecipient("", addr);
    }
  }
  
  switch (msgType) {
    case MT_ALERT:
      msg.subject = String(panicLevel + " Alert from: " + settings.getOwner());
      msg.text.content = settings.getMessage();
      break;
    case MT_PARTIAL:
      msg.subject = String(panicLevel + " Alert from: " + settings.getOwner());
      msg.text.content = F("Not all recipients were able to receive the alert!\nYou may want to take that into account with your response!!!");
      break;
    case MT_CANCEL:
      msg.subject = String("Canceled: " + panicLevel + " Alert from: " + settings.getOwner());
      msg.text.content = F("The prior alert has been Canceled by the sender!");
      break;
  }

  SMTPSession smtp;
  smtp.debug(1);
  if (msgType == MT_ALERT) {
    smtp.callback([](SMTP_Status status) { // <---- Start of CallBack Function
      Serial.printf("\nSend results...\n\tCompleated Count: %d\n\tFailed Count: %d\n\tInformation:\n\t\t%s\n\n", status.completedCount(), status.failedCount(), status.info());
      if (status.failedCount() != 0) { // Some or all sends failed...
        lastAlertSendError = true;
        if (status.completedCount() == 0) { // No sends completed...
          display.show(F("Send Error!!!"));
          display.ledOn();
        } else { // Partial send occurred...
          display.show(F("Partial Send!"));
          display.ledFlash();
          // sendMessage(MessageType::MT_PARTIAL); // FIXME: Could result in many partial messages as callback gets called many times during send.
        }
      }
    }); // <---- End of CallBack Function
  } else if (msgType == MT_PARTIAL) {
    smtp.callback([](SMTP_Status status) {
      Serial.printf("\nSend results...\n\tCompleated Count: %d\n\tFailed Count: %d\n\tInformation:\n\t\t%s\n\n", status.completedCount(), status.failedCount(), status.info());
      if (status.failedCount() != 0) {
        if (status.completedCount() == 0) {
          display.show(F("Send Error!!!"));
          display.ledOn();
        } else {
          display.show(F("Partial Send!"));
          display.ledFlash();
        }
        lastAlertSendError = true;
      }
    });
  } else if (msgType == MT_CANCEL) {
    smtp.callback([](SMTP_Status status) {
      Serial.printf("\nSend results...\n\tCompleated Count: %d\n\tFailed Count: %d\n\tInformation:\n\t\t%s\n\n", status.completedCount(), status.failedCount(), status.info());
      if (status.failedCount() != 0) {
        if (status.completedCount() == 0) {
          display.show("Send Error!!!");
          display.ledOn();
          yield();
          delay(3000);
        } else {
          display.show("Partial Send!");
          display.ledFlash();
          yield();
          delay(3000);
        }
      }
    });
  }

  smtp.connect(&config);
  if (!MailClient.sendMail(&smtp, &msg)) {
    display.show(F("Send Error!!!"));
    display.ledOn();
    Serial.println("Error Sending, Reason: " + smtp.errorReason());
    if (msgType == MT_ALERT || msgType == MT_PARTIAL) {
      lastAlertSendError = true;
    } else if (msgType == MT_CANCEL) {
      yield();
      delay(3000);
    }
  } else {
    if (msgType == MT_ALERT || msgType == MT_PARTIAL) {
      display.show(F("Alerts Sent!"));
      display.ledFlash();
      Serial.println(F("Alerts have been successfuly sent!"));
      lastAlertSendError = false;
    } else if (msgType == MT_CANCEL) {
      display.show("Cancel Sent!");
      display.ledFlash();
      Serial.println(F("Cancel has been successfuly sent!"));
      yield();
      delay(3000);
      lastAlertSendError = false;  // No matter what because cancel is best effort.
    }
  }
}

/**
 * This funciton is called to initialize the network of the device.
 * Either the device is put into AP Mode if not configured, or it
 * will attempt to connect to the assigned network.
*/
void initNetwork() {
  if (!settings.isNetworkSet()) {
    activateApMode();
  } else {
    display.show(F("Connecting..."));
    display.ledOn();
    connectToNetwork();
  }
}

/**
 * Puts the device into AP Mode so that user can
 * connect via WiFi directly to the device to configure
 * the device.
 */
void activateApMode() {
  Serial.print(F("Configuring AP mode... "));
  
  WiFi.setOutputPower(20.5F);
  WiFi.setHostname(settings.getHostname(deviceId).c_str());
  WiFi.mode(WiFiMode::WIFI_AP);
  WiFi.softAPConfig(
    IpUtils::stringIPv4ToIPAddress(settings.getApNetIp()), 
    IpUtils::stringIPv4ToIPAddress(settings.getApGateway()), 
    IpUtils::stringIPv4ToIPAddress(settings.getApSubnet())
  );

  bool ret = WiFi.softAP(settings.getApSsid(deviceId), settings.getApPwd());
  if (ret) { // AP Mode enabled...
    Serial.printf(
      "\nUse the following information to connect to and configure the device:\n\tSSID: '%s'\n\tPwd: '%s'\n\n\tAdmin Page: 'https://%s/admin'\n\tAdmin User: '%s'\n\tAdmin Pwd: '%s'\n\n", 
      settings.getApSsid(deviceId).c_str(), 
      settings.getApPwd().c_str(),
      settings.getApNetIp().c_str(),
      settings.getAdminUser().c_str(),
      settings.getAdminPwd().c_str()
    );
  } else { // Unable to enable AP Mode...
    Serial.println(F("FATAL: AP Setup Failed!"));
    display.show(F("AP Failed!!!"));
    exit(1);
  }
}

/**
 * Puts the device into client mode such that it will
 * connect to a specified WiFi network based on its
 * SSID and Password.
 */
void connectToNetwork() {
  Serial.printf("\n\nConnecting to: %s...\n", settings.getSsid().c_str());
  
  WiFi.setOutputPower(20.5F);
  WiFi.setHostname(settings.getHostname(deviceId).c_str());
  WiFi.mode(WiFiMode::WIFI_STA);
  WiFi.begin(settings.getSsid(), settings.getPwd());

  while (WiFi.status() != WL_CONNECTED) { // Waiting for WiFi to connect...
    delay(500);
    yield();
  }

  Serial.println(F("WiFi connected."));
}

/**
 * Initializes the web server which is used so that the device can be configured initially.
 * The web server is not started when the device is configured. As such it is only ever
 * started when in AP mode.
*/
void initWeb() {
  if (WiFi.getMode() == WIFI_STA) {
    webServer.stop();
    Serial.println(F("Device is configured so WebService will not be started!"));
  } else {
    Serial.println(F("Initializing Web-Server..."));
    #ifndef Secrets_h
      webServer.getServer().setRSACert(new BearSSL::X509List(SAMPLE_SERVER_CERT), new BearSSL::PrivateKey(SAMPLE_SERVER_KEY));
    #else
      webServer.getServer().setRSACert(new BearSSL::X509List(server_cert), new BearSSL::PrivateKey(server_key));
    #endif
    webServer.getServer().setCache(&serverCache);

    /* Setup Endpoint Handlers */
    webServer.on(F("/"), endpointHandlerRoot);
    webServer.on(F("/admin"), endpointHandlerAdmin);
    webServer.on(F("/update"), endpointHandlerUpdate);
    webServer.onNotFound(notFoundHandler);
    webServer.onFileUpload(fileUploadHandler);

    webServer.begin();
    Serial.println(F("Web-Server started."));
  }
}

/**
 * Dumps the Device's information to the Serial console.
*/
void dumpDeviceInfo() {
    Serial.println(F("\n\n=================================="));
    Serial.printf("Device ID: %s\n", deviceId.c_str());
    Serial.printf("Firmware Version: %s\n", FIRMWARE_VERSION);
    Serial.println(F("==================================\n"));
}

/**
 * This function is used to Generate the HTML for a web page where the 
 * title, heading and content is provided to the function as a String 
 * type, then inserted into template HTML and finally sent to client.
 * 
 * @param code The HTTP Code as int.
 * @param title The page title as String.
 * @param heading The page heading as String.
 * @param content A reference to the main content of the page as String.
 */
void sendHtmlPageUsingTemplate(int code, String title, String heading, String &content) {
  String result = HTML_PAGE_TEMPLATE;
  if (!result.reserve(3000U)) {
    Serial.println(F("WARNING!!! Failed to reserve desired memory for webpage!"));
  }

  result.replace("${title}", title);
  result.replace("${heading}", heading);
  result.replace("${content}", content);

  webServer.send_P(code, "text/html", result.c_str());
  yield();
}

/**
 * #### HANDLER - NOT FOUND ####
 * This is a function which is used to handle web requests when the requested resource is not valid.
 * 
*/
void notFoundHandler() {
  String content = F("Just kidding...<br>But seriously what you were looking for doesn't exist.");
  
  sendHtmlPageUsingTemplate(404, F("404 Not Found"), F("OOPS! You broke it!!!"), content);
}

/**
 * #### HANDLER - File Upload ####
 * This function handles file upload requests.
*/
void fileUploadHandler() {
  String content = F("Um, I don't want your nasty files, go peddle that junk elsewhere!");
  
  sendHtmlPageUsingTemplate(400, F("400 Bad Request"), F("Uhhh, Wuuuuut!?"), content);
}

/**
 * This is the endpoint handler for the root.
 * 
*/
void endpointHandlerRoot() {
  String content = ROOT_PAGE;
  content.replace("${firmware_version}", String(FIRMWARE_VERSION));

  sendHtmlPageUsingTemplate(200, F("Device Information"), F("Information"), content);
}

/**
 * Endpoint Handler Function
 * This function handles the Admin Endpoint.
*/
void endpointHandlerAdmin() {
  /* Ensure user authenticated */
  Serial.println(F("Client requested access to '/admin'."));
  if (!webServer.authenticate(settings.getAdminUser().c_str(), settings.getAdminPwd().c_str())) { // User not authenticated...
    Serial.println(F("Client not(yet) Authenticated!"));

    return webServer.requestAuthentication(DIGEST_AUTH, "AdminRealm", "Authentication failed!");
  }
  Serial.println(F("Client has been Authenticated."));

  String content = ADMIN_PAGE;
  content.replace("${settings}", getSettingsAsJson());
    
  sendHtmlPageUsingTemplate(200, F("Device Configuration Page"), F("Device Settings"), content);
}


void endpointHandlerUpdate() {
  /* Ensure user authenticated */
  Serial.println(F("Client requested access to '/update'."));
  if (!webServer.authenticate(settings.getAdminUser().c_str(), settings.getAdminPwd().c_str())) { // User not authenticated...
    Serial.println(F("Client not(yet) Authenticated!"));

    return webServer.requestAuthentication(DIGEST_AUTH, "AdminRealm", "Authentication failed!");
  }
  Serial.println(F("Client has been Authenticated."));

  String setts = webServer.arg("data");
  setts.trim();
  JsonDocument jDoc;

  if (!setts.isEmpty()) {
    DeserializationError err = deserializeJson(jDoc, setts);
    if (err.code() != DeserializationError::Ok) { // Problem with incoming JSON...
      String errMsg = String("Deserialization of JSON settings failed: ") + err.c_str();
      Serial.println(errMsg);
      
      return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), errMsg);
    } else { // JSON Seemed good...
      /* ************ *
       * UPDATE: ssid *
       * ************ */
      String ssid = jDoc["ssid"];
      ssid.trim();
      if (!ssid.isEmpty() && !ssid.equals(String(settings.getFactorySettings().ssid))) {
        if (ssid.length() < 33U) {
          settings.setSsid(ssid.c_str());
        } else {
          String msg = F("SSID must not be longer than 32 characters!");
          Serial.println(msg);
          
          return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
        }
      } else {
        String msg = F("SSID is required for configuration!");
        sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
        Serial.println(msg);
        
        return; 
      }

      /* *********** *
       * UPDATE: pwd *
       * *********** */
      String pwd = jDoc["pwd"];
      pwd.trim();
      if (!pwd.isEmpty() && !pwd.equals(String(settings.getFactorySettings().pwd))) {
        if (pwd.length() < 64U) {
          settings.setPwd(pwd.c_str());
        } else {
          String msg = F("Pwd must not be longer than 63 characters!");
          Serial.println(msg);
          
          return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
        }
      } else {
        String msg = F("Pwd is required for configuration!");
        Serial.println(msg);
        
        return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
      }

      /* ***************** *
       * UPDATE: smtp_host * 
       * ***************** */
      String sHost = jDoc["smtp_host"];
      sHost.trim();
      if (!sHost.isEmpty() && !sHost.equals(String(settings.getFactorySettings().smtpHost))) {
        if (sHost.length() < 121U) {
          settings.setSmtpHost(sHost.c_str());
        } else {
          String msg = F("SMTP Host must not be longer than 120 characters!");
          Serial.println(msg);
          
          return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
        }
      } else {
        String msg = F("SMTP Host is required for configuration!");
        Serial.println(msg);
        
        return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
      }

      /* ***************** *
       * UPDATE: smtp_port *
       * ***************** */
      String sPort = jDoc["smtp_port"];
      sPort.trim();
      if (!sPort.isEmpty()) {
        unsigned int p = sPort.toInt();
        if (p > 0 && p <= 65535U) {
          settings.setSmtpPort(p);
        } else {
          String msg = F("SMTP Port must be within valid port range!");
          Serial.println(msg);
          
          return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
        }
      } else {
        String msg = F("SMTP Port is required for configuration!");
        Serial.println(msg);
        
        return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
      }

      /* ***************** *
       * UPDATE: smtp_user *
       * ***************** */
      String sUser = jDoc["smtp_user"];
      sUser.trim();
      if (!sUser.isEmpty() && !sUser.equals(String(settings.getFactorySettings().smtpUser))) {
        if (sUser.length() < 121U) {
          settings.setSmtpUser(sUser.c_str());
        } else {
          String msg = F("SMTP User must be no longer than 120 characters in length!");
          Serial.println(msg);
          
          return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
        }
      } else {
        String msg = F("SMTP User is required for configuration!");
        Serial.println(msg);
        return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
      }

      /* **************** *
       * UPDATE: smtp_pwd *
       * **************** */
      String smtpPwd = jDoc["smtp_pwd"];
      smtpPwd.trim();
      if (!smtpPwd.isEmpty() && !smtpPwd.equals(String(settings.getFactorySettings().smtpPwd))) {
        if (smtpPwd.length() < 121U) {
          settings.setSmtpPwd(smtpPwd.c_str());
        } else {
          String msg = F("SMTP Password must be no longer than 120 characters in length!");
          Serial.println(msg);
          
          return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
        }
      } else {
        String msg = F("SMTP Password is required for configuration!");
        Serial.println(msg);
        
        return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
      }

      /* ***************** *
       * UPDATE: from_name *
       * ***************** */
      String fName = jDoc["from_name"];
      fName.trim();
      if (!fName.isEmpty()) {
        if (fName.length() < 51U) {
          settings.setFromName(fName.c_str());
        } else {
          String msg = F("The 'From Name' must be no longer than 50 characters in length!");
          Serial.println(msg);
          
          return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
        }
      } else {
        String msg = F("The 'From Name' is required for configuration!");
        Serial.println(msg);
        
        return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
      }

      /* ****************** *
       * UPDATE: from_email *
       * ****************** */
      String fEmail = jDoc["from_email"];
      fEmail.trim();
      if (!fEmail.isEmpty()) {
        if (fEmail.length() < 121U) {
          settings.setFromEmail(fEmail.c_str());
        } else {
          String msg = F("The 'From Email' must be no longer than 120 characters in length!");
          Serial.println(msg);
          
          return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
        }
      } else {
        String msg = F("The 'From Email' is required for configuration!");
        Serial.println(msg);
        
        return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
      }

      /* ************* *
       * UPDATE: owner *
       * ************* */
      String owner = jDoc["owner"];
      owner.trim();
      if (!owner.isEmpty() && !owner.equals(String(settings.getFactorySettings().owner))) {
        if (owner.length() < 101U) {
          settings.setOwner(owner.c_str());
        } else {
          String msg = F("Owner must be no longer than 100 characters in length!");
          Serial.println(msg);
          
          return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
        }
      } else {
        String msg = F("Owner is required for configuration!");
        Serial.println(msg);
        
        return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
      }

      /* *************** *
       * UPDATE: message *
       * *************** */
      String msg = jDoc["message"];
      msg.trim();
      if (!msg.isEmpty()) {
        if (msg.length() < 101U) {
          settings.setMessage(msg.c_str());
        } else {
          String msg = F("Message must be no longer than 100 characters in length!");
          Serial.println(msg);

          return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
        }
      } else {
        String msg = F("Message is required for configuration!");
        Serial.println(msg);
        
        return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
      }

      /* ******************* *
       * UPDATE: panic_level *
       * ******************* */
      String pLevel = jDoc["panic_level"];
      pLevel.trim();
      if (!pLevel.isEmpty()) {
        int tmp = pLevel.toInt();
        if (tmp > 0 && tmp <= 5) {
          settings.setPanicLevel(tmp);
        } else {
          String msg = F("Panic Level must be greater than 0 and less than 6!");
          Serial.println(msg);
          
          return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
        }
      } else {
        String msg = F("Panic Level is required for configuration!");
        Serial.println(msg);
        
        return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
      }

      /* ****************** *
       * UPDATE: recipients *
       * ****************** */
      String recips = jDoc["recipients"];
      recips.trim(); // To make check isBlank.
      if (!recips.isEmpty() && !recips.equals(String(settings.getFactorySettings().recipients))) { // Something to there...
        if (recips.length() < 510U) { // Will fit in memory...
          settings.setRecipients(recips.c_str());
        } else {
          String msg = F("Recipients must be no longer than 509 characters in length!");
          Serial.println(msg);
          
          return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
        }
      } else {
        String msg = F("Recipients are required for configuration!");
        Serial.println(msg);
        
        return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), msg);
      }
      
      /* Save Settings to Flash */
      if (settings.saveSettings()) {
        String content = "<h3>Settings update Successful!</h3><h4>Device will reboot now...</h4>";
        sendHtmlPageUsingTemplate(200, F("Update Successful"), F("Update Result"), content);
        Serial.println(content);

        ESP.restart();
      } else  {
        String content = "<h3>Error Saving Settings!!!</h3>";
        Serial.println(content);
        
        return sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), content);
      }
    }
  }

  String content = "Update request didn't contain any data! Sending admin page content!";
  Serial.println(content);
  endpointHandlerAdmin();
  // sendHtmlPageUsingTemplate(500, F("500 - Internal Server Error"), F("500 - Internal Server Error"), content);
}

String getSettingsAsJson() {
  JsonDocument jDoc;
  jDoc["ssid"] = settings.getSsid();
  jDoc["pwd"] = settings.getPwd();
  jDoc["smtp_host"] = settings.getSmtpHost();
  jDoc["smtp_port"] = settings.getSmtpPort();
  jDoc["smtp_user"] = settings.getSmtpUser();
  jDoc["smtp_pwd"] = settings.getSmtpPwd();
  jDoc["from_name"] = settings.getFromName();
  jDoc["from_email"] = settings.getFromEmail();
  jDoc["owner"] = settings.getOwner();
  jDoc["message"] = settings.getMessage();
  jDoc["panic_level"] = settings.getPanicLevel();
  jDoc["recipients"] = settings.getRecipients();

  String output;
  serializeJsonPretty(jDoc, output);

  return output;
}