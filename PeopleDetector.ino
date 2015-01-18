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
 *
 */

// Pins, Constants, and Enums defined in People Detector header file
// This is needed due to limitations with the Arduino compiler handling enums
#include "PeopleDetector.h"

// Define inputs
// a -1 indicates not to look for an external event.
// Make sure these are defined as INPUT in setup()
const int END_READY_PIN = 7;
const int END_DELAY_PIN = -1;
const int END_FIRE_PIN = -1;
const int END_REARM_PIN = -1;
const int RESET_PIN = 9;

// Current Action being performed
Action action = NONE;

// Wait time in Millis for each state
// Use 0 to indicate skipping the state.
const unsigned long READY_TIME = 120000;  // 2 minutes if not triggered
const unsigned long DELAY_TIME = 1000;    // 1 second
const unsigned long FIRE_TIME = 500;      // 1/2 second
const unsigned long REARM_TIME = 20000;   // 20 seconds

// Current State this is in
State state = READY;

// State indicator LEDs and Signals
// a -1 indicates not send signal.
// Make sure these are defined as OUTPUT in setup()
const int READY_PIN = 13;
const int DELAY_PIN = 2;
const int FIRE_PIN = 3;
const int REARM_PIN = 4;

// Transient variables
unsigned long currTime;
unsigned long waitUntil;
int actionPin;
bool waiting;

void setup() {
    // For debugging
    //Serial.begin(9600);

    // Set up the trigger and input pins
    pinMode(END_READY_PIN, INPUT);
    pinMode(RESET_PIN, INPUT);
    
    // Set up the indicator and output pins
    pinMode(READY_PIN, OUTPUT);
    pinMode(DELAY_PIN, OUTPUT);
    pinMode(FIRE_PIN, OUTPUT);
    pinMode(REARM_PIN, OUTPUT);

    changeState(READY);
}

void loop() {
  
  // Assume there is nothing to do
  action = NONE;
  //Serial.println("action=NONE");

  // Need current time for waiting count down
  currTime = millis();
  
  // Identify what incoming action, if any, has been detected
  
  // Waiting for time to elapse (lowest priority)
  if (doneWaiting()) {
    // Done waiting
    action = END_TIME;
    //Serial.println("action=END_TIME");
  }
  
  // See if triggered  (medium priotity)
  // TRIGGER is a valid action only in READY state
  if (actionPin != -1 && digitalRead(actionPin) == HIGH) {
    action = TRIGGER;
    //Serial.println("action=TRIGGER");
  }
  
  // See if reset  (highest priority)
  if (RESET_PIN != -1 && digitalRead(RESET_PIN) == HIGH) {
    action = RESET;
    //Serial.println("action=RESET");
  }

  // Respond to the action
  switch (action) {
  case (RESET):
      changeState(READY);
      break;
  
  case (END_TIME):
  case (TRIGGER):
      switch (state) {
      case (READY):
          changeState(DELAY);
          break;
      case (DELAY):
          changeState(FIRE);
          break;
      case (FIRE):
          changeState(REARM);
          break;
      case (REARM):
          changeState(READY);
          break;
      }
      break;

  case (NONE):
  default:
      // Nothing to do
      break;
  }
  
}

/*
 * Change to a new state.
 */
void changeState (enum State newstate) {
  int indicatorPin;
  //char *statename;
  
  // Turn all indicators off because we are changing state
  allLedsOff();
  
  switch (newstate) {
  case (READY):
      // Ready for someone to trigger
      wait(READY_TIME, END_READY_PIN);
      indicatorPin = READY_PIN;
      //statename = "READY";
      break;

  case (DELAY):
      // Triggered - wait for a delayed fire
      wait(DELAY_TIME, END_DELAY_PIN);
      indicatorPin = DELAY_PIN;
      //statename = "DELAY";
      break;

  case (FIRE):
      // Fire the animation
      wait(FIRE_TIME, END_FIRE_PIN);
      indicatorPin = FIRE_PIN;
      //statename = "FIRE";
      break;

  case (REARM):
      // Wait before arming again.
      wait(REARM_TIME, END_REARM_PIN);
      indicatorPin = REARM_PIN;
      //statename = "REARM";
      break;
  }
  
  state = newstate;
  //Serial.print("state=");
  //Serial.println(statename);
  
  if (indicatorPin != -1) {
      // Indicate current state
      digitalWrite(indicatorPin, HIGH);
  }
}


/*
 * Turn off all the LEDs.
 * Most likely there is a state change.
 */
void allLedsOff() {
    digitalWrite(READY_PIN, LOW);
    digitalWrite(DELAY_PIN, LOW);
    digitalWrite(FIRE_PIN, LOW);
    digitalWrite(REARM_PIN, LOW);
}

/*
 * Setup to wait
 */
void wait (unsigned long waittime, int waitPin) {
  if (waitPin != -1 && waittime == 0) {
    // Only using a pin HIGH to for action not wait time
    waiting = false;
  }
  else {
    // set up wait time
    waiting = true;
    waitUntil = currTime + waittime;
  }
  actionPin = waitPin;
}

/*
 * See if we are done waiting
 */
bool doneWaiting() {
  return (waiting && currTime > waitUntil);
}


