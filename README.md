# AirQuality-Nexus
AirQuality-Nexus is a research-focused IoT project designed to analyze indoor air quality and detect odour patterns using smart environmental sensors.

# 🌫️ IoT Air Quality Monitoring System

A **real-time environmental monitoring system** built using **ESP32**, designed to measure indoor air quality parameters and visualize them through a **modern web dashboard**.

The system measures:

- 🌡 **Temperature**
- 💧 **Humidity**
- 🌫 **Total Volatile Organic Compounds (TVOC)**
- 🫁 **Equivalent CO₂ (eCO₂)**
- 📊 **Air Quality Index (AQI)**

Data is collected using environmental sensors and streamed via **WiFi to a live dashboard**.

---

# 📌 Project Goal

Indoor air pollution can significantly affect human health and productivity. Poor ventilation and chemical pollutants can increase concentrations of:

- Volatile organic compounds
- Carbon dioxide
- Humidity imbalance

The goal of this project is to create a **low-cost IoT air quality monitor** capable of:

- Real-time environmental sensing
- Visualizing air quality data
- Detecting pollution levels indoors
- Enabling smart ventilation decisions

---

# 🧠 System Architecture
```
Environment Air
│
▼
ENS160 Gas Sensor ──► Air Quality Data
│
▼
AHT20 Sensor ──► Temperature + Humidity
│
▼
ESP32 Microcontroller
│
├── Data Processing
├── AQI Calculation
├── Sensor Calibration
│
▼
WiFi Access Point
│
▼
Web Dashboard
```

---

# 🔬 Hardware Components

| Component | Purpose |
|-----------|--------|
| ESP32 | Microcontroller + WiFi |
| ENS160 | Air quality gas sensor |
| AHT20 | Temperature & humidity sensor |
| LED Indicator | System status |
| Power Supply | USB or battery |

---

# ⚙️ How the System Works

The system continuously reads environmental parameters from two sensors:

### AHT20 Sensor

Measures:

- Ambient temperature
- Relative humidity

### ENS160 Sensor

Measures:

- Air Quality Index
- Total Volatile Organic Compounds
- Equivalent CO₂ concentration

The ESP32 processes sensor readings and sends them to a **web interface hosted locally**.

Users connect to the ESP32 hotspot to monitor the environment in real time.

---

# 📡 WiFi Web Dashboard

The ESP32 creates a **WiFi Access Point**.
SSID: ESP32_Air_Glass
Password: password123

After connecting, open:
http://192.168.4.1

The dashboard displays:

- Temperature
- Humidity
- AQI
- TVOC
- eCO₂

A real-time graph visualizes:

- TVOC trend
- CO₂ trend

Data refresh rate:
3 seconds

---

# 🌡 Temperature Measurement

Temperature is measured using the **AHT20 digital environmental sensor**.

Typical operating range:
-40°C to 85°C

Temperature helps in evaluating environmental comfort and sensor compensation.

---

# 💧 Humidity Measurement

Relative humidity represents the amount of water vapor present in air.
Relative Humidity = (Actual Water Vapor / Maximum Water Vapor) × 100

Typical indoor comfort level:
40% – 60%

---

# 🌫 Total Volatile Organic Compounds (TVOC)

TVOCs are gases emitted from:

- Paints
- Furniture
- Cleaning products
- Electronics

TVOC is measured in:

---

# 🌫 Total Volatile Organic Compounds (TVOC)

TVOCs are gases emitted from:

- Paints
- Furniture
- Cleaning products
- Electronics

TVOC is measured in:
ppb (parts per billion)

Typical ranges:

| TVOC Level | Air Quality |
|-----------|------------|
| 0 – 200 ppb | Excellent |
| 200 – 500 ppb | Good |
| 500 – 1000 ppb | Moderate |
| >1000 ppb | Poor |

---

# 🫁 Equivalent CO₂ (eCO₂)

The ENS160 sensor estimates **equivalent CO₂ concentration** based on VOC levels.

Measured in:
ppm (parts per million)

Typical indoor levels:

| CO₂ Level | Interpretation |
|----------|---------------|
| 400 ppm | Fresh air |
| 400–1000 ppm | Good ventilation |
| 1000–2000 ppm | Moderate air |
| >2000 ppm | Poor ventilation |

---

# 📊 Air Quality Index (AQI)

AQI is a simplified air quality score produced by the ENS160.

Range:
1 = Excellent
2 = Good
3 = Moderate
4 = Poor
5 = Very Poor

---

# 🔄 Sensor Data Flow
```
Sensors
│
├── AHT20
│ ├── Temperature
│ └── Humidity
│
└── ENS160
├── AQI
├── TVOC
└── eCO₂
│
▼
ESP32
│
▼
JSON API
│
▼
Web Dashboard
```

---

# 🌐 REST Data API

The ESP32 provides sensor data through a JSON endpoint.
GET /data

Example response:

```json
{
"temp": 27.4,
"hum": 52.8,
"aqi": 2,
"tvoc": 48,
"eco2": 435
}
```

# 📈 Real-Time Visualization

The web dashboard includes a live chart that shows trends of:

TVOC concentration

CO₂ concentration

The graph continuously updates every 3 seconds, providing a quick overview of air quality changes.

# 🧮 Sensor Communication

Both sensors communicate using the I²C protocol.
ESP32 SDA → GPIO 19
ESP32 SCL → GPIO 20

I²C clock speed:
100 kHz

This ensures stable communication with multiple sensors.

# 🚀 Features

✔ Real-time environmental monitoring
✔ Modern glass-style web dashboard
✔ WiFi hotspot connectivity
✔ Live data graph visualization
✔ REST API for data access
✔ Low power IoT design
✔ Portable indoor air monitoring

# 🔬 Potential Applications

Smart homes
Indoor air quality monitoring
Office environment monitoring
Laboratory air analysis
HVAC system optimization
IoT environmental sensing



# 📜 License

This project is open-source and intended for educational and research purposes.
