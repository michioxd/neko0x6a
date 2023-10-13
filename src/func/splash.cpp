#include "../headers/display.h"
#include "../components/screen.h"

void splash()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.drawBitmap(0, 0, splashScreen, 128, 64, 1);
    display.setCursor(40, 50);
    display.println("neko0x6a");
    display.display();
    delay(1000);
    display.clearDisplay();
    display.drawBitmap(0, 0, splashScreen, 128, 64, 1);
    display.setCursor(8, 50);
    display.println("powered by michioxd");
    display.display();
    delay(2000);
}