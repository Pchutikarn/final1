#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

uint8_t broadcastAddress[] = {0x80, 0x7D, 0x3A, 0x37, 0x72, 0x2B};

const int BOARD_ID = 3;
typedef struct struct_message
{
  int id;
  int temp;
  int humi;
  int soilPer;
} struct_message;
struct_message myData;

// Create peer interface
// esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(sendStatus == 0 ? "Delivery Success" : "Delivery Fail");
}

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;
int valSoil;
unsigned status;
unsigned long now;
unsigned long lastMeasure;
int interval = 10000;

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);

  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  status = bme.begin(0x76);
  if (!status)
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    Serial.print("SensorID was: 0x");
    Serial.println(bme.sensorID(), 16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1)
      delay(10);
  }
  Serial.println("-- Default Test --");
}

void loop()
{
  now = millis();
  if (now - lastMeasure > interval)
  {
    lastMeasure = now;

    myData.id = BOARD_ID;
    myData.temp = bme.readTemperature();
    Serial.print("Temperature = ");
    Serial.print(myData.temp);
    Serial.println(" °C");

    myData.humi = bme.readHumidity();
    Serial.print("Humidity = ");
    Serial.print(myData.humi);
    Serial.println(" %");

    valSoil = analogRead(A0);
    myData.soilPer = map(valSoil, 0, 770, 100, 0);
    Serial.print("valSoil: ");
    Serial.println(valSoil);

    Serial.print("soilPer: ");
    Serial.print(myData.soilPer);
    Serial.println(" %");

    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  }
}