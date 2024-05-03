#include <Arduino.h>
#include <Settings.h>
#include <IpUtils.h>
#include <Utils.h>
#include <ParseUtils.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServerSecure.h>
#include <Adafruit_SSD1306.h>
#include <DisplayWrapper.h>

#include <ESP_Mail_Client.h>

#include "ExampleSecrets.h"
#include "Secrets.h"
#include "HtmlContent.h"

#define FIRMWARE_VERSION "1.0.0"
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

void fileUploadHandler();
void notFoundHandler();
void endpointHandlerAdmin();
void endpointHandlerRoot();

void activateApMode();
void connectToNetwork();

void sendHtmlPageUsingTemplate(int code, String title, String heading, String &content);
bool handleAdminPageUpdates();

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
unsigned int lastVerifyDeviceStatusStep = 0U;
unsigned long lastInternetVerify = 0UL;

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
        display.show("Panic in... " + countDown);
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
        display.show("Cancel in... " + countDown);
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
    if (WiFi.getMode() == WIFI_STA && millis() - lastInternetVerify > 120000UL) {
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
    }
  } else if (lastVerifyDeviceStatusStep != 0U) {
    lastVerifyDeviceStatusStep = 0U;
  }

  delay(50);
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
    } else {
      Serial.println(F("Factory reset aborted."));
      display.show(F("Reset Aborted."));
      yield();
      delay(3000);
    }

    display.show(F("Initializing..."));
  } else { // Normal load restore pin not pressed...
    // Load from EEPROM if applicable...
    settings.loadSettings();
  }
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

  // /*
  //  Set the NTP config time
  //  For times east of the Prime Meridian use 0-12
  //  For times west of the Prime Meridian add 12 to the offset.
  //  Ex. American/Denver GMT would be -6. 6 + 12 = 18
  //  See https://en.wikipedia.org/wiki/Time_zone for a list of the GMT/UTC timezone offsets
  //  */
  // config.time.ntp_server = "pool.ntp.org,time.nist.gov";
  // config.time.gmt_offset = 3;
  // config.time.day_light_offset = 0;

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
  String addresses[addrCnt];
  ParseUtils::split(recips, ';', addresses, sizeof(addresses));
  for (String addr : addresses) {
    msg.addRecipient("", addr);
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
      msg.subject = String("Canceled:" + panicLevel + " Alert from: " + settings.getOwner());
      msg.text.content = F("The prior alert has been Canceled by the sender!");
      break;
  }

  SMTPSession smtp;
  smtp.debug(1);
  if (msgType == MT_ALERT) {
    smtp.callback([](SMTP_Status status) {
      Serial.printf("\nSend results...\n\tCompleated Count: %d\n\tFailed Count: %d\n\tInformation:\n\t\t%s\n\n", status.completedCount(), status.failedCount(), status.info());
      if (status.failedCount() != 0) {
        lastAlertSendError = true;
        if (status.completedCount() == 0) {
          display.show(F("Send Error!!!"));
          display.ledOn();
        } else {
          display.show(F("Partial Send!"));
          display.ledFlash();
          sendMessage(MessageType::MT_PARTIAL);
        }
      }
    });
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
  webServer.onNotFound(notFoundHandler);
  webServer.onFileUpload(fileUploadHandler);

  webServer.begin();
  Serial.println(F("Web-Server started."));
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
  if (!result.reserve(4000U)) {
    Serial.println(F("WARNING!!! htmlPageTemplate() failed to reserve desired memory!"));
  }

  result.replace("${title}", title);
  result.replace("${heading}", heading);
  result.replace("${content}", content);

  Serial.println("\ncontent: \n" + content);
  Serial.println("\nresult: \n" + result);

  webServer.send(code, "text/html", result);
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

  String content = INITIAL_SETUP_PAGE;
  if (!handleAdminPageUpdates()) { // Client response not yet handled...
    Serial.println("DEBUG!!! No update so admin page will be displayed!");

    bool isAPMode = (WiFi.getMode() == WIFI_AP);
    if (!isAPMode) {
      content = ADMIN_PAGE;
    }

    /* Fill out information that can only be edited in Initial Setup */
    content.replace("${ssid}", settings.getSsid());
    content.replace("${owner}", settings.getOwner());
    content.replace("${message}", settings.getMessage());
    content.replace("${smtp_host}", settings.getSmtpHost());
    content.replace("${smtp_port}", String(settings.getSmtpPort()));
    content.replace("${smtp_user}", settings.getSmtpUser());
    content.replace("${from_email}", settings.getFromEmail());
    content.replace("${from_name}", settings.getFromName());
    content.replace("${panic_level}", String(settings.getPanicLevel()));
    
    /* Fill out information that can only be edited and viewed in Initial Setup */
    if (isAPMode) { // In Initial Setup mode...
      content.replace("${pwd}", settings.getPwd());
      content.replace("${smtp_pwd}", settings.getSmtpPwd());  
    }
    
    /* Fill out information that can always be edited */
    content.replace("${recips}", settings.getRecipients());
    content.replace("${admin_pwd}", settings.getAdminPwd());

    sendHtmlPageUsingTemplate(200, F("Device Configuration Page"), F("Device Settings"), content);
  } else {
    Serial.println("DEBUG!!! Update so NO admin page will be displayed!");
  }
}

/**
 * This function handles incoming updates which are made via the Initial Settings
 * or Admin Page.
*/
bool handleAdminPageUpdates() {
  bool isChange = false;
  bool needReboot = false;
  bool changeSkipped = false;
  
  if (WiFi.getMode() == WIFI_AP) { // In initial setup...
    String ssid = webServer.arg("ssid");
    ssid.trim();
    if (!ssid.isEmpty() && !ssid.equals(settings.getSsid())) {
      if (ssid.length() < 33U) {
        settings.setSsid(ssid.c_str());
        isChange = true;
        needReboot = true;
      } else {
        changeSkipped = true;
      }
    }

    String msg = webServer.arg("message");
    msg.trim();
    if (!msg.isEmpty() && !msg.equals(settings.getMessage())) {
      if (msg.length() < 101U) {
        settings.setMessage(msg.c_str());
        isChange = true;
      } else {
        changeSkipped = true;
      }
    }

    String sHost = webServer.arg("smtp_host");
    sHost.trim();
    if (!sHost.isEmpty() && !sHost.equals(settings.getSmtpHost())) {
      if (sHost.length() < 121U) {
        settings.setSmtpHost(sHost.c_str());
        isChange = true;
      } else {
        changeSkipped = true;
      }
    }

    String sPort = webServer.arg("smtp_port");
    sPort.trim();
    if (!sPort.isEmpty() && !sPort.equals(String(settings.getSmtpPort()))) {
      unsigned int p = sPort.toInt();
      if (p > 0 && p <= 65535U) {
        settings.setSmtpPort(p);
        isChange = true;
      } else {
        changeSkipped = true;
      }
    }

    String sUser = webServer.arg("smtp_user");
    sUser.trim();
    if (!sUser.isEmpty() && !sUser.equals(settings.getSmtpUser())) {
      if (sUser.length() < 121U) {
        settings.setSmtpUser(sUser.c_str());
        isChange = true;
      } else {
        changeSkipped = true;
      }
    }

    String fEmail = webServer.arg("from_email");
    fEmail.trim();
    if (!fEmail.isEmpty() && !fEmail.equals(settings.getFromEmail())) {
      if (fEmail.length() < 121U) {
        settings.setFromEmail(fEmail.c_str());
        isChange = true;
      } else {
        changeSkipped = true;
      }
    }
    
    String fName = webServer.arg("from_name");
    fName.trim();
    if (!fName.isEmpty() && !fName.equals(settings.getFromName())) {
      if (fName.length() < 51U) {
        settings.setFromName(fName.c_str());
        isChange = true;
      } else {
        changeSkipped = true;
      }
    }

    String pLevel = webServer.arg("panic_level");
    pLevel.trim();
    if (!pLevel.isEmpty() && !pLevel.equals(String(settings.getPanicLevel()))) {
      int tmp = pLevel.toInt();
      if (tmp > 0 && tmp <= 5) {
        settings.setPanicLevel(tmp);
        isChange = true;
      } else {
        changeSkipped = true;
      }
    }

    String smtpPwd = webServer.arg("smtp_pwd");
    smtpPwd.trim();
    if (!smtpPwd.isEmpty() && !smtpPwd.equals(settings.getSmtpPwd())) {
      if (smtpPwd.length() < 121U) {
        settings.setSmtpPwd(smtpPwd.c_str());
        isChange = true;
      } else {
        changeSkipped = true;
      }
    }
    
    String pwd = webServer.arg("pwd");
    pwd.trim();
    if (!pwd.isEmpty() && !pwd.equals(settings.getPwd())) {
      if (pwd.length() < 64U) {
        settings.setPwd(pwd.c_str());
        isChange = true;
        needReboot = true;
      } else {
        changeSkipped = true;
      }
    }
  }
  
  /* Check and update Recipients */
  String recips = webServer.arg("recips");
  recips.trim(); // To make check isBlank.
  if (!recips.isEmpty() && !recips.equals(settings.getRecipients())) { // Something to there...
    if (recips.length() < 510U) { // Will fit in memory...
      settings.setRecipients(recips.c_str());
      isChange = true;
    } else {
      changeSkipped = true;
    }
  }

  /* Check and update Admin Password */
  String adminPwd = webServer.arg("admin_pwd");
  adminPwd.trim();
  if (!adminPwd.isEmpty() && !adminPwd.equals(settings.getAdminPwd())) {
    if (adminPwd.length() < 64U) {
      isChange = true;
      // TODO: Wish could expire past auths...
      settings.setAdminPwd(adminPwd.c_str());
    } else {
      changeSkipped = true;
    }
  }

  String content = "";

  if (changeSkipped) {
    content.concat("<h3>ERROR: No changes made because some requested changes were invalid due to length(s) or value(s)!</h3>");

    sendHtmlPageUsingTemplate(400, "Update Failure", "Update Failure", content);
    yield();

    return true;
  }

  if (isChange) {
    if (settings.saveSettings()) {
      if (needReboot) {
        content = "<h3>Settings update Successful!</h3><h4>Device will reboot now...</h4>";
        sendHtmlPageUsingTemplate(200, "Update Successful", "Update Result", content);
        delay(5000);

        ESP.restart();
      } else if (isChange) {
        content = "<h3>Settings update Successful!</h3>";
        sendHtmlPageUsingTemplate(200, "Update Successful", "Update Result", content);

        return true;
      }
    } else  {
      content = "<h3>Error Saving Settings!!!</h3>";
      sendHtmlPageUsingTemplate(500, "Internal Error", "500 - Internal Server Error", content);

        return true;
    }
  }

  return false;
}