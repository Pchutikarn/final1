// Master (แม่ค่าย , ตัวรับ)
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <espnow.h>

const char *ssid = "AMM-Guest";
const char *password = "FM@AMM2019";

const char *host = "script.google.com";
const int httpsPort = 443;
String GAS_ID = "";

WiFiClientSecure client;

unsigned long now = millis();
unsigned long lastMeasure = 0;
const unsigned long interval = 60 * 60 * 1000;

typedef struct struct_message
{
  int id;
  float temp;
  float humi;
  float soilPer;
} struct_message;

struct_message myData;

struct_message boardsStruct[10]; // Array to store data from 10 slaves

void OnDataRecv(uint8_t *mac_addr, uint8_t *incomingData, uint8_t len)
{
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);

  memcpy(&myData, incomingData, sizeof(myData));

  if (myData.id >= 1 && myData.id <= 10)
  {
    boardsStruct[myData.id - 1].temp = myData.temp;
    boardsStruct[myData.id - 1].humi = myData.humi;
    boardsStruct[myData.id - 1].soilPer = myData.soilPer;

    Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
    Serial.printf("temp: %f \n", boardsStruct[myData.id - 1].temp);
    Serial.printf("humi: %f \n", boardsStruct[myData.id - 1].humi);
    Serial.printf("soil per: %f \n", boardsStruct[myData.id - 1].soilPer);
    Serial.println();
  }
  else
  {
    Serial.println("Invalid Board ID");
  }
}

void reConnectToWiFi()
{
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Re-Connecting");
  int attemptCount = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(250);
    if (++attemptCount > 30)
    {
      Serial.println("WiFi connection failed. Please check credentials or router. (In Re Connect To WiFi)");
      return;
    }
  }
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  client.setInsecure();
}

void sendDataToGoogle(struct_message boardsData[])
{
  Serial.println("==========");
  Serial.print("Connecting to ");
  Serial.println(host);

  if (!client.connect(host, httpsPort))
  {
    Serial.println("Connection failed");
    return;
  }

  String url = "/macros/s/" + GAS_ID + "/exec?";

  for (int i = 0; i < 10; i++)
  {
    url += "temp" + String(i + 1) + "=" + String(boardsData[i].temp) +
           "&humi" + String(i + 1) + "=" + String(boardsData[i].humi) +
           "&soilPer" + String(i + 1) + "=" + String(boardsData[i].soilPer);

    if (i < 9)
    {
      url += "&";
    }
  }

  Serial.print("Requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("Request sent");

  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r")
    {
      Serial.println("Headers received");
      break;
    }
  }

  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\""))
  {
    Serial.println("Success: Data sent to Google Sheet!");
  }
  else
  {
    Serial.println("Failed to send data to Google Sheet");
  }

  Serial.print("Reply was: ");
  Serial.println(line);
  Serial.println("Closing connection");
  Serial.println("==========");
  Serial.println();
}

void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(250);
  }

  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  client.setInsecure();

  if (esp_now_init() != 0)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop()
{
  now = millis();

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi connection lost. Reconnecting...");
    reConnectToWiFi();
  }

  if (now - lastMeasure > interval)
  {
    lastMeasure = now;

    sendDataToGoogle(boardsStruct);
  }
}
