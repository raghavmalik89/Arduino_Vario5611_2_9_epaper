// All code by Rolf R Bakke, Oct 2012

#include <Wire.h>

unsigned int calibrationData[7];
unsigned long time = 0;
float seaLevelPressure = 1012; // atmospheric pressure at sea level

float toneFreq, toneFreqLowpass, pressure, lowpassFast, lowpassSlow ;

int ddsAcc;

const int n = 10;  // number of readings to average
int pressureReadings[n];  // array to store the readings
int index = 0;  // current index in the array

void setup()
{
Wire.begin();
Serial.begin(115200);
setupSensor();

pressure = getPressure();
lowpassFast = lowpassSlow = pressure;
}
void loop()
{
  unsigned long startTime = millis();
pressure = getPressure();
//Serial.println( pressure );
lowpassFast = lowpassFast + (pressure - lowpassFast) * 0.1;
lowpassSlow = lowpassSlow + (pressure - lowpassSlow) * 0.05;
toneFreq = (lowpassSlow - lowpassFast) * 50;
float altitude = 44330 * (1.0 - pow((lowpassSlow/100) / seaLevelPressure, 0.1903));
//Serial.println( altitude );
toneFreqLowpass = toneFreqLowpass + (toneFreq - toneFreqLowpass) * 0.1;
//Serial.println( toneFreqLowpass );
if (toneFreqLowpass > 20){
//Serial.println( toneFreqLowpass );
toneFreq = constrain(toneFreqLowpass, -500, 500);
ddsAcc += toneFreq * 100 + 2000;
if (toneFreq < 0.2 || ddsAcc > -0.2)
{
tone(2, toneFreq + 510);
}
else
{
noTone(2);
}
}
else
{
noTone(2);
}
unsigned long endTime = millis();
unsigned long executionTime = endTime - startTime;
//Serial.print("Execution time of loop(): ");
Serial.print(executionTime);
Serial.println("\n");
//Serial.println(" ms");

while (millis() < time); //loop frequency timer
time += 20;

}

float ToneFrequencyGenerator(int pressure){
lowpassFast = lowpassFast + (pressure - lowpassFast) * 0.1;
lowpassSlow = lowpassSlow + (pressure - lowpassSlow) * 0.05;
toneFreq = (lowpassSlow - lowpassFast) * 50;
Serial.println( (pressure/100) +10 );
Serial.println( lowpassSlow/100 );
float altitude = 44330 * (1.0 - pow((lowpassSlow/100) / seaLevelPressure, 0.1903));
Serial.println( altitude );

toneFreqLowpass = toneFreqLowpass + (toneFreq - toneFreqLowpass) * 0.1;

return toneFreq = constrain(toneFreqLowpass, -500, 500);
}

int filterPressure(int pressure) {
  pressureReadings[index] = pressure;
  index = (index + 1) % n;
  int sum = 0;
  for (int i = 0; i < n; i++) {
    sum += pressureReadings[i];
  }
  int average = sum / n;
  return average;
}

long getPressure()
{
long D1, D2, dT, P;
float TEMP;
int64_t OFF, SENS;

D1 = getData(0x48, 10);
D2 = getData(0x50, 1);

dT = D2 - ((long)calibrationData[5] << 8);
TEMP = (2000 + (((int64_t)dT * (int64_t)calibrationData[6]) >> 23)) / (float)100;
OFF = ((unsigned long)calibrationData[2] << 16) + (((int64_t)calibrationData[4] * dT) >> 7);
SENS = ((unsigned long)calibrationData[1] << 15) + (((int64_t)calibrationData[3] * dT) >> 8);
P = (((D1 * SENS) >> 21) - OFF) >> 15;

//Serial.println(TEMP);
//Serial.println(P);

return P;
}

long getData(byte command, byte del)
{
long result = 0;
twiSendCommand(0x77, command);
delay(del);
twiSendCommand(0x77, 0x00);
Wire.requestFrom(0x77, 3);
if(Wire.available()!=3) Serial.println("Error: raw data not available");
for (int i = 0; i <= 2; i++)
{
result = (result<<8) | Wire.read();
}
return result;
}

void setupSensor()
{
twiSendCommand(0x77, 0x1e);
delay(100);

for (byte i = 1; i <=6; i++)
{
unsigned int low, high;

twiSendCommand(0x77, 0xa0 + i * 2);
Wire.requestFrom(0x77, 2);
if(Wire.available()!=2) Serial.println("Error: calibration data not available");
high = Wire.read();
low = Wire.read();
calibrationData[i] = high<<8 | low;
Serial.print("calibration data #");
Serial.print(i);
Serial.print(" = ");
Serial.println( calibrationData[i] );
}
}

void twiSendCommand(byte address, byte command)
{
Wire.beginTransmission(address);
if (!Wire.write(command)) Serial.println("Error: write()");
if (Wire.endTransmission())
{
Serial.print("Error when sending command: ");
Serial.println(command, HEX);
}
}

