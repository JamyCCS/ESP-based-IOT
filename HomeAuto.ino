/************  CHANGE ONLY THESE (from Arduino IoT Cloud PDF)  ************/
const char DEVICE_ID[]  = "b3fcdc65-ac5f-4c0e-9193-b642c288a35f";
const char DEVICE_KEY[] = "LEsyu9jKdWH7UVcwfHT@9yker";
/**************************************************************************/

/************  HARDCODED WIFI (temporary)  ************/
const char WIFI_SSID[] = "Heaven";
const char WIFI_PASS[] = "avatar@123A";   // "" for open Wi-Fi
/*****************************************************/

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

#include <WiFi.h>
#include <Preferences.h>

#include <DHTesp.h>  // DHTesp library (beegee-tokyo). [web:107]

// ---------------- Pins (match your diagram) ----------------
const int RELAY_LIGHT_PIN  = 26;   // Relay IN for Light (active-LOW)
const int RELAY_FAN_PIN    = 27;   // Relay IN for Fan   (active-LOW)

const int SWITCH_FAN_PIN   = 32;   // FAN_SWITCH   -> GPIO32 -> to GND when pressed
const int SWITCH_LIGHT_PIN = 33;   // LIGHT_SWITCH -> GPIO33 -> to GND when pressed

const int PIR_PIN          = 34;   // PIR OUT -> GPIO34 (input-only pin)

const int DHT_PIN          = 4;    // DHT11 DATA -> D4 (GPIO4)
// -----------------------------------------------------------

// -------- Cloud variables (must match your Thing variable names) ------
bool  light = false;
bool  fan   = false;
bool  eco   = false;

// Don’t initialize with NAN (dashboards may show blank / no value)
float temperature = 0.0f;
float humidity    = 0.0f;
// ---------------------------------------------------------------------

// ---------------- Saved settings / state ----------------
Preferences prefs;

// Saved relay + eco state
const char* NS_APP = "homeauto";
const char* K_L    = "L";
const char* K_F    = "F";
const char* K_ECO  = "ECO";

// ---------------- Sensors ----------------
DHTesp dht;

// ---------------- Timings ----------------
unsigned long lastMotionTime = 0;
const unsigned long ECO_OFF_TIME = 5UL * 60UL * 1000UL;  // 5 minutes

unsigned long lastDhtRead = 0;
const unsigned long DHT_PERIOD = 5000;                  // 5 seconds (safe for DHT11)

// ---------------- Wi-Fi / Cloud runtime ----------------
char wifiSsid[33] = {0};
char wifiPass[65] = {0};

WiFiConnectionHandler* cloudConn = nullptr;
bool cloudStarted = false;

// Prevent “instant overwrite” when cloud reconnects
unsigned long ignoreCloudUntil = 0;

// ---------------- Wall switch edge detect ----------------
bool prevLightSw = false;
bool prevFanSw   = false;

// ---------- Cloud callbacks ----------
void onLightChange() {
  if (millis() < ignoreCloudUntil) return;
  prefs.putBool(K_L, light);
}

void onFanChange() {
  if (millis() < ignoreCloudUntil) return;
  prefs.putBool(K_F, fan);
}

void onEcoChange() {
  if (millis() < ignoreCloudUntil) return;
  if (eco) lastMotionTime = millis();
  prefs.putBool(K_ECO, eco);
}

// ---------- Cloud properties ----------
void initProperties() {
  ArduinoCloud.setBoardId(DEVICE_ID);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);

  ArduinoCloud.addProperty(light, READWRITE, ON_CHANGE, onLightChange);
  ArduinoCloud.addProperty(fan,   READWRITE, ON_CHANGE, onFanChange);
  ArduinoCloud.addProperty(eco,   READWRITE, ON_CHANGE, onEcoChange);

  ArduinoCloud.addProperty(temperature, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(humidity,    READ, ON_CHANGE, NULL);
}

// ---------- Helpers ----------
void applyRelays(bool allowOn) {
  bool outLight = allowOn && light;
  bool outFan   = allowOn && fan;

  // active-LOW relay
  digitalWrite(RELAY_LIGHT_PIN, outLight ? LOW : HIGH);
  digitalWrite(RELAY_FAN_PIN,   outFan   ? LOW : HIGH);
}

void loadSavedState() {
  light = prefs.getBool(K_L, false);
  fan   = prefs.getBool(K_F, false);
  eco   = prefs.getBool(K_ECO, false);
}

bool connectWiFi(unsigned long timeoutMs = 15000) {
  if (strlen(wifiSsid) == 0) return false;

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSsid, wifiPass);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeoutMs) {
    delay(50);
  }
  return (WiFi.status() == WL_CONNECTED);
}

void startCloud() {
  if (cloudConn) {
    delete cloudConn;
    cloudConn = nullptr;
  }

  cloudConn = new WiFiConnectionHandler(wifiSsid, wifiPass);
  ArduinoCloud.begin(*cloudConn);
  cloudStarted = true;

  // After reconnect, push local values first for a moment
  ignoreCloudUntil = millis() + 2000;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  // GPIO setup
  pinMode(RELAY_LIGHT_PIN, OUTPUT);
  pinMode(RELAY_FAN_PIN,   OUTPUT);
  digitalWrite(RELAY_LIGHT_PIN, HIGH); // off (active-LOW)
  digitalWrite(RELAY_FAN_PIN,   HIGH); // off (active-LOW)

  pinMode(SWITCH_LIGHT_PIN, INPUT_PULLUP);
  pinMode(SWITCH_FAN_PIN,   INPUT_PULLUP);
  pinMode(PIR_PIN,          INPUT);

  // DHTesp init for DHT11. [web:107]
  dht.setup(DHT_PIN, DHTesp::DHT11);

  // Load saved relay/eco state
  prefs.begin(NS_APP, false);
  loadSavedState();

  // Initialize edge detector baselines
  prevLightSw = (digitalRead(SWITCH_LIGHT_PIN) == LOW);
  prevFanSw   = (digitalRead(SWITCH_FAN_PIN)   == LOW);

  lastMotionTime = millis();
  applyRelays(true);

  // Cloud setup
  initProperties();

  // Hardcoded Wi-Fi credentials
  strlcpy(wifiSsid, WIFI_SSID, sizeof(wifiSsid));
  strlcpy(wifiPass, WIFI_PASS, sizeof(wifiPass));

  if (connectWiFi()) {
    startCloud();
  }

  Serial.print("WiFi: ");
  Serial.println((WiFi.status() == WL_CONNECTED) ? "connected" : "offline");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Keep the IoT Cloud connection alive (must be called frequently). [web:36]
  if (cloudStarted) {
    ArduinoCloud.update();
  }

  // 1) Motion sensor (for Eco mode)
  if (digitalRead(PIR_PIN) == HIGH) lastMotionTime = millis();

  bool allowOn = true;
  if (eco) allowOn = (millis() - lastMotionTime < ECO_OFF_TIME);

  // 2) Wall switches (manual control)
  bool lightSw = (digitalRead(SWITCH_LIGHT_PIN) == LOW);
  bool fanSw   = (digitalRead(SWITCH_FAN_PIN)   == LOW);

  if (lightSw != prevLightSw) {
    prevLightSw = lightSw;
    light = lightSw;
    prefs.putBool(K_L, light);
  }

  if (fanSw != prevFanSw) {
    prevFanSw = fanSw;
    fan = fanSw;
    prefs.putBool(K_F, fan);
  }

  // 3) DHT11 sensor (periodic reading)
  if (millis() - lastDhtRead >= DHT_PERIOD) {
    lastDhtRead = millis();

    TempAndHumidity th = dht.getTempAndHumidity(); // Single call reads both. [web:107]
    if (!isnan(th.temperature) && !isnan(th.humidity)) {
      temperature = th.temperature;
      humidity    = th.humidity;
    }

    // Debug (optional)
    Serial.print("DHT -> H: ");
    Serial.print(th.humidity, 0);
    Serial.print("%  T: ");
    Serial.print(th.temperature, 0);
    Serial.print("C  Status: ");
    Serial.println(dht.getStatusString());
  }

  // 4) Update relays
  applyRelays(allowOn);

  delay(30);
}
