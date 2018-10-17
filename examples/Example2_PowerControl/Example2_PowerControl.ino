/*
  This example shows how to turn on the two onboard voltage regulators.

  Regulator 1 can be set to 5V or 3.3V
  Regulator 2 can be set to 5, 4.2, 3.7, and 3.3V. This is handy to simulate Lipo power or charging.

  Pete Lewis, started on 11/3/2016
  Contributions by NES November 15th, 2016

  Select Mega2560 from the boards list
*/

#define STATUS_LED 13

#include "SparkFun_Flying_Jalapeno_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_Jalapeno
FlyingJalapeno FJ(STATUS_LED, 3.3); //Blink status msgs on pin 13. Board has VCC jumper set to 3.3V.

void setup()
{
  Serial.begin(9600);
  Serial.println("Power Control Example");

  //Regulator 1 can be set to 5V or 3.3V
  //Regulator 2 can be set to 5, 4.2, 3.7, and 3.3V. This is handy to simulate Lipo power or charging.
  FJ.setRegulatorVoltage1(5.0); //Set regulator1 to 5V
  FJ.setRegulatorVoltage2(3.3); //Set regulator2 to 3.3V
}

void loop()
{
  if (FJ.isPretestPressed() == true)
  {
    Serial.println("Powering up regulator 1!");

    FJ.enableRegulator1(); //Turn on regulator
    FJ.disableRegulator2(); //Turn off the 2nd regulator
    
    digitalWrite(LED_PT_PASS, HIGH);
    digitalWrite(LED_PT_FAIL, LOW);

    delay(50); //Debounce
  }
  else if (FJ.isTestPressed() == true)
  {
    Serial.println("Powering up regulator 2!");
    
    FJ.disableRegulator1(); //Turn off the 1st regulator
    FJ.enableRegulator2(); //Turn on regulator

    digitalWrite(LED_PASS, HIGH);
    digitalWrite(LED_FAIL, LOW);

    delay(50); //Debounce
  }
}

