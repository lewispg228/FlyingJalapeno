/*
  This example shows how to read the two capacitive touch buttons on every test jig.

  Pete Lewis, started on 11/3/2016
  Contributions by NES November 15th, 2016

  Select Mega2560 from the boards list
*/

#define STATUS_LED 13

#include "SparkFun_Flying_Jalapeno_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_Jalapeno
//The FJ library depends on the CapSense library that can be obtained here: http://librarymanager/All#CapacitiveSensor_Arduino

FlyingJalapeno FJ(STATUS_LED, 3.3); //Blink status msgs on pin 13. Board has VCC jumper set to 3.3V.

void setup()
{
  Serial.begin(9600);
  Serial.println("Reading the Test buttons example");
}

void loop()
{
  //Is user pressing PreTest button?
  if (FJ.isPretestPressed() == true)
  {
    Serial.println("You pressed pretest!");
    
    digitalWrite(LED_PT_PASS, HIGH);
    digitalWrite(LED_PT_FAIL, LOW);

    delay(50); //Debounce
  }
  else if (FJ.isTestPressed() == true)
  {
    Serial.println("You pressed test!");
    
    digitalWrite(LED_PASS, HIGH);
    digitalWrite(LED_FAIL, LOW);

    delay(50); //Debounce
  }
}

