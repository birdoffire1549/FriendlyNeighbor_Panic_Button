/*
 * Settings - A class to contain, maintain, store and retreive settings needed
 * by the application. This Class object is intented to be the sole manager of 
 * data used throughout the applicaiton. It handles storing both volitile and 
 * non-volatile data, where by definition the non-volitile data is persisted
 * in flash memory and lives beyond the running life of the software and the 
 * volatile data is lost and defaulted each time the software runs.
 *  
 * Written by: Scott Griffis
 * Date: 10-01-2023
*/

#ifndef Settings_h
    #define Settings_h

    #include <string.h> // NEEDED by ESP_EEPROM and MUST appear before WString
    #include <ESP_EEPROM.h>
    #include <WString.h>
    #include <HardwareSerial.h>
    #include <MD5Builder.h>

    // *****************************************************************************
    // Structure used for storing of settings related data and persisted into flash
    // *****************************************************************************
    struct NonVolatileSettings {
        char           ssid             [33]       ; // 32 chars is max size + 1 null
        char           pwd              [64]       ; // 63 chars is max size + 1 null
        char           owner            [101]      ;
        char           message          [101]      ;
        char           smtpHost         [121]      ;
        unsigned int   smtpPort                    ;
        char           smtpUser         [121]      ;
        char           smtpPwd          [121]      ; 
        char           fromEmail        [121]      ;
        char           fromName         [51]       ;
        char           recipients       [510]      ; // CSV 10 Addresses each max of 50 chars + null
        bool           inPanicMode                 ;
        int            panicLevel                  ;
        char           sentinel         [33]       ; // Holds a 32 MD5 hash + 1
    };
    
    class Settings {
        private:
            struct NonVolatileSettings nvSettings;
            const struct NonVolatileSettings factorySettings = {
                "SET_ME", // <-------------------------- ssid
                "SET_ME", // <-------------------------- pwd
                "SET_ME", // <-------------------------- owner
                "Please send help ASAP!", // <---------- message
                "SET_ME", // <-------------------------- smtpHost
                465, // <------------------------------- smtpPort
                "SET_ME", // <-------------------------- smtpUser
                "SET_ME", // <-------------------------- smtpPwd
                "no-reply@panic-button.com", // <------- fromEmail
                "FriendlyNeighbor PanicButton", // <---- fromName
                "test@email.com", // <------------------ recipients
                false, // <----------------------------- inPanicMode
                5, // <--------------------------------- panicLevel
                "NA" // <------------------------------- sentinel
            };

            // ******************************************************************
            // Structure used for storing of settings related data NOT persisted
            // ******************************************************************

            /*
                PLEASE NOTE: Just below is an example of what I would use if there
                were also some settings which were not constant but also not persisted
                to storage. I left this block here as a reminder incase I want to use 
                it later.
            */

            // struct VolatileSettings {
            //     bool           unused            ;
            // } vSettings;

            struct ConstantSettings {
                String         hostnamePrefix    ;
                String         apSsidPrefix      ;
                String         apPwd             ;
                String         apNetIp           ;
                String         apSubnet          ;
                String         apGateway         ;
                String         adminUser         ;
                String         adminPwd          ;
            } constSettings = {
                "FNPB-", // <--------------- hostnamePrefix (*later ID is added)
                "Panic_Button_", // <------- apSsidPrefix (*later ID is added)
                "P@ssw0rd123", // <--------- apPwd 
                "192.168.1.1", // <--------- apNetIp
                "255.255.255.0", // <------- apSubnet
                "0.0.0.0", // <------------- apGateway
                "admin", // <--------------- adminUser
                "P@ssw0rd123" // <---------- adminPwd
            };
            
            void defaultSettings();
            String hashNvSettings(struct NonVolatileSettings nvSet);


        public:
            Settings();

            bool factoryDefault();
            bool loadSettings();
            bool saveSettings();
            bool isFactoryDefault();
            bool isNetworkSet();

            NonVolatileSettings getFactorySettings();

            /*
            =========================================================
                                Getters and Setters 
            =========================================================
            */
            void           setSsid                    (const char* ssid)          ;
            String         getSsid                    ()                          ;
            bool           isSsidFactory              ()                          ;

            void           setPwd                     (const char* pwd)           ;
            String         getPwd                     ()                          ;
            bool           isPwdFactory               ()                          ;

            void           setOwner                   (const char* owner)         ;
            String         getOwner                   ()                          ;
            bool           isOwnerFactory             ()                          ;

            void           setMessage                 (const char* message)       ;
            String         getMessage                 ()                          ;
            bool           isMessageFactory           ()                          ;

            void           setSmtpHost                (const char* host)          ;
            String         getSmtpHost                ()                          ;
            bool           isSmtpHostFactory          ()                          ;

            void           setSmtpPort                (unsigned int port)         ;
            unsigned int   getSmtpPort                ()                          ;
            bool           isSmtpPortFactory          ()                          ;

            void           setSmtpUser                (const char* user)          ;
            String         getSmtpUser                ()                          ;
            bool           isSmtpUserFactory          ()                          ;

            void           setSmtpPwd                 (const char* pwd)           ;
            String         getSmtpPwd                 ()                          ;
            bool           isSmtpPwdFactory           ()                          ;

            void           setFromEmail               (const char* email)         ;
            String         getFromEmail               ()                          ;
            bool           isFromEmailFactory         ()                          ;

            void           setFromName                (const char* name)          ;
            String         getFromName                ()                          ;
            bool           isFromNameFactory          ()                          ;

            void           setRecipients              (const char* recips)        ;
            String         getRecipients              ()                          ;
            bool           isRecipientsFactory        ()                          ;

            void           setPanicLevel              (int level)                 ;
            int            getPanicLevel              ()                          ;
            bool           isPanicLevelFactory        ()                          ;

            void           setInPanicMode             (bool inPanic)              ;
            bool           getInPanicMode             ()                          ;
            

            String         getHostname       (String deviceId)        ;
            String         getApSsid         (String deviceId)        ;
            String         getApPwd          ()                       ;
            String         getApNetIp        ()                       ;
            String         getApSubnet       ()                       ;    
            String         getApGateway      ()                       ;
            String         getAdminUser      ()                       ;
            String         getAdminPwd       ()                       ;     
    };
    
#endif