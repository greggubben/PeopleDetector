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

#define NOTUSED -1

// Define the Actions the People Detector to change state
enum Action {ACTION_NONE = 0, ACTION_TRIGGER = 1, ACTION_END_TIME = 2, ACTION_RESET = 3};

static const char * ActionStrings[] = {"None", "Trigger", "End Time", "Reset"};

// Define the states the People Detector can transition through.
enum State {STATE_READY = 0, STATE_DELAY = 1, STATE_FIRE = 2, STATE_REARM = 3};

static const char * StateStrings[] = {"Ready", "Delay", "Fire", "ReArm"};

#define SECONDS 1000
#define MINUTES (60 * SECONDS)
#define HOURS (60 * MINUTES)
#define TIME_CALC(x, y) (x * y)

#endif
