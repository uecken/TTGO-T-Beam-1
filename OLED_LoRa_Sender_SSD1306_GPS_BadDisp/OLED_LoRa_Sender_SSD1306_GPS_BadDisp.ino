#include <SPI.h>
#include <LoRa.h>
#include <Wire.h> 
#include<Adafruit_GFX.h>
#include<Adafruit_SSD1306.h>
//#include "SSD1306.h" 
#include "images.h"

//---GPS---
#include <TinyGPS++.h>
#include <axp20x.h>
TinyGPSPlus gps;
HardwareSerial GPS(1);
AXP20X_Class axp;
//---GPS end---


#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISnO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND  923.2E6


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


unsigned int counter = 0;

void setup() {
  pinMode(16,OUTPUT);
  pinMode(2,OUTPUT);
  
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
  
  Serial.begin(115200);
  while (!Serial);
  Serial.println();
  Serial.println("LoRa Sender Test");

  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);

  Serial.println("================");
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

//---GPS---
  Wire.begin(21, 22);
  if (!axp.begin(Wire, AXP192_SLAVE_ADDRESS)) {
    Serial.println("AXP192 Begin PASS");
  } else {
    Serial.println("AXP192 Begin FAIL");
  }
  axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);
  axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);
  axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
  axp.setPowerOutPut(AXP192_EXTEN, AXP202_ON);
  axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);
  GPS.begin(9600, SERIAL_8N1, 34, 12);   //17-TX 18-RX
//---GPS END---

  
  //LoRa.onReceive(cbk);
  //  LoRa.receive();
  Serial.println("init ok at Sender");

   // ディスプレイの初期化
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    // ディスプレイをクリア
  display.clearDisplay();
  
  delay(1500);
}

void loop() {

  // ディスプレイをクリア
  display.clearDisplay();
  // 出力する文字の大きさ
  display.setTextSize(2);
  // 出力する文字の色
  display.setTextColor(WHITE);
  // 文字の一番端の位置
  display.setCursor(0, 0);

 // display.print(BAND,5);
 //display.print(",");
  display.print(String(counter));
  display.print(",");
 // display.println("La,Lo,Al,Sat,Tim,kmph");
  display.print(gps.location.lat(), 5);
  display.print(",");
  display.print(gps.location.lng(), 5);
  display.print(",");
  display.print(gps.altitude.feet() / 3.2808);
  display.print("M");
  display.print(",");
  display.print(gps.satellites.value());
  display.print(",");
  display.print(gps.time.hour());
  display.print(":");
  display.print(gps.time.minute());
  display.print(":");
  display.print(gps.time.second());
  display.print(",");
  display.println(gps.speed.kmph()); 
  display.display();

  Serial.print(String(BAND));
  Serial.print(",");
  Serial.println("TxCnt: " + String(counter));
  Serial.println("Lat,Lon,Alt_m,SatN,Time,kmph");
  Serial.print(gps.location.lat(), 5);
  Serial.print(",");
  Serial.print(gps.location.lng(), 5);
  Serial.println(",");
  Serial.print(gps.altitude.feet() / 3.2808);
  Serial.print("M");
  Serial.print(",");
  Serial.print(gps.satellites.value());
  Serial.print(",");
  Serial.print(gps.time.hour());
  Serial.print(":");
  Serial.print(gps.time.minute());
  Serial.print(":");
  Serial.print(gps.time.second());
  Serial.print(",");
  Serial.println(gps.speed.kmph()); 


//GPS Check
  smartDelay(1000);
  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));

  delay(500);
  
  // send packet
  LoRa.beginPacket();
  LoRa.print(counter);
  LoRa.print(",");  
  LoRa.print(gps.location.lat(), 5);
  LoRa.print(",");
  LoRa.print(gps.location.lng(), 5);
  LoRa.print(",");
  LoRa.print(gps.altitude.feet() / 3.2808);
  LoRa.print(",");
  LoRa.print(gps.satellites.value());
  LoRa.print(",");
  LoRa.print(gps.time.hour());
  LoRa.print(":");
  LoRa.print(gps.time.minute());
  LoRa.print(":");
  LoRa.print(gps.time.second());
  LoRa.print(",");
  LoRa.print(gps.speed.kmph()); 
  LoRa.endPacket();

  LoRa.endPacket();


  counter++;
  digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(2, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}



static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (GPS.available())
      gps.encode(GPS.read());
  } while (millis() - start < ms);
}
