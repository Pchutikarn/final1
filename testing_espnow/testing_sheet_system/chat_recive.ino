// Include necessary libraries
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <espnow.h>

const char *ssid = "AMM-Guest";
const char *password = "FM@AMM2019";
const char *host = "script.google.com";
String GAS_ID = "AKfycbz7L1jERCCdj0Lp2RwE9s7PpqHlKWbbuaP4PjMoI0mMf7QQgyAzpmXX0hy2ZU8w7n6F"; //--> spreadsheet script ID
const int httpsPort = 443;
WiFiClientSecure client;
unsigned long TIMEOUT_DURATION = 5000;

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
int packetsReceived = 0;

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
    packetsReceived++;
}




void setup() {
    Serial.begin(115200);
}

void loop() {
    connectESPNow();

    unsigned long startTime = millis(); 
    while (packetsReceived < boardAmount && millis() - startTime < TIMEOUT_DURATION) {
        delay(100); 
    }

    if (packetsReceived == boardAmount) {
        connectWiFi();
        sendDataToGoogle(boardsStruct);
    } else {
        Serial.println("Timeout occurred. Did not receive data from all sender devices.");
    }
    packetsReceived = 0;
}


// Function to send data to Google Sheets
void sendDataToGoogle(struct_message boardsData[]) {
    // Connect to Google Sheets
    if (!client.connect(host, httpsPort)) {
        Serial.println("Connection to Google Sheets failed");
        return;
    }

    // Construct URL for Google Sheets API
    String url = "/macros/s/" + GAS_ID + "/exec?";
    for (int i = 0; i < boardAmount; i++) {
        url += "temp" + String(i + 1) + "=" + String(boardsData[i].temp) +
               "&humi" + String(i + 1) + "=" + String(boardsData[i].humi) +
               "&soilPer" + String(i + 1) + "=" + String(boardsData[i].soilPer);
        if (i < boardAmount - 1) {
            url += "&";
        }
    }

    // Send HTTP GET request to Google Sheets API
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "User-Agent: ESP8266\r\n" +
                 "Connection: close\r\n\r\n");

    // Wait for response from server
    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            break; // Headers received
        }
    }

    // Read and print response from server
    String line = client.readStringUntil('\n');
    if (line.startsWith("{\"state\":\"success\"")) {
        Serial.println("Data sent to Google Sheets successfully");
    } else {
        Serial.println("Failed to send data to Google Sheets");
    }

    // Close connection
    client.stop();
}


// Function to connect to Wi-Fi
void connectWiFi() {
    esp_now_deinit(); // Disconnect from ESP-NOW
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
}

// Function to connect to ESP-NOW
void connectESPNow() {
    // WiFi.mode(WIFI_STA);
    // WiFi.disconnect(true); // Disconnect from Wi-Fi
    if (esp_now_init() != 0) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(OnDataRecv);
}
