#include <Ports.h>
#include <RF12.h>
#include <SoftPWM.h>

/////////rf12_data[i] est le bits (0-255) de reception. Changer "i"
/// par n° d'envoi depuis WCat.
/// le jeenode à des numéro de pin particulier en fonction du port.
///regarder la doc.


void setup () {
    Serial.begin(14400); //doit corespondre avec le baud rate de whitecat
     SoftPWMBegin();
   //frequence 
   TCCR0B = TCCR0B & 0b11111000 | 001;
   //Modifie le fréquence pwm : à activer si la lampe "chante"
    Serial.print("\n[rfStrings]");
   rf12_initialize(2, RF12_868MHZ, 212);
    pinMode (4, OUTPUT); //port 1 sortie D
    pinMode (5, OUTPUT); //port 1 sortie I
    pinMode (6, OUTPUT); //port 2 sortie D
    pinMode (7, OUTPUT); //port 2 sortie I
    pinMode (14, OUTPUT); //port 3 sortie D
    pinMode (15, OUTPUT); //port 3 sortie I
    pinMode (16, OUTPUT); //port 4 sortie D
    pinMode (17, OUTPUT); //port 4 sortie I
}

void loop () {
    if (rf12_recvDone() && rf12_crc == 0) {
      //MONITORING
        // a packet has been received
       Serial.print("GOT ");
        for (byte i = 0; i < rf12_len; i++)
        {
           Serial.print(rf12_data[i], DEC);
           Serial.print(" ");
        }
       Serial.println();
       
      //reception en int / 4 char headers, circuit par 2
    //Jeenode P1
    SoftPWMSet  (4, rf12_data[2]);  //D (n° de port du jeeNode, n° d'envoi depuis WCat)
    SoftPWMSet(14, rf12_data[3]);   //A
    //Jeenode P2
    SoftPWMSet (5, rf12_data[4]);  //D (n° de port du jeeNode, n° d'envoi depuis WCat)
    SoftPWMSet(15, rf12_data[5]);   //A
    //Jeenode P2
    SoftPWMSet (6, rf12_data[6]);  //D (n° de port du jeeNode, n° d'envoi depuis WCat)
    SoftPWMSet(16, rf12_data[7]);   //A
    //Jeenode P4
    SoftPWMSet (7, rf12_data[8]);  //D(n° de port du jeeNode, n° d'envoi depuis WCat)
    SoftPWMSet(17, rf12_data[9]);   //A
 
  }
}
