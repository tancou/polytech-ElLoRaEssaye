#include <SPI.h>
#include <LoRa.h>
#include <MKRWAN.h>


const long freq = 868E6;
int counter = 1;
const int led = 6;
LoRaModem modem;

String outgoing;              // outgoing message
byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0x00;     // address of this device
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

volatile bool doRead = true;
volatile bool openGate = false;

void setup()
{
  Serial.begin(9600);
  while (!Serial);  // On attend que le port série (série sur USBnatif) soit dispo

  modem.dumb();     // On passe le modem en mode transparent

  Serial.println("LoRa Duplex with callback");

  pinMode(led, OUTPUT);
  LoRa.setPins(LORA_IRQ_DUMB, 6, 1); // set CS, reset, IRQ pin
  LoRa.setTxPower(17, PA_OUTPUT_RFO_PIN);
  LoRa.setSPIFrequency(125E3);
  LoRa.setSignalBandwidth(31.25E3);
  LoRa.setSpreadingFactor(9);
  LoRa.setSyncWord(0x34);
  LoRa.setCodingRate4(5);

  LoRa.enableCrc();
  
  LoRa.setPreambleLength(65535);
  //  LoRa.begin(freq);
  Serial.println("LoRa Sender");

  if (!LoRa.begin(freq))
  {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.print("LoRa Started, F = ");
  Serial.println(freq);

  Serial.println("LoRa init succeeded.");

}

void loop() {
  if (doRead) {
    doRead = false;
    onReceive(LoRa.parsePacket());
    doRead = true;
  }
}

void sendMessage(byte dest, byte lastByte) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(0x00);
  LoRa.write(dest);

  LoRa.write(0x00);
  LoRa.write(0x00);
  LoRa.write(0x00);

  if(openGate){
    LoRa.write(0x01);
  } else {
    LoRa.write(0x00);
  }
  openGate = !openGate;

  LoRa.endPacket();

  Serial.println("Response to: 0x" + String(dest, HEX));
  msgCount++;
}

void onReceive(int packetSize) {
  if (packetSize != 6) {
    return;          // if there's no packet, return
  }

  byte senderID = LoRa.read();
  byte receiverID = LoRa.read();

  byte RFIDdata[4];

  RFIDdata[0] = (byte) LoRa.read();
  RFIDdata[1] = (byte) LoRa.read();
  RFIDdata[2] = (byte) LoRa.read();
  RFIDdata[3] = (byte) LoRa.read();

  // if the recipient isn't this device or broadcast,
  if (senderID == localAddress) return;
  if (receiverID != localAddress) {
    Serial.println("This message is not for me. => " + String(receiverID, HEX));
    return;
  }

  // if message is for this device, print details:
//    Serial.println("Received from: 0x" + String(senderID, HEX) + "to 0x" + String(receiverID, HEX));
//    Serial.println("RFIDdata: 0x" + String(RFIDdata[0],HEX) +
//      " 0x" + String(RFIDdata[1],HEX) +
//      " 0x" + String(RFIDdata[2],HEX) +
//      " 0x" + String(RFIDdata[3],HEX));
//    Serial.println("Checksum "+ String(checksum, HEX));
  Serial.println("LOG:"
                 + String(senderID, HEX) + ":"
                 + String(RFIDdata[0], HEX) + ":"
                 + String(RFIDdata[1], HEX) + ":"
                 + String(RFIDdata[2], HEX) + ":"
                 + String(RFIDdata[3], HEX));

  delay(40);
  sendMessage(senderID, 0x01);
}


//void loop()
//{
//static int count;
//count ++;
//
//// Tous les 1000 passages ...
//// Todo : Utiliser le temps sytème pour plus de précision
//if (count > 1000)
//  {
//  digitalWrite(led, HIGH);
//  Serial.print("Sending packet: ");
//  Serial.println(counter);
//
//  //LoRa.send packet
//  LoRa.beginPacket();
//  LoRa.print("Msg N° : ");
//  LoRa.print(counter);
//  LoRa.endPacket();
//  count =0;
//  counter++;
//  delay(40);
//  digitalWrite(led, LOW);
//  }
//
//
//
//
//
////Serial.print("Check for packet  ");
//int packetSize = LoRa.parsePacket();
//  if (packetSize){
//  Serial.print("RX packet size =  ");
//  Serial.println(packetSize);
//  while (LoRa.available()) {
//      Serial.print((char)LoRa.read());
//    }
//   // print RSSI of packet
//  Serial.print("' with RSSI ");
//  Serial.println(LoRa.packetRssi());
//  LoRa.endPacket();
//  }
//}
