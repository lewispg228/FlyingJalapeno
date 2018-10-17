/*
  The Flying Jalapeño has an I2C buffer on board. This shows how to enable the buffer
  and transmit I2C to the target

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
  Serial.println("I2C Enable Example");

  FJ.enablePCA(); //Enable the I2C buffer

  FJ.setRegulatorVoltage1(3.3); //Set regulator1 to 3.3V

  if(FJ.testRegulator1() == false)
  {
    Serial.println("Whoa! Short on power rail 1");
    while(1);
  }

  //Now power up the target
  FJ.enableRegulator1();
}

void loop()
{

}

