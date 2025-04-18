/*
 * Code for Adafruit HUZZAH32 with BaseCam MPU6050 IMU Communication over BLE
 * Authors: Ayman Laaroussi + Claude
 */
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Create the MPU6050 sensor object
Adafruit_MPU6050 imuSensor;

// BLE configuration
BLECharacteristic *bleCharacteristic;
#define SERVICE_UUID        "22345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "bbcdefab-1234-1234-1234-abcdefabcdef"

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // Wait for the serial port to be ready
  }
  Serial.println("MPU6050 test on Adafruit HUZZAH32 with BLE");
  
  // Initialize I2C (Wire) using GPIO23=SDA, GPIO22=SCL
  Wire.begin(23, 22);
  
  // Start up the MPU6050
  if (!imuSensor.begin()) {
    Serial.println("Could not find MPU6050. Check your wiring!");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 detected!");
  
  // Configure accelerometer range
  imuSensor.setAccelerometerRange(MPU6050_RANGE_2_G);
  Serial.print("Accelerometer range set to: ");
  switch (imuSensor.getAccelerometerRange()) {
    case MPU6050_RANGE_2_G:  Serial.println("±2G");  break;
    case MPU6050_RANGE_4_G:  Serial.println("±4G");  break;
    case MPU6050_RANGE_8_G:  Serial.println("±8G");  break;
    case MPU6050_RANGE_16_G: Serial.println("±16G"); break;
  }
  
  // Initialize BLE
  BLEDevice::init("XIAO_BLE");
  BLEServer *bleServer = BLEDevice::createServer();
  BLEService *bleService = bleServer->createService(SERVICE_UUID);
  
  bleCharacteristic = bleService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  bleCharacteristic->addDescriptor(new BLE2902());
  
  bleService->start();
  BLEDevice::getAdvertising()->start();
  
  Serial.println("BLE ready, waiting for connection...");
  delay(100);
}

void loop() {
  // Read sensor events (accelerometer, gyroscope, temperature)
  sensors_event_t accelEvent, gyroEvent, tempEvent;
  imuSensor.getEvent(&accelEvent, &gyroEvent, &tempEvent);
  
  // Build the comma‑separated data string
  String dataString = String(int(1500 + accelEvent.acceleration.x * 100)) + "," +
                      String(int(1500 + accelEvent.acceleration.y * 100)) + "," +
                      String(int(1500 + accelEvent.acceleration.z * 100));
  
  // Send over BLE
  bleCharacteristic->setValue(dataString.c_str());
  bleCharacteristic->notify();
  
  // Also print to serial
  Serial.println(dataString);
  
  delay(20); // ~50 Hz update rate
}
