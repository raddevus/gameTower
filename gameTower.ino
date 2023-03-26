#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <string>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define PERIPHERAL_NAME                "GameTower"
#define SERVICE_UUID                "EBC0FCC1-2FC3-44B7-94A8-A08D0A0A5079"
#define CHARACTERISTIC_INPUT_UUID   "C1AB2C55-7914-4140-B85B-879C5E252FE5"
#define CHARACTERISTIC_OUTPUT_UUID  "643954A4-A6CC-455C-825C-499190CE7DB0"
#define CHARACTERISTIC_INPUT_STRING_UUID   "622B2C55-7914-4140-B85B-879C5E252DA0"

#define CHAR_NUMBER_OF_PLAYERS_OUT "02c9cc15-5e88-4d0d-9f69-540675058dc1"
#define CHAR_NUMBER_OF_PLAYERS "02c9cc15-5e88-4d0d-9f69-54067506f63a"
#define CHAR_GAME_IS_STARTED_OUT "39f37f0a-04f4-49e5-a20f-cce75876f63a"
#define CHAR_GAME_IS_STARTED "39f37f0a-04f4-49e5-a20f-cce75872c290"
#define CHAR_GAME_WINNER_ID_OUT "f2194320-89f2-46b0-8694-4d471212c290"
#define CHAR_REGISTER_PLAYER "61462f3d-c01a-4ea0-b46f-7ee98362e1e4"

std::string currentOutput = "Started...";

// Current value of output characteristic persisted here
static uint8_t outputData[1];

// Output characteristic is used to send the response back to the connected phoneX
BLECharacteristic *pOutputChar;
BLECharacteristic *pNumberOfPlayers;
BLECharacteristic *pGameIsStarted;
BLECharacteristic *pGameWinnerId;

int playerCounterId = 0;
//std::string[] allPlayers = std::string[10];
std::vector<std::string> allPlayers;  

// Class defines methods called when a device connects and disconnects from the service
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        Serial.println("BLE Client Connected");
        currentOutput = "BLE Client Connected";
    }
    void onDisconnect(BLEServer* pServer) {
        BLEDevice::startAdvertising();
        Serial.println("BLE Client Disconnected");
        currentOutput = "BLE Client Disconnected";
    }
};

class InputReceivedCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharWriteState) {
        uint8_t *inputValues = pCharWriteState->getData();
          
        switch(inputValues[2]) {
          case 0x00: // add
            Serial.printf("Adding:   %02x %02x\r\n", inputValues[0], inputValues[1]);  
            outputData[0] = inputValues[0] + inputValues[1];  
            break;
          case 0x01: // subtract
            Serial.printf("Subtracting:   %02x %02x\r\n", inputValues[0], inputValues[1]);  
            outputData[0] = inputValues[0] - inputValues[1];  
            break;
          default: // multiply
            Serial.printf("Multiplying:   %02x %02x\r\n", inputValues[0], inputValues[1]);  
            outputData[0] = inputValues[0] * inputValues[1];  
        }
        
        Serial.printf("Sending response:   %02x\r\n", outputData[0]);  
        
        pOutputChar->setValue((uint8_t *)outputData, 1);
        pOutputChar->notify();
    }
};

class StringReceivedCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharWriteState) {
    
        std::string inputString = pCharWriteState->getValue();
        size_t strLen = pCharWriteState->getLength();
        //char  output_msg[1024];
        //snprintf(output_msg, sizeof output_msg, "Got %u chars: %s\n", strLen, inputString.c_str());
        std::string output_msg = "Got " + std::to_string(strLen) + " chars: " + inputString + "\n";
        currentOutput = output_msg;
        Serial.printf("%s",output_msg.c_str());

        pOutputChar->setValue(output_msg);
       // pOutputChar->setValue((uint8_t *)outputData, 1);
       pOutputChar->notify();
    }
};

class RegisterPlayerCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharWriteState) {
        std::string userName = pCharWriteState->getValue();
        allPlayers.push_back(userName);
        
        size_t strLen = pCharWriteState->getLength();
        //char  output_msg[1024];
        //snprintf(output_msg, sizeof output_msg, "Got %u chars: %s\n", strLen, inputString.c_str());
        std::string output_msg = "Got " + std::to_string(strLen) + " userName: " + userName + "\n";
        currentOutput = output_msg;
        Serial.printf("%s",output_msg.c_str());

        pNumberOfPlayers->setValue(output_msg);
       // pOutputChar->setValue((uint8_t *)outputData, 1);
       pNumberOfPlayers->notify();
    }

    void onRead(BLECharacteristic *pCharReadState) {
        std::string output_msg = "There are " + std::to_string(allPlayers.size()) + " registered players.";
        currentOutput = output_msg;
        Serial.printf("%s", output_msg.c_str());
    }
};

void setup() {

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
  }
  
  // Use the Arduino serial monitor set to this baud rate to view BLE peripheral logs 
  Serial.begin(115200);
  Serial.println("Begin Setup BLE Service and Characteristics");

  // Configure thes server

  BLEDevice::init(PERIPHERAL_NAME);
  BLEServer *pServer = BLEDevice::createServer();

  // Create the service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a characteristic for the service
  BLECharacteristic *pInputChar = pService->createCharacteristic(
                              CHARACTERISTIC_INPUT_UUID,                                        
                              BLECharacteristic::PROPERTY_WRITE_NR | BLECharacteristic::PROPERTY_WRITE);
  // #####################################################
  // ############ OUTPUT CHARACTERISTICS   ###############
  // #####################################################
  pOutputChar = pService->createCharacteristic(
                              CHARACTERISTIC_OUTPUT_UUID,
                              BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pNumberOfPlayers = pService->createCharacteristic(
                              CHAR_NUMBER_OF_PLAYERS_OUT,
                              BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pGameIsStarted = pService->createCharacteristic(
                              CHAR_GAME_IS_STARTED_OUT,
                              BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pGameWinnerId = pService->createCharacteristic(
                              CHAR_GAME_WINNER_ID_OUT,
                              BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

  // #####################################################
  // ############ INPUT CHARACTERISTICS   ###############
  // #####################################################
BLECharacteristic *pInputString = pService->createCharacteristic(
                              CHARACTERISTIC_INPUT_STRING_UUID,                                        
                              BLECharacteristic::PROPERTY_WRITE_NR | BLECharacteristic::PROPERTY_WRITE);

BLECharacteristic *pRegisterPlayer = pService->createCharacteristic(
                              CHAR_REGISTER_PLAYER,                                        
                              BLECharacteristic::PROPERTY_WRITE_NR | BLECharacteristic::PROPERTY_WRITE 
                              | BLECharacteristic::PROPERTY_READ);

// BLECharacteristic *pInputString = pService->createCharacteristic(
//                               CHARACTERISTIC_INPUT_STRING_UUID,                                        
//                               BLECharacteristic::PROPERTY_WRITE_NR | BLECharacteristic::PROPERTY_WRITE);
                              
//  BLECharacteristic *pInputString = pService->createCharacteristic(
//                               CHARACTERISTIC_INPUT_STRING_UUID,                                        
//                               BLECharacteristic::PROPERTY_WRITE_NR | BLECharacteristic::PROPERTY_WRITE);
                              
  // Hook callback to report server events
  pServer->setCallbacks(new ServerCallbacks());
  pInputChar->setCallbacks(new InputReceivedCallbacks());
  pInputString->setCallbacks(new StringReceivedCallbacks());
  pRegisterPlayer->setCallbacks(new RegisterPlayerCallbacks());

  // Initial characteristic value
  outputData[0] = 0x00;
  pOutputChar->setValue((uint8_t *)outputData, 1);

  // Start the service
  pService->start();

  // Advertise the service
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE Service is advertising");
}

void loop() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(currentOutput.c_str());
  display.display();
  // put your main code here, to run repeatedly:
  delay(20);
}