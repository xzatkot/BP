#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BME280.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

Adafruit_BME280 bme;
Adafruit_SSD1306 display(128, 64, &Wire, -1);

const char* ssid = "zetak1_src";
const char* password = "deci58bel";
const char* host = "www.zetak1.com";
const char* path = "/data.php";

int second = 0;
int minute = 0;
int hour = 0;

float SPL = 0;
float temperature = 0;
float pressure = 0;
float humidity = 0;

WiFiClientSecure client;
DynamicJsonDocument doc(8192);
JsonArray data = doc.to<JsonArray>();

void setup() {
  Serial.begin(115200);
  Wire.begin();

  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  client.setInsecure();

  const int httpPort = 443;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
}

void loop() {
  int analogValue = analogRead(A0);
  float voltage = analogValue * (3.3 / 1023.0);
  float receivedDataTemp = mapFloat(voltage, 0.0, 3.3, 40.0, 120.0);
  float temperatureTemp = bme.readTemperature();
  float pressureTemp = bme.readPressure() / 100.0F;
  float humidityTemp = bme.readHumidity();

  // Serial.println(receivedDataTemp);
  // Serial.println(temperatureTemp);
  // Serial.println(pressureTemp);
  // Serial.println(humidityTemp);

  SPL += receivedDataTemp;
  temperature += temperatureTemp;
  pressure += pressureTemp;
  humidity += humidityTemp;
  second++;

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("LA: ");
  display.print(receivedDataTemp, 1);
  display.println(" dBA");
  display.print("Temp: ");
  display.print(temperatureTemp, 1);
  display.drawCircle(63, 9, 1, SSD1306_WHITE);
  display.println(" C");
  display.print("Baro: ");
  display.print(pressureTemp, 1);
  display.println(" hPa");
  display.print("Humid: ");
  display.print(humidityTemp, 1);
  display.println(" %");
  display.display();

  if (second == 15) { // Save every 15 seconds
    JsonObject reading = data.createNestedObject();
    reading["LA"] = round(SPL/60*10)/10;
    reading["Temperature"] = round(temperature/60*10)/10;
    reading["Pressure"] = round(pressure/60*10)/10;
    reading["Humidity"] = round(humidity/60*10)/10;
    second = 0;
    minute++;
    SPL = 0;
    temperature = 0;
    pressure = 0;
    humidity = 0;
  }

  if (minute == 60) { // Send every 15 minutes (save 4 times per minute => 4*15min = 60)
    sendJSONtoServer();
    doc.clear();
    data = doc.to<JsonArray>();
    minute = 0;
  }

  delay(1000);
}

void sendJSONtoServer() {
  if ((WiFi.status() == WL_CONNECTED)) { // Check the current connection status
    HTTPClient http;
    
    String url = String("https://") + host + "/data.php";
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json"); // Specify content-type header

    String jsonString;
    serializeJson(doc, jsonString); //Convert JSON document to string
    Serial.println(jsonString);
    int httpResponseCode = http.POST(jsonString); // Send the POST request

    if (httpResponseCode>0) {
      String response = http.getString(); // Get the response
      Serial.println(httpResponseCode);
      Serial.println(response);
    }
    else {
      Serial.print("Error on sending POST: ");
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  }
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}