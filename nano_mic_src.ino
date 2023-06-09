#include <I2S.h>

#define SENSITIVITY -26 // -26dBV/Pa ... 50mV/Pa
#define SPLref 94
#define SAMPLING_FREQ 8000
#define FAST 8

float samples[SAMPLING_FREQ/FAST];
int j = 0;
float SPL = 0;
int second = 0;
float values[FAST];
float final = 0;

void setup() {
  Serial.begin(9600);

  while (!Serial) { ; }
  if (!I2S.begin(I2S_PHILIPS_MODE, SAMPLING_FREQ, 32)) {
    Serial.println("Failed to initialize I2S!");
    while (1) ;
  }
}

void loop() {
  if (I2S.available()) {
    int sample = I2S.read();
    float amplitude = sample / 2147483647.0f; // Convert sample to a float value from the interval <-1.0;1.0>
    samples[j] = abs(amplitude);
    j++;

    if (j == SAMPLING_FREQ/FAST) {
      float sum = 0;
      for (int i = 0; i < SAMPLING_FREQ/FAST; i++) {
        sum += samples[i];
      }
      float average = sum / (SAMPLING_FREQ/FAST);
      float voltage = average * 3.3;
      float dBV = 20 * log10(voltage / 1);
      SPL = (dBV - SENSITIVITY) + SPLref;
      values[second] = SPL;
      second++;
      if (second == FAST) {
        float secondSum = 0;
        for (int k = 0; k < FAST; k++) {
          secondSum += values[k];
        }
        final = secondSum/FAST;
        // Serial.println(final);
        float temp = final*10;
        int tempInt = temp;
        int sentValue = map(tempInt, 400, 1200, 0, 255);
        analogWrite(A0, sentValue);
        // Serial.println(tempInt);
        second = 0;
      }
      j = 0;
    }
  }
}