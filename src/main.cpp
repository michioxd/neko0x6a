#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "headers/display.h"
#include "headers/preference.h"
#include "headers/default.h"
#include "components/www.h"

#define VERSION "0.1"

Adafruit_SSD1306 display(128, 64, &Wire, -1);

AsyncWebServer server(80);
AsyncWebSocket ws("/neko0x6a");

IPAddress AP_IP(0, 0, 0, 0);
IPAddress NW_IP(0, 0, 0, 0);

bool flashLed = false;
bool clientConnected = false;

void printLog(const char *msg, bool line, bool clear)
{
    if (clear == true)
        display.clearDisplay();
    display.setCursor(0, 0);
    if (line)
        display.println(msg);
    else
        display.print(msg);
    display.display();
}

void handleClientRequest(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0;
        if (strcmp((char *)data, "establishConnection") == 0)
        {
            Serial.println(F("[WS] Connection established!"));
            clientConnected = true;
        }
        else if (strcmp((char *)data, "sendReboot") == 0)
        {
            Serial.println(F("[WS] Rebooting..."));
            ESP.restart();
        }
    }
}

void setup()
{
    Serial.begin(115200);
    preferences.begin("neko0x6a", false);
    pinMode(2, OUTPUT);
    Serial.println("============================================");
    Serial.print("    neko0x6a v");
    Serial.print(VERSION);
    Serial.println("\n    https://github.com/michioxd/neko0x6a");
    Serial.println("============================================\n\nBooting neko0x6a...");

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("[ERROR] SSD1306 allocation failed"));
        for (;;)
            ;
    }
    splash();

    printLog("", false, true);
    display.print("neko0x6a v");
    display.print(VERSION);
    Serial.println(F("[BOOTING] Starting Wi-Fi AP..."));
    display.print("\n> Wi-Fi AP... ");
    WiFi.softAP(
        preferences.getString("cfg_ap_ssid", "neko0x6a"),
        preferences.getString("cfg_ap_pass", "neko0x6a"));
    AP_IP = WiFi.softAPIP();
    display.print("OK");
    display.display();
    Serial.println(F("[BOOTING] Starting Web interface..."));
    display.print("\n> Web UI... ");
    display.display();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  if (!request->authenticate(
                    preferences.getString("cfg_username", "admin").c_str(), 
                  preferences.getString("cfg_password", "admin").c_str()))
                  {
                      return request->requestAuthentication();
                  }

                  request->send_P(200, "text/html", indexHtml); });

    server.on("/www/setting.json", HTTP_GET,
              [](AsyncWebServerRequest *request)
              {
                  if (!request->authenticate(
                          preferences.getString("cfg_username", "admin").c_str(),
                          preferences.getString("cfg_password", "admin").c_str()))
                  {
                      return request->requestAuthentication();
                  }

                  DynamicJsonDocument apiSetting(1024);
                  apiSetting["apIP"] = AP_IP;
                  apiSetting["nwIP"] = NW_IP;
                  apiSetting["apSSID"] = preferences.getString("cfg_ap_ssid", "neko0x6a");
                  apiSetting["apPass"] = preferences.getString("cfg_ap_pass", "neko0x6a");
                  apiSetting["nwSSID"] = preferences.getString("cfg_ssid", "");
                  apiSetting["nwPass"] = preferences.getString("cfg_pass", "");

                  apiSetting["username"] = preferences.getString("cfg_username", "admin");
                  apiSetting["password"] = preferences.getString("cfg_password", "admin");

                  apiSetting["version"] = VERSION;

                  String outputStr;
                  serializeJson(apiSetting, outputStr);

                  request->send(200, "application/json", outputStr);
              });

    server.on("/www/save.cgi", HTTP_POST, [](AsyncWebServerRequest *request)
              {
    // Check authentication
    if (!request->authenticate(
            preferences.getString("cfg_username", "admin").c_str(),
            preferences.getString("cfg_password", "admin").c_str())) {
        return request->requestAuthentication();
    }
    if (request->hasArg("CFG_SSID"))
    {
        preferences.putString("cfg_ap_ssid", request->arg("CFG_SSID"));
        preferences.putString("cfg_ap_pass", request->arg("CFG_PASS"));
        preferences.putString("cfg_ssid", request->arg("WF_SSID"));
        preferences.putString("cfg_pass", request->arg("WF_PASS"));
        preferences.putString("cfg_username", request->arg("CFG_USERNAME"));
        preferences.putString("cfg_password", request->arg("CFG_PASSWORD"));

        Serial.println("[WEBUI] Saved configuration!");

        request->send(200, "application/json", "OK");
    }
    else
    {
        request->send(400, "text/plain", "Invalid request.");
    } });

    server.on("/www/reboot.cgi", HTTP_POST,
              [](AsyncWebServerRequest *request)
              {
                  if (!request->authenticate(
                          preferences.getString("cfg_username", "admin").c_str(),
                          preferences.getString("cfg_password", "admin").c_str()))
                  {
                      return request->requestAuthentication();
                  }

                  Serial.println("[WEBUI] Reboot sent!");
                  request->send(200, "text/plain", "OK");
                  ESP.restart();
              });
    server.begin();

    display.print("OK\nIP: ");
    display.print(AP_IP);
    Serial.print(F("[DONE] AP IP: "));
    Serial.print(AP_IP);
    display.display();

    display.print("\n> Wi-Fi... ");
    display.display();
    if (preferences.getString("cfg_ssid", "") == "")
    {
        display.print("Error");
        display.display();
        delay(2000);
        printLog("Error: Please go to the web interface, then add Wi-Fi credentials!", true, true);
        Serial.println(F("\n[WARN] No Wi-Fi credentials are saved! Go to web interface then add them."));
        Serial.print(F("[WARN] Web interface address: http://"));
        Serial.print(AP_IP);
        display.print("IP: ");
        display.print(AP_IP);
        display.display();
    }
    else
    {
        Serial.println(F("\n[BOOTING] Connecting to Wi-Fi..."));
        WiFi.begin(preferences.getString("cfg_ssid", ""), preferences.getString("cfg_pass", ""));
        while (WiFi.status() != WL_CONNECTED)
        {
            flashLed = !flashLed;
            digitalWrite(2, flashLed == true ? HIGH : LOW);
            delay(500);
        }
        digitalWrite(2, HIGH);
        NW_IP = WiFi.localIP();
        display.print("OK\nIP: ");
        Serial.print(F("[DONE] Local IP: "));
        Serial.print(NW_IP);
        display.print(NW_IP);
        display.display();
        delay(2000);
        display.clearDisplay();
        display.display();
    }
}

void loop()
{
    ws.cleanupClients();
    if (!clientConnected)
    {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("Waiting client...");
        display.display();
    }
    else
    {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("OK");
        display.display();
    }
}
