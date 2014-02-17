//dernière mise à jour: 20 février 2011 pour UNO sous logiciel 0022
//ce sketch est conçu pour fonctionner avec WhiteCat Lighting Board
//sit: http://www.le-chat-noir-numerique.fr

// modifié par bret75 les 9 mars 2011 

//chaine des ordres venant de whitecat
#define maxLength 36
String inString = String(maxLength);   

int threshold_analog=0;
//definitions paramètres com:
const int BAUDRATE=9600;//9600 pour 13 IO et 6 ANA ok //11400 pour 54 IO
//11400 ppour 54 IO
//définitions du nombre de capteurs ( doit concorder aussi dans whitecat -> arduino cfg)
const int NBR_ANALOG=6;
const int NBR_DIGITAL=13;
//création d'un tableau de définition pour les I/O 
boolean digital_is_output[NBR_DIGITAL];
boolean allow_analog_read[NBR_ANALOG];
boolean allow_pwm_write[NBR_DIGITAL];
////////////////////////////////////////////////////
//tableaux de stockage des valeurs reçues en input
byte buffer_analog[NBR_ANALOG];
byte old_buffer_analog[NBR_ANALOG];
byte buffer_digital[NBR_DIGITAL];
byte old_buffer_digital[NBR_DIGITAL];
byte valeur_pwm[NBR_DIGITAL];

/////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(BAUDRATE);
  inString = "";      // remet le buffer à 0

  // initialisation des pin digitals en entrée (haute impédance...)
  for (int i =2; i<NBR_DIGITAL; i++){
    digital_is_output[i]=0;
    pinMode(i,INPUT);    
    allow_pwm_write[i]=0;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // a personnaliser en fonction de l'utilisation de l'arduino
  digital_is_output[2]=1;
  digital_is_output[3]=1;
  digital_is_output[4]=1;
  digital_is_output[5]=1;
  digital_is_output[6]=1;
  digital_is_output[7]=1;
  digital_is_output[8]=1;
  digital_is_output[9]=1;//sortie mais PWM
  allow_pwm_write[9]=1;
  digital_is_output[10]=1;//sortie mais PWM
  allow_pwm_write[10]=1;
  digital_is_output[11]=1;
  allow_pwm_write[10]=1;
  digital_is_output[12]=0;
  digital_is_output[13]=0;

  /*autorisation de lecture des analogues. Si vous ne définnissez pas =1 l'analogue ne
   sera pas envoyé à WhiteCat*/
  allow_analog_read[0]=1;
  allow_analog_read[1]=1;
  allow_analog_read[2]=1;
  allow_analog_read[3]=1;
  allow_analog_read[4]=1;
  allow_analog_read[5]=1;
  ////////////////////////

  //INITIALISATION DES ON/OFF sur la carte
  for(int i=2; i<NBR_DIGITAL; i++){
    digital_is_output[i] ? pinMode(i,OUTPUT): pinMode(i,INPUT);
  }
}

/////////////////////////////////////////////////////////////////
void loop()
{
  while(Serial.available() > 0) {
    char inChar=Serial.read(); // lire caractere
    if(inChar!='\0'){    // un caractere nul marque la fin d'un ordre...
      (inString.length() <maxLength) ? inString+=inChar : inString = inChar;  // vérifie taille max du buffer de réception
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
  inString="";//nettoyage de l'ordre
}
///////////////////////////////////////////////////////////////////////
void whitecat_to_dig()
{
  for(int i=2; i<NBR_DIGITAL; i++)
  {
    if(digital_is_output[i]==1 &&  allow_pwm_write[i]==0)
    {                            //32= valeur 0
      if(inString[i+3]!=32){
        digitalWrite(i,HIGH);
      } 
      else {
        digitalWrite(i,LOW);
      }
    }
  }
}
//////////////////////////////////////////////////////////////////////
void whitecat_to_pwm()
{
  for(int i=2; i<NBR_DIGITAL; i++)    // saute les 2 premiers tx/rx ...
  {
    if(allow_pwm_write[i]==1)
    {
      valeur_pwm[i]=byte(inString[i+5]-1);    // attention, caracteres PW/, puis 2 premiers canaux tx/rx, puis valeurs valides !!
      analogWrite(i,valeur_pwm[i]);
    }

  }
}
//////////////////////////////////////////////////////////////////////
void arduino_to_whitecat()
{
  //digitales///////////////////
  bool change_on_dig=0;
  for(int i=2;i<NBR_DIGITAL;i++)//on zappe les états de TX RX en pins 0 et 1
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
  for(int i=0;i<4;i++){  // on vérifie qu'il n'y a pas de valeur 0
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




