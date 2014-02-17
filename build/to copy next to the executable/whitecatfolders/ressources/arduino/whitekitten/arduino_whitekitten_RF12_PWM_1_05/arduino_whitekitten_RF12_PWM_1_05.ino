////librarie
#include <SoftPWM.h>
///pour RF12
#include <Ports.h>
#include <RF12.h>
#include <RF12sio.h>
/////////////////////

//dernière mise à jour: 22 mai 2012 pour UNO et duemilanove et une board RF12 ou un jeenode ou un jeelink 
//sous IDE 0022
//ce sketch est conçu pour fonctionner avec WhiteCat Lighting Board
//site: http://www.le-chat-noir-numerique.fr
// modifié par Anton le 22 mai 2012

///////////Seul variabale à modifier, doivent correspondre dans whitecat

const int BAUDRATE=14400;//9600 pour 13 IO et 6 ANA ok //14400 pour 54 IO
//11400 ppour 54 IO
//définitions du nombre de capteurs ( doit concorder aussi dans whitecat -> arduino cfg)
const int NBR_ANALOG=6;//nombre de pin analogue
const int NBR_DIGITAL=13;// nombre de pin digital
const int last_io=54;////nombre de port utiliser par l'arduino en comptant les RF12 max 54
//////////////////////////////////////



//chaine des ordres venant de whitecat
#define maxLength 128
String inString = String(maxLength);   
const int total_pin = NBR_ANALOG+NBR_DIGITAL;
int threshold_analog=0;
//definitions paramètres com:

//création d'un tableau de définition pour les I/O 
boolean digital_is_output[last_io+1];
boolean allow_analog_read[last_io+1];
boolean allow_pwm_write[last_io+1];
boolean allow_servo_write[last_io+1];
boolean allow_RF12_write[last_io+1];
boolean pullup[last_io+1];
boolean unaffected[last_io+1];
////////////////////////////////////////////////////
//////////variable RF12
RF12 RF12; //variable de la library RF12sio
byte pending; //variable de déclanchement  
MilliTimer sendTimer; //timer d'envoi


//tableaux de stockage des valeurs reçues en input
byte buffer_analog[NBR_ANALOG+1];
byte old_buffer_analog[NBR_ANALOG+1];
byte buffer_digital[total_pin+1];
byte old_buffer_digital[total_pin+1];
byte valeur_pwm[last_io+1];
byte typePin[last_io+1];
char valeur_RF12[last_io+1];
/////////////////////////////////////////////////////////////////
void setup()
{
  SoftPWMBegin();
  Serial.begin(BAUDRATE);
  rf12_initialize(1, RF12_868MHZ, 212);
  inString = "";      // remet le buffer à 0

  // initialisation des pin digitals en entrée (haute impédance...)
  for (int i =2; i<NBR_DIGITAL+1; i++){
     if(i!=2&&i!=10&&i!=11&&i!=12&&i!=13)
   { 
    pinMode(i,INPUT);
    digitalWrite(i,LOW); 
    digital_is_output[i]=0;
    allow_pwm_write[i]=0;
    allow_servo_write[i]=0;
    allow_RF12_write[i]=0;
    pullup[i]=0;
    unaffected[i]=1;
   }
  }

}

/////////////////////////////////////////////////////////////////
void loop()
{
    while(Serial.available() > 0) {
    char inChar=Serial.read(); // lire caractere
    if(inChar!='\0'){    // un caractere nul marque la fin d'un ordre...
     if (inString.length() <maxLength)
     {
       inString+=inChar;
      } 
     else
    {
     inString ="";
      inString += inChar;
    }  // vérifie taille max du buffer de réception
    } 
    else {
      if (inString.length() >= 3){   // reçu 0, fin d'un ordre, on décode
        read_order();
        break;
      }
      else {
        inString = "";    // on a recu une fin d'ordre, mais moins de 2 caractères, purge le buffer
      }
    }
  
    node(); //fonction d'envoie
  }  
}

/////////////////////
void node(){
  
  rf12_recvDone(); //permet la transimition

//permet la reception
 if (sendTimer.poll(100)) {
    pending = 1; //initialise le canSend
    rf12_canSend();
  }
 
 //fonction d'envoie 
if (rf12_canSend()) {
    rf12_sendStart(0, valeur_RF12, sizeof valeur_RF12);
   
  }  
   
}

//////////////////////
void inputOutput_config()
{
  for(int i=2; i<=total_pin; i++){
    if(i!=2&&i!=10&&i!=11&&i!=12&&i!=13)
   { 
    if(digital_is_output[i]==1&&allow_servo_write[i]==0)
   {
   pinMode(i,OUTPUT); 
   digitalWrite(i,LOW);
   } 
   
   else
   {
   pinMode(i,INPUT);  
   if(pullup[i]==1)
   {
     digitalWrite(i,HIGH);
   }
   else
    {
     digitalWrite(i,LOW);
   }
   }
   }
  }
     SoftPWMBegin();
 }  
/////////////////////////////////////////////////////////////
void read_order()
{
// les trois premiers caractères déterminent l'ordre...
  String WCcde = inString.substring(0,3);    // commande whitecat
  if (WCcde == "SD/"){
    arduino_to_whitecat();//lecture des IO et analogs IN et envoi à whitecat
  }
  else if(WCcde == "DO/"){
    whitecat_to_dig();//envoi par white cat des IO
  }
  else if(WCcde == "PW/"){
    whitecat_to_pwm();//envoi par whitecat des pwm

  } 
  
  else if(WCcde == "CO/"){
    whitecat_to_config();//envoi par whitecat des pwm
    
  }
  
  inString="";//nettoyage de l'ordre
}
///////////////////////////////////////////////////////////////////////
void whitecat_to_dig()
{
  for(int i=2; i<total_pin+1; i++)
  {
    if(digital_is_output[i]==1 &&  allow_pwm_write[i]==0 &&allow_servo_write[i]==0)
    {  
      
      //32= valeur 0
  if(i!=2&&i!=10&&i!=11&&i!=12&&i!=13)
   {      
      if(inString[i+3]!=32){
        SoftPWMSet(i,255);
      } 
      else {
        SoftPWMSet(i,0);
     }   
      }
    }
  }
}
//////////////////////////////////////////////////////////////////////
void whitecat_to_pwm()
{
  for(int i=2; i<last_io+1; i++)    // saute les 2 premiers tx/rx ...
  {  
      if(allow_RF12_write[i]==1)
   {
      valeur_RF12[i]=byte(inString[i+3]-1);
   }
   
     if(i<=total_pin&&i!=2&&i!=10&&i!=11&&i!=12&&i!=13)
    {
     if(allow_pwm_write[i]==1&&allow_servo_write[i]==0)
    {
      valeur_pwm[i]=byte(inString[i+3]-1);    // attention, caracteres PW/, puis 2 premiers canaux tx/rx, puis valeurs valides !!
      SoftPWMSet(i,valeur_pwm[i]);
      Serial.print(valeur_pwm[i],DEC);
    }
    }
    else
   {
   if(allow_RF12_write[i]==1)
   {
      valeur_RF12[i]=byte(inString[i+3]-1);
   }
   }
  }

}

//////////////////////////////////////////////////////////////////////
void whitecat_to_config()
{
  for(int i=2; i<last_io+1; i++)    // saute les 2 premiers tx/rx ...
  {
      typePin[i]=byte(inString[i+3]);    // attention, caracteres PW/, puis 2 premiers canaux tx/rx, puis valeurs valides !!

      switch(typePin[i])
      {
       case 30: //NONE
       unaffected[i]=1;
       digital_is_output[i]=0;
       allow_pwm_write[i]=0;
       allow_analog_read[i]=1;
       allow_servo_write[i]=0;
       allow_RF12_write[i]=0;
       pullup[i]=0;
       break;
       
    case 31: //INPUT
       unaffected[i]=0;
       digital_is_output[i]=0;
       allow_pwm_write[i]=0;
       allow_analog_read[i]=0;
       allow_servo_write[i]=0;
      allow_RF12_write[i]=0;
      pullup[i]=0;
      break;
      
       case 33: //PULLUP
       unaffected[i]=0;      
       digital_is_output[i]=0;
       allow_pwm_write[i]=0;
       allow_analog_read[i]=0;
       allow_servo_write[i]=0;
       allow_RF12_write[i]=0;
       pullup[i]=1;
       break;
       
       case 34: //OUTPUT
       unaffected[i]=0;  
       digital_is_output[i]=1;
       allow_pwm_write[i]=0;
       allow_analog_read[i]=0;
       allow_servo_write[i]=0;
       allow_RF12_write[i]=1;
       pullup[i]=0;
       break;
       
      case 35: //PWM
       unaffected[i]=0; 
       digital_is_output[i]=1;
       allow_pwm_write[i]=1;
       allow_analog_read[i]=0;
       allow_servo_write[i]=0;
       allow_RF12_write[i]=1;
       break;
       
       case 36: //SERVO 
       unaffected[i]=0;   
       digital_is_output[i]=1;
       allow_pwm_write[i]=0;
       allow_analog_read[i]=0;
       allow_servo_write[i]=1;
       allow_RF12_write[i]=1;
       pullup[i]=0;
       break;
       case 37: //RF12 PWM
       unaffected[i]=0;   
       digital_is_output[i]=1;
       allow_pwm_write[i]=0;
       allow_analog_read[i]=0;
       allow_servo_write[i]=0;
       allow_RF12_write[i]=1;
       pullup[i]=0;
       break;
      }
   }
   
   for(int i=0;i<NBR_ANALOG+1;i++)
   {
    if( unaffected[i+NBR_DIGITAL]==1)
    {
     allow_analog_read[i]=1; 
    }
   }
     inputOutput_config();
   
}
//////////////////////////////////////////////////////////////////////
void arduino_to_whitecat()
{
  //digitales///////////////////
  bool change_on_dig=0;
  for(int i=2;i<total_pin+1;i++)//on zappe les états de TX RX en pins 0 et 1
  {
    old_buffer_digital[i]=buffer_digital[i];
    //si le digital est une entrée
    if(digital_is_output[i]==0 && allow_pwm_write[i]==0)
    {
      buffer_digital[i]= digitalRead(i);
      if(old_buffer_digital[i]!=buffer_digital[i])
      {
        change_on_dig=1; 
      } 
    }
  }

  //analogues////////////////////
  for(int i=0;i<NBR_ANALOG;i++){  // on vérifie qu'il n'y a pas de valeur 0
    if(buffer_analog[i] == 0)buffer_analog[i]=1;
  }


  bool change_on_ang=0;
  for(int i=0;i<NBR_ANALOG;i++)
  {
    old_buffer_analog[i]=buffer_analog[i]; 
    if(allow_analog_read[i]==1)//si on autorise la lecture de cet analog
    {
      int temp_val=analogRead(i); 
      if((temp_val/4)>old_buffer_analog[i]+threshold_analog || (temp_val/4)<old_buffer_analog[i]-threshold_analog)
      {
        buffer_analog[i]=byte(temp_val/4);
        change_on_ang=1;
      }
    }  
  }
  
  
  //envois
  if(change_on_dig==1)
  {
    Serial.print("DG/");
    Serial.write(buffer_digital,NBR_DIGITAL);
    Serial.println("");
  }
  if(change_on_ang==1)
  {
    Serial.print("AN/");
    Serial.write(buffer_analog,NBR_ANALOG);
    Serial.println(""); 
  } 
}
/////////////////////////////////////////////////////////////////////////




