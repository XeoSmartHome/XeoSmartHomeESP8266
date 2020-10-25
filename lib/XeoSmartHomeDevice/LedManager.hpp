#pragma once

#include <Arduino.h>
#include <FastLED.h>
#include <TaskScheduler.h>

#define LED_PIN D8


enum ColorCode {
    SETTINS_MODE,
    WITI_DISCONNECT
};


class LedManager{
public:
    LedManager();
    ~LedManager();
    Task * getTask();
    bool setColor(CRGB color);
    void setColorCode(ColorCode color_code);
private:
    CRGB _leds[1];
    Task _task;
    void _setColor(CRGB color);
};


LedManager :: LedManager() {
    FastLED.addLeds<NEOPIXEL, D8>(this->_leds, sizeof(this->_leds) / sizeof(CRGB));
}


LedManager :: ~LedManager(){

}


Task * LedManager :: getTask(){
    return & this->_task;
}


bool LedManager :: setColor(CRGB color){

}


void LedManager :: setColorCode(ColorCode color_code){

}


void LedManager :: _setColor(CRGB color){
    this->_leds[0] = color;
    FastLED.show();
}


extern LedManager ledManager;

void main(){
    Scheduler _taskScheduler;
    ledManager.setColor(CRGB::Red);
    _taskScheduler.addTask(*ledManager.getTask());
}