#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <FileData.h>
#include <LittleFS.h>
#include "Dashboard.h"
#include "SpeedSensor.h"
#include "FuelSensorAnalog.h"
#include "FuelSensorDigital.h"

// === CONST ===
const int DASHBOARD_MODE_WORK = 0;
const int DASHBOARD_MODE_TEST = 1;
const int TIRE_120_60_R17 = 0;
const int TIRE_120_70_R17 = 1;
const int FUEL_SENSOR_NONE = 0;
const int FUEL_SENSOR_ANALOG = 1;
const int FUEL_SENSOR_DIGITAL = 2;

// === CONFIG ===
// 6-11 unused
// 1 TX
// 2 Buildin LED
// 3 RX
const int PIN_DASHBOARD_SPEED = 4;
const int PIN_DASHBOARD_FUEL1 = 16;
const int PIN_DASHBOARD_FUEL2 = 14;
const int PIN_DASHBOARD_FUEL3 = 12;
const int PIN_DASHBOARD_FUEL4 = 13;
const int PIN_DASHBOARD_FUEL5 = 15;
const int PIN_SPEED_SENSOR = 3;
const int PIN_FUEL_SENSOR_ANALOG_OUT = 5;
const int PIN_FUEL_SENSOR_ANALOG_IN = A0;
const int PIN_FUEL_SENSOR_DIGITAL = 1;
const int PIN_LED = 2;

const char* AP_SSID = "SV400";
const int AP_TIMEOUT = 60000; // 60s
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

struct Config {
  int dashboardMode = DASHBOARD_MODE_WORK;
  int dashboardTestSpeed = 0;
  int dashboardTestFuel = 0;
  int speedSensorTire = TIRE_120_60_R17;
  int fuelSensorType = FUEL_SENSOR_NONE;
  int fuelSensorAnalogThresholdLow = 500;
  int fuelSensorAnalogThresholdHigh = 500;
  int fuelSensorAnalogTarget = 20;
  int fuelSensorDigitalFullValue = 20;
  int fuelSensorDigitalEmptyValue = 10;
};
Config config;
FileData configData(&LittleFS, "/config", 'B', &config, sizeof(config));

// === VARIABLES ===
Dashboard dashboard(PIN_DASHBOARD_SPEED, PIN_DASHBOARD_FUEL1, PIN_DASHBOARD_FUEL2, PIN_DASHBOARD_FUEL3, PIN_DASHBOARD_FUEL4, PIN_DASHBOARD_FUEL5);
SpeedSensor speedSensor(PIN_SPEED_SENSOR);
FuelSensorAnalog fuelSensorAnalog(PIN_FUEL_SENSOR_ANALOG_OUT, PIN_FUEL_SENSOR_ANALOG_IN);
FuelSensorDigital fuelSensorDigital(PIN_FUEL_SENSOR_DIGITAL);

ESP8266WebServer server(80);
String serverAlert = "";

unsigned long timeLED;
unsigned long timeAP;

void setup() {
  // Buildin LED
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  // Read config
  LittleFS.begin();
  configData.read();
  applyConfig();

  // Soft AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID);
  WiFi.softAPConfig(local_ip, gateway, subnet);

  // Web server
  server.on("/", serverHandleIndex);
  server.on("/dashboard", serverHandleDashboard);
  server.on("/speed", serverHandleSpeed);
  server.on("/fuel", serverHandleFuel);
  server.on("/state", serverHandleState);
  server.begin();

  // Time
  timeLED = timeAP = millis();
}

void loop() {
  unsigned long time = millis();

  // Dashboard values
  if (config.dashboardMode == DASHBOARD_MODE_WORK) {
    dashboard.setSpeed(speedSensor.getValue());
    float fuel = 0;
      fuel = 0.5;
    if (config.fuelSensorType == FUEL_SENSOR_ANALOG) {
      fuel = fuelSensorAnalog.getValue();
    } else if (config.fuelSensorType == FUEL_SENSOR_DIGITAL) {
      fuel = fuelSensorDigital.getValue();
    }
    dashboard.setFuel(fuel);
  } else if (config.dashboardMode == DASHBOARD_MODE_TEST) {
    dashboard.setSpeed(config.dashboardTestSpeed);
    dashboard.setFuel(float(config.dashboardTestFuel) / 100);
  }

  // Soft AP management
  if (WiFi.getMode() == WIFI_AP) {
    if (WiFi.softAPgetStationNum() > 0) {
      timeAP = time;
    } else {
      if (timeAP + AP_TIMEOUT < time) {
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_OFF);
      }
    }
  }

  // Buildin LED
  if (timeLED + 50 < time) {
    if (WiFi.getMode() != WIFI_AP) {
      digitalWrite(PIN_LED, HIGH);
    }
  } else {
    digitalWrite(PIN_LED, LOW);
  }
  if (timeLED + 3000 < time) {
    timeLED = time;
  }

  // Process handlers
  dashboard.loop();
  speedSensor.loop();
  fuelSensorAnalog.loop();
  fuelSensorDigital.loop();
  server.handleClient();
  configData.tick();
}

void applyConfig() {
  // Speed sensor
  if (config.speedSensorTire == TIRE_120_60_R17) {
    speedSensor.setTireWidth(120);
    speedSensor.setTireAspectRatio(60);
    speedSensor.setTireRimSize(17);
  } else if (config.speedSensorTire == TIRE_120_70_R17) {
    speedSensor.setTireWidth(120);
    speedSensor.setTireAspectRatio(70);
    speedSensor.setTireRimSize(17);
  }

  // Fuel sensor analog
  fuelSensorAnalog.setThresholdLow(config.fuelSensorAnalogThresholdLow);
  fuelSensorAnalog.setThresholdHigh(config.fuelSensorAnalogThresholdHigh);
  fuelSensorAnalog.setTarget(float(config.fuelSensorAnalogTarget) / 100);

  // Fuel sensor digital
  fuelSensorDigital.setFullValue(config.fuelSensorDigitalFullValue);
  fuelSensorDigital.setEmptyValue(config.fuelSensorDigitalEmptyValue);
}

const char* layoutMain PROGMEM = R"=====(
  <!doctype html>
  <html lang="en">
    <head>
      <meta charset="utf-8">
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <title>{title}</title>
      <style>
        * {
          box-sizing: border-box;
          font-family: sans-serif;
          font-size: 14px;        
        }
        body {
          max-width: 400px;
        }
        .btn, button {
          background: #000;
          border: 1px #000 solid;
          border-radius: 4px;
          color: #fff;
          cursor: pointer;
          display: inline-block;
          min-width: 100px;
          padding: 12px;
          text-align: center;
          text-decoration: none;
        }
        input[type=text], select {
          background: #fff;
          border: 1px #999 solid;
          border-radius: 4px;
          color: #333;
          padding: 8px;
          width: 100%;
        }
        input[type=text][disabled] {
          background: #ddd;
        }
        select {
          padding: 8px 4px;
        }
        table {
          width: 100%;
        }
        .h {
          font-size: 18px;
          font-weight: bold;
          padding: 8px 0;
        }
        .mv {
          margin: 8px 0;
        }
        td {
          white-space: nowrap;
        }
        .w100 {
          width: 100%;
        }
        .heading {
          align-items: center;
          color: #333;
          display: flex;
          text-decoration: none;
        }
        svg {
          display: block;
        }
        .title {
          font-size: 42px;
          font-weight: 300;
          line-height: 1;
          margin-left: 24px;
        }
        .alert {
          background: #ddffe3;
          border: 1px solid #3b8247;
          border-radius: 4px;
          color: #3b8247;
          padding: 8px;
        }
        .hidden {
          display: none;
        }
      </style>
    </head>
    <body>
      <a href="/" class="heading">
        <div class="logo">
          <svg x="0px" y="0px" width="50" height="50" viewBox="0 0 50 50"><linearGradient id="fill" x1="40%" y1="40%" x2="70%" 
          y2="70%"><stop offset="0" stop-color="#aaa" /><stop offset="1" 
          stop-color="#555" /></linearGradient><path d="M 36.277344 
          23.632813 C 36.769531 23.671875 37.269531 23.691406 37.785156 23.691406 C 
          40.976563 23.691406 45.308594 22.953125 47.8125 19.4375 C 48.132813 18.988281 
          48.03125 18.371094 47.585938 18.046875 L 25.660156 2.1875 C 25.273438 1.90625 
          24.738281 1.941406 24.390625 2.273438 C 20.722656 5.734375 7.359375 12.917969 
          2.710938 14.519531 C 2.359375 14.636719 2.105469 14.941406 2.046875 15.304688 
          C 1.996094 15.636719 2.117188 15.964844 2.359375 16.1875 L 30.417969 
          36.070313 L 28.632813 37.257813 L 12.960938 26.152344 C 12.484375 26.097656 
          11.992188 26.066406 11.488281 26.066406 C 8.160156 26.066406 5.078125 
          27.339844 2.328125 29.84375 C 2.105469 30.042969 1.988281 30.332031 2 
          30.628906 C 2.015625 30.925781 2.160156 31.203125 2.398438 31.382813 L 
          24.289063 47.796875 C 24.46875 47.933594 24.679688 48 24.890625 48 C 
          25.128906 48 25.367188 47.910156 25.558594 47.742188 C 29.433594 44.246094 
          42.121094 36.667969 46.449219 35.265625 C 46.804688 35.148438 47.066406 
          34.847656 47.128906 34.480469 C 47.1875 34.109375 47.039063 33.738281 
          46.742188 33.515625 L 46.410156 33.265625 L 19.4375 14.152344 L 21.222656 
          12.964844 Z" fill="url(#fill)"></path></svg>
        </div>
        <div class="title">SV400</div>
      </a>
      {content}
    </body>
  </html>
)=====";

const char* templateIndex PROGMEM = R"=====(
  <div>
    <div class="mv">
      <a href="/dashboard" class="btn w100">Приборная панель</a>
    </div>
    <div class="mv">
      <a href="/speed" class="btn w100">Датчик скорости</a>
    </div>
    <div class="mv">
      <a href="/fuel" class="btn w100">Датчик топлива</a>
    </div>
    <div class="mv">
      <p>https://github.com/dkhlystov/sv400</p>
    </div>
  </div>
)=====";

const char* templateDashboard PROGMEM = R"=====(
  {alert}
  <form method="post">
    <table><tbody>
      <tr><td colspan="2" class="h">Приборная панель</td></tr>
      <tr><td>Режим:</td><td><select name="mode" id="mode" onchange="modeChange()">{mode}</select></td></tr>
      <tr class="mode_work"><td>Скорость (км/ч):</td><td><input type="text" name="speed" id="speed" value="{speed}" disabled></td></tr>
      <tr class="mode_work"><td>Топливо (%):</td><td><input type="text" name="fuel" id="fuel" value="{fuel}" disabled></td></tr>
      <tr class="mode_test"><td>Скорость (км/ч):</td><td><input type="text" name="test_speed" id="speed" value="{test_speed}"></td></tr>
      <tr class="mode_test"><td>Топливо (%):</td><td><input type="text" name="test_fuel" id="fuel" value="{test_fuel}"></td></tr>
    </tbody></table>
    <div class="mv">
      <button type="submit">Сохранить</button>
      <a href="/" class="btn">Отмена</a>
    </div>
  </form>
  <script type="text/javascript">
    function modeChange() {
      const v = document.getElementById('mode').value;
      let work = document.querySelectorAll('.mode_work');
      let test = document.querySelectorAll('.mode_test');
      if (v == 0) {
        work.forEach(el => {el.classList.remove('hidden')});
        test.forEach(el => {el.classList.add('hidden')});
      } else {
        work.forEach(el => {el.classList.add('hidden')});
        test.forEach(el => {el.classList.remove('hidden')});
      }
    }
    modeChange();
    setInterval(function() {
      const x = new XMLHttpRequest();
      x.open("GET", "/state", false);
      x.send(null);
      const state = JSON.parse(x.responseText);
      document.getElementById('speed').value = state.dashboardSpeed;
      document.getElementById('fuel').value = state.dashboardFuel;
      document.getElementById('digital_raw_data').value = state.fuelSensorDigitalRawData;
    }, 500);
  </script>
)=====";

const char* templateSpeed PROGMEM = R"=====(
  {alert}
  <form method="post">
    <table><tbody>
      <tr><td colspan="2" class="h">Датчик скорости</td></tr>
      <tr><td>Текущая скорость:</td><td><input type="text" id="speed" value="{speed}" disabled></td></tr>
      <tr><td>Размер шины:</td><td><select name="tire">{tire}</select></td></tr>
    </tbody></table>
    <div class="mv">
      <button type="submit">Сохранить</button>
      <a href="/" class="btn">Отмена</a>
    </div>
  </form>
  <script type="text/javascript">
    setInterval(function() {
      const x = new XMLHttpRequest();
      x.open("GET", "/state", false);
      x.send(null);
      const state = JSON.parse(x.responseText);
      document.getElementById('speed').value = state.speedSensorValue;
    }, 500);
  </script>
)=====";

const char* templateFuel PROGMEM = R"=====(
  {alert}
  <form method="post">
    <table><tbody>
      <tr><td colspan="2" class="h">Датчик топлива</td></tr>
      <tr><td>Тип:</td><td><select name="type">{type}</select></td></tr>
      <tr><td colspan="2" class="h">Аналоговый</td></tr>
      <tr><td>Текущие показания:</td><td><input type="text" id="analog_raw_data" value="{analog_raw_data}" disabled></td></tr>
      <tr><td>Верхний порог:</td><td><input type="text" name="analog_threshold_high" value="{analog_threshold_high}"></td></tr>
      <tr><td>Нижний порог:</td><td><input type="text" name="analog_threshold_low" value="{analog_threshold_low}"></td></tr>
      <tr><td>Текущее состояние:</td><td><input type="text" id="analog_is_on" value="{analog_is_on}" disabled></td></tr>
      <tr><td>Низкий уровень (%):</td><td><input type="text" name="analog_target" value="{analog_target}"></td></tr>
      <tr><td colspan="2" class="h">Цифровой</td></tr>
      <tr><td>Текущие показания:</td><td><input type="text" id="digital_raw_data" value="{digital_raw_data}" disabled></td></tr>
      <tr><td>Полный бак:</td><td><input type="text" name="digital_full" value="{digital_full}"></td></tr>
      <tr><td>Пустой бак:</td><td><input type="text" name="digital_empty" value="{digital_empty}"></td></tr>
    </tbody></table>
    <div class="mv">
      <button type="submit">Сохранить</button>
      <a href="/" class="btn">Отмена</a>
    </div>
  </form>
  <script type="text/javascript">
    setInterval(function() {
      const x = new XMLHttpRequest();
      x.open("GET", "/state", false);
      x.send(null);
      const state = JSON.parse(x.responseText);
      document.getElementById('analog_raw_data').value = state.fuelSensorAnalogRawData;
      document.getElementById('analog_is_on').value = state.fuelSensorAnalogIsOn ? "ON" : "OFF";
      document.getElementById('digital_raw_data').value = state.fuelSensorDigitalRawData;
    }, 500);
  </script>
)=====";

String htmlOption(const String &value, const String &content, const boolean &selected) {
  String s = "";
  if (selected) {
    s = " selected";
  }
  return "<option value=\"" + value + "\"" + s + ">" + content + "</option>";
}

void serverHandleIndex() {
  String layout = String(layoutMain);
  layout.replace("{title}", "SV400");
  layout.replace("{content}", templateIndex);

  server.send(200, "text/html", layout);
}

void serverHandleDashboard() {
  if (server.hasArg("mode")) {
    config.dashboardMode = server.arg("mode").toInt();
    config.dashboardTestSpeed = server.arg("test_speed").toInt();
    config.dashboardTestFuel = server.arg("test_fuel").toInt();
    configData.update();
    applyConfig();

    serverAlert = "Изменения успешно сохранены.";
    server.sendHeader("Location", "/dashboard");
    server.send(302);
    return;
  }

  String content = String(templateDashboard);
  String options = htmlOption(String(DASHBOARD_MODE_WORK), "Работа", config.dashboardMode == DASHBOARD_MODE_WORK);
  options += htmlOption(String(DASHBOARD_MODE_TEST), "Тестирование", config.dashboardMode == DASHBOARD_MODE_TEST);
  content.replace("{mode}", options);
  content.replace("{speed}", String(dashboard.getSpeed()));
  content.replace("{fuel}", String(dashboard.getFuel() * 100, 0));
  content.replace("{test_speed}", String(config.dashboardTestSpeed));
  content.replace("{test_fuel}", String(config.dashboardTestFuel));
  if (serverAlert != "") {
    serverAlert = "<div class=\"alert\">" + serverAlert + "</div>";
  }
  content.replace("{alert}", serverAlert);
  serverAlert = "";

  String layout = String(layoutMain);
  layout.replace("{title}", "SV400 - Приборная панель");
  layout.replace("{content}", content);

  server.send(200, "text/html", layout);
}

void serverHandleSpeed() {
  if (server.hasArg("tire")) {
    config.speedSensorTire = server.arg("tire").toInt();
    configData.update();
    applyConfig();

    serverAlert = "Изменения успешно сохранены.";
    server.sendHeader("Location", "/speed");
    server.send(302);
    return;
  }

  String content = String(templateSpeed);
  content.replace("{speed}", String(speedSensor.getValue()));
  String options = htmlOption(String(TIRE_120_60_R17), "120/60 R17", config.speedSensorTire == TIRE_120_60_R17);
  options += htmlOption(String(TIRE_120_70_R17), "120/70 R17", config.speedSensorTire == TIRE_120_70_R17);
  content.replace("{tire}", options);
  if (serverAlert != "") {
    serverAlert = "<div class=\"alert\">" + serverAlert + "</div>";
  }
  content.replace("{alert}", serverAlert);
  serverAlert = "";

  String layout = String(layoutMain);
  layout.replace("{title}", "SV400 - Датчик скорости");
  layout.replace("{content}", content);

  server.send(200, "text/html", layout);
}

void serverHandleFuel() {
  if (server.hasArg("type")) {
    config.fuelSensorType = server.arg("type").toInt();
    config.fuelSensorAnalogThresholdLow = server.arg("analog_threshold_low").toInt();
    config.fuelSensorAnalogThresholdHigh = server.arg("analog_threshold_high").toInt();
    config.fuelSensorAnalogTarget = server.arg("analog_target").toInt();
    config.fuelSensorDigitalFullValue = server.arg("digital_full").toInt();
    config.fuelSensorDigitalEmptyValue = server.arg("digital_empty").toInt();
    configData.update();
    applyConfig();

    serverAlert = "Изменения успешно сохранены.";
    server.sendHeader("Location", "/fuel");
    server.send(302);
    return;
  }

  String content = String(templateFuel);
  String options = htmlOption(String(FUEL_SENSOR_NONE), "Нет", config.fuelSensorType == FUEL_SENSOR_NONE);
  options += htmlOption(String(FUEL_SENSOR_ANALOG), "Аналоговый", config.fuelSensorType == FUEL_SENSOR_ANALOG);
  options += htmlOption(String(FUEL_SENSOR_DIGITAL), "Цифровой", config.fuelSensorType == FUEL_SENSOR_DIGITAL);
  content.replace("{type}", options);
  content.replace("{analog_raw_data}", String(fuelSensorAnalog.getRawData()));
  content.replace("{analog_threshold_low}", String(config.fuelSensorAnalogThresholdLow));
  content.replace("{analog_threshold_high}", String(config.fuelSensorAnalogThresholdHigh));
  content.replace("{analog_is_on}", fuelSensorAnalog.isOn() ? "ON" : "OFF");
  content.replace("{analog_target}", String(config.fuelSensorAnalogTarget));
  content.replace("{digital_raw_data}", String(fuelSensorDigital.getRawData()));
  content.replace("{digital_full}", String(config.fuelSensorDigitalFullValue));
  content.replace("{digital_empty}", String(config.fuelSensorDigitalEmptyValue));
  if (serverAlert != "") {
    serverAlert = "<div class=\"alert\">" + serverAlert + "</div>";
  }
  content.replace("{alert}", serverAlert);
  serverAlert = "";

  String layout = String(layoutMain);
  layout.replace("{title}", "SV400 - Датчик топлива");
  layout.replace("{content}", content);

  server.send(200, "text/html", layout);
}

void serverHandleState() {
  JsonDocument json;
  json["dashboardSpeed"] = dashboard.getSpeed();
  json["dashboardFuel"] = round(dashboard.getFuel() * 100);
  json["speedSensorValue"] = speedSensor.getValue();
  json["fuelSensorAnalogRawData"] = fuelSensorAnalog.getRawData();
  json["fuelSensorAnalogIsOn"] = fuelSensorAnalog.isOn();
  json["fuelSensorDigitalRawData"] = fuelSensorDigital.getRawData();
  
  String content;
  serializeJson(json, content);
  server.send(200, "application/json", content);
}
