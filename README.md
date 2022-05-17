# Smart Pot Arduino

This repository contains the source code for the Arduino firmware that powers the Smart Pot.
In this document the installation of the firmware on the microcontroller as well as the construction of the circuit are described.
This guide does not cover the construction of the actual pot or how to integrate and waterproof the electronics.

## Setup

### Hardware Requirements

To build your own Smart Pot you need the following components.

 - An [Arduino Nano 33 BLE][arduino-nano-33-ble].
 - Two capacitive soil moisture sensors.
   One of which is used to measure the soil moisture and the other to monitor the water level of the water tank.
 - A water pump and a compatible relay.
   We recommend to use a peristaltic pump.
   They are slower but more precise and don't have to be submerged under water.
 - A DC power supply and matching power plug.
   Make sure that your power supply can deliver enough power to drive all other components simultaneously.
   Ideally the voltage of the power supply matches the rating of the pump.
   The Arduino Nano 33 BLE can accept an input voltage of at most 21V.

Since the Arduino Nano 33 BLE uses 3.3V, the sensors and the relay must be rated for a voltage of 3.3V as well.

### Wiring

Connect the components to the microcontroller as follows.

| Component            | Pin            |
|----------------------|----------------|
| Soil moisture sensor | Analog pin A0  |
| Water level meter    | Analog pin A1  |
| Pump relay           | Digital pin D2 |
| DC power plug        | VIN and GND    |

Remember to connect all components to GND and VCC according to their datasheets and connect the pump via the relay to the DC input voltage in the normally open position.

### Dependencies

The firmware uses the [ArduinoBLE library][arduino-ble-library].
The correct version of the library is included with this repository as a Git submodule.
Use the following commands after cloning this repository to initialize and download the library.

```bash
git submodule init
git submodule update
```

### Firmware Installation

To install the firmware on your Arduino, proceed as follows.

 - Open the `smart-pot` sketch in the Arduino IDE.
 - Install `Arduino Mbed OS Nano Boards` under `Tools > Board > Boards Manager`.
 - Connect the Arduino with your PC via USB.
 - Select the correct COM port under `Tools > Port`.
 - Select `Ardunino Nano 33 BLE` under `Tools > Board > Arduino Mbed OS Nano Boards`.
 - Click the `Upload` button and wait for the firmware to be flashed.

> **Note:** Before installing the firmware you may have to tweak the constants at the top of `smart-pot.ino` to match your setup.
> For example, adjust `FLOW_PER_MINUTE` if your pump does not deliver the configured value of 150 milliliters per minute.

## License

The Smart Pot Arduino firmware is licensed under the MIT license agreement.
See the [LICENSE][smart-pot/arduino/LICENSE] file for details.

[arduino-ble-library]:
  https://github.com/arduino-libraries/ArduinoBLE
  "ArduinoBLE library for Arduino"
[arduino-nano-33-ble]:
  http://store.arduino.cc/products/arduino-nano-33-ble
  "Arduino Nano 33 BLE — Arduino Official Store"

[smart-pot/arduino/LICENSE]:
  https://github.com/mysmartpot/arduino/blob/main/LICENSE
  "The MIT License"
