// Slave (ตัวส่ง , ทาส)

#include <espnow.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

const int BOARD_ID = 4;
uint8_t broadcastAddress[] = {0x80, 0x7D, 0x3A, 0x37, 0x72, 0x2B};
typedef struct struct_message
{
  int id;
  float temp;
  float humi;
  float soilPer;
} struct_message;
struct_message myData;

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;

#define ON_Board_LED 2

int valSoil;
unsigned status;
long now = millis();
long lastMeasure = 0;
// #define DEEP_SLEEP_DURATION_MS (1e6 * 60 * 60)
#define DEEP_SLEEP_DURATION_MS (1e6 * 59 * 60 + 57 * 1e3)


void DeepSleep()
{
  Serial.println(F("I'm awake, but I'm going into deep sleep mode for a hour"));
  Serial.flush();
  ESP.deepSleep(DEEP_SLEEP_DURATION_MS);
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
  Serial.print("\r\nLast Packet Send Status: ");
  if (sendStatus == 0)
  {
    Serial.println("Delivery success");
    DeepSleep();
  }
  else
  {
    Serial.println("Delivery fail");
  }
}

void setup()
{
  Serial.begin(115200);

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

  pinMode(ON_Board_LED, OUTPUT);
  digitalWrite(ON_Board_LED, HIGH);
}

void loop()
{
  now = millis();

  if (now - lastMeasure > 3000)
  {
    lastMeasure = now;
    myData.id = BOARD_ID;
    myData.temp = bme.readTemperature();
    myData.humi = bme.readHumidity();

    Serial.print("Temperature = ");
    Serial.print(myData.temp);
    Serial.println(" °C");

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

    esp_now_send(0, (uint8_t *)&myData, sizeof(myData));
  }
}