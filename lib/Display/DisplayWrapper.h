#ifndef DisplayWrapper_h
    #define DisplayWrapper_h

    #include <Adafruit_SSD1306.h>

    #define SCREEN_ADDRESS 0x3C

    class DisplayWrapper {
        private:
            Adafruit_SSD1306* disp;
            int ledPin;

            unsigned int ledStatus = LOW;
            bool isLedFlashing = false;

        public:
            DisplayWrapper(Adafruit_SSD1306* display, int ledPin);

            void begin();
            void print(String text);
            void println(String text);
            void clear();
            void show(String text);
            void ledOn();
            void ledOff();
            void ledFlash();

            void run();
    };

#endif