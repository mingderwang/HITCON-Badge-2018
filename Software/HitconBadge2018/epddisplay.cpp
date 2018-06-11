#include "epddisplay.hpp"
#include <GxEPD.h>
#include <GxGDE0213B1/GxGDE0213B1.cpp>
#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>
#include <Fonts/Roboto_Medium12pt7b.h>
#include <Fonts/Roboto_Medium8pt7b.h>

#include "control.hpp"
//QR code generator
#include "qrcode.h"
#include "icon.h"
#include "wallet.hpp"
#include "util.hpp"
#include "LFlash.h"

GxIO_Class io(SPI, 10, 7, 5);
GxGDE0213B1 display(io,5,4);

void init_display(){

  display.init();
  Serial.println("setup done");
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(3);
  // draw background

  //display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, true);
  //delay(1000);
  display.fillScreen(GxEPD_WHITE);
  display.update();
  delay(1000);
}
//Battery_voltage ... etc
void display_static(){
  display.setFont(&Roboto_Medium8pt7b);
  display.setCursor(190, 15);
  display.print(readBatteryVoltage(),3);
  display.println(" V");

  display.drawBitmap(wifi_full, 0, 2, 20, 15, GxEPD_BLACK);
  display.drawBitmap(BT, 25, 0, 20, 20, GxEPD_BLACK);
  display.drawBitmap(NFC, 50, 0, 20, 20, GxEPD_BLACK);
}


void main_menu(){
  display.setFont(&Roboto_Medium12pt7b);

  display.fillScreen(GxEPD_WHITE);
  display.setCursor(10, 50);
  display.println("Balance:");
  display.setCursor(10, 80);

  char Balance[5] = {0};
  uint32_t size = 4;
  LFlash.read("Wallet","Balance",(uint8_t *)&Balance,&size);
  display.print(Balance);
  display.println(" ETH");

  display.setFont(&Roboto_Medium8pt7b);
  
  display.setCursor(0,122-7);
  display.println("[A]:SET");
 
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, publicAddress_string.c_str());

  int x0 = 140;
  int y0 = 25;
  int expension = 3;
  for (uint8_t y = 0; y < qrcode.size; y++) {
      for (uint8_t x = 0; x < qrcode.size; x++) {
          if (qrcode_getModule(&qrcode, x, y)) {
            display.fillRect(x0+x*expension,y0+y*expension,expension,expension,GxEPD_BLACK);
          }else{
            display.fillRect(x0+x*expension,y0+y*expension,expension,expension,GxEPD_WHITE);
          }
      }
  }
  display_static();
  display.update();
  
}
void setting_menu(){
  display.fillScreen(GxEPD_WHITE);
  display.setFont(&Roboto_Medium8pt7b);
  
  display.setCursor(50, 40);
  display.print("NFC: ");display.print("ON");
  display.println("   [4]:on,[5]:off");
  display.setCursor(50, 60);
  display.print("BLE: ");display.print("ON");
  display.println("   [7]:on,[8]:off");
  display.setCursor(50, 75);
  display.print("[B]:ReGenerateCode");
  
  display.setFont(&Roboto_Medium8pt7b);
  
  display.setCursor(0,122-7);
  display.println("[A]:Menu ");
 
  display_static();
  display.update();
  
  while(1){
      switch(readButton()){
        case 0xA: return;
        case 0xB:{
            BLE_pairing();
            return;
        }
        default:
            continue;
      }
  }

}

void BLE_pairing(){
  display.fillScreen(GxEPD_WHITE);
  display_static();
  display.setCursor(10, 60);

  display.setCursor(0,122-7);
  display.setFont(&Roboto_Medium8pt7b);
  display.println("[A]:Reset");
  
  uint32_t AES_key[4] = {0};
  for (int i = 0; i < 4; ++i)
  {
    AES_key[i] = randomUint32_t_generator();
  }

  char ServiceUUID[37]= {0};
  random_UUID_generator(ServiceUUID);
  char ServiceUUIDHead[9]= {0};
  memcpy(ServiceUUIDHead,ServiceUUID,8);
  Serial.print("Service UUID:");Serial.println(ServiceUUID);

  char UUIDHead[6][9] = {0};



  for (uint8_t i = 0; i < 6; ++i)
  {
    random_UUID_generator_head(UUIDHead[i]);
    Serial.print("UUID Head:");Serial.println(UUIDHead[i]);
    for (uint8_t j = 0; j < 6; ++j)
    {
      if (String(UUIDHead[i]) == String(UUIDHead[j]) && j!=i)
      {
         Serial.println(j);
         Serial.println(i);
         Serial.println(UUIDHead[j]);
         i = i-1;
      }
      if (String(UUIDHead[i]) == String(ServiceUUIDHead[j]))
      {
        Serial.println(i);
        Serial.println("ServiceUUID");
        i = i-1;
      }
    }
  }

  LFlash.write("BLE","Service_UUID",LFLASH_RAW_DATA,(const uint8_t *)&ServiceUUID,sizeof(ServiceUUID));
  LFlash.write("BLE","Transaction",LFLASH_RAW_DATA,(const uint8_t *)&UUIDHead[0],8);
  LFlash.write("BLE","Txn",LFLASH_RAW_DATA,(const uint8_t *)&UUIDHead[1],8);
  LFlash.write("BLE","AddERC20",LFLASH_RAW_DATA,(const uint8_t *)&UUIDHead[2],8);
  LFlash.write("BLE","Balance",LFLASH_RAW_DATA,(const uint8_t *)&UUIDHead[3],8);
  LFlash.write("BLE","General_CMD",LFLASH_RAW_DATA,(const uint8_t *)&UUIDHead[4],8);
  LFlash.write("BLE","General_Data",LFLASH_RAW_DATA,(const uint8_t *)&UUIDHead[5],8);

  LFlash.write("BLE","BLE_AES_key",LFLASH_RAW_DATA,(const uint8_t *)&AES_key,sizeof(AES_key));
  

  char AES_key_buffer[200] = {0};

  sprintf (AES_key_buffer, "Hitcon://pair?v=18&a=%s&k=%08x%08x%08x%08x&s=%s&c=%s%s%s%s%s%s",publicAddress_string.c_str(), AES_key[0], AES_key[1], AES_key[2], AES_key[3],ServiceUUID,UUIDHead[0],UUIDHead[1],UUIDHead[2],UUIDHead[3],UUIDHead[4],UUIDHead[5]);

  Serial.print("QR:");Serial.println(AES_key_buffer);
  Serial.println(qrcode_getBufferSize(8));
//HitconBadge2018://?address=808c2257d778e5f1340d9325116f5a7273b33f5d?key=F12D9A8CA89EC63C16F393066A956D66?SerUUID=B039B324?Charastic=22A0442C23DB270532550656AE6BC4C6AAA17C9BCE38AA8B
//Hitcon://pair?v=18&a=808c2257d778e5f1340d9325116f5a7273b33f5d&k=45ec209bc5aa6ec32aae7db226952cad&s=fcf2881e-110e-4dbb-d110-4ca507163e0b&c=c51612c39bd5dc0d5189bbacae88b8293aef4bc5d6006c4a

  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(8)];
  qrcode_initText(&qrcode, qrcodeData, 8, ECC_LOW, AES_key_buffer);

  int x0 = 75;
  int y0 = 23;
  int expension = 2;
  for (uint8_t y = 0; y < qrcode.size; y++) {
      for (uint8_t x = 0; x < qrcode.size; x++) {
          if (qrcode_getModule(&qrcode, x, y)) {
            display.fillRect(x0+x*expension,y0+y*expension,expension,expension,GxEPD_BLACK);
          }else{
            display.fillRect(x0+x*expension,y0+y*expension,expension,expension,GxEPD_WHITE);
          }
      }
  }

  display.update();

  while(1){
    if(readButton()==0xA){
     WDT_Reset();
    }
  }

}
