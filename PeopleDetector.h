/*
 * People Detector header file.
 *
 * Used to define enums due to a limitation of the Arduino compiler.
 *
 *
 * Created by: Gregg Ubben and Mark Lebioda
 * Created on: 06-Jan-2015
 *
 */
#ifndef PeopleDetector_h
#define PeopleDetector_h

// Define the Actions the People Detector to change state
enum Action {NONE, TRIGGER, END_TIME, RESET};

// Define the states the People Detector can transition through.
//enum State {READY=READY_PIN, DELAY=DELAY_PIN, FIRE=FIRE_PIN, REARM=REARM_PIN};
enum State {READY, DELAY, FIRE, REARM};

#endif
