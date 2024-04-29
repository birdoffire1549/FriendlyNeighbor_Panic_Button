#ifndef Utils_h
    #define Utils_h

    #include <Arduino.h>
    #include <WString.h>

    class Utils {
        private:

        public:
            static String hashString(String string);
            static String genDeviceIdFromMacAddr(String macAddress);
    };

#endif
