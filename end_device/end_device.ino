// Feather9x_TX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (transmitter)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_RX

#include <SPI.h>
#include <RH_RF95.h>
#include <Adafruit_VC0706.h>
//Camera configuration
#define cameraconnection Serial1

/* for feather32u4 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
*/

/* for feather m0  */
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

#define DEBUG 1 //para hacer debug
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

//single instance of the camera
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);
char temp_buf[10][48];

void print_debug(char*buff){
  #if DEBUG
    Serial.println(buff);
  #endif
}


void setup() 
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

#if DEBUG
  Serial.begin(115200);
  while (!Serial) {
    delay(1);
  }
#endif

  delay(100);

  print_debug("Feather LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  cam.begin(); //inicia comunicación con la cámara
  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  print_debug("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  print_debug("Set Freq to: "); 
  //print_debug((char*)RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}


void loop()
{
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  char radiopacket[20];
  print_debug("Going to sleep"); 
  rf95.sleep(); //set Lora transceiver in low power mode
  delay(5000);
  for(uint8_t i = 0; i<sizeof(buf); i++){
    buf[i]=0;
  }
  while (!strstr((char*)buf, "take")){//wait for command to take a picture
    if(rf95.available()){ //there is some message?
      print_debug("Packet received");
      rf95.recv(buf, &len); //get the message
      print_debug((char*)buf);
      rf95.sleep();
      delay(1000);
    }
  }

  //rf95.clearRxBuf(); //clear local reception buffer
  sprintf(radiopacket,"%s","Img");
  rf95.send((uint8_t*)radiopacket,3);
  print_debug("Enviando: ");
  print_debug(radiopacket);
  delay(10);
  rf95.waitPacketSent();//wait for packet sent
  print_debug(".....enviado.");

  if (check_reply()){ //check reply
    print_debug("reply to Img received");
    cam.setImageSize(VC0706_640x480); //set image size
    cam.resumeVideo(); //start image capture
    delay(3000);
    if(cam.takePicture()){
      if (send_picture(cam)){
        sprintf(radiopacket,"%s","Rdy");//Imagen enviada correctamente. Esperar por siguiente comando.
        print_debug(radiopacket);
      }
      else{
        sprintf(radiopacket,"%s","Err00003"); //Error enviando la imagen. Necesario reenvio.
        print_debug(radiopacket);
      }
    }
    else{
      sprintf(radiopacket,"%s","Err00002"); //Cámara no pudo tomar foto. ¿Reintentar?
    }
  }
  else{
    sprintf(radiopacket,"%s","Err00001"); //Gateway no recibió el comando
  }
   print_debug(radiopacket);
   rf95.send((uint8_t*)radiopacket, sizeof(radiopacket)); //Cámara no pudo tomar foto. ¿Reintentar?
   delay(10);
   rf95.waitPacketSent();
} 
  
  
  
//  itoa(packetnum++, radiopacket+13, 10);
//  Serial.print("Sending "); Serial.println(radiopacket);
//  radiopacket[19] = 0;
//  
//  Serial.println("Sending...");
//  delay(10);
//  rf95.send((uint8_t *)radiopacket, 20);
//
//  Serial.println("Waiting for packet to complete..."); 
//  delay(10);
//  rf95.waitPacketSent();
//  // Now wait for a reply
//  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
//  uint8_t len = sizeof(buf);
//
//  Serial.println("Waiting for reply...");
//  if (rf95.waitAvailableTimeout(1000))
//  { 
//    // Should be a reply message for us now   
//    if (rf95.recv(buf, &len))
//   {
//      Serial.print("Got reply: ");
//      Serial.println((char*)buf);
//      Serial.print("RSSI: ");
//      Serial.println(rf95.lastRssi(), DEC);    
//    }
//    else
//    {
//      Serial.println("Receive failed");
//    }
//  }
//  else
//  {
//    Serial.println("No reply, is there a listener around?");
//  }
//
//}

//bool check_reply(uint16_t numpack){
//  uint8_t data[10] = " ";
//  char desired [10];
//  uint8_t len =sizeof(data);
//  uint16_t timeout = 500;
//  sprintf(desired,"%05i",numpack);
//  rf95.waitAvailableTimeout(timeout);
//  //rf95.sleep();
//  for (uint8_t i = 0; !strstr((char*)data,desired)&& i < 1; i++){ //try 5 times as maximum get the correct reply
//    print_debug("reply....wait for matching num pack");
//    if(rf95.available()){
//      print_debug("while");
//      rf95.recv(data, &len);
//      print_debug((char*) data);
//      //rf95.sleep();
//    }
//
//  }
//  return (strstr((char*)data,desired));
//}
bool check_reply(uint16_t packet){
  uint8_t data[7];
  uint8_t num[10];
  char desired [7] = "OK";
  uint8_t len =sizeof(data);
  uint16_t timeout = 500;
  if (packet >=0){
    sprintf(desired,"OK%05i",packet);
  }
  rf95.waitAvailableTimeout(timeout);
  //rf95.sleep();
  for (uint8_t i = 0; !strstr((char*)data,desired)&& i < 1; i++){ //try 5 times as maximum get the correct reply
    print_debug("reply....wait available");
    if(rf95.available()){
      rf95.recv(data, &len);
      print_debug("desired - received\n");
      print_debug((char*)desired);
      print_debug("-");
      print_debug((char*)data);
      print_debug("\n");
      print_debug((char*) data);
      //rf95.sleep();
    }

  }
  return (strstr((char*)data,desired));
}

bool check_reply(){
  uint8_t data[7];
  uint8_t num[10];
  char desired [7] = "OK";
  uint8_t len =sizeof(data);
  uint16_t timeout = 500;

  rf95.waitAvailableTimeout(timeout);
  //rf95.sleep();
  for (uint8_t i = 0; !strstr((char*)data,desired)&& i < 1; i++){ //try 5 times as maximum get the correct reply
    print_debug("reply....wait available");
    if(rf95.available()){
      rf95.recv(data, &len);
      print_debug("desired - received\n");
      print_debug((char*)desired);
      print_debug("-");
      print_debug((char*)data);
      print_debug("\n");
      print_debug((char*) data);
      //rf95.sleep();
    }

  }
  return (strstr((char*)data,desired));
}


bool send_picture(Adafruit_VC0706 cam){
  char buf[15];
  uint16_t jpglen = cam.frameLength();
  byte wCount = 0;
  bool result = false;
  bool resend = false;
  uint8_t *packet;
  uint8_t pic [100];
  uint8_t bytesToRead;
  uint16_t totalpackets = 0;
  uint16_t enviados =0;
  char info[100];
  sprintf(buf,"%s%i","len",jpglen); //concatenate length and jpglen
  rf95.send((uint8_t*)buf,sizeof(buf));//Send length of picture
  rf95.waitPacketSent();
  print_debug(buf);
  totalpackets = (jpglen/32);
  if (jpglen%32 > 0){
    totalpackets ++;
  }
  sprintf(buf,"Número de paquetes %i", totalpackets);
  print_debug(buf);
  rf95.available();
  if (check_reply()){

    while (jpglen > 0){
   
      bytesToRead = min ((uint16_t)32, jpglen); //Read 32 bytes as maximum
      packet = cam.readPicture(bytesToRead); //get packet from camera
      //Serial.println("Sending packet...");
      print_debug("Enviando nuevo paquete");
      sprintf(info,"Tamaño Paquete: %i\n%s: %05i",bytesToRead, packet, enviados);
      print_debug(info);
      do {
        memset(pic,0,sizeof(pic));
       
        sprintf((char*)pic,"<Pic>%05i",enviados);
        
        memcpy(pic+10,packet,bytesToRead);
        memcpy(pic+10+bytesToRead,(uint8_t*)"</Pic>",6);
        for (uint8_t j=0;j<(16+bytesToRead);j++){
//          if (j<5 || j>42){
          Serial.print(pic[j],HEX);
//          }
//          else{
//            Serial.print(pic[j],HEX);
//          }
        }
        Serial.println("");
        print_debug((char*)pic);
        rf95.send(pic,(bytesToRead+16)); //send packet
        print_debug("mandado");
        rf95.waitPacketSent(); //wait until packet is sent
        rf95.available();
        print_debug("Pic Packet Sent");
        print_debug("Waitting confirm");
        resend = !check_reply(enviados);
      }while(resend);
      enviados++;
       
      
      
      //Serial.write(packet, bytesToRead);
      //Serial.flush(); //wait until packet is sent
  //    rf95.send(packet, bytesToRead);
  //    rf95.waitPacketSent();
  //    Serial.println("Packet sent");
  //    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  //    uint8_t len = sizeof(buf);
  //    Serial.println("Waiting for reply...");
  //    if (rf95.waitAvailableTimeout(1000))
  //    { 
  //      // Should be a reply message for us now   
  //      if (rf95.recv(buf, &len))
  //      {
  //        Serial.print("Got reply: ");
  //        Serial.println((char*)buf);
  //        Serial.print("RSSI: ");
  //        Serial.println(rf95.lastRssi(), DEC);    
  //      }
  //      else
  //      {
  //        Serial.println("Receive failed");
  //      }
  //    }
  //    else
  //    {
  //      Serial.println("No reply, is there a listener around?");
  //    }
      jpglen -= bytesToRead;
    }
    //rf95.sleep();
    print_debug("Pic sent!");
    //rf95.waitAvailable();
    print_debug("PORFIN!");
    //result = check_reply();
    print_debug("Out");
  }
  return true;
  
}
