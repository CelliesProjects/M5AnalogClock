#include <WiFi.h>
#include <TFT_eSPI.h>
#include <TFTShape.h>
#include "Free_Fonts.h"

#define BACKLIGHT     32
#define TFT_GREY      0xa514

const char * WIFI_NETWORK = "";
const char * WIFI_PASSWORD = "";

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite tft2 = TFT_eSprite(&tft);

void clock1( const uint8_t hour, const uint8_t minute, const uint8_t second, const char * title, const char * footer ) {
  tft2.fillScreen( TFT_NAVY );
  tft2.setTextDatum( TC_DATUM ); // Centre text on x,y position
  tft2.setTextColor( TFT_GREEN );
  tft2.setFreeFont( FS18 );
  tft2.drawString( title, 120, 10 );
  tft2.setFreeFont( FS12 );
  tft2.drawString( footer, tft.width() / 2, 300 );
  TFTShape dot = TFTShapeBuilder::buildNgon(3, 12); dot.setScale(.5, 1);
  TFTShape face = TFTShapeBuilder::buildNgon(12, 100);
  //clock-face
  face.draw(&tft2, 120, 160, dot, TFT_WHITE);
  //hour-hand
  TFTShape hand = TFTShapeBuilder::buildNgon(5, 60);
  uint16_t rotation = map( hour, 0, 12, 0, 360 );//need to add the minutes as an extra angle added between 0 - 30 degrees
  uint16_t extra = map( minute, 0, 60, 0, 30 );
  rotation += extra;
  hand.setScale(.08, .8); hand.setOffset(0, -30); hand.setRotation(rotation);
  hand.fill(&tft2, 120, 160, TFT_GREY);
  //minutes-hand
  rotation = map( minute, 0, 60, 0, 360 );
  hand.setScale(.08, 1); hand.setRotation(rotation);
  hand.fill(&tft2, 120, 160, TFT_LIGHTGREY);
  //seconds-hand
  rotation = map( second, 0, 60, 0, 360 );
  hand.setScale(.03, 1); hand.setRotation(rotation);
  hand.fill(&tft2, 120, 160, TFT_RED);
  dot.fill(&tft2, 120, 160, TFT_RED);
  dot.setScale(.4);
  dot.fill(&tft2, 120, 160, TFT_DARKGREY);
  tft2.pushSprite(0, 0);
}
void setup() {
  if ( WIFI_NETWORK == "" )
    WiFi.begin();
  else
    WiFi.begin( WIFI_NETWORK, WIFI_PASSWORD );

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLUE);
  tft2.createSprite(240, 320);

  ledcAttachPin( BACKLIGHT, 0);
  ledcSetup( 0, 1300, 16 );
  ledcWrite( 0, 0xFFFF  / 20  );

  while ( !WiFi.isConnected() ) {
    delay( 10 );
  }
  configTzTime( "CET-1CEST,M3.5.0/2,M10.5.0/3", "nl.pool.ntp.org" );
  struct tm timeinfo = {0};
  while ( !getLocalTime( &timeinfo, 0 ) )
    vTaskDelay( 10 / portTICK_PERIOD_MS );
}

void loop() {
  static time_t savedTime;
  time_t now = time(NULL);
  if ( now != savedTime ) {
    struct tm localTime;
    localtime_r( &now, &localTime );
    char timestr[40];
    strftime( timestr, sizeof( timestr ), "%b %e %Y", &localTime );
    clock1( localTime.tm_hour, localTime.tm_min, localTime.tm_sec, timestr, "M5Stack sntp clock" );
    savedTime = now;
  }
  delay( 10 );
}
