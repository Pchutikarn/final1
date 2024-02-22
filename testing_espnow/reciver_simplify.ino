#include <ESP8266WiFi.h>
#include <espnow.h>

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
  // Acess the variables for each board
  /*int board1X = boardsStruct[0].x;
  int board1Y = boardsStruct[0].y;
  int board2X = boardsStruct[1].x;
  int board2Y = boardsStruct[1].y;
  int board3X = boardsStruct[2].x;
  int board3Y = boardsStruct[2].y;*/

  // delay(10000);
}