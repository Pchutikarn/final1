// ตัวรับ

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#ifndef STASSID
#define STASSID "AMM-Guest"
#define STAPSK "FM@AMM2019"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

int relayPin = D0;

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    String topicString = String(topic);
    if (topicString == "tao6969/turnOn")
    {
        Serial.println("ON");
        digitalWrite(relayPin, HIGH);
        
    }
    else if (topicString == "tao6969/turnOff")
    {
        Serial.println("OFF");
        digitalWrite(relayPin, LOW);
    }
}

void setup()
{
    // put your setup code here, to run once
    Serial.begin(9600);
    pinMode(relayPin, OUTPUT);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    client.setServer("broker.emqx.io", 1883);
    client.setCallback(callback);
    Serial.print("Connected");
}

void loop()
{
    if (!client.connected())
    {
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str()))
        {
            Serial.println("connected");
            client.subscribe("tao6969/#");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
    client.loop();
}