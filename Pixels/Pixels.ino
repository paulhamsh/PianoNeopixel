// Install Adafruit DotStar library
#include <M5StickC.h>
#include <Adafruit_NeoPixel.h>

#define PIN 0
#define NUMPIXELS 144

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  M5.begin();
  M5.Lcd.setRotation(-1);
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("Pixels");

    
  pixels.begin();
}

void loop() {
  M5.update();
  pixels.clear(); 
  
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 60, 0));
    pixels.show(); 
    delay(40);
  }
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(60, 0, 0));
    pixels.show(); 
    delay(40);
  }
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 60));
    pixels.show(); 
    delay(40);
  }
}
