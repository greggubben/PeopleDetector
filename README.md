People Detector
The People Detector is intended to identify if a person walked passed a point
(like a hallway or door) and fire an animatronic object.

The People Detector moves through several states.  It starts in a READY state
where it is looking for a person to TRIGGER the detector.  Once triggered, it
goes into an optional DELAY state for a delayed firing effect.  When the time
elapses it then goes to a FIRE state for a specified amount of time.  After
that it will transition to an optional REARM state before being ready to be
triggered again.

<pre>

    READY --------------------------+------> RESET ----+
                                    |                  |
                                    V                  |
      +----- TRIGGER(END_READY_PIN) or END_TIME        |
      |                                                |
      V                                                V
    DELAY --------------------------+------> RESET ----+
                                    |                  |
                                    V                  |
      +----- TRIGGER(END_DELAY_PIN) or END_TIME        |
      |                                                |
      V                                                V
     FIRE --------------------------+------> RESET ----+
                                    |                  |
                                    V                  |
      +------ TRIGGER(END_FIRE_PIN) or END_TIME        |
      |                                                |
      V                                                V
    REARM  -------------------------+------> RESET ----+
                                    |                  |
                                    V                  |
             TRIGGER(END_REARM_PIN) or END_TIME        |
                                    |                  |
    READY  &lt;------------------------+------------------+

</pre>

The People Detector has 2 ways to advance through the states.  One is via an
external event causing the Action Pin to go HIGH.  The other way is waiting
for the END\_TIME duration to elapse.

Each of the States have a pin which could TRIGGER and end of that state.  The
naming convention for the pins are END\_<i>state</i>\_PIN.  A value of -1 indicates
not to look for an external event.

When the <i>state</i>\_TIME has a value greater than 0 the People Detector will
wait for the specified amount of milli-seconds before going to the next state.

Using a <i>state</i>\_TIME value of 0 and an END\_<i>state</i>\_PIN of -1 will cause the
People Detector to skip that state.  If the <i>state</i>\_TIME value is 0 and the
END_<i>state</i>\_PIN is not -1 then it will wait for the END\_<i>state</i>\_PIN to go
HIGH.

If both the END\_<i>state</i>\_PIN and <i>state</i>\_TIME are specified the People Detector
will wait for the specified amount of time or the END\_<i>state</i>\_PIN to go HIGH
whichever comes first.

There is also a RESET\_PIN defined that will cause the People Detector to
reset back to READY state no matter where it is.

The typical congiguration is below with a single pin for triggering the
People Detector using a PIC sensor(HC-SR501 Human Sensor Module Pyroelectric
Infrared).  There is a single button for resetting
People Detector state.  Each of the 4 states have an indicator LED and the
expection is to attach the animatron to the FIRE\_PIN.

Created by: Gregg Ubben and Mark Lebioda
Created on: 06-Jan-2015

