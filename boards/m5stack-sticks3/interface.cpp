#include "powerSave.h"
#include <Wire.h>
#include <interface.h>

#define TFT_BRIGHT_CHANNEL 0
#define TFT_BRIGHT_Bits 8
#define TFT_BRIGHT_FREQ 5000

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio() {
    M5.begin();
    M5.Power.setExtOutput(false); // Disable 5V output to external port
    //  https://github.com/pr3y/Bruce/blob/main/media/connections/cc1101_stick_SDCard.jpg
    //  Keeps this pin high to allow working with the following pinout
    //  Keeps this pin high to allow working with the following pinout
    pinMode(3, OUTPUT); // SD Card CS
    digitalWrite(3, HIGH);
    pinMode(5, OUTPUT); // CC1101 CS
    digitalWrite(5, HIGH);
    pinMode(6, OUTPUT); // nRF24L01 CS
    digitalWrite(6, HIGH);
    pinMode(9, OUTPUT);
    digitalWrite(9, LOW); // RF jamming prevention
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH); // CS for modules

    pinMode(SEL_BTN, INPUT_PULLUP);
    pinMode(DW_BTN, INPUT_PULLUP);
    pinMode(TFT_BL, OUTPUT);
}
/***************************************************************************************
** Function name: _post_setup_gpio()
** Location: main.cpp
** Description:   second stage gpio setup to make a few functions work
***************************************************************************************/
void _post_setup_gpio() {
    // PWM backlight setup
    ledcAttach(TFT_BL, TFT_BRIGHT_FREQ, TFT_BRIGHT_Bits);
    ledcWrite(TFT_BL, 250);
}
/*********************************************************************
** Function: setBrightness
** location: settings.cpp
** set brightness value
**********************************************************************/
void _setBrightness(uint8_t brightval) {
    int dutyCycle;
    if (brightval == 100) dutyCycle = 250;
    else if (brightval == 75) dutyCycle = 130;
    else if (brightval == 50) dutyCycle = 70;
    else if (brightval == 25) dutyCycle = 20;
    else if (brightval == 0) dutyCycle = 5;
    else dutyCycle = ((brightval * 250) / 100);

    // Serial.printf("dutyCycle for bright 0-255: %d\n", dutyCycle);

    vTaskDelay(10 / portTICK_PERIOD_MS);
    if (!ledcWrite(TFT_BL, dutyCycle)) {
        // Serial.println("Failed to set brightness");
        ledcDetach(TFT_BL);
        ledcAttach(TFT_BL, TFT_BRIGHT_FREQ, TFT_BRIGHT_Bits);
        ledcWrite(TFT_BL, dutyCycle);
    }
}

/***************************************************************************************
** Function name: getBattery()
** location: display.cpp
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
int getBattery() {
    int level = M5.Power.getBatteryLevel();
    return (level < 0) ? 0 : (level >= 100) ? 100 : level;
}

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
**********************************************************************/
void InputHandler(void) {
    static unsigned long tm = 0;
    static bool selDown = false;
    static unsigned long dwFirstPress = 0;
    static bool dwWaiting = false;
    static unsigned long dwPressStart = 0;
    static bool dwDown = false;
    static bool dwLongFired = false;
    constexpr unsigned long doublePressWindowMs = 300;
    constexpr unsigned long longPressMs = 600;
    unsigned long now = millis();
    if (now - tm < 200 && !LongPress) return;
    if (!wakeUpScreen()) AnyKeyPress = true;
    else return;

    bool selPressed = (digitalRead(SEL_BTN) == BTN_ACT);
    bool dwPressed = (digitalRead(DW_BTN) == BTN_ACT);

    AnyKeyPress = selPressed || dwPressed || dwWaiting;

    if (selPressed && !selDown) {
        SelPress = true;
        tm = now;
    }
    selDown = selPressed;

    if (dwPressed && !dwDown) {
        dwPressStart = now;
        dwLongFired = false;
    }
    if (dwPressed) {
        if (!dwLongFired && (now - dwPressStart) > longPressMs) {
            PrevPress = true;
            dwLongFired = true;
            dwWaiting = false;
            tm = now;
        }
    } else if (dwDown && !dwLongFired) {
        if (dwWaiting && (now - dwFirstPress) <= doublePressWindowMs) {
            PrevPress = true;
            dwWaiting = false;
            tm = now;
        } else {
            dwWaiting = true;
            dwFirstPress = now;
        }
    }
    dwDown = dwPressed;
    if (dwWaiting && !dwPressed && (now - dwFirstPress) > doublePressWindowMs) {
        NextPress = true;
        dwWaiting = false;
        tm = now;
    }
}

/*********************************************************************
** Function: powerOff
** location: mykeyboard.cpp
** Turns off the device (or try to)
**********************************************************************/
void powerOff() { M5.Power.powerOff(); }

/*********************************************************************
** Function: checkReboot
** location: mykeyboard.cpp
** Btn logic to tornoff the device (name is odd btw)
**********************************************************************/
void checkReboot() {}
