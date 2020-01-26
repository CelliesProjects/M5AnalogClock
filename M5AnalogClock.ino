#include <WiFi.h>
#include <TFTShape.h>
#include "Free_Fonts.h"

#define BACKLIGHT     32
#define TFT_GREY      0xa514

const char * WIFI_NETWORK  = "";
const char * WIFI_PASSWORD = "";

const char * TIME_ZONE   = "CET-1CEST,M3.5.0/2,M10.5.0/3";  //https://remotemonitoringsystems.ca/time-zone-abbreviations.php
const char * TIME_SERVER = "nl.pool.ntp.org";

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite  clockSprite = TFT_eSprite(&tft);

void drawClock(const uint8_t hour, const uint8_t minute, const uint8_t second, const char * title, const char * footer) {
  const uint16_t middleH = clockSprite.width() / 2;
  const uint16_t middleV = clockSprite.height() / 2;
  clockSprite.fillScreen(TFT_NAVY);
  clockSprite.setTextColor(TFT_GREEN);
  clockSprite.setFreeFont(FS18);
  clockSprite.drawString(title, middleH, 10);
  clockSprite.setFreeFont(FS12);
  clockSprite.drawString(footer, middleH, clockSprite.height() - 14); //because font size == 12
  TFTShape dot = TFTShapeBuilder::buildNgon(3, 12); dot.setScale(.5, 1);
  TFTShape face = TFTShapeBuilder::buildNgon(12, 100);
  // clockSprite-face
  face.draw(&clockSprite, middleH, middleV, dot, TFT_WHITE);
  //hour-hand
  TFTShape hand = TFTShapeBuilder::buildNgon(5, 60);
  uint16_t rotation = map(hour, 0, 12, 0, 360);
  uint16_t extra = map(minute, 0, 60, 0, 30);  // need to add the minutes as an extra angle added between 0 - 30 degrees
  rotation += extra;
  hand.setScale(.08, .8); hand.setOffset(0, -30); hand.setRotation(rotation);
  hand.fill(&clockSprite, middleH, middleV, TFT_GREY);
  //minutes-hand
  rotation = map(minute, 0, 60, 0, 360);
  hand.setScale(.08, 1); hand.setRotation(rotation);
  hand.fill(&clockSprite, middleH, middleV, TFT_LIGHTGREY);
  //seconds-hand
  rotation = map(second, 0, 60, 0, 360);
  hand.setScale(.03, 1); hand.setRotation(rotation);
  hand.fill(&clockSprite, middleH, middleV, TFT_RED);
  dot.fill(&clockSprite, middleH, middleV, TFT_RED);
  dot.setScale(.4);
  dot.fill(&clockSprite, middleH, middleV, TFT_DARKGREY);
  clockSprite.pushSprite(0, 0);
}
void setup() {
  tft.init();
  tft.setRotation(0);
  tft.setFreeFont(FS12);
  tft.setTextDatum(CC_DATUM); // CC = Center Center

  ledcAttachPin(BACKLIGHT, 0);
  ledcSetup(0, 1300, 16);
  ledcWrite(0, 0xFFFF / 20);

  clockSprite.setColorDepth(8);
  if (!clockSprite.createSprite(240, 320)) {
    tft.drawString("No memory for sprite", 120, 160);
    while (1) delay(100);
  }
  clockSprite.setTextDatum(TC_DATUM); // TC = Top Center

  if (!strlen(WIFI_NETWORK)) {
    tft.drawString("No WiFi defined", 120, 160);
    while (1) delay(100);
  }
  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);

  char buff[40];
  snprintf(buff, sizeof(buff), "Connecting to %s", WIFI_NETWORK);
  tft.drawString(buff, 120, 160);

  while (!WiFi.isConnected())
    delay(10);

  configTzTime(TIME_ZONE, TIME_SERVER);
  struct tm timeinfo = {0};
  while (!getLocalTime(&timeinfo, 0))
    vTaskDelay(10 / portTICK_PERIOD_MS);
}

void loop() {
  static time_t savedTime;
  time_t now = time(NULL);
  if (now != savedTime) {
    struct tm localTime;
    localtime_r(&now, &localTime);
    char timestr[40];
    strftime(timestr, sizeof(timestr), "%b %e %Y", &localTime);
    drawClock(localTime.tm_hour, localTime.tm_min, localTime.tm_sec, timestr, "M5Stack - sntp clock");
    savedTime = now;
  }
  delay(10);
}
