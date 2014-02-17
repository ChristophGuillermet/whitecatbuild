#include <Ports.h>
#include <RF12.h>
#include <Servo.h>

/////////rf12_data[i] est le bits (0-255) de reception. 
/////Remplacer "i" par le n° d'envoi depuis WCat.
/// le jeenode à des numéro de pin particulier en fonction du port.
///regardez la doc.
//par default sortie 2 à 9 HF = 8 servo sur les pin D et A

int val[8];
Servo servo[8];

void setup () {



   rf12_initialize(2, RF12_868MHZ, 212);
   
    pinMode (4, OUTPUT); //port 1 sortie D
    pinMode (5, OUTPUT); //port 1 sortie I
    pinMode (6, OUTPUT); //port 2 sortie D
    pinMode (7, OUTPUT); //port 2 sortie I
    pinMode (14, OUTPUT); //port 3 sortie D
    pinMode (15, OUTPUT); //port 3 sortie I
    pinMode (16, OUTPUT); //port 4 sortie D
    pinMode (17, OUTPUT); //port 4 sortie I
    
    //attache les servo 0 à 3 au pin 4 à 7 sortie jeenode  D
    for(int i=0; i<4;i++)
    {servo[i].attach(i+4);}
    //attache les servo 0 à 3 au pin 14 à 17 sortie jeenode  A
    for(int i=4; i<8;i++)
    {servo[i].attach(i+10);}

}

void loop () {
    if (rf12_recvDone() && rf12_crc == 0) 
    {
    for(int i=0; i<8;i++)
    {  
    val[i]=map( rf12_data[i+2],0,255,0,179); // les servo sont sur les circuit 2 à 9 de whitecat
    servo[i].write(val[i]);
    }
    }       
}
