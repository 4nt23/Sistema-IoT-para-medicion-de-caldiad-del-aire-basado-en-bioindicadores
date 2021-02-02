// Feather9x_RX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (receiver)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_TX

#include <SPI.h>
#include <RH_RF95.h>

/* for Feather32u4 RFM9x
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
*/

/* for feather m0 RFM9x*/
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3


/* for shield 
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 7
*/

/* Feather 32u4 w/wing
#define RFM95_RST     11   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_INT     2    // "SDA" (only SDA/SCL/RX/TX have IRQ!)
*/

/* Feather m0 w/wing 
#define RFM95_RST     11   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_INT     6    // "D"
*/
//
//#if defined(ESP8266)
//  /* for ESP w/featherwing */ 
//  #define RFM95_CS  2    // "E"
//  #define RFM95_RST 16   // "D"
//  #define RFM95_INT 15   // "B"
//
//#elif defined(ESP32)  
//  /* ESP32 feather w/wing */
//  #define RFM95_RST     27   // "A"
//  #define RFM95_CS      33   // "B"
//  #define RFM95_INT     12   //  next to A
//
//#elif defined(NRF52)  
//  /* nRF52832 feather w/wing */
//  #define RFM95_RST     7   // "A"
//  #define RFM95_CS      11   // "B"
//  #define RFM95_INT     31   // "C"
//  
//#elif defined(TEENSYDUINO)
//  /* Teensy 3.x w/wing */
//  #define RFM95_RST     9   // "A"
//  #define RFM95_CS      10   // "B"
//  #define RFM95_INT     4    // "C"
//#endif


// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Blinky on receipt
//#define LED 13
//uint8_t buf[100];
//uint8_t buf1[100];
//uint8_t len = sizeof(buf);
//uint8_t len_rest,len_prev;
//uint8_t eol[2] = {0x0D,0x0A};
  
void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  Serial.begin(115200);
  while (!Serial) {
    delay(1);
  }
  delay(100);

  //Serial.println("Feather LoRa RX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  //memset(buf,(uint8_t)'0',sizeof(buf));
  //memset(buf1,(uint8_t)'0',sizeof(buf1));
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  rf95.init();
//  while (!rf95.init()) {
//    Serial.println("LoRa radio init failed");
//    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
//    while (1);
//  }
//  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  rf95.setFrequency(RF95_FREQ);
//  if (!rf95.setFrequency(RF95_FREQ)) {
//    Serial.println("setFrequency failed");
//    while (1);
//  }
//  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBmb     :
  rf95.setTxPower(23, false);
}

void loop()
{
  uint8_t buf[100];
  uint8_t buf1[100];
  uint8_t len = sizeof(buf);
  uint8_t len_rest,len_prev;
  uint8_t eol[2] = {0x0D,0x0A};
  uint8_t* data_init;
  uint8_t* data_end;
  bool recepcion_OK = false;
  uint8_t data_len,data_prev;
  if (rf95.available())
  {
    // Should be a message for us now
    len = sizeof(buf);
    //memset(buf,'\0',sizeof(buf));//limpia buffer
    //memset(buf1,'\0',sizeof(buf1));//limpia buffer
    recepcion_OK = rf95.recv(buf, &len);
//    strncat((char*)buf1,(char*)buf,len);
//    memset(buf,(uint8_t)'\0',sizeof(buf));
//    strcpy((char*)buf,(char*)buf1);
//    memset(buf1,(uint8_t)'\0',sizeof(buf));
//    len_rest=0;
//    len_prev=0;
    if (recepcion_OK)
    {
      digitalWrite(LED, HIGH);
      //RH_RF95::printBuffer("Received: ", buf, len);
      if (strstr((char*)buf, "Img") || strstr((char*)buf, "len")|| strstr((char*)buf, "Err")|| strstr((char*)buf, "Rdy")){
//        sprintf((char*)buf, "%s%s",(char*)buf,(char*)eol);
//        Serial.write((char*)buf,len+2);
        //memset(buf1,'\0',sizeof(buf1));//limpia buffer
        memcpy(buf1,buf,len);
        Serial.println((char*)buf1);
        Serial.flush();
        memset(buf,'\0',sizeof(buf));//limpia buffer
      }
      else{
//        data_init =(uint8_t*)strstr((char*)buf, "<Pic>");
//        data_end = (uint8_t*)strstr((char*)buf, "</Pic>");
//        data_end = data_end + 5;
//        data_len = (((int32_t)data_end-(int32_t)data_init)/sizeof(uint8_t))+1;
        //memset(buf1,'\0',sizeof(buf1));//limpia buffer
//        memcpy(buf1,data_init,48);
        //strncpy(buf1,buf,len);
        Serial.write(buf,len);
        Serial.flush();
        //memset(buf,'\0',sizeof(buf));//limpia buffer
        
//        for(uint8_t z=0; z<len_rest;z++){
//          buf[len_rest+z]=buf[z];
//          buf[z]=buf1[z];
//          buf1[z]=0;
//        }
//        len_rest=0;
//      
//        data_init = strstr((char*)buf, "<Pic>");
//        data_end = strstr((char*)buf, "</Pic>");
//        data_end = data_end + 5;
//        len_prev = (((int32_t)data_init-(int32_t)buf)/sizeof(char))+1;
//        data_len = (((int32_t)data_end-(int32_t)data_init)/sizeof(char))+1;
//        Serial.write(data_init,data_len);
//        Serial.flush();
//        for (uint8_t j = 0; j<data_len; j++){
//          buf[j]=buf[data_len+j];
//          buf[data_len+j]=0;
//        }
//        len_rest=(len-data_len)>=0?(len-data_len):0;
//        memset(buf1,(uint8_t)'\0',sizeof(buf1)); //limpia buffer de restos.
//        strncpy((char*)buf1,(char*)buf,len_rest);    
      }
//      else{
//        len_rest = len;
//        strncpy((char*)buf1,(char*)buf,len_rest); //copiamos restos
//      }
      
     

//      // Send a reply
//      uint8_t data[] = "OK";
//      rf95.send(data, sizeof(data));
//      rf95.waitPacketSent();
//      Serial.println("Sent a reply");
//      //digitalWrite(LED, LOW);
    }
    else
    {
      Serial.println("Receive failed");
    }
    //memset(buf,'\0',sizeof(buf));//limpia buffer
    
  }
  if (Serial.available())
  {
    len = 0;
    uint8_t data[10];
    Serial.setTimeout(30000);
    len = Serial.readBytesUntil('\n', data, 100);
    //if (strstr((char*)data,"OK")){
      //memset(buf,'\0',sizeof(buf));
      //memset(buf1,'\0',sizeof(buf1));
    //}
    //flush_receive();
    if(len > 0){
        rf95.send(data, len);
        //Serial.println((char*)data);
        rf95.waitPacketSent();
        rf95.available();
//    
//      if (strstr((char*)data,"OK") )
//      {
//        rf95.send(data, sizeof(data));
//        Serial.println((char*)data);
//        delay(10);
//        rf95.waitPacketSent();
//        
//        
//      }
//      else{
//        rf95.send(data, sizeof(data));
//        //Serial.println((char*)data);
//        delay(10);
//        rf95.waitPacketSent();
//      }
    }
    
  
  }
}

void flush_receive(){
  while(Serial.available()){
    Serial.read();
  }
}
