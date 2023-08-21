/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

BLEServer* bleServer = NULL;
BLECharacteristic* bleChar1 = NULL;
BLECharacteristic* bleChar2 = NULL;
BLEDescriptor* descr1;
BLEDescriptor* descr2;
BLE2902* charConfig1;

bool deviceConnected = false;
bool oldDeviceConnected = false;
String valueString = "Not Passed";

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define uuidService "9a424344-7de4-46f3-9486-9ccb676d9f05"
#define uuidChar1 "eb05064c-da33-4a23-9f30-71b5cd9ac27b"
#define uuidChar2 "7b10be22-026c-46c4-8569-2a769e8a9ad1"

class BleServerCallbacks: public BLEServerCallbacks 
{
    void onConnect(BLEServer* bleServer)
    {
        deviceConnected = true;
    }

    void onDisconnect(BLEServer* bleServer)
    {
        deviceConnected = false;
    }
};

class CharacteristicsCallBack: public BLECharacteristicCallbacks 
{
    void onWrite (BLECharacteristic* dataChar) override 
    {
        std::string dataChar2ValueStdStr = dataChar -> getValue();
        String dataChar2ValueString = String(dataChar2ValueStdStr.c_str());
        Serial.println("dataChar2: " + dataChar2ValueString);
        
        // int dataChar2ValueInt = dataChar2ValueString.toInt(); // sent data is NOT an int, so don't convert it to one
        valueString = dataChar2ValueString;
        // value = dataChar2ValueInt;

        bleChar1 -> setValue(dataChar2ValueStdStr);
        bleChar1 -> notify();
    }
};

void setup() 
{
    // PIN OUT LED Setup
    pinMode(1, OUTPUT);

    // BLE Communication Initialization
    Serial.begin(115200);

    // BLE Device initialization
    BLEDevice::init("ESP32");

    // Serial.println("Starting BLE work!");

    // BLE Server creation
    bleServer = BLEDevice::createServer();
    bleServer -> setCallbacks(new BleServerCallbacks());

    // BLE Service creation
    BLEService* service = bleServer->createService(uuidService);

    // BLE Characteristic creation
    bleChar1 = service->createCharacteristic(uuidChar1, 
                                             BLECharacteristic::PROPERTY_READ   | 
                                             BLECharacteristic::PROPERTY_WRITE  | 
                                             BLECharacteristic::PROPERTY_NOTIFY);

    bleChar2 = service->createCharacteristic(uuidChar2, 
                                             BLECharacteristic::PROPERTY_READ   | 
                                             BLECharacteristic::PROPERTY_WRITE  | 
                                             BLECharacteristic::PROPERTY_NOTIFY);

    // BLE Descriptor creation
    descr1 = new BLEDescriptor((uint16_t) 0x2901);
    descr1 -> setValue("Status of Test Result");
    bleChar1 -> addDescriptor(descr1);

    charConfig1 = new BLE2902();
    charConfig1 -> setNotifications(true);
    bleChar1 -> addDescriptor(charConfig1);
 
    bleChar2 -> addDescriptor(new BLE2902());
    bleChar2 -> setCallbacks(new CharacteristicsCallBack());

    // Start the Service
    service->start();

    // BLE Advertising
    BLEAdvertising* bleAdvert = BLEDevice::getAdvertising();
    bleAdvert->addServiceUUID(uuidService);
    bleAdvert->setScanResponse(false);
    bleAdvert->setMinPreferred(0x0);  // set value to 0x00 to not advertise this paramter
    BLEDevice::startAdvertising();
    Serial.println("Waiting for client connection to server ...");
}

void loop() 
{
    // notify changed value
    if (deviceConnected)
    {
        if (valueString == "Passed")
        {
            digitalWrite(1, HIGH);
        }
        else
        {
            digitalWrite(1, LOW);
        }
    }

    // disconnecting
    if (!deviceConnected && oldDeviceConnected)
    {
        delay(500); // preparation time for bluetooth stack
        bleServer -> startAdvertising(); // restart advertising
        Serial.println("Start Advertising");
        oldDeviceConnected = deviceConnected;
    }
    
    // connecting
    if (deviceConnected && !oldDeviceConnected)
    {
        // do stuff here for connecting
        oldDeviceConnected = deviceConnected;
    }
}



