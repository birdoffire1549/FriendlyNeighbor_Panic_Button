/*
  Settings - A class to contain, maintain, store and retreive settings needed
  by the application. This Class object is intented to be the sole manager of 
  data used throughout the applicaiton. It handles storing both volitile and 
  non-volatile data, where by definition the non-volitile data is persisted
  in flash memory and lives beyond the running life of the software and the 
  volatile data is lost and defaulted each time the software runs.

  Written by: Scott Griffis
  Date: 10-01-2023
*/

#include "Settings.h"

/**
 * #### CLASS CONSTRUCTOR ####
 * Allows for external instantiation of
 * the class into an object.
*/
Settings::Settings() {
    // Initially default the settings...
    defaultSettings();
}

/**
 * Performs a factory default on the information maintained by this class
 * where that the data is first set to its factory default settings then
 * it is persisted to flash.
 * 
 * @return Returns true if successful saving defaulted settings otherwise
 * returns false as bool.
*/
bool Settings::factoryDefault() {
    defaultSettings();
    bool ok = saveSettings();

    return ok;
}

/**
 * Used to load the settings from flash memory.
 * After the settings are loaded from flash memory the sentinel value is 
 * checked to ensure the integrity of the loaded data. If the sentinel 
 * value is wrong then the contents of the memory are deemed invalid and
 * the memory is wiped and then a factory default is instead performed.
 * 
 * @return Returns true if data was loaded from memory and the sentinel 
 * value was valid.
*/
bool Settings::loadSettings() {
    bool ok = false;
    // Setup EEPROM for loading and saving...
    EEPROM.begin(sizeof(NonVolatileSettings));

    // Persist default settings or load settings...
    delay(15);

    /* Load from EEPROM if applicable... */
    if (EEPROM.percentUsed() >= 0) { // Something is stored from prior...
        Serial.println(F("\nLoading settings from EEPROM..."));
        EEPROM.get(0, nvSettings);
        if (strcmp(nvSettings.sentinel, hashNvSettings(nvSettings).c_str()) != 0) { // Memory is corrupt...
            EEPROM.wipe();
            factoryDefault();
            Serial.println("Stored settings footprint invalid, stored settings have been wiped and defaulted!");
        } else { // Memory seems ok...
            Serial.print(F("Percent of ESP Flash currently used is: "));
            Serial.print(EEPROM.percentUsed());
            Serial.println(F("%"));
            ok = true;
        }
    }
    
    EEPROM.end();

    return ok;
}

/**
 * Used to save or persist the current value of the non-volatile settings
 * into flash memory.
 *
 * @return Returns a true if save was successful otherwise a false as bool.
*/
bool Settings::saveSettings() {
    strcpy(nvSettings.sentinel, hashNvSettings(nvSettings).c_str()); // Ensure accurate Sentinel Value.
    EEPROM.begin(sizeof(NonVolatileSettings));

    EEPROM.wipe(); // usage seemd to grow without this.
    EEPROM.put(0, nvSettings);
    
    bool ok = EEPROM.commit();

    EEPROM.end();
    
    return ok;
}

/**
 * Used to determine if the current network settings are in default values.
 * 
 * @return Returns a true if default values otherwise a false as bool. 
*/
bool Settings::isFactoryDefault() {
    
    return (strcmp(hashNvSettings(nvSettings).c_str(), hashNvSettings(factorySettings).c_str()) == 0);
}

/**
 * Used to check to see if the settings for connecting to a network have been altered from default.
 * To be considered non-default both ssid and pwd must have been changed from default values.
 * 
 * @return Returns true if the settings have been changed from default, otherwise returns false as bool.
*/
bool Settings::isNetworkSet() {
    if (getSsid().equals(String(factorySettings.ssid)) || getPwd().equals(String(factorySettings.pwd))) {
        
        return false;
    }

    return true;
}

/*
=================================================================
Getter and Setter Functions
=================================================================
*/


String Settings::getSsid() { // <------------------------------------------------- getSsid
  
    return String(nvSettings.ssid);
}

void Settings::setSsid(const char *ssid) { // <----------------------------------- setSsid
    if (sizeof(ssid) <= sizeof(nvSettings.ssid)) {
        strcpy(nvSettings.ssid, ssid);
    }
}


String Settings::getPwd() { // <-------------------------------------------------- getPwd

    return String(nvSettings.pwd);
}

void Settings::setPwd(const char *pwd) { // <------------------------------------- setPwd
    if (sizeof(pwd) <= sizeof(nvSettings.pwd)) {
        strcpy(nvSettings.pwd, pwd);
    }
}


String Settings::getOwner() { // <------------------------------------------------ getOwner

    return String(nvSettings.owner);
}

void Settings::setOwner(const char* owner) { // <--------------------------------- setOwner 
    if (sizeof(owner) <= sizeof(nvSettings.owner)) {
        strcpy(nvSettings.owner, owner);
    }
}


String Settings::getMessage() { // <---------------------------------------------- getMessage

    return String(nvSettings.message);
}

void Settings::setMessage(const char* message) { // <----------------------------- setMessage
    if (sizeof(message) <= sizeof(nvSettings.message)) {
        strcpy(nvSettings.message, message);
    }
}


String Settings::getSmtpHost() { // <--------------------------------------------- getSmtpHost

    return String(nvSettings.smtpHost);
}

void Settings::setSmtpHost(const char* host) { // <------------------------------- setSmtpHost
    if (sizeof(host) <= sizeof(nvSettings.smtpHost)) {
        strcpy(nvSettings.smtpHost, host);
    }
}


unsigned int Settings::getSmtpPort() { // <--------------------------------------- getSmtpPort

    return nvSettings.smtpPort;
}

void Settings::setSmtpPort(unsigned int port) { // <------------------------------ setSmtpPort
    nvSettings.smtpPort = port;
}


String Settings::getSmtpUser() { // <--------------------------------------------- getSmtpUser

    return String(nvSettings.smtpUser);
}

void Settings::setSmtpUser(const char* user) { // <------------------------------- setSmtpUser
    if (sizeof(user) <= sizeof(nvSettings.smtpUser)) {
        strcpy(nvSettings.smtpUser, user);
    }
}


String Settings::getSmtpPwd() { // <---------------------------------------------- getSmtpPwd

    return String(nvSettings.smtpPwd);
}

void Settings::setSmtpPwd(const char* pwd) { // <--------------------------------- setSmtpPwd
    if (sizeof(pwd) <= sizeof(nvSettings.smtpPwd)) {
        strcpy(nvSettings.smtpPwd, pwd);
    }
}


String Settings::getFromEmail() { // <-------------------------------------------- getFromEmail

    return String(nvSettings.fromEmail);
}

void Settings::setFromEmail(const char* email) { // <----------------------------- setFromEmail
    if (sizeof(email) <= sizeof(nvSettings.fromEmail)) {
        strcpy(nvSettings.fromEmail, email);
    }
}


String Settings::getFromName() { // <--------------------------------------------- getFromName

    return String(nvSettings.fromName);
}

void Settings::setFromName(const char* name) { // <------------------------------- setFromName
    if (sizeof(name) <= sizeof(nvSettings.fromName)) {
        strcpy(nvSettings.fromName, name);
    }
}


String Settings::getRecipients() { // <------------------------------------------- getRecipients
    
    return String(nvSettings.recipients);
}

void Settings::setRecipients(const char* recips) { // <--------------------------- setRecipients
    if (sizeof(recips) <= sizeof(nvSettings.recipients)) {
        strcpy(nvSettings.recipients, recips);
    }
}


bool Settings::getInPanicMode() { // <-------------------------------------------- getInPanicMode

    return nvSettings.inPanicMode;
}

void Settings::setInPanicMode(bool inPanic) { // <-------------------------------- setInPanicMode
    nvSettings.inPanicMode = inPanic;
}


int Settings::getPanicLevel() { // <---------------------------------------------- getPanicLevel

    return nvSettings.panicLevel;
}

void Settings::setPanicLevel(int level) { // <------------------------------------ setPanicLevel
    nvSettings.panicLevel = level;
}


String Settings::getAdminPwd() { // <--------------------------------------------- getAdminPwd

    return String(nvSettings.adminPwd);
}

void Settings::setAdminPwd(const char* pwd) { // <-------------------------------- setAdminPwd
    if (sizeof(pwd) <= sizeof(nvSettings.adminPwd)) {
        strcpy(nvSettings.adminPwd, pwd);
    }
}



String Settings::getHostname(String deviceId) {
    String temp = constSettings.hostnamePrefix;
    temp.concat(deviceId);

    return temp;
}


String Settings::getApSsid(String deviceId) {
    String temp = constSettings.apSsidPrefix;
    temp.concat(deviceId);
    
    return temp;
}


String Settings::getApPwd() {

    return constSettings.apPwd;
}


String Settings::getApNetIp() {

    return constSettings.apNetIp;
}


String Settings::getApSubnet() {

    return constSettings.apSubnet;
}


String Settings::getApGateway() {

    return constSettings.apGateway;
}


String Settings::getAdminUser() {

    return String(constSettings.adminUser);
}

/*
=================================================================
Private Functions
=================================================================
*/

/**
 * #### PRIVATE ####
 * This function is used to set or reset all settings to 
 * factory default values but does not persist the value 
 * changes to flash.
*/
void Settings::defaultSettings() {
    // Default the settings..
    strcpy(nvSettings.ssid, factorySettings.ssid);
    strcpy(nvSettings.pwd, factorySettings.pwd);
    strcpy(nvSettings.adminPwd, factorySettings.adminPwd);
    strcpy(nvSettings.owner, factorySettings.owner);
    strcpy(nvSettings.message, factorySettings.message);
    strcpy(nvSettings.smtpHost, factorySettings.smtpHost);
    nvSettings.smtpPort = factorySettings.smtpPort;
    strcpy(nvSettings.smtpUser, factorySettings.smtpUser);
    strcpy(nvSettings.smtpPwd, factorySettings.smtpPwd);
    strcpy(nvSettings.fromEmail, factorySettings.fromEmail);
    strcpy(nvSettings.fromName, factorySettings.fromName);
    nvSettings.inPanicMode = factorySettings.inPanicMode;
    nvSettings.panicLevel = factorySettings.panicLevel;
    strcpy(nvSettings.recipients, factorySettings.recipients);
    strcpy(nvSettings.sentinel, hashNvSettings(factorySettings).c_str());

    // Note: Volatile settings would be setup here if needed.
}

/**
 * Used to provide a hash of the given NonVolatileSettings.
 * 
 * @param nvSet An instance of NonVolatileSettings to calculate a hash for.
 * 
 * @return Returns the calculated hash value as String.
*/
String Settings::hashNvSettings(struct NonVolatileSettings nvSet) {
    String content = "";
    content = content + String(nvSet.ssid);
    content = content + String(nvSet.pwd);
    content = content + String(nvSet.adminPwd);
    content = content + String(nvSet.owner);
    content = content + String(nvSet.message);
    content = content + String(nvSet.smtpHost);
    content = content + String(nvSet.smtpPort);
    content = content + String(nvSet.smtpUser);
    content = content + String(nvSet.smtpPwd);
    content = content + String(nvSet.fromEmail);
    content = content + String(nvSet.fromName);
    content = content + String(nvSet.recipients);
    content = content + String(nvSet.inPanicMode);
    content = content + String(nvSet.panicLevel);

    MD5Builder builder = MD5Builder();
    builder.begin();
    builder.add(content);
    builder.calculate();

    return builder.toString();
}