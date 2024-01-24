//Slave (ตัวส่ง , ทาส)

#include <ESP8266WiFi.h>
#include <espnow.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x80, 0x7D, 0x3A, 0x37, 0x72, 0x2B};

// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Sender #2 = BOARD_ID 2, etc)
#define BOARD_ID 1

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  int id;
  float temp;
  float humi;
  float soilPer;
} struct_message;

// Create a struct_message called test to store variables to be sent
struct_message myData;

unsigned long lastTime = 0;
unsigned long timerDelay = 10000;





#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C
// Adafruit_BME280 bme(BME_CS); // hardware SPI
// Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

#define ON_Board_LED 2

// float temp;
// float humi;
// int soilPer;
int valSoil;
unsigned status;



// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("\r\nLast Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
     Serial.println("I'm awake, but I'm going into deep sleep mode for 15 seconds");
    ESP.deepSleep(15e6);
  }
  else {
    Serial.println("Delivery fail");
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Set ESP-NOW role
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  // Once ESPNow is successfully init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);





  status = bme.begin(0x76);
  // You can also pass in a Wire library object like &Wire2
  // status = bme.begin(0x76, &Wire2)
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

long now = millis();
long lastMeasure = 0;

void loop() {
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






    // Send message via ESP-NOW
    esp_now_send(0, (uint8_t *) &myData, sizeof(myData));
   

  }



}