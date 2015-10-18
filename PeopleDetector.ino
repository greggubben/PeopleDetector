/*
 * People Detector
 *
 * The People Detector is intended to identify if a person walked passed a point
 * (like a hallway or door) and fire an animatronic object.
 *
 * The People Detector moves through several states.  It starts in a READY state
 * where it is looking for a person to TRIGGER the detector.  Once triggered, it
 * goes into an optional DELAY state for a delayed firing effect.  When the time
 * elapses it then goes to a FIRE state for a specified amount of time.  After
 * that it will transition to an optional REARM state before being ready to be
 * triggered again.
 *
 *     READY --------------------------+------> RESET ----+
 *                                     |                  |
 *                                     V                  |
 *       +----- TRIGGER(END_READY_PIN) or END_TIME        |
 *       |                                                |
 *       V                                                V
 *     DELAY --------------------------+------> RESET ----+
 *                                     |                  |
 *                                     V                  |
 *       +----- TRIGGER(END_DELAY_PIN) or END_TIME        |
 *       |                                                |
 *       V                                                V
 *      FIRE --------------------------+------> RESET ----+
 *                                     |                  |
 *                                     V                  |
 *       +------ TRIGGER(END_FIRE_PIN) or END_TIME        |
 *       |                                                |
 *       V                                                V
 *     REARM  -------------------------+------> RESET ----+
 *                                     |                  |
 *                                     V                  |
 *              TRIGGER(END_REARM_PIN) or END_TIME        |
 *                                     |                  |
 *     READY  <------------------------+------------------+
 *
 * The People Detector has 2 ways to advance through the states.  One is via an
 * external event causing the Action Pin to go HIGH.  The other way is waiting
 * for the END_TIME duration to elapse.
 *
 * Each of the States have a pin which could TRIGGER and end of that state.  The
 * naming convention for the pins are END_<state>_PIN.  A value of -1 indicates
 * not to look for an external event.
 *
 * When the <state>_TIME has a value greater than 0 the People Detector will
 * wait for the specified amount of milli-seconds before going to the next state.
 *
 * Using a <state>_TIME value of 0 and an END_<state>_PIN of -1 will cause the
 * People Detector to skip that state.  If the <state>_TIME value is 0 and the
 * END_<state>_PIN is not -1 then it will wait for the END_<state>_PIN to go
 * HIGH.
 *
 * If both the END_<state>_PIN and <state>_TIME are specified the People Detector
 * will wait for the specified amount of time or the END_<state>_PIN to go HIGH
 * whichever comes first.
 *
 * There is also a RESET_PIN defined that will cause the People Detector to
 * reset back to READY state no matter where it is.
 *
 * The typical congiguration is below with a single pin for triggering the
 * People Detector using a PIC sensor (HC-SR501 Human Sensor Module Pyroelectric
 * Infrared). There is a single button for resetting
 * People Detector state.  Each of the 4 states have an indicator LED and the
 * expection is to attach the animatron to the FIRE_PIN.
 *
 * Created by: Gregg Ubben and Mark Lebioda
 * Created on: 06-Jan-2015
 * Modified on: 10-Oct-2015
 *
 */

// Pins, Constants, and Enums defined in People Detector header file
// This is needed due to limitations with the Arduino compiler handling enums
#include "PeopleDetector.h"

// Uncomment the following line to debug via the Serial Monitor
#define SERIAL_DEBUG

// Define digital inputs for triggering actions
// NOTUSED (-1) indicates not to look for an external event.
const int END_READY_PIN = 2;        // Trigger Pin
const int END_DELAY_PIN = NOTUSED;
const int END_FIRE_PIN = NOTUSED;
const int END_REARM_PIN = NOTUSED;
const int RESET_PIN = NOTUSED;
const int BLINK_PIN = 13;

// Define analog inputs for changing delay times
// NOTUSED (-1) indicates not to look for external time adjustments.
// If set the max value is defined by appropriate _TIME entry below.
const int READY_TIME_PIN = A3;
const int DELAY_TIME_PIN = A2;
const int FIRE_TIME_PIN = A1;
const int REARM_TIME_PIN = A0;

// Define outputs
// State indicator LEDs and Signals
// NOTUSED (-1) indicates not send signal.
const int READY_PIN = 7;
const int DELAY_PIN = 8;
const int FIRE_PIN = 9;
const int REARM_PIN = 10;

// Wait time in Millis for each state
// Use 0 to indicate skipping the state.
// This is either:
//   The amount of time if the _TIME_PIN value above is NOTUSED
//   The max amount of time if the _TIME_PIN is assigned.
const unsigned long READY_TIME = TIME_CALC(10, MINUTES);  // 10 minutes if not triggered
const unsigned long DELAY_TIME = TIME_CALC(30, SECONDS); // 2 seconds
const unsigned long FIRE_TIME  = TIME_CALC(10, SECONDS);    // 2 seconds
const unsigned long REARM_TIME = TIME_CALC(20, MINUTES);   // 20 seconds

const unsigned long BLINK_TIME = 1000;    // 1 second

// Current Action being performed
Action action = ACTION_NONE;

// Current State this is in
State state = STATE_READY;

// Transient variables
unsigned long currTime;
unsigned long waitUntil;
int actionPin;
bool waiting;
bool blinkLedState = false;
unsigned long nextBlinkMillis = 0;


/*
 * Initialize the Inputs, Outputs, and Variables
 */
void setup() {
    // For debugging
#ifdef SERIAL_DEBUG
    Serial.begin(9600);
#endif

    // Set up the trigger and input pins
    setupPin(END_READY_PIN, INPUT);
    setupPin(RESET_PIN, INPUT);
    
    // Set up the indicator and output pins
    setupPin(READY_PIN, OUTPUT);
    setupPin(DELAY_PIN, OUTPUT);
    setupPin(FIRE_PIN, OUTPUT);
    setupPin(REARM_PIN, OUTPUT);

    setupPin(BLINK_PIN, OUTPUT);

    blinkLed();
    changeState(STATE_READY);
}


/*
 * Continue looking for Actions performed and responding with appropriate state changes.
 */
void loop() {
  
  // Assume there is nothing to do
  action = ACTION_NONE;

  // Need current time for waiting count down
  currTime = millis();
  
  // Identify what incoming action, if any, has been detected
  
  // Waiting for time to elapse (lowest priority)
  if (doneWaiting()) {
    // Done waiting
    action = ACTION_END_TIME;
#ifdef SERIAL_DEBUG
    printAction();
#endif
  }
  
  // See if triggered  (medium priotity)
  // TRIGGER is a valid action only in READY state
  if (readPin(actionPin, LOW) == HIGH) {
    action = ACTION_TRIGGER;
#ifdef SERIAL_DEBUG
    printAction();
#endif
  }
  
  // See if reset  (highest priority)
  if (readPin(RESET_PIN, LOW) == HIGH) {
    action = ACTION_RESET;
#ifdef SERIAL_DEBUG
    printAction();
#endif
  }

  // Respond to the action
  switch (action) {
  case (ACTION_RESET):
      changeState(STATE_READY);
      break;
  
  case (ACTION_END_TIME):
  case (ACTION_TRIGGER):
      // Advance to next state
      switch (state) {
      case (STATE_READY):
          changeState(STATE_DELAY);
          break;
      case (STATE_DELAY):
          changeState(STATE_FIRE);
          break;
      case (STATE_FIRE):
          changeState(STATE_REARM);
          break;
      case (STATE_REARM):
          changeState(STATE_READY);
          break;
      }
      break;

  case (ACTION_NONE):
  default:
      // Nothing to do
      break;
  }

  // Blink the LED to show it is working
  blinkLed();

#ifdef SERIAL_DEBUG
  if (requested2Tune()) {
    performTuning();
  }
#endif
}

/*
 * Change to a new state.
 */
void changeState (State newstate) {
  int indicatorPin;
  
  // Turn all indicators off because we are changing state
  allLedsOff();
  
  switch (newstate) {
  case (STATE_READY):
      // Ready for someone to trigger
      wait(READY_TIME_PIN, READY_TIME, END_READY_PIN);
      indicatorPin = READY_PIN;
      break;

  case (STATE_DELAY):
      // Triggered - wait for a delayed fire
      wait(DELAY_TIME_PIN, DELAY_TIME, END_DELAY_PIN);
      indicatorPin = DELAY_PIN;
      break;

  case (STATE_FIRE):
      // Fire the animation
      wait(FIRE_TIME_PIN, FIRE_TIME, END_FIRE_PIN);
      indicatorPin = FIRE_PIN;
      break;

  case (STATE_REARM):
      // Wait before arming again.
      wait(REARM_TIME_PIN, REARM_TIME, END_REARM_PIN);
      indicatorPin = REARM_PIN;
      break;
  }
  
  state = newstate;

#ifdef SERIAL_DEBUG
  printState();
  printAllDelays();
#endif
  
  // Indicate current state
  writePin(indicatorPin, HIGH);

#ifdef SERIAL_DEBUG
  printEndLoop();
#endif

}


/*
 * Turn off all the LEDs.
 * Most likely there is a state change.
 */
void allLedsOff() {
    writePin(READY_PIN, LOW);
    writePin(DELAY_PIN, LOW);
    writePin(FIRE_PIN, LOW);
    writePin(REARM_PIN, LOW);
}

/*
 * Setup to wait
 */
void wait (int waitTimePin, unsigned long waitTime, int waitPin) {
  if (waitPin != NOTUSED && waitTime == 0) {
    // Only using a pin HIGH to for action not wait time
    waiting = false;
  }
  else {
    // set up wait time
    waiting = true;
    unsigned long waitValue = getWaitValue(waitTimePin, waitTime);
    waitUntil = currTime + waitValue;
#ifdef SERIAL_DEBUG
    printWait(waitValue, waitTime);
#endif
  }
  actionPin = waitPin;
}

/*
 * See if we are done waiting
 */
bool doneWaiting() {
  return (waiting && (currTime > waitUntil));
}

/*****************************************
 * Pin Helper Functions
 ****************************************/

unsigned long getWaitValue(int waitTimePin, unsigned long waitTime) {
  unsigned long returnWaitTime = waitTime;
  if (waitTimePin != NOTUSED) {
    int rawVal = analogRead(waitTimePin);
    returnWaitTime = map(rawVal, 0, 1023, 0, waitTime);
  }
  return returnWaitTime;
}

/*
 * Setup the Pin
 */
void setupPin (int pin, uint8_t mode) {
  if (pin != NOTUSED) {
    pinMode(pin, mode);
  }
}


/*
 * Write to the Pin
 */
void writePin (int pin, uint8_t state) {
  if (pin != NOTUSED) {
    digitalWrite(pin, state);
  }
}


/*
 * Read from the Pin
 */
int readPin (int pin, int deflt) {
  int retVal = deflt;
  if (pin != NOTUSED) {
    retVal = digitalRead(pin);
  }
  return retVal;
}

/*
 * Blink the LED to show it is working
 */
void blinkLed() {
  if (currTime > nextBlinkMillis) {
    nextBlinkMillis = currTime + BLINK_TIME;
    blinkLedState = !blinkLedState;
    writePin(BLINK_PIN, blinkLedState);
  }
}


/*****************************************
 * Serial Debugging Helper Functions
 ****************************************/

void printAction() {
  Serial.print("Action = ");
  Serial.println(ActionStrings[action]);
}

void printState() {
  Serial.print("State = ");
  Serial.println(StateStrings[state]);
}

void printEndLoop() {
  Serial.println();
}

void printWait(unsigned long waitValue, unsigned long waitTime) {
  if (waiting) {
    Serial.print("Wait for ");
    Serial.print(waitValue);
    Serial.print(" millis out of ");
    Serial.print(waitTime);
    Serial.println();
  }
  else {
    Serial.println("No Waiting");
  }

}

bool requested2Tune() {
  if (Serial.available() > 0) {
    int inByte = Serial.read();
    if (inByte == 't') {
      return true;
    }
  }
  return false;
}

void performTuning() {
  Serial.println("Entering Tuning Mode");
  bool tuning = true;
  while (tuning) {
    if (requested2Tune()) {
      tuning = false;
    }
    else {
      
    }
  }
}

void printAllDelays() {
  unsigned long waitValue;
  waitValue = getWaitValue(READY_TIME_PIN, READY_TIME);
  Serial.print("READY wait ");
  Serial.println(waitValue);
  waitValue = getWaitValue(DELAY_TIME_PIN, DELAY_TIME);
  Serial.print("DELAY wait ");
  Serial.println(waitValue);
  waitValue = getWaitValue(FIRE_TIME_PIN, FIRE_TIME);
  Serial.print("FIRE  wait ");
  Serial.println(waitValue);
  waitValue = getWaitValue(REARM_TIME_PIN, REARM_TIME);
  Serial.print("REARM wait ");
  Serial.println(waitValue);
}

