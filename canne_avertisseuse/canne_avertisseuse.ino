//interuptions + GPS
/* test lora :
 ajout des librairue 
 "Conversion.h"
 "Lora_Module.h" 
 commissioning.sample.h
 modification des send all et vie
 */
 //desactivation de la led gps, nouvelles leds
 
#include <ArduinoLowPower.h>
#include "MMA8451_IRQ.h"
#include "Adafruit_GPS.h"
#include "variables.h"

//----------------------declaration lora----------------------------------
#include "Lora_Module.h"  //library to convert data in uint8 in old canne_avertisseuse
#include "Conversion.h" //library use the LoRa network in old canne_avertisseuse

Lora_Module lora;
Conversion conv;
int32_t latitude = conv.float_int32("0.010101", 5);
int32_t longitude = conv.float_int32("0.101010", 5);
uint8_t batterie=12;
//------------------------fin lora-----------------------------------

Adafruit_GPS GPS(&GPSSerial);
MMA8451_IRQ mma = MMA8451_IRQ();

//fonctions 
void startGPS(void);
void lectureGPS(void);
void infoGPS(void);
void SENDALL(void);
void SENDVIE(void);

//fonctions exécutées lors d'une interuption
void alarmEventEAU(void);
void alarmEventMOV(void);
void alarmEventCLK(void);

//en cour de dévelopement:
bool RECEPTION(void);

void setup() {
  
  Serial.begin(9600);
  while (!Serial) ;             //tant que on n'a pas ouvert le moniteur série le programme ne s'execute pas !!!
  Serial.println("- Serial start");

//-------------------------------------------------------------LoRa initialisation-------------------------------------------------------------------------------------
  Serial.println("- LoRa initialisation ..."); 
  lora.Init();
  lora.info_connect(); 
//digitalWrite(PinLEDLoRa, HIGH);
//************************************************************LoRa initialisation***************************************************************************************

//------------------------------------------------------------MMA initialisation----------------------------------------------------------------------------------------
  Serial.println("- MMA initialisation ..."); 
if (! mma.begin()) {
    Serial.println("Couldnt start");
    //digitalWrite(PinLEDMMA, LOW);
    while (1);
  }
  Serial.println("MMA8451 found!");
  //digitalWrite(PinLEDMMA, HIGH);
 
  mma.enableInterrupt(); // Setup the I2C accelerometer
//***********************************************************MMA initialization*****************************************************************************************

  // attach pin 0 to accelerometer INT1 and enable the interrupt on voltage falling event
  pinMode(LED_BUILTIN, OUTPUT); //Pin 6
  pinMode(PinLEDEAU, OUTPUT);
  pinMode(PinLEDMOV, OUTPUT);
  pinMode(PinLEDSENDMSG, OUTPUT);
  pinMode(GPS_EN, OUTPUT);
//  pinMode(PinLEDGPS, OUTPUT);
  pinMode(PinLEDLoRa, OUTPUT);
  pinMode(PinLEDMMA, OUTPUT);
  
  digitalWrite(GPS_EN, HIGH); //--------start gps ICI---------
    
//tests des LEDS
Serial.println("tests des LEDS");
  digitalWrite(LED_BUILTIN,HIGH);
  digitalWrite(PinLEDEAU,HIGH);
  digitalWrite(PinLEDMOV,HIGH);
  digitalWrite(PinLEDSENDMSG,HIGH);
 // digitalWrite(PinLEDGPS, HIGH);
  digitalWrite(PinLEDLoRa, HIGH);
  digitalWrite(PinLEDMMA, HIGH);
  
  pinMode(PinEAU, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PinEAU), alarmEventEAU, FALLING);  //antit rebont !!
  pinMode(PinMOV, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PinMOV), alarmEventMOV, FALLING);
//  pinMode(PinCLK, INPUT_PULLUP);
//  attachInterrupt(digitalPinToInterrupt(PinCLK), alarmEventCLK, FALLING);

  delay(800); //le temps que l'on voie des leds s'allumer
  
  Serial.println("start V7.1");
  digitalWrite(LED_BUILTIN,LOW);
  digitalWrite(PinLEDEAU,LOW);
  digitalWrite(PinLEDMOV,LOW);
  digitalWrite(PinLEDSENDMSG,LOW);
  digitalWrite(PinLEDLoRa, LOW);
  digitalWrite(PinLEDMMA, LOW);
  
//--------------------------GPS--------------------------
Serial.println("Start GPS");
  startGPS();
  
  GPStime = millis(); 
  timer = millis(); 
  
Serial.println("Recuperation GPS");

do {
  GPS.read();
  GPS.parse(GPS.lastNMEA());
 
  if((millis() - timer) >= 1000)
  {
    Serial.println("attente d'un fix");
  //  if(etatledGPS) etatledGPS=false;
  //  else if(!etatledGPS) {etatledGPS=true; }
  //  digitalWrite(PinLEDGPS,etatledGPS);
    timer=millis();
  }
 } while (!GPS.fix && (millis() - GPStime) <= GPStimeout); // il faut qu'on est une position gps ou timout de 3 mins
 
 digitalWrite(GPS_EN, LOW); //on etein le GPS ICI
 //digitalWrite(PinLEDGPS,LOW);
 
 Serial.println("fix? " + String(GPS.fix));
 
  SENDALL();
  mma.clearInterrupt(); //au cas ou il y a eu detection de mouvement pendant la recherge cela l'anulle (meme s'il n'y a aucune conséquance sur l'envoie de nv msg)
  Serial.println("fin Setup");
}

void loop()
{
  
  
  if (millis() - timer >= 5000)
{
  digitalWrite(LED_BUILTIN,HIGH);
  Serial.println("Millis");

  digitalWrite(PinLEDEAU,!digitalRead(PinEAU));

  alarmOccurredEAUP = false;
  alarmOccurredMOVP = false;
  alarmOccurredEAU = false;
  alarmOccurredMOV = false;

 // Serial.println(mma.readRegister(0x0C) && 0x04);
  mma.clearInterrupt();
  delay(100);
  
  bool flag = (mma.readRegister(0x0C) && 0x04);
  digitalWrite(PinLEDMOV,flag);
  
  digitalWrite(LED_BUILTIN,LOW);
  tour++;
  timer=millis();
}


if (alarmOccurredEAU == true && alarmOccurredEAUP==false) 
{
  Serial.println("INTERUPTION_EAU");
  digitalWrite(PinLEDEAU,HIGH);
  
  alerte=alerte_EAU;
  
  SENDALL();
   alarmOccurredEAU = false;
   alarmOccurredEAUP = true;
}

if (alarmOccurredMOV == true && alarmOccurredMOVP==false) {

  Serial.println("INTERUPTION_MOV");
  digitalWrite(PinLEDMOV,HIGH);
  
  alerte=alerte_MOV;
  mma.enableInterrupt();
  delay(100);
  SENDALL();
   alarmOccurredMOV = false;
   alarmOccurredMOVP = true;
}

if(tour >= 25)
{
  alerte=alerte_VIE;
  SENDVIE();
  tour=0;
}

}//fin loop

//---------------------------------------------INTERUPTIONS-----------------------------------------------------------
void alarmEventEAU() {
  alarmOccurredEAU = true;
}

void alarmEventMOV() {
  alarmOccurredMOV = true;
}

void alarmEventCLK (void)
{
  alarmOccurredCLK = true;
}

//---------------------------------------------LORA------------------------------------------------------------
void SENDALL()
 {
  digitalWrite(PinLEDSENDMSG, HIGH);
  
  //--------------------------GPS----------------------
   if(alerte!=0){                 //quand on a une alerte=init, il n'y a pas besoins de refaire une recherge puisqu'on vient tout juste d'avoir un fix
    digitalWrite(GPS_EN, HIGH);
   
    delay(50);
    lectureGPS();
    digitalWrite(GPS_EN, LOW);
   }
    infoGPS();
  //-------------------fin GPS-------------------------
  int errorsendA;
  Serial.print("\t \t \t Send alerte: " + String(alerte) +"\n");
  //do{
  uint8_t buffer[9];
  buffer[0] = (uint8_t)(alerte << 5) + (uint8_t)(batterie & 0b11111);
  buffer[1] = (uint8_t)(longitude >> 24);
  buffer[2] = (uint8_t)(longitude >> 16);
  buffer[3] = (uint8_t)(longitude >> 8);
  buffer[4] = (uint8_t)longitude;
  buffer[5] = (uint8_t)(latitude >> 24);
  buffer[6] = (uint8_t)(latitude >> 16);
  buffer[7] = (uint8_t)(latitude >> 8);
  buffer[8] = (uint8_t)latitude;
  errorsendA = lora.send(buffer, 9);
  Serial.println("Voici le code d'erreur_: " + String(errorsendA)); 
 // if(errorsendA<0) delay(500);
    
 // }while(errorsendA < 0);
  
  delay(50);
  digitalWrite(PinLEDSENDMSG, LOW);

  //reception du message retour 
  //if (RECEPTION())
  //si pas reçu alors on réenvoi le msg (1 ou 2 fois)
 }

void SENDVIE()
{
  digitalWrite(PinLEDSENDMSG, HIGH);
  Serial.println("Send VIE");
  int errorsendB;
 // do{
  uint8_t buffer[1];  
  buffer[0] = (uint8_t)(alerte << 5) + (uint8_t)batterie; 
  errorsendB =lora.send(buffer, 1); 
  Serial.println("Voici le code d'erreur_: " + String(errorsendB)); 
 
 //  if(errorsendB<0) delay(500);
 // }while(errorsendB);
  
  delay(50);
  digitalWrite(PinLEDSENDMSG, LOW); 
}

/*
bool RECEPTION()
{
bool etat=false;
int32_t timeout = millis(); 

do{
lora.read();

}while(!etat && (millis()-timeout) <= 60000);
  return etat;
}
*/
//---------------------------------------------GPS------------------------------------------------------------
void startGPS(){
  
  GPS.begin(9600);

  // Mode GGA et RMC
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_ALLDATA);
 
  Serial.println("Get version!");
  GPS.println(PMTK_Q_RELEASE);

  GPS.sendCommand(PGCMD_ANTENNA);
}

void infoGPS(void)
{
      Serial.print("Time: ");
    if (GPS.hour < 10) { Serial.print('0'); }
    Serial.print(GPS.hour, DEC); Serial.print(':');
    if (GPS.minute < 10) { Serial.print('0'); }
    Serial.print(GPS.minute, DEC); Serial.print(':');
    if (GPS.seconds < 10) { Serial.print('0'); }
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    if (GPS.milliseconds < 10) {
      Serial.print("00");
    } else if (GPS.milliseconds > 9 && GPS.milliseconds < 100) {
      Serial.print("0");
    }
    Serial.println(GPS.milliseconds);
 /*   Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);*/
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
    
    if (GPS.fix) {
    //  Serial.print("Location: ");
    //  Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
    //  Serial.print(", ");
     // Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
     //     Serial.print("Location (in degrees, works with Google Maps): ");
     //   Serial.print(GPS.latitudeDegrees, 4);
     //   Serial.print(", "); 
     //   Serial.println(GPS.longitudeDegrees, 4);

    //  Serial.print("Speed (knots): "); Serial.println(GPS.speed);
    //  Serial.print("Angle: "); Serial.println(GPS.angle);
    //  Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
    }
}

void lectureGPS(void)
{
  GPStime = millis(); 
//  timer = millis();
     //si je recoit une nouvelle trame je sort de la boucle et que que son CRC est OK
  do {   //attente d'une nouvelle trame
     
  //   Serial.println("attente nouvelle trame");
    GPS.read(); 
    
    if (GPS.newNMEAreceived()) 
    {
 /* if((millis() - timer) >= 1000) // LED qui fait alumé/eteint
      { 
        if(etatledGPS) etatledGPS=false;
        else if(!etatledGPS) etatledGPS=true;
  
        digitalWrite(PinLEDGPS,etatledGPS);
        timer=millis();
      }  
*/
      Serial.print("NEW");
      if(!GPS.parse(GPS.lastNMEA()))
      {
        Serial.println("\t CRC NO \t" +  String(nombre) + "\t" + String(GPS.fix) + "\t" + String(GPS.satellites) + "\t" + String(GPS.speed));
      } else
      {
      nombre++;
      Serial.println("\t CRC \t OK \t" + String(nombre) + "\t" + String(GPS.fix) + "\t" + String(GPS.satellites) + "\t" + String(GPS.speed));
//   j'attend de recevoir deux trames bonnes pour sortir de la boucle (pb rencontré : si je prend la premire recue c'est très probable que c'est celle que j'ai reçu la derniere fois
//  if(!GPS.fix) nombre=0;
      }
    }
  } while ((nombre<2 || !GPS.fix) && (millis() - GPStime) <= GPStimeout); //2 minutes

  Serial.println("\t fix? " + String(GPS.fix) + "\t temps mis pour trouver le fix: " + String(millis() - GPStime));

  nombre=0 ;
  longitude= conv.float_int32(GPS.longitudeDegrees, 5);  //ajout-------------------------------------------lora------------------------
  latitude= conv.float_int32(GPS.latitudeDegrees, 5);  //ajout-------------------------------------------lora------------------------
 // digitalWrite(PinLEDGPS,LOW);
}
