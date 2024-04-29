#include "DisplayWrapper.h"

/**
 * Class Constructor
 * Used to set the display diver during instantiation.
 * 
 * @param display A reference to the Adafruit_SSD1306 display driver.
 * @param ledPin The pin of the alerting LED as int.
*/
DisplayWrapper::DisplayWrapper(Adafruit_SSD1306* display, int ledPin) {
  disp = display;
  this->ledPin = ledPin;
}

/**
 * Initializes and starts the display.
 * 
*/
void DisplayWrapper::begin() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  if (!disp->begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("FATAL: Tried and Failed to start display!!!"));
    yield();
    delay(10000);
    ESP.restart();
  }

  disp->setTextColor(SSD1306_WHITE);
  disp->cp437(true);  // Use full 256 char 'Code Page 437' font
  disp->setTextSize(1);
  
  disp->clearDisplay();
  disp->display();

  yield();
}

/**
 * Clears the display when called.
*/
void DisplayWrapper::clear() {
    disp->clearDisplay();
    disp->display();
    yield();
}

/**
 * Doesn't clear the display simply prints the given text to the
 * display. Since the display is not cleared first it is possible 
 * that depending on how full the display is already some or all 
 * of the text may not actually appear on the screen.
 * 
 * @param text The text to show on the screen as String.
*/
void DisplayWrapper::print(String text) {
    disp->print(text);
    disp->display();
    yield();
}

/**
 * Doesn't clear the display simply prints the given text to the
 * display along with a newline character at the end. Since the 
 * display is not cleared first it is possible that depending on
 * how full the display is already some or all of the text may not
 * actually appear on the screen.
 * 
 * @param text The text to show on the screen as String.
*/
void DisplayWrapper::println(String text) {
    disp->println(text);
    disp->display();
    yield();
}

/**
 * Clears the display and then shows the given text.
 * 
 * @param text The text to show on the screen as String.
*/
void DisplayWrapper::show(String text) {
    disp->clearDisplay();
    disp->setCursor(0, 0);
    disp->print(text);
    disp->display();
    yield();
}

/**
 * Turns on the alerting LED.
*/
void DisplayWrapper::ledOn() {
  ledStatus = HIGH;
  digitalWrite(ledPin, ledStatus);
  isLedFlashing = false;
}

/**
 * Turns off the alerting LED.
*/
void DisplayWrapper::ledOff() {
  ledStatus = LOW;
  digitalWrite(ledPin, ledStatus);
  isLedFlashing = false;
}

void DisplayWrapper::ledFlash() {
  isLedFlashing = true;
}

void DisplayWrapper::run() {
  static unsigned long lastSwitch = 0UL;
  if (isLedFlashing && millis() - lastSwitch >= 1000UL) {
    if (ledStatus == LOW) {
      ledStatus = HIGH;
    } else {
      ledStatus = LOW;
    }
    digitalWrite(ledPin, ledStatus);
    lastSwitch = millis();
  }
}