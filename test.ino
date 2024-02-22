#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C
// Adafruit_BME280 bme(BME_CS); // hardware SPI
// Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

#define ON_Board_LED 2

const char *ssid = "M";            //--> Nama Wifi / SSID.
const char *password = "22222222"; //-->  Password wifi .

// const char *ssid = "AMM-Guest";      //--> Nama Wifi / SSID.
// const char *password = "FM@AMM2019"; //-->  Password wifi .

const char *host = "script.google.com";
const int httpsPort = 443;

WiFiClientSecure client;
WiFiClient espClient;
PubSubClient clientMqtt(espClient);

long now = millis();
long lastMeasure = 0;

float temp;
float humi;
int soilPer;
int valSoil;
unsigned status;

String GAS_ID = "AKfycbzbO7hzUBbNWh9Qd0o390tjFCqb0W8SdLXku0YbIaWm_jXvWvpKj2Adprt8XTHJX2WR";

void setup()
{

    Serial.begin(115200);
    delay(500);
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

    WiFi.begin(ssid, password);
    Serial.println("");

    pinMode(ON_Board_LED, OUTPUT);
    digitalWrite(ON_Board_LED, HIGH);

    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        digitalWrite(ON_Board_LED, LOW);
        delay(250);
        digitalWrite(ON_Board_LED, HIGH);
        delay(250);
    }
    digitalWrite(ON_Board_LED, HIGH);
    Serial.println("");
    Serial.print("Successfully connected to : ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    client.setInsecure();

    clientMqtt.setServer("broker.emqx.io", 1883);
    clientMqtt.setCallback(callback);
    Serial.print("Connected");
}

void loop()
{
    if (!clientMqtt.connected())
    {
        String clientMqttId = "ESP8266Client-";
        clientMqttId += String(random(0xffff), HEX);

        if (clientMqtt.connect(clientMqttId.c_str()))
        {
            Serial.println("connected");
            clientMqtt.subscribe("tao6969/#");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(clientMqtt.state());
            Serial.println(" try again in 5 seconds");

            delay(5000);
        }
    }
    clientMqtt.loop();

    now = millis();

    if (now - lastMeasure > 3000)
    {
        lastMeasure = now;

        temp = bme.readTemperature();
        humi = bme.readHumidity();

                   Serial.print("Temperature = ");
        Serial.print(temp);
        Serial.println(" °C");

        Serial.print("Humidity = ");
        Serial.print(humi);
        Serial.println(" %");

        valSoil = analogRead(A0);
        soilPer = map(valSoil, 0, 770, 100, 0);
        Serial.print("valSoil: ");
        Serial.println(valSoil);

        Serial.print("soilPer: ");
        Serial.print(soilPer);
        Serial.println(" %");

        sendData(temp, humi, soilPer);
    }

    Serial.println("I'm awake, but I'm going into deep sleep mode for 15 seconds");
    ESP.deepSleep(15e6);
}

void sendData(float value, float value2, float value3)
{
    Serial.println("==========");
    Serial.print("connecting to ");
    Serial.println(host);

    if (!client.connect(host, httpsPort))
    {
        Serial.println("connection failed");
        return;
    }

    float string_temp = value;
    float string_humi = value2;
    float string_soil = value3;
    String url = "/macros/s/" + GAS_ID + "/exec?temp=" + string_temp + "&humi=" + string_humi + "&soil=" + string_soil; //  3 variables
    Serial.print("requesting URL: ");
    Serial.println(url);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "User-Agent: BuildFailureDetectorESP8266\r\n" +
                 "Connection: close\r\n\r\n");

    Serial.println("request sent");

    while (client.connected())
    {
        String line = client.readStringUntil('\n');
        if (line == "\r")
        {
            Serial.println("headers received");
            break;
        }
    }
    String line = client.readStringUntil('\n');
    if (line.startsWith("{\"state\":\"success\""))
    {
        Serial.println("esp8266/Arduino CI successfull!");
    }
    else
    {
        Serial.println("esp8266/Arduino CI has failed");
    }
    Serial.print("reply was : ");
    Serial.println(line);
    Serial.println("closing connection");
    Serial.println("==========");
    Serial.println();
}

void callback(char *topic, byte *payload, unsigned int length)
{
}

