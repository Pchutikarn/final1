#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <espnow.h>

const char *ssid = "AMM-Guest";
const char *password = "FM@AMM2019";
const char *host = "script.google.com";
String GAS_ID = "AKfycbz7L1jERCCdj0Lp2RwE9s7PpqHlKWbbuaP4PjMoI0mMf7QQgyAzpmXX0hy2ZU8w7n6F"; //--> spreadsheet script ID
const int httpsPort = 443;
WiFiClientSecure client;
WiFiClient espClient;

unsigned long now ;
unsigned long lastMeasure = 0;
// const unsigned long interval = 60 * 60 * 1000;
const unsigned long interval = 3000;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message
{
    int id;
    int temp;
    int humi;
    int soilPer;
} struct_message;
struct_message myData;

// Create a structure to hold the readings from each board
struct_message board555;
// struct_message board2;
// struct_message board3;

// Create an array with all the structures
// struct_message boardsStruct[3] = {board1, board2, board3};
struct_message boardsStruct[1] = {board555};
int boardAmount = 1;

// callback function that will be executed when data is received
void OnDataRecv(uint8_t *mac_addr, uint8_t *incomingData, uint8_t len)
{
    char macStr[18];
    Serial.print("Packet received from: ");
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.println(macStr);
    memcpy(&myData, incomingData, sizeof(myData));
    Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
    // Update the structures with the new incoming data
    boardsStruct[myData.id - 1].temp = myData.temp;
    boardsStruct[myData.id - 1].humi = myData.humi;
    boardsStruct[myData.id - 1].soilPer = myData.soilPer;
    Serial.printf("Temperature : %d \n", boardsStruct[myData.id - 1].temp);
    Serial.printf("Humidity: %d \n", boardsStruct[myData.id - 1].humi);
    Serial.printf("soilPer: %d \n", boardsStruct[myData.id - 1].soilPer);
    Serial.println();

    WiFi.begin(ssid, password); //--> Connect to your WiFi router
    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Successfully connected to : ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    client.setInsecure();
    
    sendDataToGoogle(boardsStruct);
}

void setup()
{

    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    

    if (esp_now_init() != 0)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Once ESPNow is successfully Init, we will register for recv CB to
    // get recv packer info
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(OnDataRecv);
}

void loop()
{

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

    for (int i = 0; i < boardAmount; i++)
    {
        url += "temp" + String(i + 1) + "=" + String(boardsData[i].temp) +
               "&humi" + String(i + 1) + "=" + String(boardsData[i].humi) +
               "&soilPer" + String(i + 1) + "=" + String(boardsData[i].soilPer);

        if (i < boardAmount)
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