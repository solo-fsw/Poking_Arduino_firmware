/*
  Poking servo device
  Copyright Leiden University SOLO - Jarik den Hartog
  
  Based on https://github.com/solo-fsw/UsbParMarker/tree/main/Firmware/UsbParMarkerV2

  ****************************************************
  * Works on CDC capable devices like Leonardo only! *
  ****************************************************

  Serial protocol:
  This firmware expects 2 bytes (position and speed) with a baud rate of 115200. It
  will acknowledge receival when the servo (theoretically) reaches that position,
  with one byte, the position.
  position  (byte) 0 to 180
  speed     (byte) 0=full speed, 1-255 slower to faster
  Note the time it will take the servo to get to a position depends on the speed byte,
  but also on the starting position of the servo.

  Or a command with a baud rate of 4800 (see DumpMyInfo() in subs.ino).
  For example: sending 'v' at 4800 to the Poking servo, returns the version info.
  And sending 'i' returns true if the servo (object) is still moving.
  
  ****************************************************
  HW0 - Still on breadboard
  HW1 - <todo>
  
  20251204  1.0 Initial release
  
*/
#include <EEPROM.h> // for to read/write serial number and hardware version
#include <VarSpeedServo.h>  // for controlling the servo

#define DEBUG // command out this line for non debug release!
#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

//Globals
const String FirmwareVersion = "FW1.0";
const String VersionInfo;
const String Serialnumber;
const String HardwareVersion;
const VarSpeedServo servo; // create servo object to control the servo

//Error codes 
const int ERROR_SERVO_MOVING = -1;

bool acknowledgeSend = false;

void setup() {
 //writeStringToEEPROM(10, "S00000"); //Use ones to program the serial number in the eeprom of the device
 //writeStringToEEPROM(20, "HW0"); //Use ones to program the hardware version in the eeprom of the device

  Serial.begin(115200);     // opens serial port, sets data rate to 115200 bps
  Serialnumber = readStringFromEEPROM(10);
  HardwareVersion = readStringFromEEPROM(20);
  VersionInfo = String(HardwareVersion + ":" + FirmwareVersion);   // Set HW version always

  servo.attach(9); // attaches the servo on pin 9 to the servo object
  pinMode(LED_BUILTIN, OUTPUT); // initialize digital pin LED_BUILTIN as an output.
}

void loop() {
  if(!acknowledgeSend && !servo.isMoving()) {
    DEBUG_PRINTLN("Sending acknowledge");  
    Serial.println(servo.read()); // acknowledge with the servo (theoretical) position 
    acknowledgeSend = true;
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED off
  }
  if (Serial.baud() == 115200 ) { // data mode
    if (Serial.available() > 0) {
      char position = (char)Serial.read(); // read position byte
      char speed = (char)Serial.read(); // read speed byte
      servo.write(position, speed); // make servo object move to position with given speed   
      acknowledgeSend = false;
      digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on

      String s = String(position, DEC);
      DEBUG_PRINTLN("Received position: " + s);
      s = String(speed, DEC);
      DEBUG_PRINTLN("Received speed: " + s);
    }
  }
  else if (Serial.baud() == 4800) { // command mode
    if (Serial.available() > 0) {
      handleCommands();
    }
  }
}
