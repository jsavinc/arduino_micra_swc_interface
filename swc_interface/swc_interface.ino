/* Interface between Blaupunkt stereo and Micra K12 steering wheel controls

Version 1.0, 26 Sept 2021
First try in the car - it works! I needed to change the output pin to 10 (D10 is 10, who knew?)

*/

#include <AceButton.h>
using ace_button::AceButton;
using ace_button::ButtonConfig;
using ace_button::LadderButtonConfig;

// There are 5 buttons on the steering wheel that need configuring
// Each is associated with a particular volage across one of two wires
//
// When no button is pressed, there should be 5V between both remote control A (10, L) and B (11, W) wires, and ground (12, L) [note, this is for the mini-DIN connector on the radio side, approximate values].
// MODE (SRC): A, 0V
// SEEK UP: A, 1.68V
// VOL UP: A, 3.34V
// nothing pressed: A, 5V
// SEEK DOWN: B, 1.68V
// VOL DOWN: B, 3.34V
// nothing pressed: B, 5V
//
// Conversely, each button press is associated with a resistance between the wire and ground [this is for pins on the steering wheel side, approximate values]:
// MODE (SRC): terminal 14, 0ohm
// VOL UP: terminal 14, 165ohm
// SEEK UP: terminal 14, 652ohm
// VOL DOWN: terminal 15, 165ohm
// SEEK DOWN: terminal 15, 652ohm
//
// So we need to monitor two voltages on two analogue pins, and detect changes in state from 5V down to 3.34, 1.68V, or 0. Note that these are approximate values
// AceButton supports LadderButtonConfig(), which can handle a ladder resistor configuration connected to an analog pin!
//
// Blaupunkt protocol: Each button is encoded as a pulse with a different length for each button, followed by a pulse which is always 660μs long 
// Pulse lengths:
// V+    2.6 ms
// V-    3.2 ms
// RIGHT 5.6 ms
// LEFT  6.2 ms
// UP    3.8 ms
// DOWN  4.4 ms
// SRC   6.8 ms

static const uint8_t OUTPUT_PIN = 10;  // pin D10 = 10
static const uint8_t BUTTON_PIN = A0;

static const uint8_t NUM_BUTTONS = 6;  // 5 buttons + 1 button for 'nothing pressed'
static AceButton b0(nullptr, 0);
static AceButton b1(nullptr, 1);
static AceButton b2(nullptr, 2);
static AceButton b3(nullptr, 3);
static AceButton b4(nullptr, 4);
static AceButton b5(nullptr, 5);

static AceButton* const BUTTONS[NUM_BUTTONS] = {
    &b0, &b1, &b2, &b3, &b4, &b5,
};

static const uint8_t NUM_LEVELS = NUM_BUTTONS + 1;
static const uint16_t LEVELS[NUM_LEVELS] = {
  0,  // SRC/MODE 0
341,  // SEEK UP 1
679,  // VOL UP 2
736,  // SEEK DOWN 3
820,  // VOL DOWN 4
1023,  // nothing pressed, 5V
};

static LadderButtonConfig buttonConfig(
  BUTTON_PIN, NUM_LEVELS, LEVELS, NUM_BUTTONS, BUTTONS
);

void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {

  // Print out a message for all events.
  Serial.print(F("handleEvent(): "));
  Serial.print(F("virtualPin: "));
  Serial.print(button->getPin());
  Serial.print(F("; eventType: "));
  Serial.print(eventType);
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);

// eventTypes:
// 0 click
// 1 release
// 2 press?
// 4 long click
// 5 held down

  if ((button->getPin() == 0) && (eventType == 2)) {
    Serial.println("SRC/MODE");
    src_mode();
  }
  if ((button->getPin() == 1) && ((eventType == 2) || (eventType == 5))) {
    Serial.println("SEEK UP");
    seek_up();
  }
  if ((button->getPin() == 2) && ((eventType == 2) || (eventType == 5))) {
    Serial.println("VOL UP");
    vol_up();
    delay(50);
  }
  if ((button->getPin() == 2) && (eventType == 3)) {
    Serial.println("2x VOL UP");  // double click vol up
    vol_up();
    delay(50);
    vol_up();
  }
  if ((button->getPin() == 3) && ((eventType == 2) || (eventType == 5))) {
    Serial.println("SEEK DOWN");
    seek_down();
  }
  if ((button->getPin() == 4) && ((eventType == 2) || (eventType == 5))) {
    Serial.println("VOL DOWN");
    vol_down();
    delay(50);
  }
  if ((button->getPin() == 4) && (eventType == 3)) {
    Serial.println("2x VOL DOWN");
    vol_down();
    delay(50);
    vol_down();
  }
  if ((button->getPin() == 1) && (eventType == 3)) {  // double click SEEK UP
    Serial.println("SEEK RIGHT");
    seek_right();
  }
  if ((button->getPin() == 3) && (eventType == 3)) {  // double click SEEK DOWN
    Serial.println("SEEK LEFT");
    seek_left();
  }
}

void checkButtons() {
  static uint16_t prev = millis();
  // This polls buttonpresses 5ms apart, or 200/second
  // the 
  // DO NOT USE delay(5) to do this.
  uint16_t now = millis();
  if (now - prev >= 5) {
    prev = now;
    buttonConfig.checkButtons();
  }
}

void send_command(int pulseLengthMicroSec) {
  // the pulses go low (0V) from high (5V)
  digitalWrite(OUTPUT_PIN, LOW);
  delayMicroseconds(pulseLengthMicroSec);
  digitalWrite(OUTPUT_PIN, HIGH);
  delayMicroseconds(pulseLengthMicroSec);
  digitalWrite(OUTPUT_PIN, LOW);
  delayMicroseconds(660);  // 660us end pulse after command
  digitalWrite(OUTPUT_PIN, HIGH);  // reset to high after being done
}

//void send_command_micros(int pulseLengthMicroSec) {
//  // the pulses go low (0V) from high (5V)
//  // TODO: how do I implement this in a loop-lke manner?
//  digitalWrite(OUTPUT_PIN, LOW)
//  static uint16_t prev = micros();
//  uint16_t now = micros();
//  if (now - prev >= pulseLengthMicroSec) {
//    prev = now;
//    digitalWrite(OUTPUT_PIN, HIGH);
//  }
//  digitalWrite(OUTPUT_PIN, LOW);
//  now = micros();
//  if (now - prev >= 660) {
//    prev = now;
//    digitalWrite(OUTPUT_PIN, HIGH);
//  }
//}

void src_mode() {
  send_command(6800);
}
void vol_up() {
  send_command(2600);
}
void vol_down() {
  send_command(3200);
}
void seek_right() {
  send_command(5600);
}
void seek_left() {
  send_command(6200);
}
void seek_up() {
  send_command(3800);
}
void seek_down() {
  send_command(4400);
}

void setup() {
  delay(1000); // some microcontrollers reboot twice
  Serial.begin(9600);
  while (! Serial); // Wait until Serial is ready - Leonardo/Micro
  Serial.println(F("setup(): begin"));

  // set internal pull-up resistor on output pin
  pinMode(OUTPUT_PIN, OUTPUT);
  digitalWrite(OUTPUT_PIN, HIGH);  // set to 5V to begin with

  // Don't use internal pull-up resistor because it will change the effective
  // resistance of the resistor ladder.
  pinMode(BUTTON_PIN, INPUT);  

  // Configure the ButtonConfig with the event handler, and enable all higher
  // level events.
  buttonConfig.setEventHandler(handleEvent);
  buttonConfig.setFeature(ButtonConfig::kFeatureClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig.setFeature(ButtonConfig::kFeatureRepeatPress);
  buttonConfig.setFeature(ButtonConfig::kFeatureSuppressAfterClick);  // supresses released event after clicked

  Serial.println(F("setup(): ready"));
}

void loop() {
  checkButtons();
}
