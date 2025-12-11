void writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int addrOffset)
{
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0'; // the character may appear in a weird way, you should read: 'only one backslash and 0'
  return String(data);
}

void DumpMyInfo() { //in JSON format
  Serial.print("{\"VersionInfo\":\"");
  Serial.print(VersionInfo);
  Serial.print("\",");
  Serial.print("\"Serialnumber\":\"");
  Serial.print(Serialnumber);
  Serial.print("\",");
  Serial.println("\"Device\":\"PokingServo\"}");
}

void handleCommands() {
  String tempstr = "";
  switch (Serial.read()) {
    case 'v':
      DumpMyInfo();
      break;
    case 'u':
      tempstr = Serial.readString(); //read until timeout
      tempstr.trim(); // remove any \r \n whitespace at the end of the string
      if (tempstr == "nlock") {
        servoUnlocked = "true";
        Serial.println("Servo unlocked");
      }
      break;
    case 'i':
      Serial.println(servo.isMoving());
      break;
    case 's':
      servo.stop();
      Serial.println("Servo is stopped");
      //finalPositionSent and LED will be handled in main loop()
      break;
    default:
      Serial.println("Unknown command");
      break;
  }
}
