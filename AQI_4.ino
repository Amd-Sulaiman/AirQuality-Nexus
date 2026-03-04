#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include "ScioSense_ENS160.h"

// ---------------------------------------------------------
// HARDWARE CONFIGURATION
// ---------------------------------------------------------
#define SDA_PIN 19
#define SCL_PIN 20
#define LED_PIN 15

const char* ssid = "ESP32_Air_Glass";
const char* password = "password123";

// ---------------------------------------------------------
Adafruit_AHTX0 aht;
ScioSense_ENS160 ens160(ENS160_I2CADDR_1);
WebServer server(80);

// Sensor globals
float t_temp = 0.0;
float t_hum = 0.0;
uint8_t aqi = 0;
uint16_t tvoc = 0;
uint16_t eco2 = 400;

unsigned long lastAHT = 0;
unsigned long lastENS = 0;

const unsigned long ahtInterval = 5000;   // 3 sec
const unsigned long ensInterval = 5000;   // 3 sec

// ---------------------------------------------------------
// HTML PAGE (Same glass UI, unchanged - omitted here for brevity)
// KEEP YOUR EXISTING index_html STRING EXACTLY SAME
// ---------------------------------------------------------

// ---------------------------------------------------------
// WEBPAGE HTML/CSS/JS (Stored in Flash Memory)
// ---------------------------------------------------------
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Air Quality Monitor</title>
  <style>
    :root {
      /* Slightly more transparent glass for the lighter background */
      --glass-bg: rgba(255, 255, 255, 0.1);
      --glass-border: rgba(255, 255, 255, 0.25);
      --text-color: #ffffff;
      --accent-cyan: #00d2ff;
      --accent-pink: #ff007f;
    }
    body {
      margin: 0;
      font-family: 'Segoe UI', system-ui, sans-serif;
      /* NEW BLUEISH-GREEN GRADIENT BACKGROUND */
      background: linear-gradient(to bottom right, #43cea2, #185a9d);
      /* Alternative deeper option: background: linear-gradient(135deg, #134E5E 0%, #71B280 100%); */
      color: var(--text-color);
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      padding-bottom: 30px;
      /* Ensure gradient doesn't repeat on tall screens */
      background-repeat: no-repeat;
      background-attachment: fixed;
    }
    h1 { margin: 30px 0 10px; font-weight: 200; letter-spacing: 4px; font-size: 1.5rem; text-shadow: 0 2px 4px rgba(0,0,0,0.2); }
    
    .container {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(130px, 1fr));
      gap: 15px;
      width: 90%;
      max-width: 800px;
      margin: 20px 0;
    }

    .card {
      background: var(--glass-bg);
      backdrop-filter: blur(15px);
      -webkit-backdrop-filter: blur(15px);
      border: 1px solid var(--glass-border);
      border-radius: 20px;
      padding: 15px;
      text-align: center;
      transition: transform 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
      box-shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.15);
    }
    .card:hover { transform: scale(1.05); background: rgba(255,255,255,0.15); }
    
    .value { font-size: 2.2rem; font-weight: 700; margin: 5px 0; text-shadow: 0 2px 4px rgba(0,0,0,0.1); }
    .unit { font-size: 0.8rem; opacity: 0.8; font-weight: 400; }
    .label { font-size: 0.75rem; text-transform: uppercase; letter-spacing: 1.5px; opacity: 0.9; font-weight: 600;}

    /* Specific Card Colors - adjusted slightly for better contrast on new bg */
    #temp-val { color: #ffe082; } /* Lighter orange */
    #hum-val { color: #80d8ff; }  /* Lighter blue */
    #aqi-val { color: #b9f6ca; }  /* Lighter green */

    .chart-container {
      width: 90%;
      max-width: 800px;
      background: var(--glass-bg);
      backdrop-filter: blur(15px);
      border: 1px solid var(--glass-border);
      border-radius: 20px;
      padding: 20px;
      box-sizing: border-box;
      box-shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.15);
    }
    .chart-header {
        display: flex;
        justify-content: space-between;
        align-items: center;
        margin-bottom: 15px;
    }
    .legend { display: flex; gap: 15px; font-size: 0.8rem; }
    .legend-item { display: flex; align-items: center; gap: 5px; }
    .dot { width: 10px; height: 10px; border-radius: 50%; box-shadow: 0 0 5px currentColor; }

    canvas { width: 100%; height: 250px; }
  </style>
</head>
<body>
  <h1>AIR MONITOR</h1>
  
  <div class="container">
    <div class="card"><div class="label">Temp</div><div class="value" id="temp-val">--</div><div class="unit">°C</div></div>
    <div class="card"><div class="label">Humidity</div><div class="value" id="hum-val">--</div><div class="unit">%</div></div>
    <div class="card"><div class="label">AQI</div><div class="value" id="aqi-val">--</div><div class="unit">1-5</div></div>
    <div class="card"><div class="label">TVOC</div><div class="value" id="tvoc">--</div><div class="unit">ppb</div></div>
    <div class="card"><div class="label">eCO2</div><div class="value" id="eco2">--</div><div class="unit">ppm</div></div>
  </div>

  <div class="chart-container">
    <div class="chart-header">
        <div class="label">Trends (Auto-Scaling)</div>
        <div class="legend">
            <div class="legend-item" style="color:var(--accent-cyan)"><span class="dot" style="background:var(--accent-cyan)"></span> TVOC</div>
            <div class="legend-item" style="color:var(--accent-pink)"><span class="dot" style="background:var(--accent-pink)"></span> eCO2</div>
        </div>
    </div>
    <canvas id="sensorChart"></canvas>
  </div>

<script>
  const canvas = document.getElementById('sensorChart');
  const ctx = canvas.getContext('2d');
  
  // DPI Setup
  const dpr = window.devicePixelRatio || 1;
  let rect;
  function resize() {
      rect = canvas.getBoundingClientRect();
      canvas.width = rect.width * dpr;
      canvas.height = rect.height * dpr;
      ctx.scale(dpr, dpr);
      drawChart(); // Redraw on resize
  }
  window.addEventListener('resize', resize);
  // Initial sizing delayed slightly to ensure DOM is ready
  setTimeout(resize, 100);

  const maxPoints = 40;
  let dataPointsTVOC = new Array(maxPoints).fill(0);
  let dataPointsCO2 = new Array(maxPoints).fill(400);

  function drawChart() {
    if(!rect) return; // Guard clause if resize hasn't run yet
    const w = rect.width;
    const h = rect.height;
    ctx.clearRect(0, 0, w, h);
    
    // 1. DYNAMIC SCALING LOGIC
    let peakTVOC = Math.max(...dataPointsTVOC, 100); 
    let peakCO2 = Math.max(...dataPointsCO2, 600);
    let scaleTVOC = peakTVOC * 1.15;
    let scaleCO2 = peakCO2 * 1.15;

    // Grid Lines
    ctx.strokeStyle = 'rgba(255,255,255,0.1)';
    ctx.beginPath();
    for(let i=1; i<=4; i++) {
      let y = (h/5)*i;
      ctx.moveTo(0, y); ctx.lineTo(w, y);
    }
    ctx.stroke();

    const mapY = (val, max) => h - (val / max) * h;
    const step = w / (maxPoints - 1);

    // Draw Line Function
    function drawLine(data, color, maxScale) {
        ctx.beginPath();
        ctx.strokeStyle = color;
        ctx.lineWidth = 3;
        ctx.lineJoin = 'round';
        ctx.shadowBlur = 10;
        ctx.shadowColor = color;
        
        for (let i = 0; i < maxPoints; i++) {
            let x = i * step;
            let y = mapY(data[i], maxScale);
            if(i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
        }
        ctx.stroke();
        ctx.shadowBlur = 0;
    }

    drawLine(dataPointsTVOC, '#00d2ff', scaleTVOC);
    drawLine(dataPointsCO2, '#ff007f', scaleCO2);

    // Scale Labels
    ctx.fillStyle = 'rgba(255,255,255,0.6)';
    ctx.font = '10px sans-serif';
    ctx.fillText(Math.round(scaleTVOC) + " ppb", 5, 12);
    ctx.fillText(Math.round(scaleCO2) + " ppm", 5, 25);
  }

  function fetchData() {
    fetch('/data')
      .then(r => r.json())
      .then(data => {
        // Note: I updated IDs in HTML to correspond to specific color styling
        document.getElementById('temp-val').innerText = data.temp.toFixed(1);
        document.getElementById('hum-val').innerText = data.hum.toFixed(1);
        document.getElementById('aqi-val').innerText = data.aqi;
        document.getElementById('tvoc').innerText = data.tvoc;
        document.getElementById('eco2').innerText = data.eco2;

        dataPointsTVOC.push(data.tvoc);
        dataPointsTVOC.shift();
        dataPointsCO2.push(data.eco2);
        dataPointsCO2.shift();
        drawChart();
      }).catch(e => console.log("Fetch error (AP Mode?)"));
  }

  setInterval(fetchData, 2000);
  // drawChart() is called by resize() initially
</script>
</body>
</html>
)rawliteral";

// ---------------------------------------------------------
// SERVER HANDLERS
// ---------------------------------------------------------
void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleData() {
  String json = "{";
  json += "\"temp\":" + String(t_temp) + ",";
  json += "\"hum\":" + String(t_hum) + ",";
  json += "\"aqi\":" + String(aqi) + ",";
  json += "\"tvoc\":" + String(tvoc) + ",";
  json += "\"eco2\":" + String(eco2);
  json += "}";
  server.send(200, "application/json", json);
}

// ---------------------------------------------------------
// SETUP
// ---------------------------------------------------------
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);  // Stable I2C for C6

  Serial.println("Initializing Sensors...");

  if (!aht.begin()) {
    Serial.println("AHT20 not found");
    while (1);
  }
  Serial.println("AHT20 OK");

  ens160.begin();
  if (ens160.available()) {
    ens160.setMode(ENS160_OPMODE_STD);
    delay(2000);

    Serial.println("ENS160 OK - Standard Mode");
  } else {
    Serial.println("ENS160 FAILED");
  }

  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

// ---------------------------------------------------------
// LOOP
// ---------------------------------------------------------
unsigned long lastRead = 0;

void loop() {

  server.handleClient();

  if (millis() - lastRead > 3000) {
    lastRead = millis();

    // ---- AHT20 ----
    sensors_event_t humidity, temp;
    if (aht.getEvent(&humidity, &temp)) {
      t_temp = temp.temperature;
      t_hum  = humidity.relative_humidity;
    }

    // ---- ENS160 (NO ENV DATA, NO RAW) ----
    if (ens160.available()) {
      ens160.measure(false);

      aqi  = ens160.getAQI();
      tvoc = ens160.getTVOC();
      eco2 = ens160.geteCO2();
    }

    Serial.print("Temp: "); Serial.print(t_temp);
    Serial.print(" | Hum: "); Serial.print(t_hum);
    Serial.print(" | AQI: "); Serial.print(aqi);
    Serial.print(" | TVOC: "); Serial.print(tvoc);
    Serial.print(" | eCO2: "); Serial.println(eco2);
  }
}
