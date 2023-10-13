#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include "headers/display.h"
#include "headers/preference.h"
#include "components/www.h"

Adafruit_SSD1306 display(128, 64, &Wire, -1);

AsyncWebServer server(80);
AsyncWebSocket ws("/neko0x6a");

IPAddress AP_IP(0, 0, 0, 0);

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

void setup()
{
    Serial.begin(115200);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ;
    }
    splash();

    printLog("", false, true);
    if (preferences.getString("neko0x6a_cfg_wifi", "") == "" || preferences.getBool("neko0x6a_cfg_disableAP", false) == false)
    {
        display.print("Wi-Fi AP... ");
        WiFi.softAP(
            preferences.getString("neko0x6a_cfg_ap_wifi", "neko0x6a"),
            preferences.getString("neko0x6a_cfg_ap_pass", "neko0x6a"));
        AP_IP = WiFi.softAPIP();
        display.print("OK");
        display.display();
    }

    display.print("\nWeb UI... ");
    display.display();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", indexHtml); });
    server.begin();
    Serial.println(indexHtml);
    display.print("OK\nIP: ");
    display.print(AP_IP);
    display.display();

    display.print("\nWi-Fi... ");
    display.display();
    if (preferences.getString("neko0x6a_cfg_wifi", "") == "")
    {
        display.print("Error");
        display.display();
        delay(2000);
        printLog("Error: Please go to the web interface, then add Wi-Fi credentials!", true, true);
        display.print("IP: ");
        display.print(AP_IP);
        display.display();
    }
}

void loop()
{
}
