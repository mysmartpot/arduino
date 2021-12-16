#include <ArduinoBLE.h>

// Don't wait for serial port to connect unless in debugging mode.
#define DEBUG false

// The data pin of the soil moisture sensor should be connected to A0.
#define SOIL_MOISTURE_SENSOR_PIN A0

// A second soil moisture sensor is used to measure the water level of the water tank.
// Its data pin should be connected to A1.
#define WATER_LEVEL_METER_PIN    A1

// The signal pin of the relay that controls the water pump should be connected to D2.
#define PUMP_RELAY_PIN           2

// The built-in LED is used to signalize when there is a BLE connection.
#define CONNECTION_LED_PIN       LED_BUILTIN

// The time between two consecutive measurements of the soil moisture and water level.
#define MEASUREMENT_INTERVAL     (10 * 1000)

// How much water in ml can be pumped per minute.
#define FLOW_PER_MINUTE          150

// A structure that contains the readings of the soil moisture and water level sensors.
struct Measurements {
  unsigned short soilMoisture;
  unsigned short waterLevel;
};

// The smart pots advertise a service such that the central node can recognize them.
BLEService smartPotService("b62a0000-069a-4fc6-9d5b-1daadc0cda33");

// The Pi can read the soil moisture sensor value and is notified when a new value is available.
BLETypedCharacteristic<Measurements> measurementsCharacteristic("b62a0001-069a-4fc6-9d5b-1daadc0cda33", BLERead | BLENotify);

// The Pi can set the amount of water (in ml) that should be pumped into the tank.
BLEByteCharacteristic pumpAmountCharacteristic("b62a0002-069a-4fc6-9d5b-1daadc0cda33", BLERead | BLEWrite);

// Time until the pump should be turned off.
unsigned long pumpTimeout = -1;

void setup() {
  // Initialize serial interface.
  Serial.begin(9600);
  while (!Serial && DEBUG) {
    digitalWrite(CONNECTION_LED_PIN, HIGH);
    delay(100);
    digitalWrite(CONNECTION_LED_PIN, LOW);
    delay(100);
  }
  
  // Initialize IO pins.
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  pinMode(CONNECTION_LED_PIN, OUTPUT);

  // Initialize the bluetooth module.
  Serial.println("Initializing BLE...");
  if (!BLE.begin()) {
    Serial.println("Error: Failed to initialize BLE!");
    while (1); 
  }
  Serial.print("BLE address: ");
  Serial.println(BLE.address());

  // Set initial values of characteristics.
  measurementsCharacteristic.writeValue({0, 0});
  pumpAmountCharacteristic.writeValue(0);

  // Add event listener for changes of the pump characteristic.
  pumpAmountCharacteristic.setEventHandler(BLEWritten, pumpAmountCharacteristicWritten);

  // Initialize the smart pot service.
  smartPotService.addCharacteristic(measurementsCharacteristic);
  smartPotService.addCharacteristic(pumpAmountCharacteristic);
  BLE.addService(smartPotService);

  // Add event listers for connection state.
  BLE.setEventHandler(BLEConnected, connectHandler);
  BLE.setEventHandler(BLEDisconnected, disconnectHandler);
  
  // Advertise the smart pot service.
  BLE.setLocalName("Smart Pot");
  BLE.setDeviceName("Smart Pot");
  BLE.setAdvertisedService(smartPotService);
  BLE.advertise();
}

void loop() {
  // Measure soil moisture and water level periodically.
  static unsigned long nextMeasurement = 0;
  if (nextMeasurement <= millis()) {
    performMeasurements();
    nextMeasurement = millis() + MEASUREMENT_INTERVAL;
  }

  // Turn off the pump.
  if (pumpTimeout <= millis()) {
    turnOffPump();
  }
  
  // Listen for BLE events. Continue when there is no event until the next measurement is due
  // or the pump needs to be turned off.
  unsigned long timeout = min(pumpTimeout, nextMeasurement) - millis();
  BLE.poll(timeout);
}

void performMeasurements() {
  // Measure soil moisture and water level.
  Measurements measurements;
  measurements.soilMoisture = analogRead(SOIL_MOISTURE_SENSOR_PIN);
  measurements.waterLevel = analogRead(WATER_LEVEL_METER_PIN);

  // Print measurements for debugging purposes.
  Serial.print("soil moisture = ");
  Serial.print(measurements.soilMoisture);
  Serial.print(", water level = ");
  Serial.println(measurements.waterLevel);

  // Write measurements into characteristic.
  measurementsCharacteristic.writeValue(measurements);
}

void connectHandler(BLEDevice central) {
  Serial.print("Connected to central: ");
  Serial.println(central.address());
  digitalWrite(CONNECTION_LED_PIN, HIGH);
}

void disconnectHandler(BLEDevice central) {
  Serial.print("Disconnected from central: ");
  Serial.println(central.address());
  digitalWrite(CONNECTION_LED_PIN, LOW);
}

void pumpAmountCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  // Check value length.
  if (characteristic.valueLength() != 1) {
    Serial.print("Expected one byte for pump characteristic, but got ");
    Serial.println(characteristic.valueLength());
    return;
  }

  // Read written value.
  byte amount = characteristic.value()[0];
  Serial.print("Pump characteristic written: ");
  Serial.println(amount);

  // Calculate how long to turn on the pump.
  int duration = amount * 60 * 1000 / FLOW_PER_MINUTE;

  // Turn on the pump for the calculated amount of time.
  digitalWrite(PUMP_RELAY_PIN, HIGH);
  pumpTimeout = millis() + duration;
}

void turnOffPump() {
  // Turn off the pump.
  digitalWrite(PUMP_RELAY_PIN, LOW);

  // Reset timeout.
  pumpTimeout = -1;
}
