/* First time ever you run it(pressing the power button):
  - the light is green
  - it is open, aka myServo.write(0);
  - you can move the numbers around to set a passkey, then click the power button to save it, you can still move around to change it again, then click the power button again to save
  - long click the power button to turn off and activate lock, and then one click to turn it on
  - if this is the actual first time ever, and not just the state we get in after entering the right key then you will notice the following:-
    - after 2 minutes it will sleep on its on, but once you hit the power back we will get back to this state, and the lock state will not be entered if you havn't saved a passKey
    ,aka pressed the powerButton and heard the confirm sound(playSuccessTone), if you have already saved a passKey then the lock state will be entered right away
  - if this the state we get in after entering the passKey right, then:-
    - after 2 minutes program will sleep on its own and upon wake up will enter lock mode

   second+ times you run the code(pressing the power button) ((lock mode))
  - you will see red light on
  - it is closed, aka myServo.write(180); but you might see that the motor is in position 0, that is due to the power being cut off while in the stage of setting a password
  so you can't cut the power in that stage, if you do, when you turn it on, the passKey will be the last passKey saved(aka the one on the screen when we short pressed the
  power button last time and the light was green)
  - you can try typing the passkey and press the power button to check if it is right
  - green light will turn on if right and red will be off
  - now it is open, aka myServo.write(0);
  - you will be back to the first state above and you can set a new passkey
  - after 3 minutes program will sleep on its own
  
  NOTE: first time ever code runs this   if (EEPROM.read(4) == 'z')   will be false but anyother time it will be right forever because a password was set for sure
  NOTE2: I used EEPROM to make sure the program works even if power was cut off suddenly, the only flaw is that it might cutt off while we are in the setting password state, then 
  we will need to physacilly close the door of the safe, so technically could be bypassed with a smart design of the safe*/

#include <EEPROM.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>  // we need to include this to use the softwareReset function
#include "SevSeg.h"
SevSeg sevseg;  //Initiate a seven segment controller object
#include <Servo.h>
Servo myServo;

int powerButton = 21;  // has to be 21 because only a number of pins can wake the board (only 2, 3, 18, 19, 20, 21 can be used to wake the Mega from sleep using attachInterrupt())
int button0 = 49;
int button1 = 50;
int button2 = 51;
int button3 = 52;
int pinTone = A0;
int greenLED = 46;
int redLED = 47;
char chars[4] = { '0', '0', '0', '0' };
char emptyChars[4] = { '0', '0', '0', '0' };
char passKey[4] = { '0', '0', '0', '0' };
unsigned long timeWhenPressed = 0;
unsigned long timeSinceLastActivity = 0;  // since last we enterd a new passKey or checked if the passKEy was correct
bool buttonHeld = 0;

void (*resetFunc)(void) = 0;

void softwareReset() {
  wdt_enable(WDTO_15MS);  // Enable WDT(watchdog timer) with 15ms timeout, if we don't run wdt_disable() or wdt_reset() before 15ms then the arduino will force reset
  while (1)
    ;  // Wait until it resets, we can use this to make sure we stay in this no matter what and nothing will stop the timer
}

void playSuccessTone(int buzzerPin) {
  tone(buzzerPin, 1000, 150);  // 1000 Hz for 150 ms
  digitalWrite(greenLED, 0);
  delay(200);
  tone(buzzerPin, 1500, 150);  // 1500 Hz for 150 ms
  digitalWrite(greenLED, 1);
  delay(200);
  tone(buzzerPin, 2000, 300);  // 2000 Hz for 300 ms
  digitalWrite(greenLED, 0);
  delay(300);
  digitalWrite(greenLED, 1);
}

void playErrorTone(int buzzerPin) {
  tone(buzzerPin, 400, 300);  // 400 Hz for 300 ms
  delay(350);
  tone(buzzerPin, 300, 300);  // 300 Hz for 300 ms
  delay(350);
}

void changeDigit(int buttonPin, int indexOfDigit) {
  // int counterOfSoundDuration = 0;
  if (digitalRead(buttonPin) == 0) {
    timeSinceLastActivity = millis();  // reset last activity timer if we click any of the numbers buttons

    if (chars[indexOfDigit] != '9') {
      chars[indexOfDigit] = ((chars[indexOfDigit] - '0') + 1) + '0';
    } else {
      chars[indexOfDigit] = '0';
    }
    sevseg.setChars(chars);
    // 2 ways to make the sound happen when we first press the button and stay for 300ms if we are still pressing the button and cut off before
    // the sound finishes right away if we let go of the button even before the sound is finished:-
    // 1- using tone fucntion with 3 parameters
    if (digitalRead(buttonPin) == 0) {
      sevseg.refreshDisplay();
      tone(pinTone, 523.25, 300);
    }
    while (digitalRead(buttonPin) == 0) { sevseg.refreshDisplay(); }  // makes sure we don't force stop the sound unless we let go of the button
    noTone(pinTone);
    // 2- using tone fucntion with 2 parameters
    // while (digitalRead(buttonPin) == 0) {
    //   sevseg.refreshDisplay();
    //   delay(2);
    //   if (counterOfSoundDuration < 150)
    //     tone(pinTone, 523.25);
    //   else  // this else refrenced below
    //     noTone(pinTone);
    //   counterOfSoundDuration++;
    // }
    //noTone(pinTone);  // because if we let go before the counter reaches 150 the else above wont run and it will keep beeping till the next time the counter reaches 150
    for (int i = 0; i < 125; i++) {
      sevseg.refreshDisplay();
      delay(2);
    }
  }
}

void goToSleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // Deepest sleep mode
  sleep_enable();

  noInterrupts();
  EIFR = bit(INTF2);  // Clear pending interrupt flag for INT2 (pin 21)
  interrupts();
  attachInterrupt(digitalPinToInterrupt(powerButton), wakeUp, FALLING);  // Enable wake interrupt
  sleep_enable();
  //sevseg.blank(); // setting the screen to blank, we can do it here or before calling the goToSleep func
  sleep_cpu();  // anyhting before sleep_cpu will fire before sleep, anything after will fire after
  // when wake up event happens, aka clicking the powerButton, the code will run the wakeUp fucntion and then continues here
  sleep_disable();                                      // prevents it from sleeping again the next time sleep_cpu() is called.
  detachInterrupt(digitalPinToInterrupt(powerButton));  // Disable interrupt after waking
  while (digitalRead(powerButton) == 0) {}
}

void wakeUp() {
  // does nothing just needs to be fired
}

bool arraysEqual(char* a, char* b, int size) {
  for (int i = 0; i < size; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

void delayAndRefresh(int delayDuration) {  // delayDuration has to be in ms
                                           // we use this when we want to delay and keep updating the screen
  for (int i = 0; i < (delayDuration / 2); i++) {
    sevseg.refreshDisplay();
    delay(2);
  }
}

void setup() {
  byte numDigits = 4;
  byte digitPins[] = { 2, 3, 4, 5 };
  byte segmentPins[] = { 6, 7, 8, 9, 10, 11, 12, 13 };
  bool resistorsOnSegments = 0;
  // variable above indicates that 4 resistors were placed on the digit pins.
  // set variable to 1 if you want to use 8 resistors on the segment pins.
  sevseg.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins, resistorsOnSegments);
  sevseg.setBrightness(90);
  pinMode(powerButton, INPUT_PULLUP);
  pinMode(button0, INPUT_PULLUP);
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  myServo.attach(48);
  if (EEPROM.read(4) == 'z') {  // we check if we assigned 'z' to address 4 in the EEPROM memory, if we did it means that a past passKey has been set
    for (int i = 0; i < 4; i++) {
      passKey[i] = EEPROM.read(i);
    }
    myServo.write(180);  // if we have a password turn to lock state (180) if it was on 0 before(if power cut off when it was  in the unlock state)
    delay(650);          // to wait for the servo to turn before entering sleep
    goToSleep();         // so that when we connect it to electricity it won't work until we click the power button
    return;
  } else {        // if first time ever this else runs
    goToSleep();  // so that when we connect it to electricity it won't work until we click the power button
  }
  myServo.write(0);
  digitalWrite(greenLED, 1);
  while (digitalRead(powerButton) == 0) {}  // to keep the program here so that the loop fucntion doens't start until
  // we let go of the first button press(aka the start press)
  delayAndRefresh(250);
  while (1) {
    sevseg.setChars(chars);
    sevseg.refreshDisplay();
    changeDigit(button3, 3);
    changeDigit(button2, 2);
    changeDigit(button1, 1);
    changeDigit(button0, 0);
    if ((digitalRead(powerButton) == 0) && !buttonHeld) {  // if the button was just pressed and the button was not being held it means that is a new press so restart the timer
      timeWhenPressed = millis();
      buttonHeld = 1;
    }
    if (buttonHeld) {
      if ((millis() - timeWhenPressed >= 2000)) {
        for (int i = 0; i < 75; i++) {
          sevseg.setChars(emptyChars);
          sevseg.refreshDisplay();
          delay(20);
        }
        sevseg.blank();
        myServo.write(180);
        delay(1000);
        digitalWrite(greenLED, 0);
        goToSleep();
        // softwareReset();
        // resetFunc(); // we can use this function or the one above to reset but
        buttonHeld = 0;
        for (int i = 0; i < 4; i++) {
          chars[i] = '0';
        }
        timeSinceLastActivity = millis();  // reset last activity timer after waking up
        break;
      } else if (!(digitalRead(powerButton) == 0)) {
        for (int i = 0; i < 4; i++) {
          passKey[i] = chars[i];
          EEPROM.write(i, passKey[i]);
        }
        EEPROM.write(4, 'z');
        sevseg.blank();
        playSuccessTone(pinTone);
        buttonHeld = 0;
        timeSinceLastActivity = millis();  // reset last activity time since last time we set a new password
      }
    }
    if (millis() - timeSinceLastActivity >= 120000) {
      for (int i = 0; i < 75; i++) {
        sevseg.setChars(emptyChars);
        sevseg.refreshDisplay();
        delay(20);
      }
      sevseg.blank();
      myServo.write(180);
      delay(1000);
      digitalWrite(greenLED, 0);
      goToSleep();
      buttonHeld = 0;
      for (int i = 0; i < 4; i++) {
        chars[i] = '0';
      }
      timeSinceLastActivity = millis();
      if (EEPROM.read(4) == 'z') {
        break;
      }
      digitalWrite(greenLED, 1);
      myServo.write(0);
    }
  }
}

void loop() {
  digitalWrite(redLED, 1);
  bool buttonJustPressed = (digitalRead(powerButton) == 0);

  sevseg.setChars(chars);
  sevseg.refreshDisplay();
  changeDigit(button3, 3);
  changeDigit(button2, 2);
  changeDigit(button1, 1);
  changeDigit(button0, 0);
  if (buttonJustPressed && !buttonHeld) {  // if the button was just pressed and the button was not being held it means that is a new press so restart the timer
    timeWhenPressed = millis();
    buttonHeld = 1;
  }
  /* now there are two ways to do it
  1- activate the restart after we let go, and after clicking for more than 2 seconds:
  if (!buttonJustPressed && buttonHeld) {      // if the button was not just pressed and the buttonHeld is true, aka we have pressed the button before that means we let go of the button
    if (millis() - timeWhenPressed >= 2000) {  // check how long we pressed the button, anything more than 2 seconds is reset
      for (int i = 0; i < 75; i++) {
        sevseg.setChars(emptyChars);
        sevseg.refreshDisplay();
        delay(20);
      }
      resetFunc();
    } else {
      sevseg.blank();
      delay(100);
      buttonHeld = 0;  // first reset the buttonHeld because that means we let go of the button
    }
  }
  ====================================================================================================================
  2- activate the restart even if we are still clicking the button, and we have been clicking for more than 2 seconds: */
  if (buttonHeld) {
    if ((millis() - timeWhenPressed >= 2000)) {
      for (int i = 0; i < 75; i++) {
        sevseg.setChars(emptyChars);
        sevseg.refreshDisplay();
        delay(20);
      }
      sevseg.blank();
      myServo.write(180);
      delay(1000);
      digitalWrite(redLED, 0);
      goToSleep();
      // softwareReset();
      // resetFunc(); // we can use this function or the one above to reset but
      buttonHeld = 0;
      for (int i = 0; i < 4; i++) {
        chars[i] = '0';
      }
      timeSinceLastActivity = millis();  // reset last activity timer after waking up
    } else if (!buttonJustPressed) {
      if (arraysEqual(chars, passKey, 4)) {
        sevseg.blank();
        delay(100);
        myServo.write(0);
        digitalWrite(redLED, 0);
        digitalWrite(greenLED, 1);
        timeSinceLastActivity = millis();  // reset last activity timer before we enter the unlock state

        buttonHeld = 0;
        playSuccessTone(pinTone);
        while (1) {
          sevseg.setChars(chars);
          sevseg.refreshDisplay();
          changeDigit(button3, 3);
          changeDigit(button2, 2);
          changeDigit(button1, 1);
          changeDigit(button0, 0);
          if ((digitalRead(powerButton) == 0) && !buttonHeld) {  // if the button was just pressed and the button was not being held it means that is a new press so restart the timer
            timeWhenPressed = millis();
            buttonHeld = 1;
          }
          if (buttonHeld) {
            if ((millis() - timeWhenPressed >= 2000)) {
              for (int i = 0; i < 75; i++) {
                sevseg.setChars(emptyChars);
                sevseg.refreshDisplay();
                delay(20);
              }
              sevseg.blank();
              myServo.write(180);
              delay(1000);
              digitalWrite(greenLED, 0);
              goToSleep();
              buttonHeld = 0;
              for (int i = 0; i < 4; i++) {
                chars[i] = '0';
              }
              timeSinceLastActivity = millis();  // reset last activity timer after waking up
              break;
            } else if (!(digitalRead(powerButton) == 0)) {
              for (int i = 0; i < 4; i++) {
                passKey[i] = chars[i];
                EEPROM.write(i, passKey[i]);
              }
              EEPROM.write(4, 'z');
              sevseg.blank();
              playSuccessTone(pinTone);
              buttonHeld = 0;
              timeSinceLastActivity = millis();  // reset last activity timer if we set a new password
            }
          }
          if (millis() - timeSinceLastActivity >= 120000) {
            for (int i = 0; i < 75; i++) {
              sevseg.setChars(emptyChars);
              sevseg.refreshDisplay();
              delay(20);
            }
            sevseg.blank();
            myServo.write(180);
            delay(1000);
            digitalWrite(greenLED, 0);
            goToSleep();
            buttonHeld = 0;
            for (int i = 0; i < 4; i++) {
              chars[i] = '0';
            }
            timeSinceLastActivity = millis();  // reset last activity timer after waking up
            break;
          }
        }
      } else {
        sevseg.blank();
        delay(100);
        playErrorTone(pinTone);
        timeSinceLastActivity = millis();  //reset last activity timer if we click the button
      }
      buttonHeld = 0;
    }
  }
  if (millis() - timeSinceLastActivity >= 180000) {
    for (int i = 0; i < 75; i++) {
      sevseg.setChars(emptyChars);
      sevseg.refreshDisplay();
      delay(20);
    }
    sevseg.blank();
    myServo.write(180);
    delay(1000);
    digitalWrite(redLED, 0);
    goToSleep();
    buttonHeld = 0;
    for (int i = 0; i < 4; i++) {
      chars[i] = '0';
    }
    timeSinceLastActivity = millis();  // reset last activity timer after waking up
  }
  /* the below was used as the above for a button that restarts after clicking for 2 seconds, it is problem was that the first action would be called no matter what
  and then the second, the above makes the second(restart) work without firing the first(checking the passkey)
  if (digitalRead(powerButton) == 0) {
    for (int i = 0; i < 125; i++) {
      sevseg.refreshDisplay();
      delay(2);
    }
    if (digitalRead(powerButton) == 0) {
      sevseg.blank();
    }
  }
  while (digitalRead(powerButton) == 0) {
    sevseg.refreshDisplay();
    delay(2);
    counterOfPressDuration++;
    if (counterOfPressDuration > 1000) {
      for (int i = 0; i < 75; i++) {
        sevseg.setChars(emptyChars);
        sevseg.refreshDisplay();
        delay(20);
      }
      resetFunc();
    }
  } */
}