#include <SoftPWM.h>

//dernière mise à jour: 22 mai 2012 pour UNO et duemilanove sous IDE 022 ou 023
//ce sketch est conçu pour fonctionner avec WhiteCat Lighting Board
//site: http://www.le-chat-noir-numerique.fr
// modifié par Anton les 22 mai 2012

///////////Seul variabale à modifier, doivent correspondre dans whitecat
const int BAUDRATE=14400;//9600 pour 13 IO et 6 ANA ok //14400 pour 54 IO
//11400 ppour 54 IO
//définitions du nombre de capteurs ( doit concorder aussi dans whitecat -> arduino cfg)
const int NBR_ANALOG=5;//numero de la derniere pin analogue
const int NBR_DIGITAL=13;// numero de la derniere de pin digital
const int last_io=54;////nombre de port utiliser par l'arduino en comptant les RF12 max 54

//////////////////////////////////////



//chaine des ordres venant de whitecat
#define maxLength 128
String inString = String(maxLength);   
const int total_pin = NBR_ANALOG+NBR_DIGITAL+1;
int threshold_analog=0;
//definitions paramètres com:

//création d'un tableau de définition pour les I/O 
boolean digital_is_output[last_io+1];
boolean allow_analog_read[last_io+1];
boolean allow_pwm_write[last_io+1];
boolean allow_servo_write[last_io+1];
boolean unaffected[last_io+1];
boolean pullup[last_io+1];
////////////////////////////////////////////////////
//tableaux de stockage des valeurs reçues en input
byte buffer_analog[NBR_ANALOG+1];
byte old_buffer_analog[NBR_ANALOG+1];
byte buffer_digital[total_pin+1];
byte old_buffer_digital[total_pin+1];
byte valeur_pwm[last_io+1];
byte typePin[last_io+1];

boolean change_on_dig=0;
boolean change_on_ang=0;
/////////////////////////////////////////////////////////////////
void setup()
{
  SoftPWMBegin();
  Serial.begin(BAUDRATE);
  inString = "";      // remet le buffer à 0

  // initialisation des pin digitals en entrée (haute impédance...)
  for (int i =2; i<=total_pin; i++){
   digitalWrite(i,LOW);
    pinMode(i,INPUT);
    digital_is_output[i]=0;
    allow_analog_read[i]=0;   
    allow_pwm_write[i]=0;
    allow_servo_write[i]=0;
    pullup[i]=0;
    unaffected[i]=1;
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
  }
}

//////////////////////
void inputOutput_config()
{
  for(int i=2; i<=total_pin; i++){
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
  for(int i=2; i<=total_pin; i++)
  {
    if(digital_is_output[i]==1 &&  allow_pwm_write[i]==0 &&allow_servo_write[i]==0)
    {  
      //32= valeur 0
      if(inString[i+3]!=32){
        SoftPWMSet(i,255);
      } 
      else {
        SoftPWMSet(i,0);
      }
    }
  }
}
//////////////////////////////////////////////////////////////////////
void whitecat_to_pwm()
{
  for(int i=2; i<=last_io; i++)    // saute les 2 premiers tx/rx ...
  {
    if(i<=total_pin)
    {
     if(digital_is_output[i]==1&&allow_pwm_write[i]==1&&allow_servo_write[i]==0)
    {
      valeur_pwm[i]=byte(inString[i+3]-1);    // attention, caracteres PW/, puis 2 premiers canaux tx/rx, puis valeurs valides !!
      SoftPWMSet(i,valeur_pwm[i]);
      Serial.print(valeur_pwm[i],DEC);
    }
    }
  }

}

//////////////////////////////////////////////////////////////////////
void whitecat_to_config()
{
  for(int i=2; i<=last_io; i++)    // saute les 2 premiers tx/rx ...
  {
      typePin[i]=byte(inString[i+3]);    // attention, caracteres PW/, puis 2 premiers canaux tx/rx, puis valeurs valides !!

      switch(typePin[i])
      {
       case 30: ///unaffected
       unaffected[i]=1;
       digital_is_output[i]=0;
       allow_pwm_write[i]=1;
       allow_servo_write[i]=0;
       pullup[i]=0;
      
      break;
     
       case 31: ///INPUT
       unaffected[i]=0;
       digital_is_output[i]=0;
       allow_pwm_write[i]=0;
       allow_servo_write[i]=0;
       pullup[i]=0;
       break;
      
       case 33: /// PULL up
       unaffected[i]=0;
       digital_is_output[i]=0;
       allow_pwm_write[i]=0;
       allow_servo_write[i]=0;
       pullup[i]=1;
        break;
        
       case 34: //OUTPUT
       unaffected[i]=0;
       digital_is_output[i]=1;
       allow_pwm_write[i]=0;
       allow_servo_write[i]=0;
       pullup[i]=0;
       break;
       
       case 35: //PWM
       unaffected[i]=0;
       digital_is_output[i]=1;
       allow_pwm_write[i]=1;
       allow_servo_write[i]=0;
       pullup[i]=0;
       break;
      
       case 36: //SERVO
       unaffected[i]=0;
       digital_is_output[i]=1;
       allow_pwm_write[i]=0;
       allow_servo_write[i]=1;
       pullup[i]=0;
       break;
      }
   }
   
   for(int i=0;i<=NBR_ANALOG;i++)
   {
    if( unaffected[i+NBR_DIGITAL+1]==1)
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
for(int i=2; i<=last_io; i++)//on zappe les états de TX RX en pins 0 et 1
{
old_buffer_digital[i]=buffer_digital[i];
//si le digital est une entrée
if(digital_is_output[i]==0 && allow_pwm_write[i]==0)
   {
    buffer_digital[i]= digitalRead(i);
    if(old_buffer_digital[i]!=buffer_digital[i])
    {change_on_dig=1;change_on_ang=0;}
    else
    {change_on_dig=0;change_on_ang=1;}
  }
}

//analogues////////////////////

for(int i=0;i<NBR_ANALOG;i++)
{
 old_buffer_analog[i]=buffer_analog[i]; 
   if(allow_analog_read[i]==1)//si on autorise la lecture de cet analog
   {
    int temp_val=analogRead(i); 
    if((temp_val/4)>old_buffer_analog[i]+threshold_analog || (temp_val/4)<old_buffer_analog[i]-threshold_analog)
    {
    buffer_analog[i]=byte(temp_val/4);
    
    }
   }  
}
//envois
 if(change_on_dig==1&&change_on_ang==0)
{
 Serial.print("DG/");
 Serial.write(buffer_digital,NBR_DIGITAL);
 Serial.println("");
}
 if(change_on_ang==1&&change_on_dig==0)
{
 Serial.print("AN/");
 Serial.write(buffer_analog,NBR_ANALOG);
 Serial.println(""); 
} 
}
/////////////////////////////////////////////////////////////////////////



