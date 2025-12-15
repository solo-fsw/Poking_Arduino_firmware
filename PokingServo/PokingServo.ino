/*
  Poking servo device
  Copyright Leiden University SOLO - Jarik den Hartog
  
  Based on https://github.com/solo-fsw/UsbParMarker/tree/main/Firmware/UsbParMarkerV2

  ************************************************************
  * Works on CDC capable devices like Arduino Leonardo only! *
  ************************************************************
  
  *************************** Hardware safety *******************************
  * The servo moves to a position when the Arduino reboots depending on the *
  * DEFAULT_PULSE_WIDTH in VarSpeedServo.h. Make this a safe (for example   *
  * retracted) position in the hardware design!                             *
  ***************************************************************************

  Todos:
  -Check for position and speed to be in range (0-180 0-255)
  -Button on interrupt pin making the servo retract and go in an endless loop?
  -Fix need for: servo.write(90,0); //TEMPORARY to prevent faulty isMoving true state
  -Include in VarSpeedServo lib in this project?
  -Include license txt
  -Return 'manual' one 'wrong' baud rate
  -Add 'p' command to return data protocol
  
  ***************************************************************************
  Serial protocol:
  115200 baud rate = data mode:
  This firmware expects 2 bytes (position and speed) with a baud rate of 115200. It
  will acknowledge receival when the servo (theoretically) reaches that position,
  with one byte; the final position.
  position  (byte) 0 to 180
  speed     (byte) 0=full speed, 1-255 slower to faster
  Note the time it will take the servo to get to a position depends on the speed byte,
  but also on the starting position of the servo.
  When something is wrong (like the servo is already moving) a negative error code
  will be returned (see Error codes: below).

  4800 baud rate = command mode:
  Or a command with a baud rate of 4800 (see handleCommands() in subs.ino).
  For example: sending 'v' at 4800 to the Poking servo, returns the version info.
  And sending 'i' returns true if the servo (object) is still moving.
  
  For slow testing with the Arduino Serial Monitor (No Line Ending & 115200 baud)
  one can copy paste the 2 bytes below:
   (=position   1 speed 1)
  ~  (=position 126 speed 1)

  Usage:
  Start with sending 'unlock' @4800 ones to unlock the servo after a reboot.
  Send 'v' @4800 to get version info and store it in the output data ones.
  
  Now you can start sending position and speed byte sets to move the servo:
  if(!{isMoving by sending 'i' @4800})
    Serial.send(postion, speed) @115200

  ***************************************************************************

  HW0 - Still on breadboard
  HW1 - <todo>
  
  20251204  1.0 Initial release
  202xxxxx  1.1 <todo>
  Do not forget to update the FirmwareVersion string below!

*/
#include <EEPROM.h> // for to read/write serial number and hardware version
#include <VarSpeedServo.h>  // for controlling the servo

//#define DEBUG // comment out this line for non debug release!
#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

//Globals:
const String FirmwareVersion = "FW1.0";
const String VersionInfo;
const String Serialnumber;
const String HardwareVersion;
const VarSpeedServo servo; // create servo object to control the servo

//Error codes:
const char ERROR_SERVO_LOCKED = -1;
const char ERROR_SERVO_MOVING = -2;

bool servoUnlocked = false; // start servo in a locked state to prevent other software accidentally moving the servo
bool finalPositionSent = false;

void setup() {
 //writeStringToEEPROM(10, "S00000"); //Use ones to program the serial number in the eeprom of the device
 //writeStringToEEPROM(20, "HW0"); //Use ones to program the hardware version in the eeprom of the device

  Serial.begin(115200);     // opens serial port, sets data rate to 115200 bps
  Serialnumber = readStringFromEEPROM(10);
  HardwareVersion = readStringFromEEPROM(20);
  VersionInfo = String(HardwareVersion + ":" + FirmwareVersion);   // Set HW version always

  servo.attach(9); // attaches the servo on pin 9 to the servo object
  servo.write(90,0); //TEMPORARY to prevent faulty isMoving true state
  pinMode(LED_BUILTIN, OUTPUT); // initialize digital pin LED_BUILTIN as an output.
  while (!Serial);  // Wait for Serial to be ready 

#ifdef DEBUG
  DEBUG_PRINTLN("**************************************************************************");
  DEBUG_PRINTLN("**** Warning: DEBUG defined > printing debug info and servo unlocked! ****");
  DEBUG_PRINTLN("**************************************************************************");
  servoUnlocked = true;
#endif

}

void loop() {
  if (Serial.available() > 0) {
    if (Serial.baud() == 115200 ) { 
      // ***************************
      // ******** data mode ******** 
      // ***************************
      char position = (char)Serial.read(); // read position byte
      char speed = (char)Serial.read(); // read speed byte

      if (!servoUnlocked) {
        DEBUG_PRINTLN("Sending ERROR_SERVO_LOCKED");
        Serial.println(ERROR_SERVO_LOCKED);
        return;
      }

      if (servo.isMoving()) {
        DEBUG_PRINTLN("Sending ERROR_SERVO_MOVING");
        Serial.println(ERROR_SERVO_MOVING);
        return;
      }

      servo.write(position, speed);  // make servo object move to position with given speed
      finalPositionSent = false;
      digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on
      DEBUG_PRINTLN("Received position: " + String(position, DEC));
      DEBUG_PRINTLN("Received speed: " + String(speed, DEC));
    }
    else if (Serial.baud() == 4800) {
      // ******************************
      // ******** command mode ********
      // ******************************
      handleCommands();
    } 
  }

  // do stuff when servo reaches the final position
  if(!finalPositionSent && !servo.isMoving()) {
    DEBUG_PRINTLN("Sending final position");
    char byte = servo.read(); // change integer position to byte 
    Serial.println(byte); // acknowledge with the servo (theoretical) final position 
    finalPositionSent = true;
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED off
  }
}
