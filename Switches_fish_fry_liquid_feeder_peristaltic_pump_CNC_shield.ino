// Switches_fish_fry_liquid_feeder_peristaltic_pump_CNC_shield.ino

/*
Ardiuno Uno
CNC_shield V3
NEMA 17 stepper. In my case the plan is to drive a Peristaltic pump for feeding fish fry liquid ever 4 hours
It has manual move stepper buttons with LED signals

This code is for a Peristaltic pump for feeding fish fry liquid ever 4 hours.
The code here has the time and the total distance moved reduced for testing purposes.

It was originally for use with a syringe; this in this code below. I ask for a return to the start position. This is most likely not required for use or even recommended for use with a Peristaltic pump.

Both a Peristaltic pump or a Syringe will require a small amount of back movement, (not in this code) to lessen any drops of liquid.

I have issues with both liquid adhesion and cohesion, that after a drop is pushed out this caused more liquid to be pulled out. One maybe possible way of helping prevent this maybe, (still testing at this time),
is to put the end of the syringe needle or Peristaltic pump tube end in to the actual fish tank where the baby fry are.

This only swithes the Stepper control on just before moving and off after moving, this reduces heat but loses holding force, not required for a Peristaltic pump I plan to use.

Uses:
NoDelay library
AccelStepper
*/

/*
Information
   by Dejan, https://howtomechatronics.com
   Ref: https://howtomechatronics.com/tutorials/arduino/stepper-motors-and-arduino-the-ultimate-guide/#TMC2208_Stepper_Driver
   also uses other online code
*/

//  The AccelStepper library for Arduino
//  http://www.airspayce.com/mikem/arduino/AccelStepper/index.html

// AccelStepper commands
// http://www.airspayce.com/mikem/arduino/AccelStepper/classAccelStepper.html

// Based on NoDelay library. This is the Magic Delay option  https://github.com/M-tech-Creations/NoDelay/blob/baad513b9608959136b8c519803c13e470111fd7/examples/BlinkWithNoDelay/BlinkWithNoDelay.ino

// The AccelStepper library for Arduino
// http://www.airspayce.com/mikem/arduino/AccelStepper/index.html

// http://www.airspayce.com/mikem/arduino/AccelStepper/classAccelStepper.html#a748665c3962e66fbc0e9373eb14c69c1

// Debugging in Visual Code https://devblogs.microsoft.com/iotdev/debug-your-arduino-code-with-visual-studio-code/

// Timer (DID NOT WORK). So use NoDelay
// https://www.pjrc.com/teensy/td_libs_TimerOne.html
// https://playground.arduino.cc/Code/Timer1/

// interrupts (DID NOT WORK). So use NoDelay
// https://arduino.stackexchange.com/questions/30968/how-do-interrupts-work-on-the-arduino-uno-and-similar-boards

// Do not use a jumper between EN/GND on the CNC board. In code stepper enable is turned on to make a move then turned off after that move, false is on. This is to avoid holding power causing heating. Stepper may be moved by hand beacuse there is no holding power.

// On the CNC shield the limit switches are expected to be normally open and are wired to ground with the internal pullup enabled. The switch will read high when not actuated (open) and low when actuated (closed).

/* bRef: https://www.electroniclinic.com/arduino-cnc-shield-v3-0-and-a4988-hybrid-stepper-motor-driver-joystick/

X.STEP is connected with the Arduino’s pin number 2.

Y.STEP is connected with the Arduino’s pin number 3.

Z.STEP is connected with the Arduino’s pin number 4.

X.DIR is connected with the Arduino’s pin number 5

Y.DIR is connected with the Arduino’s pin number 6

Z.DIR is connected with the Arduino’s pin number 7

Pin number 8 is not connected.

Pin number 9 of the Arduino is connected with the X- and X+ pins of the CNC Shield. buttonGreenPinFeedLiquid

Pin number 10 of the Arduino is connected with the Y- and Y+ pins of the CNC Shield. buttonYellowPinUnfeedLiquid

Pin number 11 of the Arduino is connected with the Z- and Z+ pins of the CNC Shield. ledGreenFeedLiquid

Pin number 12 of the Arduino is connected with the SpnEn pin of the CNC Shield. ledYellowUnfeedLiquid

Pin number 13 of the Arduino is connected with the SpnDir pin of the CNC Shield.

A0 analog pin of the Arduino is connected with the Abort pin of the CNC shield.

A1 analog pin of the Arduino is connected with the Hold pin of the CNC shield.

A2 analog pin of the Arduino is connected with the Resume pin of the CNC shield.

A3 analog pin of the Arduino is connected with the CoolEn pin of the CNC shield.

A4 analog pin of the Arduino is connected with the SDA pin of the CNC shield.

A5 analog pin of the Arduino is connected with the SCL pin of the CNC shield.

*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <AccelStepper.h>

#include "NoDelay.h"  // The Magic Code that acutually allows delays for timed movements. https://www.arduino.cc/reference/en/libraries/nodelay/

// ACTUAL USE 4 hours: long millisecondsBetweenDrops= 14400000; // (14400000) 14,400,000 milliseconds 4 Hours between drops
long millisecondsBetweenDrops = 10000;  // TESTING (15000) milliseconds 15 seconds between drops

// noDelay Must declare function before noDelay, function can not take arguments
void FeedDropPulses();

noDelay nFeedDropPulses(millisecondsBetweenDrops);  // Creates a noDelay variable. Uses the NoDelay library; Convert milliseconds to hours https://converter.net/time/14400000-milliseconds-to-hours

// defines pins CNC Shield V3
#define stepPin 2
#define dirPin 5

AccelStepper stepper1(1, stepPin, dirPin);  // Typeof driver: with 2 pins, STEP, DIR) CNC Shield V3

int enableStepper_Pin = 8;  // Do not use a jumper between EN/GND on CNC board. In code stepper enable is turned on to make a move then turned off after that move, false is on. This is to avoid holding power causing heating. Stepper may be moved by hand beacuse there is no holding power.

// Control buttons Manual Feed and Unfeed
int buttonGreenPinFeedLiquid = 9;
int buttonYellowPinUnfeedLiquid = 10;

// LED's Signal Feed and Unfeed
int ledGreenFeedLiquid = 11;
int ledYellowUnfeedLiquid = 12;

// The Motor Two buttons Feed or Unfeed if a button is pressed
// High = Off not pressed
boolean buttonGreenPinFeedVal = LOW;
boolean buttonYellowPinUnfeedVal = LOW;

// Feeding starting postion
long pusherFeedPosition = 0;

long oneDropPulses = 200;  // Move: 200 steps x 1.8 degrees is 360 degrees i.e. one rotation at FULLSTEP setting, no jumpers on CNC board

// Feed and unfeed Limits
//////////////////////////////////////////////////const int maximumPusherFeedPositionDrops = 120 * oneDropPulses;  // Maximun drops about 4ml

const int maximumPusherFeedPositionDrops = 5 * oneDropPulses;  // Maximun drops about 4ml

const int maximumPusherUnFeedPositionDrops = 0;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);  // Standard is 9600 Serial Monitor display

  pinMode(enableStepper_Pin, OUTPUT);  // In code avoid holding Power causing heating. Do not use a jumper between EN/GND on CNC board

  pinMode(buttonGreenPinFeedLiquid, INPUT_PULLUP);
  pinMode(buttonYellowPinUnfeedLiquid, INPUT_PULLUP);

  // Set up LED Feed Unfeed
  pinMode(ledGreenFeedLiquid, OUTPUT);
  pinMode(ledYellowUnfeedLiquid, OUTPUT);

  // Buttons set to input
  pinMode(buttonGreenPinFeedLiquid, INPUT);
  pinMode(buttonYellowPinUnfeedLiquid, INPUT);

  // Buttons turn on internal pull-up resistor.
  digitalWrite(buttonGreenPinFeedLiquid, HIGH);
  digitalWrite(buttonYellowPinUnfeedLiquid, HIGH);

  // Sets the two pins as Outputs CNC
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  // NB: this code is required to function properly

  // Stepper Maximun Speed
  // The desired maximum speed in steps per second.
  // Must be > 0. Caution: Speeds that exceed the
  // maximum speed supported by the processor
  // may Result in non-linear accelerations and decelerations.
  stepper1.setMaxSpeed(1000.0);

  // Stepper Acceleration
  // Higher value=shorter
  // The desired acceleration in steps per second per second.
  // Must be > 0.0. This is an expensive call since it
  // requires a square root to be calculated.
  // Dont call more often than needed
  // 100 = No acceleration
  // Note Any deacceleration will cause a run on after while
  // the deacceleration takes place
  stepper1.setAcceleration(50.0);

  // Stepper Speed
  // The desired constant speed in steps per second.
  // Positive is clockwise. Speeds of more than
  // 1000 steps per second are unreliable.
  // Very slow speeds may be set
  // (eg 0.00027777 for once per hour, approximately.
  // Speed accuracy depends on the Arduino crystal.
  // Jitter depends on how frequently you call the
  // RunSpeed() function.
  stepper1.setSpeed(700);  // RPM

  resetStepper();  // Clear any old positions

  // Start up for Stepper
  stepper1.enableOutputs();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  // Start witha stop to clear any old positions, etc..
  stepper1.stop();

  // Turn Signal LED's off
  digitalWrite(ledGreenFeedLiquid, LOW);
  digitalWrite(ledYellowUnfeedLiquid, LOW);

  buttonGreenPinFeedVal = digitalRead(buttonGreenPinFeedLiquid);
  buttonYellowPinUnfeedVal = digitalRead(buttonYellowPinUnfeedLiquid);

  // ADDING Serial slows down movement, turning, "a lot", time taken to display

  //Serial.println("buttons");
  // Serial.print("buttonGreenPinFeedVal=");
  // Serial.print(buttonGreenPinFeedVal);
  // Serial.print("      buttonYellowPinUnfeedVal=");
  // Serial.print(buttonYellowPinUnfeedVal);
  // Serial.println();
  // Serial.println("LED");
  // Serial.print("ledGreenFeedLiquid=");
  // Serial.print(ledGreenFeedLiquid);
  // Serial.print("      ledYellowUnfeedLiquid=");
  // Serial.print(ledYellowUnfeedLiquid);
  // Serial.println();
  // Serial.println("**************************");

  // Feed after Time set: Checks to see if set time has past and no buttons pressed
  if (nFeedDropPulses.update() && (pusherFeedPosition < maximumPusherFeedPositionDrops)  // Uses the NoDelay library
      && ((buttonGreenPinFeedVal == HIGH && buttonYellowPinUnfeedVal == HIGH))) {
    FeedDropPulses(oneDropPulses, ledGreenFeedLiquid);
  }

  // Feeding button pressed /////////////////////////////////////////////////////////////
  if ((buttonGreenPinFeedVal == LOW) && (pusherFeedPosition >= maximumPusherUnFeedPositionDrops)) {
    FeedDropPulses(oneDropPulses, ledGreenFeedLiquid);
  }

  // Unfeeding button pressed ////////////////////////////////////////////////////////////
  if (buttonYellowPinUnfeedVal == LOW && (pusherFeedPosition < maximumPusherFeedPositionDrops)) {
    unFeedDropPulses(oneDropPulses, ledYellowUnfeedLiquid);  // Uses the NoDelay library
  }

  // Limit LED signals ////////////////////////////////////////////////////////////

  if (pusherFeedPosition >= maximumPusherFeedPositionDrops) {
    flashLED(ledGreenFeedLiquid);
  }

  if (pusherFeedPosition <= maximumPusherUnFeedPositionDrops) {
    flashLED(ledYellowUnfeedLiquid);
  }

  // reset punger position at start //////////////////////////////////////////////////////////////
  // Paused feed limit automattically return plunger to start and no buttons pressed, using manual
  if ((pusherFeedPosition >= maximumPusherFeedPositionDrops)
      && (buttonGreenPinFeedVal == HIGH && buttonYellowPinUnfeedVal == HIGH)) {
    moveToStartPostion(ledYellowUnfeedLiquid);

    // Allow stepper to be manually moved by Hand
    resetStepper();

    // Exit the loop. Stop the Program actions, until plugged in again
    // Otherwise because of this Arduino Loop it will just repeat and because the software limit is reached the LED will flash.
    // This isn't published on Arduino.cc but you can in fact exit from the loop routine with a simple exit(0);
    exit(0);  // The 0 is required to prevent compile error.
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FeedDropPulses(long dropPulses, int ledGreenfeedLiquid) {
  long positionNow = stepper1.currentPosition();

  digitalWrite(ledGreenfeedLiquid, HIGH);  // Turn Green LED on

  pusherFeedPosition = positionNow + oneDropPulses;  // Correct direction to feed pusher

  // ADDING Serial slows down movement, turning, "a lot", time taken to display

  // Serial.println("feed");
  // Serial.print("pusherFeedPosition=");
  // Serial.print(pusherFeedPosition);
  // Serial.print(" dropPulses=");
  // Serial.print(dropPulses);
  // Serial.print(" maximumPusherFeedPositionDrops=");
  // Serial.print(maximumPusherFeedPositionDrops);
  // Serial.println();
  // Serial.println("**************************");


  // Serial.println("FeedDropPulses Buttons & LED's");
  // Serial.print("buttonGreenPinFeedVal=");
  // Serial.print(buttonGreenPinFeedVal);

  // Serial.print("        ledGreenFeedLiquid=");
  // Serial.print(ledGreenFeedLiquid);
  // Serial.print("  HIGH ON");

  // Serial.print("            buttonYellowPinUnfeedVal=");
  // Serial.print(buttonYellowPinUnfeedVal);

  // Serial.print("      ledYellowUnfeedLiquid=");
  // Serial.print(ledYellowUnfeedLiquid);

  // Serial.println();
  // Serial.println("**************************");

  // Start up for Stepper, after stopping to avoid holding Power causing heating. Do not use a jumper between EN/GND on CNC board. In code stepper enable is turned on to make a move then turned off after that move, false is on. This is to avoid holding power causing heating. Stepper may be moved by hand beacuse there is no holding power.
  digitalWrite(enableStepper_Pin, false);

  stepper1.moveTo(pusherFeedPosition);  // Addition Set desired move: 200 steps x 1.8 degrees is 360 degrees i.e. one rotation at FULLSTEP setting, no jumpers on CNC board

  stepper1.runToPosition();  // Moves the motor to target position w/ acceleration/ deceleration and it blocks until is in position

  // Stop the Holding on the stepper. Do not use a jumper between EN/GND on CNC board. In code stepper enable is turned on to make a move then turned off after that move, false is on. This is to avoid holding power causing heating. Stepper may be moved by hand beacuse there is no holding power.
  digitalWrite(enableStepper_Pin, true);

  digitalWrite(ledGreenfeedLiquid, LOW);  // Turn Green LED off
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void unFeedDropPulses(long dropPulses, int ledYellowUnfeedLiquid) {  // Uses the NoDelay library
  long positionNow = stepper1.currentPosition();

  digitalWrite(ledYellowUnfeedLiquid, HIGH);  // Turn Yellow LED on

  pusherFeedPosition = positionNow - oneDropPulses;  // Correct direction to feed pusher

  // ADDING Serial slows down movement, turning, "a lot", time taken to display

  // Serial.println("UNfeed");
  // Serial.print("pusherFeedPosition=");
  // Serial.print(pusherFeedPosition);
  // Serial.print(" dropPulses=");
  // Serial.print(dropPulses);
  // Serial.print(" maximumPusherFeedPositionDrops=");
  // Serial.print(maximumPusherFeedPositionDrops);
  // Serial.println();
  // Serial.println("**************************");

  // Start up for Stepper, after stopping to avoid holding Power causing heating. Do not use a jumper between EN/GND on CNC board. In code stepper enable is turned on to make a move then turned off after that move, false is on. This is to avoid holding power causing heating. Stepper may be moved by hand beacuse there is no holding power.
  digitalWrite(enableStepper_Pin, false);

  stepper1.moveTo(pusherFeedPosition);  // Addition Set desired move: 200 steps x 1.8 degrees is 360 degrees i.e. one rotation at FULLSTEP setting, no jumpers on CNC board

  stepper1.runToPosition();  // Moves the motor to target position w/ acceleration/ deceleration and it blocks until is in position

  // Stop the Holding on the stepper. Do not use a jumper between EN/GND on CNC board. In code stepper enable is turned on to make a move then turned off after that move, false is on. This is to avoid holding power causing heating. Stepper may be moved by hand beacuse there is no holding power.
  digitalWrite(enableStepper_Pin, true);

  digitalWrite(ledYellowUnfeedLiquid, LOW);  // Turn Yellow LED off
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void moveToStartPostion(int ledYellowUnfeedLiquid) {

  digitalWrite(ledYellowUnfeedLiquid, HIGH);  // Turn Yellow LED on

  // Start up for Stepper, after stopping to avoid holding Power causing heating. Do not use a jumper between EN/GND on CNC board. In code stepper enable is turned on to make a move then turned off after that move, false is on. This is to avoid holding power causing heating. Stepper may be moved by hand beacuse there is no holding power.
  digitalWrite(enableStepper_Pin, false);

  stepper1.moveTo(0);  // Set desired move: 200 steps x 1.8 degrees is 360 degrees i.e. one rotation at FULLSTEP setting, no jumpers on CNC board

  stepper1.runToPosition();  // Moves the motor to target position w/ acceleration/ deceleration and it blocks until is in position

  // Stop the Holding on the stepper. Do not use a jumper between EN/GND on CNC board. In code stepper enable is turned on to make a move then turned off after that move, false is on. This is to avoid holding power causing heating. Stepper may be moved by hand beacuse there is no holding power.
  digitalWrite(enableStepper_Pin, true);

  digitalWrite(ledYellowUnfeedLiquid, LOW);  // Turn Yellow LED off
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void resetStepper() {

  stepper1.setCurrentPosition(0);

  // Stop the Holding on the stepper, false is on. Allow stepper to be manually moved by Hand. Do not use a jumper between EN/GND on CNC board
  digitalWrite(enableStepper_Pin, true);

  // Allow stepper to be manually moved by Hand
  stepper1.disableOutputs();

  stepper1.stop();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void flashLED(int led) {

  digitalWrite(led, HIGH);

  delay(250);

  digitalWrite(led, LOW);

  delay(250);

  digitalWrite(led, HIGH);

  delay(250);

  digitalWrite(led, HIGH);

  delay(250);

  // finish with LED turned off
  digitalWrite(led, LOW);

  delay(250);
}