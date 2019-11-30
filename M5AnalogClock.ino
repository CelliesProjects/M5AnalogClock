#include <WiFi.h>
#include <TFTShape.h>
#include "Free_Fonts.h"

#define BACKLIGHT     32
#define TFT_GREY      0xa514

const char * WIFI_NETWORK = "";
const char * WIFI_PASSWORD = "";

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite  clockSprite = TFT_eSprite(&tft);

void drawClock( const uint8_t hour, const uint8_t minute, const uint8_t second, const char * title, const char * footer ) {
  const uint16_t middleH = clockSprite.width() / 2;
  const uint16_t middleV = clockSprite.height() / 2;
  clockSprite.fillScreen( TFT_NAVY );
  clockSprite.setTextColor( TFT_GREEN );
  clockSprite.setFreeFont( FS18 );
  clockSprite.drawString( title, middleH, 10 );
  clockSprite.setFreeFont( FS12 );
  clockSprite.drawString( footer, middleH, clockSprite.height() - 14 ); //because font size == 12
  TFTShape dot = TFTShapeBuilder::buildNgon(3, 12); dot.setScale(.5, 1);
  TFTShape face = TFTShapeBuilder::buildNgon(12, 100);
  // clockSprite-face
  face.draw(&clockSprite, middleH, middleV, dot, TFT_WHITE);
  //hour-hand
  TFTShape hand = TFTShapeBuilder::buildNgon(5, 60);
  uint16_t rotation = map( hour, 0, 12, 0, 360 );//need to add the minutes as an extra angle added between 0 - 30 degrees
  uint16_t extra = map( minute, 0, 60, 0, 30 );
  rotation += extra;
  hand.setScale(.08, .8); hand.setOffset(0, -30); hand.setRotation(rotation);
  hand.fill(&clockSprite, middleH, middleV, TFT_GREY);
  //minutes-hand
  rotation = map( minute, 0, 60, 0, 360 );
  hand.setScale(.08, 1); hand.setRotation(rotation);
  hand.fill(&clockSprite, middleH, middleV, TFT_LIGHTGREY);
  //seconds-hand
  rotation = map( second, 0, 60, 0, 360 );
  hand.setScale(.03, 1); hand.setRotation(rotation);
  hand.fill(&clockSprite, middleH, middleV, TFT_RED);
  dot.fill(&clockSprite, middleH, middleV, TFT_RED);
  dot.setScale(.4);
  dot.fill(&clockSprite, middleH, middleV, TFT_DARKGREY);
  clockSprite.pushSprite(0, 0);
}
void setup() {
  WiFi.begin( WIFI_NETWORK, WIFI_PASSWORD );

  tft.init();
  tft.setRotation(0);
  tft.fillScreen( TFT_BLUE );
  tft.setTextDatum( TC_DATUM ); // Centre text on x,y position

  clockSprite.createSprite( 240, 320 );
  clockSprite.setTextDatum( TC_DATUM ); // Centre text on x,y position

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

  tft.drawString( "Ready.", 120, 10 );
}

void loop() {
  static time_t savedTime;
  time_t now = time(NULL);
  if ( now != savedTime ) {
    struct tm localTime;
    localtime_r( &now, &localTime );
    char timestr[40];
    strftime( timestr, sizeof( timestr ), "%b %e %Y", &localTime );
    drawClock( localTime.tm_hour, localTime.tm_min, localTime.tm_sec, timestr, "M5Stack sntp  clock" );
    savedTime = now;
  }
  delay( 10 );
}
