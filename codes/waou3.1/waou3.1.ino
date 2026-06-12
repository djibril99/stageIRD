////////////////// REBOOT SYSTEM //////////////////////////
#include <MsTimer2.h>
#include <avr/wdt.h>
#include <Wire.h>    
#include "RTClib.h"
#include <SD.h>



void (* reboot) (void) = 0;
const long watchdog_max = /*1*/5; //En minutes (multiple de 2 minutes)
long watchdog_courant = 0;
const long watchdog_delay = 60000;
int messageID = 0;




void razWatchdog()
{
  watchdog_courant = 0;
}

void startMyWatchdog()
{
  razWatchdog();
  MsTimer2::set(watchdog_delay, processRebootWatchdog);
  MsTimer2::start();
  Serial.println("start watchdog");
}

void stopMyWatchdog()
{
  MsTimer2::stop();
}
void reboot_now()
{
  MsTimer2::stop();
  razWatchdog();
  watchdog_courant = watchdog_max;
  Serial.println("rebooting via reboot_now\n");
  MsTimer2::set(10, processRebootWatchdog);
  MsTimer2::start();
}

void processRebootWatchdog()
{
    watchdog_courant++;
    Serial.println("watchdog courant got up\n");
    if ((watchdog_courant ) >= watchdog_max)
    {
      Serial.println("reboot via watchdog");
      MsTimer2::stop();
     //reboot();
      wdt_enable(WDTO_15MS);
      delay(15);
   }
}

////pour le redémarrage du capteur/////////////////////////

uint32_t startTime = 0; // date de départ en seconde
unsigned long initOffset = 1000000000; // date de départ en seconde
unsigned long lastUpdate = 1000000000;


/*
#define SENSOR_1 9
#define SENSOR_2 10
#define SENSOR_3 11
#define SENSOR_4 12
#define SENSOR_5 13
#define SENSOR_6 14
#define SENSOR_7 15
#define SENSOR_8 16
*/
//definir deux tableaux de pin (vcc et gnd) pour pouvoir faire le changement de polarité
#define SENSOR_VCC {7, 9, 11, 13, 15, 17, 19, 24}
#define SENSOR_GND {8, 10, 12, 14, 16, 18, 28, 26}
#define ANALOG_PINS {A0, A1, A2, A3, A4, A5, A6, A7}



int vccPins[] = SENSOR_VCC;
int gndPins[] = SENSOR_GND;
int analogPins[] = ANALOG_PINS;
int taille = 8 ;

float w[8];



#define SENSOR_NAME "WAOU1"

#define NB_AQUISITION 100//500

#define SerialAT Serial2


//////////////FILE///////////////////////////
File myFile;
File myFile2;
#define FILE_NAME "datalog.csv"
#define FILE_NAME_save "datalog_no_Date.csv"

DateTime now;

///////////// SETUP FUNCTION //////////////
void setup() 
{
  Serial.begin(9600);
  setupSD();
  setupSensors();
  shutDownSensors();

  



}

void setupSD()
{
  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
   pinMode(53, OUTPUT);
}

void wakeupSD()
{   
  if (!SD.begin(53)) 
  {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
}

void saveDataIn(String message)
{
    myFile = SD.open(FILE_NAME, FILE_WRITE);
   // if the file opened okay, write to it:
  if (myFile) 
  {
    //Serial.print("Writing to test.txt...");
    myFile.println(message);
  // close the file:
    myFile.close();
    //Serial.println("done.");
  } 
  else 
  {
    // if the file didn't open, print an error:
    Serial.println("error opening FILE_NAME");
  }
}


void saveDataIn_no_date(String message)
{
    myFile2 = SD.open("data.csv", FILE_WRITE);
   // if the file opened okay, write to it:
  if (myFile2) 
  {
    Serial.print("Writing to no_date.txt...");
    myFile2.println(message);
  // close the file:
    myFile2.close();
    Serial.println("done.");
  } 
  else 
  {
    // if the file didn't open, print an error:
    Serial.println("error opening FILE_NAME");
  }
}

void updateClock()
{
  SerialAT.print("AT+CCLK?\r\n");
  String input = SerialAT.readString();
  String year = input.substring(10,12);
  String month = input.substring(13,15);
  String day = input.substring(16,18);
  String hour = input.substring(19,21);
  String minute = input.substring(22,24);
  String second = input.substring(25,27);
  DateTime dt (year.toInt(),month.toInt(),day.toInt(),hour.toInt(),minute.toInt(),second.toInt());
  uint32_t mt = dt.unixtime();
  //current_time = dt;
//  DateTime.sync(dt.unixtime()); 
  //DateTime.sync(mt);
  Serial.print("unix time (s)");
  Serial.println(mt);
  lastUpdate = millis();
  startTime = mt; // date de départ en seconde

}

void setupSensors()
{
  Serial.println("Setup Sensors");
  /*
  ici
  pinMode(SENSOR_1, OUTPUT);
  pinMode(SENSOR_2, OUTPUT);
  pinMode(SENSOR_3, OUTPUT);
  pinMode(SENSOR_4, OUTPUT);
  pinMode(SENSOR_5, OUTPUT);
  pinMode(SENSOR_6, OUTPUT);
  pinMode(SENSOR_7, OUTPUT);
  pinMode(SENSOR_8, OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(8,OUTPUT);
  */

  for(int i = 0; i < taille; i++){
    pinMode(vccPins[i], OUTPUT);
    pinMode(gndPins[i], OUTPUT);
  }


  
}

void shutDownSensors()
{
  /*
  ici 
  //Serial.println("Shutdown Sensors");
  digitalWrite(SENSOR_1, HIGH);
  digitalWrite(SENSOR_2, HIGH);
  digitalWrite(SENSOR_3, HIGH);
  digitalWrite(SENSOR_4, HIGH);
  digitalWrite(SENSOR_5, HIGH);
  digitalWrite(SENSOR_6, HIGH);
  digitalWrite(SENSOR_7, HIGH);
  digitalWrite(SENSOR_8, HIGH);
  */
  int vccPins[] = SENSOR_VCC;
  int gndPins[] = SENSOR_GND;
  int analogPins[] = ANALOG_PINS;

  for(int i = 0; i < taille; i++){
    digitalWrite(vccPins[i], LOW);
    digitalWrite(gndPins[i], LOW);
  }

}

void startupSensor(int id)
{
  pinMode(id,OUTPUT);   //On passe le capteur ciblé en OUTPUT;
  //Serial.print("Startup Sensor: ");
  //Serial.println(id);
  digitalWrite(id, LOW);

  //On met tout les autres capteurs en INPUT pour éviter des interférences
  /*
  ici 
  for(int i = SENSOR_1; i <= SENSOR_8; i++){
    if(id != i){
      pinMode(i, INPUT);
    }
  }
  */
  for(int i = 0; i < taille; i++){
    if(id != vccPins[i] && id != gndPins[i]){
      //pinMode(vccPins[i], INPUT);
      //pinMode(gndPins[i], INPUT);
    }
  }

}

double getHumidity(int idSensor, int p1, int p2)
{
  double val = 0;
  double U;
  double Rmes;
  double Rcap;  
  for(int i=0; i < NB_AQUISITION; i++)
  {
    digitalWrite(p2, LOW);
    digitalWrite(p1, HIGH);
    delay(4);
    int readd =analogRead(idSensor);
    val =val+ readd;
    delay(1);
    //Serial.print(val,DEC);
    digitalWrite(p1, LOW);
    digitalWrite(p2, HIGH);
    delay(4);
    readd =analogRead(idSensor);
    //Serial.println(readd,DEC);
    val =val+ (1023-readd);
    
    delay(1);
  }
  digitalWrite(p1, LOW);
  digitalWrite(p2, LOW);
  delay(10);
  val = val /(NB_AQUISITION*2);
  //Serial.println(val,DEC);
  U=val*5/1023;

  //Serial.println(U);
  Rmes=990000; //resitance de mesure (en ohm)
  Rcap=(5/(5-U)-1)*Rmes; // ATTENTION on permutte à chaque fois 
  return Rcap;
}

void writeToConsole(DateTime dte, double val1,double val2, double val3, double val4, double val5, double val6, double val7, double val8)
{
    Serial.print(dte.unixtime(),DEC);
    Serial.print(";");
    Serial.print(dte.year(),DEC);
    Serial.print(";");
    Serial.print(dte.day(),DEC);
    Serial.print(";");
    Serial.print(dte.month(),DEC);
    Serial.print(";");
    Serial.print(dte.hour(),DEC);
    Serial.print(":");
    Serial.print(dte.minute(),DEC);
    Serial.print(":");
    Serial.print(dte.second(),DEC);
    Serial.print("::");
    Serial.print(val1,DEC);
    Serial.print(";");
    Serial.print(val2,DEC);
    Serial.print(";");
    Serial.print(val3,DEC);
    Serial.print(";");
    Serial.println(val4,DEC);
    Serial.print(";");
    Serial.println(val5,DEC);
    Serial.print(";");
    Serial.println(val6,DEC);
    Serial.print(";");
    Serial.println(val7,DEC);
    Serial.print(";");
    Serial.println(val8,DEC);
   
}


String createMessage()
{
  //String m = String(w[0]) + ":" + String(w[1]) + ":" + String(w[2]) + ":" +String(w[3])+ ":" + String(w[4]) + ":" + String(w[5]) + ":" + String(w[6]) + ":" +String(w[7]);
  String m = String(w[0]) + ";" + String(w[1]) + ";" + String(w[2]) + ";" +String(w[3])+ ";" + String(w[4]) + ";" + String(w[5]) + ";" + String(w[6]) + ";" +String(w[7]);
  String message =  buildCaptureMessage(m);
  return message;
}

String buildCaptureMessage(String data)
{ 
  int retry = 10;
  while (lastUpdate  > millis()|| (lastUpdate + 7200000)  < millis() ||startTime < 1561757324) {
    updateClock();
    retry = retry - 1;
    if(retry < 0)
      return "";
  }
  
  messageID = messageID + 1;
  String sid = String(messageID);
  uint32_t cdate = (millis() - lastUpdate ) / 1000 + startTime; ///////////////////////////////////////////////:::ERREUR? DATE?
  String sdate = String(cdate);



  String message = sdate + ";";
  message += SENSOR_NAME;
  message += ";" + sid + ";";
  message += data;
  return message;
}




unsigned long mydate;


void loop() 
{
    for (int i = 0; i < taille; i++){
      shutDownSensors();
      startupSensor(vccPins[i]);
      w[i] = getHumidity(analogPins[i], gndPins[i], vccPins[i]);
      delay(500);
    }
    shutDownSensors();
    mydate = millis();

    writeToConsole(mydate, w[0], w[1], w[2], w[3], w[4], w[5], w[6], w[7]);
    
    
    saveToSD2(String(mydate)+","+String(w[0])+","+String(w[1])+","+String(w[2])+","+String(w[3])+","+String(w[4])+","+String(w[5])+","+String(w[6])+","+String(w[7]));
    String dte = createMessage();
    if(dte.length()>0)
    {
      saveToSD(dte);
    }
    
    writeToConsole(now, w[0], w[1], w[2], w[3], w[4], w[5], w[6], w[7]);
    delay (360000); // delai de mesure 1h ////////////////////////////////////////////////////////////////////////////////////:

  }


void saveToSD2(String message)
  {
    wakeupSD();
    saveDataIn_no_date(message);
  }
  
void saveToSD(String message)
  {
    wakeupSD();
    saveDataIn(message);
  }


/*
*modificcation les la lecture des pin analogiques (c'etait partout A0 , 6,7)



*/