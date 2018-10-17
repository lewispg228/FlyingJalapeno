/*
  This example shows how to run a normal test procedure

  Pete Lewis, started on 11/3/2016
  Contributions by NES November 15th, 2016

  Select Mega2560 from the boards list
*/
#define STATUS_LED 13

#include "SparkFun_Flying_Jalapeno_Arduino_Library.h"
FlyingJalapeno FJ(STATUS_LED, 3.3); //Blink status msgs on pin 13. The FJ is setup for 3.3V I/O.


int failures = 0; //Number of failures by the main test routine

boolean targetPowered = false; //Keeps track of whether power supplies are energized

long preTestButton = 0; //Cap sense values for two main test buttons
long testButton = 0;

void setup()
{
  pinMode(STATUS_LED, OUTPUT);

  FJ.enablePCA(); //Enable the I2C buffer

  Serial.begin(9600);
  Serial.println("Full test example. Press the Pretest button to begin.");
}

void loop()
{
  if (FJ.isPretestPressed() == true)
  {
    FJ.dot(); //Blink status LED to indicate button press

    if (targetPowered == true) 
    {
      power_down(); //Power down the test jig
    }
    else
    {
      //Check both power supplies for shorts to ground
      if (FJ.isRegulator1Shorted() == false && FJ.isRegulator2Shorted() == false)
      {
        FJ.setRegulatorVoltage1(5.0); //Turn on power supply 5.0V
        FJ.enableRegulator1();
        
        delay(500);

        Serial.println("Pre-test PASS, powering up...\n\r");

        targetPowered = true;

        digitalWrite(LED_PT_PASS, HIGH);
        digitalWrite(LED_PT_FAIL, LOW);

        delay(50); //Debounce
      }
      else
      {
        //Power supply test failed
        failures++;

        FJ.disableRegulator1(); //Turn off power supply 1
        
        Serial.println("Jumper on Power Rail V1\n\r");
        
        targetPowered = false;
        
        digitalWrite(LED_PT_FAIL, HIGH);
        digitalWrite(LED_PT_PASS, LOW);

        delay(500); // debounce touching
      }
    }
  }
  else if (FJ.isTestPressed() == true && targetPowered == true)
  {
    Serial.println("Running test");
    
    //Begin main test
    FJ.statOn();

    digitalWrite(LED_PASS, LOW);
    digitalWrite(LED_FAIL, LOW);

    //test_33V();
    if(test() == true) //Run main test code
    {
      Serial.println("Test complete");
      digitalWrite(LED_PASS, HIGH);
    }
    else
    {
      digitalWrite(LED_FAIL, HIGH);
    }

    FJ.disableRegulator1(); //Turn off power supply 1
    Serial.println("Regulator 1 disabled");
    targetPowered = false;

    FJ.statOff();
  }
}

boolean test()
{
  // add in your test code here
  return(true); //If we passed
}

//This is an example of testing a 3.3V output from the board sent to A2.
void test_33V()
{
  Serial.println("testing 3.3V output on board");

  //pin = pin to test
  //correct_val = what we expect.
  //allowance_percent = allowed window for overage. 0.1 = 10%
  //debug = print debug statements
  boolean result = FJ.verify_voltage(A2, 516, 0.1, true); // 3.3V split by two 10Ks, reads 516 on my 3.3V logic FJ (using a proto known good).

  if (result == true) 
    Serial.println("test success!");
  else
  {
    Serial.println("test failure (should read near 516)");
    failures++;
  }
}

//Turn off all regulators
void power_down()
{
  Serial.println("Powering down target");

  FJ.disableRegulator1(); //Turn off power supply 1, but leave voltage selection at 5V
  FJ.disableRegulator2(); //Turn off power supply 1, but leave voltage selection at 4.2V

  targetPowered = false;

  //Turn off all LEDs
  digitalWrite(LED_PT_PASS, LOW);
  digitalWrite(LED_PT_FAIL, LOW);
  digitalWrite(LED_PASS, LOW);
  digitalWrite(LED_FAIL, LOW);

  failures = 0;
}

//Demonstrates how to modify the regulator voltage to mimic a battery getting up to
//a fully charged voltage level (4.2V). The charge LED should turn off.
void charge_led_blink()
{
  FJ.enableRegulator2();
  for (int i = 0 ; i < 3 ; i++)
  {
    FJ.setRegulatorVoltage2(3.7); // charge led ON
    delay(500);
    FJ.setRegulatorVoltage2(4.2); // charge led off
    delay(500);
  }
  FJ.disableRegulator2(); //For good measure, turn off this regulator
}
