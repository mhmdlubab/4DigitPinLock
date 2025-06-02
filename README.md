# 4-Digit PIN Lock System

A simple 4-digit PIN lock system implemented using an Arduino Mega 2560 board and a 4-digit 7-segment display. This project allows users to enter a 4-digit PIN to unlock a device or system. The display shows the entered digits, and status LEDs indicate whether the PIN entered is correct or incorrect.

## Features

- Secure 4-digit PIN entry using buttons
- Real-time display of entered digits on a 7-segment display
- Green LED indicates successful PIN entry
- Red LED indicates incorrect PIN entry
- passive buzzer for sfx(correct/incorrect pin, changing digit beep)

## Hardware

- Arduino Mega 2560 board
- 4-digit 7-segment display (12 pins, but 14 pins one worked fine in simulation software if the last 2 pins on the right are left empty in both the upper and lower sections)
- Two LEDs (green and red)
- 5 Push buttons, 4 for digit input, 1 for checking the passkey, also works as powerbutton, if clicked for more than 2 seconds makes the board enter sleep mode
- Resistors (6 470-Ohm, 4 used in the 4-digit 7-segment display, and 2 with the LEDs- you can add more 220-Ohm resistor to the green LED if too bright, as it was for me-)
- 1 Servo Motor
- connecting wires

## Wiring Diagram

Refer to the included schematic for wiring details. Note the important hardware differences below.

## Important Notes

- The 4-digit 7-segment display shown in the schematic has **14 pins**, but the actual display used in this project has **12 pins**.  
  In the diagram, the last two pins on the right (top and bottom) are left unconnected to reflect this, they are probably another type of display, but they worked the same in the simulation.

- The LEDs in the diagram are both shown as red. In the real setup:  
  - The LED on the **left** is **green** (indicates correct PIN)  
  - The LED on the **right** is **red** (indicates incorrect PIN)

- Another important note is that the program works just fine when the power sourse is the stable 5v usb, but when conneced via the battery dc source you're gonna need to connect the servo motor to a different
  source, otherwise the program will have problems excuting properly(a common problem in arduino if we connect everything on the board when not using usb), if you use this appraoch remember to connect everything
  to a common ground, the board, the Servo, and the Servo power source(another battery or lithuim battary)
  
## Installation

1. Clone the repository:  
   ```bash
   git clone https://github.com/yourusername/4DigitPinLock.git
2. Open the 4digitPIN.ino file in the Arduino IDE.

3. Connect your hardware according to the wiring diagram.

4. Upload the code to your Arduino board.

## Usage

### 🔧 First-Time Setup

When the system is powered on **for the very first time**, it does not have a predefined 4-digit PIN. Instead, it prompts the user to **set their own PIN** using physical input buttons.

#### ➤ How to Set the PIN:
1. Press the power button to wake up the board(it will automatically be in sleep mode even if power was off and we connected it), the Servo will be in position 0
1. Press the input buttons to enter your desired 4-digit PIN.
2. Each digit you press is displayed on the 4-digit 7-segment display.
3. After entering the desired passkey:
   
   - press the powerButton to save(the one to the far left)
   - The system automatically stores this PIN in memory(stored in EEPROM memory so that if power is cut off the passkey is permantly saved, and when the arduino runs next time it will check if EEPROM memory
     and act accordingly if there's a passkey or not, you can check this in the setup function, if you want to do another project please remember to clear the EEPROM memory just in case it gets in hand with
     something in your new program, you can google find a program for this in the repository).
   - It is now ready to verify future PIN attempts against this stored value.
   - while you are in this state you can keep changing the passkey and press the powerButton to save it.
   - you can go to the lock state at any point by long pressing the powerButton for 2 seconds then the board will enter the sleep mode, it is only interrupted by clicking the powerButton.


---

### 🔁 Regular Use (After First Setup)

Once a PIN has been set, the system works as a basic PIN verification lock.

#### ➤ To Enter the PIN:
1. Press the power button to wake up the board(it will automatically be in sleep mode even if power was off and we connected it), the Servo will be in position 180
2. Press the buttons to enter a 4-digit sequence.
3. At any given time press the powerButton to check the pin:-

#### ✅ If the PIN is Correct:
- A green LED (left side) will light up, and the correct pin tone will play.
- The Servo turns to position 0.
- once in this state you can change the passkey again

#### ❌ If the PIN is Incorrect:
- A red LED (right side) will light up, and the incorrect pin tone will play.

---

## No action feature
- if no acion in setting passkey mode, after 2 minutes the system will go to sleep
- if no action in entering passkey mode, after 3 minutes the system will go to sleep
- to wake up the system click the power button





