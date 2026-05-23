ESP32 IoT Home Automation

A comprehensive smart home automation system built with an ESP32 microcontroller. This project allows for the control of home appliances (lights and fans) both manually via traditional wall switches and remotely through the Arduino IoT Cloud dashboard and Google Assistant.

<img width="1600" height="1200" alt="image" src="https://github.com/user-attachments/assets/13e31db7-b45a-4306-b659-c176a1c7a328" />


<img width="897" height="919" alt="image" src="https://github.com/user-attachments/assets/0a6e97c6-c8ca-40bf-ba09-964fc2dcd52a" />

<img width="994" height="735" alt="image" src="https://github.com/user-attachments/assets/090ee344-9650-4890-a31c-234e33f01760" />



Features

Dual Control System: Control appliances manually using physical wall switches (works even offline) or remotely via Wi-Fi.
Voice Control: Fully integrated with Google Home / Google Assistant for voice-activated switching.
Eco Mode (Auto-Off): Utilizes a PIR motion sensor to automatically turn off relays if no human motion is detected for 5 minutes, saving energy.
Environmental Monitoring: Real-time temperature and humidity tracking displayed on the cloud dashboard.
State Memory: Uses flash storage (Preferences.h) to remember relay states so the system recovers properly after a power loss.

Hardware Requirements

ESP32 devkit v1 type c Microcontroller: The brain of the project, featuring built-in Wi-Fi.
<img width="1000" height="1000" alt="image" src="https://github.com/user-attachments/assets/fb9b3930-8bb6-4885-b02d-a568b6cebdff" />

4-Channel Relay Module (Active-LOW): Safely isolates and switches AC mains voltage. (Note: Only 2 channels are used for Light and Fan, allowing room for expansion).
<img width="400" height="400" alt="image" src="https://github.com/user-attachments/assets/a7bd2c23-a8e1-447d-b109-589d9d7672f2" />

DHT11 Sensor: For reading room temperature and humidity.
<img width="1500" height="1500" alt="image" src="https://github.com/user-attachments/assets/918a90ca-6d6c-405b-b963-c61db9bb2b7c" />

HC-SR501 PIR Motion Sensor: For detecting human presence to trigger Eco Mode.
<img width="800" height="800" alt="image" src="https://github.com/user-attachments/assets/96507fa4-3e29-4186-856f-2fc924e3862a" />

Standard Wall Switches: Momentary switches wired to GND for manual overrides.

5V USB Power Supply: Powers the ESP32, which in turn provides 5V to the relays and sensors. A type C charger can be used.

Wires: Jumper Wires for Logic Data transfer and AC wire for AC Power In and Out

A AC plug. 

A 3D shell

Wiring & Pin Configuration:

All ground pins are connected together. The primary GPIO connections are:

Relay Light (IN): GPIO26 - Controls Light relay (active-LOW)
Relay Fan (IN): GPIO27 - Controls Fan relay (active-LOW)
Light Switch: GPIO33 - Wall switch input (pressed = GND)
Fan Switch: GPIO32 - Wall switch input (pressed = GND)
PIR Motion Sensor: GPIO34 - Motion detection (HIGH = motion)
DHT11 Data: GPIO4 - Temp/Humidity data

(A complete schematic and flowchart can be found in the /images folder of this repository).

Software Setup & Installation

Arduino IoT Cloud Configuration
Navigate to the Arduino IoT Cloud Dashboard (create.arduino.cc/iot/things).
Create an ESP32 Device and save the DEVICE_ID and DEVICE_KEY.
Create a Thing, link your device, and add the following variables exactly as named:
light (Boolean, Read/Write)
fan (Boolean, Read/Write)
eco (Boolean, Read/Write)
temperature (Float, Read Only)
humidity (Float, Read Only)
Set up your dashboard with 3 toggle switches (light, fan, eco) and 2 value widgets (temperature, humidity).

Arduino IDE Setup
Ensure you have the ESP32 board manager installed. Then, install the following libraries via the Library Manager:
ArduinoIoTCloud
Arduino_ConnectionHandler
DHTesp (by beegee-tokyo)

Flash the Firmware
Open the .ino file in the Arduino IDE.
Update the credentials block with your specific details:
const char DEVICE_ID[] = "YOUR_DEVICE_ID";
const char DEVICE_KEY[] = "YOUR_DEVICE_KEY";
const char WIFI_SSID[] = "YOUR_WIFI_SSID";
const char WIFI_PASS[] = "YOUR_WIFI_PASSWORD";
Select your ESP32 board and COM port, then upload the code.

Google Assistant Integration
To control the system via voice:

In the Arduino IoT Cloud, enable "Google Assistant Integration".
Open the Google Home App on your phone and link your Arduino account.
You can now use commands like "Turn on the fan" or "What is the temperature?".
