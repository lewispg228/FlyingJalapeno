/*
  FlyingJalapeno.cpp - Library for SparkFun Qc's general testbed board, The Flying Jalapeno
  Created by Pete Lewis, June 29th, 2015
  Released into the public domain.
*/

#include "SparkFun_Flying_Jalapeno_Arduino_Library.h"

#include <CapacitiveSensor.h> //Click here to get the library: http://librarymanager/All#CapacitiveSensor_Arduino
CapacitiveSensor button1 = CapacitiveSensor(47, 45); //Wired to pins 47/45 on nearly every jig
CapacitiveSensor button2 = CapacitiveSensor(31, 46); //Wired to pins 31/46 on nearly every jig

//Given a pin, use that pin to blink error messages
FlyingJalapeno::FlyingJalapeno(int statLED, float FJ_VCC)
{
  _statLED = statLED;
  _FJ_VCC = FJ_VCC;

  pinMode(_statLED, OUTPUT);

  pinMode(LED_PRETEST_PASS, OUTPUT);
  pinMode(LED_PRETEST_FAIL, OUTPUT);
  pinMode(LED_PASS, OUTPUT);
  pinMode(LED_FAIL, OUTPUT);

  digitalWrite(LED_PRETEST_PASS, LOW);
  digitalWrite(LED_PRETEST_FAIL, LOW);
  digitalWrite(LED_PASS, LOW);
  digitalWrite(LED_FAIL, LOW);

  pinMode(PSU1_POWER_CONTROL, OUTPUT);
  disableRegulator1();

  pinMode(PSU2_POWER_CONTROL, OUTPUT);
  disableRegulator2();
}

//Returns true if value is over threshold
//Threshold is optional. 5000 is default
boolean FlyingJalapeno::isPretestPressed(long threshold)
{
  long preTestButton = button1.capacitiveSensor(30);
  if(preTestButton > threshold) return(true);
  return(false);	
}

//Returns true if value is over threshold
//Threshold is optional. 5000 is default
boolean FlyingJalapeno::isTestPressed(long threshold)
{
  long testButton = button2.capacitiveSensor(30);
  if(testButton > threshold) return(true);
  return(false);
}

//Turn stat LED on
void FlyingJalapeno::statOn()
{
  digitalWrite(_statLED, HIGH);
}	

//Turn stat LED on
void FlyingJalapeno::statOff()
{
  digitalWrite(_statLED, LOW);
}	

//Brief blink of status LED to indicate... something.
void FlyingJalapeno::dot()
{
  digitalWrite(_statLED, HIGH);
  delay(250);
  digitalWrite(_statLED, LOW);
  delay(250);
}

void FlyingJalapeno::dash()
{
  digitalWrite(_statLED, HIGH);
  delay(1000);
  digitalWrite(_statLED, LOW);
  delay(250);
}

// GENERIC PRE-TEST for shorts to GND on power rails, returns true if all is good, returns false if there is short detected
boolean FlyingJalapeno::PreTest_Custom(byte control_pin, byte read_pin)
{
  pinMode(control_pin, OUTPUT);
  pinMode(read_pin, INPUT);

  digitalWrite(control_pin, HIGH);
  delay(200);
  int reading = analogRead(read_pin);

  Serial.print("Jumper test reading:");
  Serial.println(reading);

  digitalWrite(control_pin, LOW);
  pinMode(control_pin, INPUT);

  int jumper_val = 486;

  if ((reading < (jumper_val * 1.03)) && (reading > (jumper_val * 0.97))) return false; // jumper detected!!
  else return true;
}

// GENERIC PRE-TEST for shorts to GND on power rails, returns FALSE if all is good, returns TRUE if there is short detected
boolean FlyingJalapeno::isShortToGround_Custom(byte control_pin, byte read_pin, boolean debug)
{
  pinMode(control_pin, OUTPUT);
  pinMode(read_pin, INPUT);

  digitalWrite(control_pin, HIGH);
  delay(200);
  int reading = analogRead(read_pin);

  if(debug) Serial.print("Jumper test reading:");
  if(debug) Serial.println(reading);

  digitalWrite(control_pin, LOW);
  pinMode(control_pin, INPUT);

  int jumper_val = 486;

  if ((reading < (jumper_val * 1.03)) && (reading > (jumper_val * 0.97))) 
	{
		if (!debug) Serial.print("Jumper test reading:"); // check debug, to avoid double printing
		if (!debug) Serial.println(reading);
		return true; // jumper detected!!
	}
  else return false;
}



//Test power circuit 1 to see if there is a short on the target
//Returns true if all is ok
//Returns false if there is a short
boolean FlyingJalapeno::testRegulator1()
{
  return (powerTest(1));
}

boolean FlyingJalapeno::testRegulator2()
{
  return (powerTest(2));
}

//Returns true if there is a short
boolean FlyingJalapeno::isRegulator1Shorted()
{
  return (!powerTest(1));
}

boolean FlyingJalapeno::isRegulator2Shorted()
{
  return (!powerTest(2));
}

//Test target board for shorts to GND
//Returns true if all is good, returns false if there is short detected
boolean FlyingJalapeno::powerTest(byte select) // select is for either "1" or "2" for using either pretest resistors on the FJ
{
  //Power down regulators
  disableRegulator1();
  disableRegulator2();

  //Setup control pin
  pinMode(POWER_TEST_CONTROL, OUTPUT);
  digitalWrite(POWER_TEST_CONTROL, HIGH);

  byte read_pin;
  if (select == 1) read_pin = A14;
  else if (select == 2) read_pin = A15;
  else
  {
    Serial.println("Error: powerTest requires pin select.");
    return (false);
  }

  pinMode(read_pin, INPUT);

  delay(200); //Wait for voltage to settle before taking a ADC reading

  int reading = analogRead(read_pin);

  //Serial.print("Power test reading (should >500 or <471): ");
  //Serial.println(reading);

  //Release the control pin
  digitalWrite(POWER_TEST_CONTROL, LOW);
  pinMode(POWER_TEST_CONTROL, INPUT);

  int jumper_val = 486;

  if ((reading < (jumper_val * 1.03)) && (reading > (jumper_val * 0.97)))
    return false; // jumper detected!!
  else
    return true;
}

//Test a pin to see what voltage is on the pin.
//Returns true if pin voltage is within a given window of the value we are looking for
//pin = pin to test
//expectedVoltage = voltage we expect. 0.0 to 5.0 (float)
//allowedPpercent = allowed window for overage. 0 to 100 (int) (default 10%)
//debug = print debug statements (default false)
boolean FlyingJalapeno::verifyVoltage(int pin, float expectedVoltage, int allowedPercent, boolean debug)
{
  //float allowanceFraction = map(allowedPercent, 0, 100, 0, 1.0); //Scale int to a fraction of 1.0
  //Grrrr! map doesn't work with floats at all

  float allowanceFraction = allowedPercent / 100.0; //Scale the allowedPercent to a float

  int reading = analogRead(pin);

  //Convert reading to voltage
  float readVoltage = _FJ_VCC / 1024 * reading;

  if (debug)
  {
    Serial.print("allowanceFraction: ");
    Serial.println(allowanceFraction);

    Serial.print("Reading: ");
    Serial.println(reading);

    Serial.print("Voltage: ");
    Serial.println(readVoltage);
  }


  if ((readVoltage <= (expectedVoltage * (1.0 + allowanceFraction))) && (readVoltage >= (expectedVoltage * (1.0 - allowanceFraction))))
    return true; // good value
  else
    return false;
}

boolean FlyingJalapeno::verify_value(int input_value, int correct_val, float allowance_percent)
{
  if ((input_value <= (correct_val * (1 + allowance_percent))) && (input_value >= (correct_val * (1 - allowance_percent)))) return true; // good value
  else return false;
}

//Enable or disable regulator #1
void FlyingJalapeno::enableRegulator1(void)
{
  digitalWrite(PSU1_POWER_CONTROL, HIGH); // turn on the high side switch
}

void FlyingJalapeno::disableRegulator1(void)
{
  digitalWrite(PSU1_POWER_CONTROL, LOW); // turn off the high side switch
}

//Enable or disable regulator #2
void FlyingJalapeno::enableRegulator2(void)
{
  digitalWrite(PSU2_POWER_CONTROL, HIGH); // turn on the high side switch
}

void FlyingJalapeno::disableRegulator2(void)
{
  digitalWrite(PSU2_POWER_CONTROL, LOW); // turn off the high side switch
}

//Setup the first power supply to the chosen voltage level
//Leaves MOSFET off so regulator is configured but not connected to target
void FlyingJalapeno::setRegulatorVoltage1(float voltage)
{
  digitalWrite(PSU1_VOLTAGE_CONTROL_TO_3V3, LOW);
  digitalWrite(PSU1_VOLTAGE_CONTROL_TO_5V0, LOW);
  pinMode(PSU1_VOLTAGE_CONTROL_TO_3V3, INPUT);
  pinMode(PSU1_VOLTAGE_CONTROL_TO_5V0, INPUT);

  if (voltage == 3.3)
  {
    pinMode(PSU1_VOLTAGE_CONTROL_TO_3V3, OUTPUT);
    digitalWrite(PSU1_VOLTAGE_CONTROL_TO_3V3, LOW);
  }
  else if (voltage == 5)
  {
    pinMode(PSU1_VOLTAGE_CONTROL_TO_5V0, OUTPUT);
    digitalWrite(PSU1_VOLTAGE_CONTROL_TO_5V0, LOW);
  }
  else
  {
    pinMode(PSU1_VOLTAGE_CONTROL_TO_3V3, OUTPUT); // default to 3.3V - even when the high side switch is turn off.
    digitalWrite(PSU1_VOLTAGE_CONTROL_TO_3V3, LOW);
  }
}

//Setup the second power supply to the chosen voltage level
//Leaves MOSFET off so regulator is configured but not connected to target
void FlyingJalapeno::setRegulatorVoltage2(float voltage)
{
  digitalWrite(PSU2_VOLTAGE_CONTROL_TO_3V3, LOW);
  digitalWrite(PSU2_VOLTAGE_CONTROL_TO_3V7, LOW);
  digitalWrite(PSU2_VOLTAGE_CONTROL_TO_4V2, LOW);
  digitalWrite(PSU2_VOLTAGE_CONTROL_TO_5V0, LOW);
  pinMode(PSU2_VOLTAGE_CONTROL_TO_3V3, INPUT);
  pinMode(PSU2_VOLTAGE_CONTROL_TO_3V7, INPUT);
  pinMode(PSU2_VOLTAGE_CONTROL_TO_4V2, INPUT);
  pinMode(PSU2_VOLTAGE_CONTROL_TO_5V0, INPUT);

  if (voltage == 3.3)
  {
    pinMode(PSU2_VOLTAGE_CONTROL_TO_3V3, OUTPUT);
    digitalWrite(PSU2_VOLTAGE_CONTROL_TO_3V3, LOW);
  }
  else if (voltage == 3.7)
  {
    pinMode(PSU2_VOLTAGE_CONTROL_TO_3V7, OUTPUT);
    digitalWrite(PSU2_VOLTAGE_CONTROL_TO_3V7, LOW);
  }
  else if (voltage == 4.2)
  {
    pinMode(PSU2_VOLTAGE_CONTROL_TO_4V2, OUTPUT);
    digitalWrite(PSU2_VOLTAGE_CONTROL_TO_4V2, LOW);
  }
  else if (voltage == 5)
  {
    pinMode(PSU2_VOLTAGE_CONTROL_TO_5V0, OUTPUT);
    digitalWrite(PSU2_VOLTAGE_CONTROL_TO_5V0, LOW);
  }
  else
  {
    pinMode(PSU2_VOLTAGE_CONTROL_TO_3V3, OUTPUT); // default to 3.3V
    digitalWrite(PSU2_VOLTAGE_CONTROL_TO_3V3, LOW);
  }
}

//Enable or disable the I2C buffer
//If you're doing I2C you'll need to enable the buffer
void FlyingJalapeno::enablePCA(void)
{
  // PCA is enabled via PD4, which is not a standard arduino pin, so we will have to write this via register calls... hmmff!
  DDRD = DDRD | B00010000; // only set PD4 as output
  PORTD = PORTD | B00010000; // PD4 HIGH

  delay(100);
}

void FlyingJalapeno::disablePCA(void)
{
  // PCA is enabled via PD4, which is not a standard arduino pin, so we will have to write this via register calls... hmmff!
  PORTD = PORTD & ~(B00010000); // PD4 LOW - Disables the PCA

  delay(100);
}

//Depricated functions
//These are here to make old code still work. Use the newer functions: enablePCA(), disablePCA()
void FlyingJalapeno::PCA_enable(boolean enable)
{
  if (enable) enablePCA();
  else disablePCA();
}

//Replaced by setRegulatorVoltage1() and enableRegulator1()
void FlyingJalapeno::setV1(boolean power_on, float voltage)
{
  if (power_on) enableRegulator1();
  else disableRegulator1();

  setRegulatorVoltage1(voltage);
}

void FlyingJalapeno::setV2(boolean power_on, float voltage)
{
  if (power_on) enableRegulator2();
  else disableRegulator2();

  setRegulatorVoltage2(voltage);
}

//Power test. Replaced by powerTest()
boolean FlyingJalapeno::PT(byte select) // select is for either "1" or "2" for using either pretest resistors on the FJ
{
  return powerTest(select);
}

//Maintained for reverse compatibility. Use verifyVoltage() instead
boolean FlyingJalapeno::verify_voltage(int pin, int correct_val, float allowance_percent, boolean debug)
{
  int scaledPercent = allowance_percent * 100; //verifyVoltage expects an int. Scale 0.1 to 10.

  float scaledVoltage = 5.0 / 1024 * correct_val; //Scale voltage to a float.

  boolean result = verifyVoltage(pin, scaledVoltage, scaledPercent, debug);

  return (result);
}

boolean FlyingJalapeno::verify_i2c_device(byte address, boolean debug)
{
  byte error;

  if (debug)
  {
    Serial.println("");
    Serial.print("Pinging address 0x");
    if (address < 16) Serial.print("0");
    Serial.print(address, HEX);
  }

  Wire.beginTransmission(address);
  error = Wire.endTransmission();

  if (error == 0)
  {
    if (debug) Serial.println("...found.");
    return true;
  }
  else if (error == 4)
  {
    if (debug)
    {
      Serial.print("Unknow error at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
    return false;
  }
  else
  {
    if (debug)
    {
      Serial.print("...address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" NOT FOUND!\n");
    }
    return false;
  }
}
