#include <Homie.h>

#define PIN_BUTTON 0
#define PIN_RELAY 12	// active on HIGH
#define PIN_LED 13		// active on LOW

HomieNode switchNode("switch", "switch");

#define FW_NAME "sonoff-slampher"
#define FW_VERSION "1.0.0"

/* Magic sequence for Autodetectable Binary Upload */
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";
/* End of magic sequence for Autodetectable Binary Upload */

int relayState = HIGH;               // the default relay state
int buttonState = HIGH;              // the current reading from the input pin
int lastButtonState = HIGH;          // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

bool switchOnHandler(String value) {
  if (value == "true") {
    relayState = HIGH;
  } else if (value == "false") {
    relayState = LOW;
  } else {
    return false;
  }
  digitalWrite(PIN_RELAY, relayState);
  digitalWrite(PIN_LED, relayState);
  Homie.setNodeProperty(switchNode, "on", (relayState == HIGH) ? "true" : "false", true);
  Serial.print("Switch is "); Serial.println((relayState == HIGH) ? "on" : "off");
  return true;
}

void setupHandler() {
  // publish current relayState when online
  Homie.setNodeProperty(switchNode, "on", (relayState == HIGH) ? "true" : "false", true);
}

void setup() {
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, relayState);
  Homie.setFirmware(FW_NAME, FW_VERSION);
  Homie.setLedPin(PIN_LED, !relayState);
  Homie.setResetTrigger(PIN_BUTTON, LOW, 5000);
  Homie.registerNode(switchNode);
  switchNode.subscribe("on", switchOnHandler);
  Homie.setSetupFunction(setupHandler);
  Homie.setup();
}

void loop() {

  // read the state of the switch into a local variable:
  int reading = digitalRead(PIN_BUTTON);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
      
      if (buttonState == HIGH) {  // Button released
        // invert/toggle relay state
        relayState = !relayState;
        // print debug message
        Serial.print("New relay state: "); Serial.println(relayState);
        // homie is online
        if (Homie.isReadyToOperate()) {
          // normal mode and network connection up
          switchOnHandler((relayState == HIGH) ? "true" : "false");
        } else {
          // not in normal mode or network connection down
          digitalWrite(PIN_RELAY, relayState);
          digitalWrite(PIN_LED, relayState);
        }
      }
    }
  }

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;
  
  Homie.loop();
}
