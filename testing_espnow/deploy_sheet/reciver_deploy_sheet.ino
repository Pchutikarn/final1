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

unsigned long now;
unsigned long lastMeasure = 0;
// const unsigned long interval = 60 * 60 * 1000;
const unsigned long interval = 10000;

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
struct_message board1;
struct_message board2;
struct_message board3;
// struct_message board4;
// struct_message board5;
// struct_message board6;
// struct_message board7;
// struct_message board8;
// struct_message board9;

// Create an array with all the structures
// struct_message boardsStruct[9] = {board1, board2, board3, board4, board5, board6, board7, board8, board9};
// int boardAmount = 9;
struct_message boardsStruct[3] = {board1, board2, board3};
int boardAmount = 3;

int board_1_temp;
int board_1_humi;
int board_1_soilPer;

int board_2_temp;
int board_2_humi;
int board_2_soilPer;

int board_3_temp;
int board_3_humi;
int board_3_soilPer;

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

    // board555temp = boardsStruct[myData.id - 1].temp;
    // board555humi = boardsStruct[myData.id - 1].humi;
    // board555soilPer = boardsStruct[myData.id - 1].soilPer;
    // Serial.printf("board555temp : %d \n", board555temp);
    // Serial.printf("board555humi: %d \n", board555humi);
    // Serial.printf("board555soilPer: %d \n", board555soilPer);
    // Serial.println();

    Serial.printf("boardsStruct_1_temp : %d \n", boardsStruct[0].temp);
    Serial.printf("boardsStruct_1_humi: %d \n", boardsStruct[0].humi);
    Serial.printf("boardsStruct_1_soilPer: %d \n", boardsStruct[0].soilPer);

    Serial.printf("boardsStruct_2_temp : %d \n", boardsStruct[1].temp);
    Serial.printf("boardsStruct_2_humi: %d \n", boardsStruct[1].humi);
    Serial.printf("boardsStruct_2_soilPer: %d \n", boardsStruct[1].soilPer);

    Serial.printf("boardsStruct_3_temp : %d \n", boardsStruct[2].temp);
    Serial.printf("boardsStruct_3_humi: %d \n", boardsStruct[2].humi);
    Serial.printf("boardsStruct_3_soilPer: %d \n", boardsStruct[2].soilPer);

    board_1_temp = boardsStruct[0].temp;
    board_1_humi = boardsStruct[0].humi;
    board_1_soilPer = boardsStruct[0].soilPer;

    board_2_temp = boardsStruct[1].temp;
    board_2_humi = boardsStruct[1].humi;
    board_2_soilPer = boardsStruct[1].soilPer;

    board_3_temp = boardsStruct[2].temp;
    board_3_humi = boardsStruct[2].humi;
    board_3_soilPer = boardsStruct[2].soilPer;

    Serial.printf("board_1_temp : %d \n", board_1_temp);
    Serial.printf("board_1_humi: %d \n", board_1_humi);
    Serial.printf("board_1_soilPer: %d \n", board_1_soilPer);

    Serial.printf("board_2_temp : %d \n", board_2_temp);
    Serial.printf("board_2_humi: %d \n", board_2_humi);
    Serial.printf("board_2_soilPer: %d \n", board_2_soilPer);

    Serial.printf("board_3_temp : %d \n", board_3_temp);
    Serial.printf("board_3_humi: %d \n", board_3_humi);
    Serial.printf("board_3_soilPer: %d \n", board_3_soilPer);
}

void setup()
{

    Serial.begin(115200);
    WiFi.mode(WIFI_AP_STA);
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
    now = millis();

    // if (WiFi.status() != WL_CONNECTED)
    // {
    //     Serial.println("WiFi connection lost. Reconnecting...");
    //     reConnectToWiFi();
    // }

    if (now - lastMeasure > interval)
    {
        lastMeasure = now;

        // board_1_temp = boardsStruct[0].temp;
        // board_1_humi = boardsStruct[0].humi;
        // board_1_soilPer = boardsStruct[0].soilPer;

        // board_2_temp = boardsStruct[1].temp;
        // board_2_humi = boardsStruct[1].humi;
        // board_2_soilPer = boardsStruct[1].soilPer;

        // board_3_temp = boardsStruct[2].temp;
        // board_3_humi = boardsStruct[2].humi;
        // board_3_soilPer = boardsStruct[2].soilPer;
        sendDataToGoogle(boardsStruct[0].temp, boardsStruct[0].humi, boardsStruct[0].soilPer, boardsStruct[1].temp, boardsStruct[1].humi, boardsStruct[1].soilPer, boardsStruct[2].temp, boardsStruct[2].humi, boardsStruct[2].soilPer);
    }
}

void sendDataToGoogle(int value, int value2, int value3, int value4, int value5, int value6, int value7, int value8, int value9)
{
    Serial.println("==========");
    Serial.print("Connecting to ");
    Serial.println(host);

    if (!client.connect(host, httpsPort))
    {
        Serial.println("Connection failed");
        return;
    }

    int string_temp1 = value;
    int string_humi1 = value2;
    int string_soil1 = value3;
    int string_temp2 = value4;
    int string_humi2 = value5;
    int string_soil2 = value6;
    int string_temp3 = value7;
    int string_humi3 = value8;
    int string_soil3 = value9;
    String url = "/macros/s/" + GAS_ID + "/exec?temp1=" + string_temp1 + "&humi1=" + string_humi1 + "&soilPer1=" + string_soil1 + "&temp2=" + string_temp1 + "&humi2=" + string_humi2 + "&soilPer2=" + string_soil2 + "&temp3=" + string_temp3 + "&humi3=" + string_humi3 + "&soilPer3=" + string_soil3; //  3 variables
    Serial.print("requesting URL: ");
    Serial.println(url);

    // String url = "/macros/s/" + GAS_ID + "/exec?";

    // for (int i = 0; i < boardAmount; i++)
    // {
    //     url += "temp" + String(i + 1) + "=" + String(boardsData[i].temp) +
    //            "&humi" + String(i + 1) + "=" + String(boardsData[i].humi) +
    //            "&soilPer" + String(i + 1) + "=" + String(boardsData[i].soilPer);

    //     if (i < boardAmount)
    //     {
    //         url += "&";
    //     }
    // }

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