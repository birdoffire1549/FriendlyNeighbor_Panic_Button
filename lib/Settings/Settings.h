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

    class Settings {
        private:
            // *****************************************************************************
            // Structure used for storing of settings related data and persisted into flash
            // *****************************************************************************
            struct NonVolatileSettings {
                char           ssid             [33]       ; // 32 chars is max size + 1 null
                char           pwd              [64]       ; // 63 chars is max size + 1 null
                char           adminPwd         [64]       ;
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

            struct NonVolatileSettings nvSettings;
            struct NonVolatileSettings factorySettings = {
                "SET_ME", // <-------------------------- ssid
                "SET_ME", // <-------------------------- pwd
                "P@ssw0rd123", // <--------------------- adminPwd
                "Jane Doe", // <------------------------ owner
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
            } constSettings = {
                "FNPB-", // <--------------- hostnamePrefix (*later ID is added)
                "Panic_Button_", // <------- apSsidPrefix (*later ID is added)
                "P@ssw0rd123", // <--------- apPwd 
                "192.168.1.1", // <--------- apNetIp
                "255.255.255.0", // <------- apSubnet
                "0.0.0.0", // <------------- apGateway
                "admin" // <---------------- adminUser
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

            /*
            =========================================================
                                Getters and Setters 
            =========================================================
            */
            void           setSsid                    (const char* ssid)          ;
            String         getSsid                    ()                          ;
            void           setPwd                     (const char* pwd)           ;
            String         getPwd                     ()                          ;
            void           setOwner                   (const char* owner)         ;
            String         getOwner                   ()                          ;
            void           setMessage                 (const char* message)       ;
            String         getMessage                 ()                          ;
            void           setSmtpHost                (const char* host)          ;
            String         getSmtpHost                ()                          ;
            void           setSmtpPort                (unsigned int port)         ;
            unsigned int   getSmtpPort                ()                          ;
            void           setSmtpUser                (const char* user)          ;
            String         getSmtpUser                ()                          ;
            void           setSmtpPwd                 (const char* pwd)           ;
            String         getSmtpPwd                 ()                          ;
            void           setFromEmail               (const char* email)         ;
            String         getFromEmail               ()                          ;
            void           setFromName                (const char* name)          ;
            String         getFromName                ()                          ;
            void           setRecipients              (const char* recips)        ;
            String         getRecipients              ()                          ;
            void           setPanicLevel              (int level)                 ;
            int            getPanicLevel              ()                          ;
            void           setInPanicMode             (bool inPanic)              ;
            bool           getInPanicMode             ()                          ;
            void           setAdminPwd                (const char* pwd)           ;
            String         getAdminPwd                ()                          ;


            String         getHostname       (String deviceId)        ;
            String         getApSsid         (String deviceId)        ;
            String         getApPwd          ()                       ;
            String         getApNetIp        ()                       ;
            String         getApSubnet       ()                       ;    
            String         getApGateway      ()                       ;
            String         getAdminUser      ()                       ;     
    };
    
#endif