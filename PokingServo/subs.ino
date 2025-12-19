void writeStringToEEPROM(int addrOffset, const String &strToWrite) {
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++) {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int addrOffset) {
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++) {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0';  // the character may appear in a weird way, you should read: 'only one backslash and 0'
  return String(data);
}

void DumpMyInfo() {  //in JSON format
  Serial.print("{\"VersionInfo\":\"");
  Serial.print(VersionInfo);
  Serial.print("\",");
  Serial.print("\"Serialnumber\":\"");
  Serial.print(Serialnumber);
  Serial.print("\",");
  Serial.println("\"Device\":\"PokingServo\"}");
}

void handleCommands() {
  switch (Serial.read()) {
    case 'v':
      DumpMyInfo();
      break;
    case 'u':
      servoUnlocked = true;
      digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on
      Serial.write('u');
      break;
    case 'i':
      Serial.write(servo.isMoving());
      break;
    case 's':
      servo.stop();
      digitalWrite(LED_BUILTIN, LOW);  // turn the LED off
      Serial.write('s');
      break;  //finalPositionSent and LED will be handled in main loop()
    default:
      Serial.write('-');
      break;
  }
}
