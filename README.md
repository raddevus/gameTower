I have a code sample that is from the following article: [Create an ESP32 BLE peripheral for iOS #1][1] 

**The Code**

A snippet of it looks like this:  (see all code at Github -> https://github.com/robkerr/BlogProjects/blob/main/BLECalculator/basic_ble_peripheral.ino)

    #include <BLEDevice.h>
    #include <BLEUtils.h>
    #include <BLEServer.h>
    
    #define PERIPHERAL_NAME                "Test Peripheral"
    #define SERVICE_UUID                "EBC0FCC1-2FC3-44B7-94A8-A08D0A0A5079"
    #define CHARACTERISTIC_INPUT_UUID   "C1AB2C55-7914-4140-B85B-879C5E252FE5"
    #define CHARACTERISTIC_OUTPUT_UUID  "643954A4-A6CC-455C-825C-499190CE7DB0"
    
    // Current value of output characteristic persisted here
    static uint8_t outputData[1];
    
    // Output characteristic is used to send the response back to the connected phone
    BLECharacteristic *pOutputChar;
    
    // Class defines methods called when a device connects and disconnects from the service
    class ServerCallbacks: public BLEServerCallbacks {
        void onConnect(BLEServer* pServer) {
            Serial.println("BLE Client Connected");

The script basically allows connection to BLE (bluetooth) and allows me to send data.  
It all works fine.

**The Device**

I have the following device (from Amazon description where I purchased):
ESP32 Development Board CP2102 Chip 38Pin Narrow Version WiFi + Bluetooth Microcontroller Dual Cores ESP-32 ESP-32D Board ESP-WROOM-32

It looks like this:

[![esp32 device][2]][2]

**ESP32 Board Choices**

[![esp32 board choices in Arduino IDE][3]][3]

**The Question**

I thought I would build this using the `ESP32 Dev Module` or `ESP32S2` or `ES32C3` but none of those worked.

I finally saw in the article that the author used the `Wrover` but I'm not sure why I need to choose that one.   

Can you explain why that one is correct versus the others?   
And how I would determine which one to use?




  [1]: https://robkerr.com/ble-peripheral-ios-development-part1/
  [2]: https://i.sstatic.net/A2W1Z9z8.png
  [3]: https://i.sstatic.net/V0Oob2gt.png
