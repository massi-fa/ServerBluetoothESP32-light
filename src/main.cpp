#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <string>
#include <iostream>
#include <Arduino.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "3778516a-3453-11ed-a261-0242ac120002"
#define CHARACTERISTIC_UUID "37785020-3453-11ed-a261-0242ac120002"

#define LIGHT_SENSOR_PIN 32 // ESP32 pin GIOP36 (ADC0)

#define REFRESH 5000 // Intervallo di tempo tra un refresh e l'altro
#define DELAY 2000 // Intervallo di tempo tra un refresh e l'altro

int light = 0;
int newLight = 0;
int detectionCount = 0;
int averageLight = 0;

unsigned long myTime; //Variabile per salvare il tempo passato dall'inizio dell'esecuzione del programma

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pCharacteristic;
BLEAdvertising *pAdvertising;


void setup()
{
  Serial.begin(115200);
  Serial.println("Starting BLE Server!");

  BLEDevice::init("ESP32-LightServer");
  pServer = BLEDevice::createServer();
  pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );


  pCharacteristic->setValue("LightServer");
  pService->start();
  pAdvertising = pServer->getAdvertising();
  //pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  //BLEDevice::startAdvertising();
  pAdvertising->start();
  Serial.println("Characteristic defined! Now you can read it in the Client!");
}

void loop()
{
  myTime = millis();

  if (myTime%REFRESH == 0) { //Se il tempo passato dall'inizio dell'esecuzione del programma è multiplo di REFRESH
    newLight = analogRead(LIGHT_SENSOR_PIN);
    Serial.print("Luminosità rilevata: ");
    Serial.println(newLight);
    detectionCount += 1;
    light += newLight;
    averageLight = light / detectionCount;
    pCharacteristic->setValue(std::to_string(averageLight));
    Serial.print("Luminosità media: ");
    Serial.println(averageLight);
  }

  if (myTime%DELAY == 0) { //Se il tempo passato dall'inizio dell'esecuzione del programma è multiplo di REFRESH
    if(pServer->getConnectedCount() == 0){
      Serial.println("Nessun dispositivo connesso inizio l'advertising");
      pAdvertising->start();
    }else{
      pCharacteristic->setValue(std::to_string(averageLight));
      light = 0;
      detectionCount = 0;
      std::string value = pCharacteristic->getValue();
      Serial.print("The new characteristic value is: ");
      Serial.println(value.c_str());
    }
  }

}