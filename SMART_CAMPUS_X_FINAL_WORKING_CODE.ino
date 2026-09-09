/*
  SMART CAMPUS X
  ESP32-S3 Smart Classroom Monitoring & Control

  Hardware:
  - ESP32-S3
  - SSD1306 128x64 I2C OLED
  - DHT11
  - MQ-135
  - LDR
  - Flame sensor
  - HC-SR501 PIR
  - 1-channel relay
  - Push button
  - 60 W lamp/load

  IMPORTANT:
  The electrical values in this version are TEST VALUES around a
  60 W load. They are NOT measurements from a physical PZEM module.
  This keeps the prototype usable even without a PZEM connected.

  BUTTON:
  Press GPIO21 button -> relay OFF -> physical lamp OFF -> electrical
  test readings become zero.
  Press again -> relay ON -> physical lamp ON -> readings resume.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ---------------- Wi-Fi ----------------
const char* AP_SSID = "SMART-CAMPUS-X";
const char* AP_PASSWORD = "campus1234";

WebServer server(80);

// ---------------- OLED ----------------
#define OLED_SDA 8
#define OLED_SCL 9
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------------- Sensors ----------------
#define DHT_PIN 20
#define DHT_TYPE DHT11
#define MQ135_PIN 1
#define LDR_PIN 2
#define FLAME_PIN 3
#define PIR_PIN 6

DHT dht(DHT_PIN, DHT_TYPE);

// ---------------- Control ----------------
#define RELAY_PIN 10
#define BUTTON_PIN 21

// Most common relay modules are active LOW.
// Change to false if your relay module is active HIGH.
const bool RELAY_ACTIVE_LOW = true;

// Most flame modules are active LOW.
const bool FLAME_ACTIVE_LOW = true;

// ---------------- Sensor data ----------------
float temperatureC = 0.0;
float humidity = 0.0;
int airQuality = 0;
int lightLevel = 0;
bool flameDetected = false;
bool presenceDetected = false;

// ---------------- Electrical test data ----------------
bool loadOn = true;

float voltageV = 230.0;
float currentA = 0.260;
float powerW = 60.0;
float frequencyHz = 50.0;
float powerFactor = 0.98;
float energyKWh = 0.0;

// ---------------- Timers ----------------
unsigned long lastDHT = 0;
unsigned long lastSensors = 0;
unsigned long lastElectrical = 0;
unsigned long lastOLED = 0;

const unsigned long DHT_INTERVAL = 2500;
const unsigned long SENSOR_INTERVAL = 1000;
const unsigned long ELECTRICAL_INTERVAL = 1000;
const unsigned long OLED_INTERVAL = 1000;

// ---------------- Button debounce ----------------
bool lastButtonReading = HIGH;
bool stableButtonState = HIGH;
unsigned long lastDebounce = 0;
const unsigned long DEBOUNCE_MS = 50;

// ============================================================
// Relay
// ============================================================

void setLoad(bool on)
{
  loadOn = on;

  if (RELAY_ACTIVE_LOW)
    digitalWrite(RELAY_PIN, on ? LOW : HIGH);
  else
    digitalWrite(RELAY_PIN, on ? HIGH : LOW);

  if (!on)
  {
    voltageV = 0.0;
    currentA = 0.0;
    powerW = 0.0;
    frequencyHz = 0.0;
    powerFactor = 0.0;
  }
}

// ============================================================
// Button
// ============================================================

void checkButton()
{
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonReading)
    lastDebounce = millis();

  if ((millis() - lastDebounce) > DEBOUNCE_MS)
  {
    if (reading != stableButtonState)
    {
      stableButtonState = reading;

      // Button connected between GPIO21 and GND.
      if (stableButtonState == LOW)
      {
        setLoad(!loadOn);

        if (loadOn)
          Serial.println("BUTTON: LOAD ON");
        else
          Serial.println("BUTTON: LOAD OFF");
      }
    }
  }

  lastButtonReading = reading;
}

// ============================================================
// Environment sensors
// ============================================================

void readEnvironment()
{
  if (millis() - lastDHT >= DHT_INTERVAL)
  {
    lastDHT = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t))
      temperatureC = t;

    if (!isnan(h))
      humidity = h;
  }

  int mq = analogRead(MQ135_PIN);
  airQuality = map(mq, 0, 4095, 0, 100);
  airQuality = constrain(airQuality, 0, 100);

  int ldr = analogRead(LDR_PIN);
  lightLevel = map(ldr, 0, 4095, 0, 100);
  lightLevel = constrain(lightLevel, 0, 100);

  int flame = digitalRead(FLAME_PIN);

  if (FLAME_ACTIVE_LOW)
    flameDetected = (flame == LOW);
  else
    flameDetected = (flame == HIGH);

  presenceDetected = digitalRead(PIR_PIN) == HIGH;
}

// ============================================================
// Electrical test readings around 60 W
// ============================================================

void updateElectrical()
{
  if (!loadOn)
  {
    voltageV = 0.0;
    currentA = 0.0;
    powerW = 0.0;
    frequencyHz = 0.0;
    powerFactor = 0.0;
    return;
  }

  // Small realistic variation around a 60 W load.
  powerW = 60.0 + ((float)random(-150, 151) / 10.0);
  powerW = constrain(powerW, 45.0, 75.0);

  voltageV = 230.0 + ((float)random(-10, 11) / 10.0);
  frequencyHz = 50.0 + ((float)random(-2, 3) / 10.0);

  powerFactor = 0.98 + ((float)random(-2, 3) / 100.0);
  powerFactor = constrain(powerFactor, 0.95, 1.00);

  currentA = powerW / (voltageV * powerFactor);

  // Energy accumulation in kWh.
  energyKWh += powerW / 3600000.0;

  Serial.print("TEST ELECTRICAL | V=");
  Serial.print(voltageV, 1);
  Serial.print(" V | I=");
  Serial.print(currentA, 3);
  Serial.print(" A | P=");
  Serial.print(powerW, 1);
  Serial.print(" W | F=");
  Serial.print(frequencyHz, 1);
  Serial.print(" Hz | PF=");
  Serial.print(powerFactor, 2);
  Serial.print(" | E=");
  Serial.print(energyKWh, 5);
  Serial.println(" kWh");
}

// ============================================================
// OLED
// ============================================================

void updateOLED()
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("SMART CAMPUS X");

  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setCursor(0, 14);
  display.print("Class 1: ");
  display.println(loadOn ? "ON" : "OFF");

  display.setCursor(0, 25);
  display.print("Power: ");
  display.print(powerW, 1);
  display.println(" W");

  display.setCursor(0, 36);
  display.print("Temp: ");
  display.print(temperatureC, 1);
  display.println(" C");

  display.setCursor(0, 47);
  display.print("Humidity: ");
  display.print(humidity, 0);
  display.println("%");

  display.setCursor(0, 58);
  display.print(flameDetected ? "FIRE ALERT" : "Safety OK");

  display.display();
}

// ============================================================
// Dashboard HTML
// ============================================================

const char INDEX_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SMART CAMPUS X</title>
<style>
*{box-sizing:border-box}
body{
  margin:0;
  font-family:Arial,Helvetica,sans-serif;
  background:#07111f;
  color:#f5f8fc;
}
header{
  padding:22px 28px;
  background:#0b1728;
  border-bottom:1px solid #20304a;
}
h1{margin:0;font-size:25px}
.subtitle{margin-top:5px;color:#8294b3;font-size:13px}
main{max-width:1450px;margin:auto;padding:24px}
.title{font-size:20px;font-weight:700;margin:4px 0 14px}
.grid{
  display:grid;
  grid-template-columns:repeat(auto-fit,minmax(230px,1fr));
  gap:16px;
}
.card{
  background:#0d1a2d;
  border:1px solid #20304a;
  border-radius:15px;
  padding:19px;
}
.name{font-size:17px;font-weight:700}
.connected{color:#35d9a0;font-size:12px;margin-top:7px}
.notconnected{color:#74839c;font-size:12px;margin-top:7px}
.big{font-size:32px;font-weight:800;margin-top:17px}
.small{font-size:12px;color:#8190aa}
.metrics{
  display:grid;
  grid-template-columns:repeat(auto-fit,minmax(160px,1fr));
  gap:14px;
}
.value{font-size:25px;font-weight:800;margin-top:8px}
.label{font-size:12px;color:#8190aa}
button{
  border:0;
  border-radius:9px;
  padding:11px 20px;
  font-weight:700;
  color:white;
  background:#168cff;
  cursor:pointer;
}
button.off{background:#d84c4c}
.control{
  display:flex;
  justify-content:space-between;
  align-items:center;
  margin-top:18px;
}
.footer{text-align:center;color:#60708c;font-size:11px;padding:30px}
.alert{color:#ff6b6b}
.ok{color:#36dca3}
</style>
</head>

<body>

<header>
  <h1>SMART CAMPUS X</h1>
  <div class="subtitle">Smart Classroom Monitoring & Control</div>
</header>

<main>

<div class="title">Classrooms</div>

<div class="grid">

  <div class="card">
    <div class="name">Classroom 1</div>
    <div class="connected">● CONNECTED</div>
    <div class="big"><span id="c1p">0</span> W</div>
    <div class="small">Electrical Power</div>

    <div class="control">
      <span>Light: <b id="light">OFF</b></span>
      <button id="btn" onclick="toggleLoad()">TURN ON</button>
    </div>
  </div>

  <div class="card">
    <div class="name">Classroom 2</div>
    <div class="notconnected">● NOT CONNECTED</div>
    <div class="big">0 W</div>
    <div class="small">Electrical Power</div>
  </div>

  <div class="card">
    <div class="name">Classroom 3</div>
    <div class="notconnected">● NOT CONNECTED</div>
    <div class="big">0 W</div>
    <div class="small">Electrical Power</div>
  </div>

  <div class="card">
    <div class="name">Classroom 4</div>
    <div class="notconnected">● NOT CONNECTED</div>
    <div class="big">0 W</div>
    <div class="small">Electrical Power</div>
  </div>

</div>

<br>

<div class="title">Electrical Parameters — Classroom 1</div>

<div class="metrics">

  <div class="card">
    <div class="label">VOLTAGE</div>
    <div class="value"><span id="v">0</span> V</div>
  </div>

  <div class="card">
    <div class="label">CURRENT</div>
    <div class="value"><span id="i">0</span> A</div>
  </div>

  <div class="card">
    <div class="label">POWER</div>
    <div class="value"><span id="p">0</span> W</div>
  </div>

  <div class="card">
    <div class="label">FREQUENCY</div>
    <div class="value"><span id="f">0</span> Hz</div>
  </div>

  <div class="card">
    <div class="label">POWER FACTOR</div>
    <div class="value"><span id="pf">0</span></div>
  </div>

  <div class="card">
    <div class="label">ENERGY</div>
    <div class="value"><span id="e">0</span> kWh</div>
  </div>

</div>

<br>

<div class="title">Environment</div>

<div class="metrics">

  <div class="card">
    <div class="label">TEMPERATURE</div>
    <div class="value"><span id="t">0</span> °C</div>
  </div>

  <div class="card">
    <div class="label">HUMIDITY</div>
    <div class="value"><span id="h">0</span> %</div>
  </div>

  <div class="card">
    <div class="label">AIR QUALITY</div>
    <div class="value"><span id="aq">0</span> %</div>
  </div>

  <div class="card">
    <div class="label">LIGHT LEVEL</div>
    <div class="value"><span id="ll">0</span> %</div>
  </div>

</div>

<br>

<div class="title">Safety & Presence</div>

<div class="metrics">

  <div class="card">
    <div class="label">PRESENCE</div>
    <div id="presence" class="value">NO</div>
  </div>

  <div class="card">
    <div class="label">FLAME</div>
    <div id="flame" class="value">NORMAL</div>
  </div>

</div>

</main>

<div class="footer">
SMART CAMPUS X • Smart • Safe • Efficient
</div>

<script>

function refresh(){

  fetch('/data')
  .then(r => r.json())
  .then(d => {

    document.getElementById('c1p').innerText = d.power.toFixed(1);

    document.getElementById('v').innerText = d.voltage.toFixed(1);
    document.getElementById('i').innerText = d.current.toFixed(3);
    document.getElementById('p').innerText = d.power.toFixed(1);
    document.getElementById('f').innerText = d.frequency.toFixed(1);
    document.getElementById('pf').innerText = d.pf.toFixed(2);
    document.getElementById('e').innerText = d.energy.toFixed(5);

    document.getElementById('t').innerText = d.temperature.toFixed(1);
    document.getElementById('h').innerText = d.humidity.toFixed(1);
    document.getElementById('aq').innerText = d.air;
    document.getElementById('ll').innerText = d.light;

    document.getElementById('light').innerText =
      d.load ? 'ON' : 'OFF';

    let b = document.getElementById('btn');

    if(d.load){
      b.innerText = 'TURN OFF';
      b.className = '';
    }else{
      b.innerText = 'TURN ON';
      b.className = 'off';
    }

    let pr = document.getElementById('presence');
    pr.innerText = d.presence ? 'DETECTED' : 'NO';
    pr.className = d.presence ? 'value ok' : 'value';

    let fl = document.getElementById('flame');
    fl.innerText = d.flame ? 'FLAME DETECTED' : 'NORMAL';
    fl.className = d.flame ? 'value alert' : 'value ok';
  });
}

function toggleLoad(){
  fetch('/toggle').then(() => refresh());
}

setInterval(refresh,1000);
refresh();

</script>

</body>
</html>
)HTML";

// ============================================================
// JSON data
// ============================================================

void handleData()
{
  String json;
  json.reserve(600);

  json += "{";

  json += "\"load\":";
  json += (loadOn ? "true" : "false");

  json += ",\"voltage\":";
  json += String(voltageV, 2);

  json += ",\"current\":";
  json += String(currentA, 3);

  json += ",\"power\":";
  json += String(powerW, 2);

  json += ",\"frequency\":";
  json += String(frequencyHz, 2);

  json += ",\"pf\":";
  json += String(powerFactor, 2);

  json += ",\"energy\":";
  json += String(energyKWh, 5);

  json += ",\"temperature\":";
  json += String(temperatureC, 1);

  json += ",\"humidity\":";
  json += String(humidity, 1);

  json += ",\"air\":";
  json += String(airQuality);

  json += ",\"light\":";
  json += String(lightLevel);

  json += ",\"presence\":";
  json += (presenceDetected ? "true" : "false");

  json += ",\"flame\":";
  json += (flameDetected ? "true" : "false");

  json += "}";

  server.send(200, "application/json", json);
}

// ============================================================
// Web toggle
// ============================================================

void handleToggle()
{
  setLoad(!loadOn);
  server.send(200, "text/plain", "OK");
}

// ============================================================
// Root
// ============================================================

void handleRoot()
{
  server.send_P(200, "text/html", INDEX_HTML);
}

// ============================================================
// Setup
// ============================================================

void setup()
{
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("=================================");
  Serial.println("       SMART CAMPUS X");
  Serial.println("=================================");

  // GPIO
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(FLAME_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);

  // Keep relay ON at startup.
  setLoad(true);

  // I2C
  Wire.begin(OLED_SDA, OLED_SCL);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println("OLED not found.");
  }
  else
  {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("SMART CAMPUS X");
    display.setCursor(0, 20);
    display.println("Starting...");
    display.display();
  }

  // DHT
  dht.begin();

  // Random seed
  randomSeed(
    micros() ^
    analogRead(MQ135_PIN) ^
    analogRead(LDR_PIN)
  );

  // Wi-Fi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  IPAddress ip = WiFi.softAPIP();

  Serial.println("Wi-Fi AP started.");
  Serial.print("SSID: ");
  Serial.println(AP_SSID);
  Serial.print("Password: ");
  Serial.println(AP_PASSWORD);
  Serial.print("Dashboard: http://");
  Serial.println(ip);

  // Web routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/data", HTTP_GET, handleData);
  server.on("/toggle", HTTP_GET, handleToggle);

  server.begin();

  Serial.println("Web server started.");
  Serial.println("=================================");
}

// ============================================================
// Loop
// ============================================================

void loop()
{
  server.handleClient();

  checkButton();

  if (millis() - lastSensors >= SENSOR_INTERVAL)
  {
    lastSensors = millis();
    readEnvironment();
  }

  if (millis() - lastElectrical >= ELECTRICAL_INTERVAL)
  {
    lastElectrical = millis();
    updateElectrical();
  }

  if (millis() - lastOLED >= OLED_INTERVAL)
  {
    lastOLED = millis();
    updateOLED();
  }

  delay(5);
}
