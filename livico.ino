#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#define BNO055_SAMPLERATE_DELAY_MS (100)

// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                   id, address
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x29);

// BLE Service
BLEService imuService("917649A0-D98E-11E5-9EEC-0002A5D5C51B"); // Custom UUID

// BLE Characteristic
BLECharacteristic imuCharacteristic("917649A1-D98E-11E5-9EEC-0002A5D5C51B", BLERead | BLENotify, 12);

long previousMillis = 0;  // last timechecked, in ms

void setup() {
    Serial.begin(9600);    // initialize serial communication

    pinMode(LED_BUILTIN, OUTPUT); // initialize the built-in LED pin to indicate when a central is connected

    if (!BLE.begin()) {
        Serial.println("starting BLE failed!");
        while (1);
    }
    //Setup BNO
    if (!bno.begin()) {
        /* There was a problem detecting the BNO055 ... check your connections */
        Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
        while (1);
    }
    bno.setExtCrystalUse(true);
    // Setup bluetooth
    BLE.setLocalName("ArduinoIMU");
    BLE.setAdvertisedService(imuService);
    imuService.addCharacteristic(imuCharacteristic);
    BLE.addService(imuService);

    // start advertising
    BLE.advertise();
    Serial.println("Bluetooth device active, waiting for connections...");


}


void getDataAndSend() {
    
    sensors_event_t event;
    bno.getEvent(&event);
   

    float eulers[3];
    
    
    eulers[0] = event.orientation.x;
    eulers[1] = event.orientation.y;
    eulers[2] = event.orientation.z;

    imuCharacteristic.setValue((byte * ) & eulers, 12);
}

void loop() {
    // wait for a BLE central
    BLEDevice central = BLE.central();

    // if a BLE central is connected to the peripheral:
    if (central) {
        Serial.print("Connected to central: ");
        // print the central's BT address:
        Serial.println(central.address());
        // turn on the LED to indicate the connection:
        digitalWrite(LED_BUILTIN, HIGH);

        // while the central is connected:
        while (central.connected()) {
            long currentMillis = millis();

            if (currentMillis - previousMillis >= 10) {
                previousMillis = currentMillis;
                getDataAndSend();

            }
        }
        // when the central disconnects, turn off the LED:
        digitalWrite(LED_BUILTIN, LOW);
        Serial.print("Disconnected from central: ");
        Serial.println(central.address());
    }
}
