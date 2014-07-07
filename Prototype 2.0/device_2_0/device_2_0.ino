/////////////////////////////////////////////////////////////
/*
  Name: Tifa
  Prototype v2
  Function: receive and send from and to server
  the client is the one that initiate connection (arduino) and nawar led based on incoming reply
  Date started: 18/06/2014
  Last modified: 18/06/2014
*/
///////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//DECLERATION

// libraries to use wifi shield
#include <SPI.h>
#include <WiFi.h>

// liberary to save to EEPROM
#include <EEPROM.h>

//places in EEPROM to save first time flag(intially OXFF), name byte 1 and byte 2
int FirstTimeFlagLocation = 92;
int NameByte1Location = FirstTimeFlagLocation + 1;
int NameByte2Location = FirstTimeFlagLocation + 2;

// Netowrk SSID & Password
char ssid[] = "Mostafaalex";  
char pass[] = "mostafaaucalex";  
int status = WL_IDLE_STATUS;

// IP & portn number of server because TCP
IPAddress server(192,168,1,3);
int port=14;

// Initialize the client library
WiFiClient client;

// intialise first time flag
boolean FirstTimeFlag;
String name;

// reconnection global because used if disconnected or if not first time
String FirstNameCommand;
String ReconnectionCommand;
String WatchdogCommand;
String error_in_format;
byte buffer[11];
int i=0;
int lamp=8;
int latch=9;
// to be removed *************
int x;

//////////////////////////////////////////////////////////////////
//CODE Start

void setup() { 
// for testing purposes added serial to monitor 
  Serial.begin(9600);
  Serial.println("Attempting to connect to WPA network...");
  Serial.print("SSID: ");
  Serial.println(ssid);
  
//initialise pins  
   pinMode(lamp,OUTPUT);
   pinMode(latch,OUTPUT);
// intiate connection to wifi
  status = WiFi.begin(ssid, pass);
  
//If couldn't connect try to reconnect
  while (status != WL_CONNECTED) {
    //to know where am I 
    Serial.println("Couldn't get a wifi connection");
    // try to reconnect
    status = WiFi.begin(ssid, pass);//connect to wifi
  }
  
//If sucessfully connect try to reconnect 
  if (status == WL_CONNECTED){
     Serial.println("Connected to wifi");
// try to connect to server
     while (!client.connect(server, port)){
       Serial.println("can't connect to server (trying to reconnect) ");
       Serial.println("troubleshooting steps:check 1)server is on, 2) ip and port numbers");
     }
//if connected to server ( just another check to be sure msh lazem    
     if (client.connect(server, port)){
// read flag from memory 
       FirstTimeFlag = EEPROM.read(FirstTimeFlagLocation);
// if first time send 
       if(FirstTimeFlag){
         FirstNameCommand = "060,,,,,";
         client.print(FirstNameCommand);
// just to observe
         Serial.println(FirstNameCommand);
         read_data();
       }
// if not first time just reconnectrion       
       else{
         get_name_from_mem();
         format_commands();
         client.print(ReconnectionCommand);
// just to observe 
         Serial.println(ReconnectionCommand);
       }
     }
  }
}
//end of setup function

// getting name from memory if not first time
void get_name_from_mem(){
     char temp;
     temp = EEPROM.read(NameByte1Location);
     name = (String)temp;
     temp = EEPROM.read(NameByte2Location);
     name += temp;       
}

// in a seprate function because used more then one time in different places
void format_commands(){
     ReconnectionCommand = "081,";
     WatchdogCommand = "082,";
     error_in_format = "069,";
     String temp = ",,,,";
     
     ReconnectionCommand += name;
     ReconnectionCommand += temp;
     
     WatchdogCommand += name;
     WatchdogCommand += temp; 
     
     error_in_format += name;
     error_in_format += temp;  
}

void loop (){
  
 read_data();
 
 if (!FirstTimeFlag){ 
///////////////////*************/////////////////////////////  
//  To be changed just to test new server 
    x++;
  if(x==1000){
    client.print(WatchdogCommand);
    Serial.println(WatchdogCommand);
    x=0;
  }
 } 
//////////////////****************///////////////////////////  
  
// check if dissconnected from server
  while (!(client.connected())) {
    
//Try to reconnect
    client.connect(server, port);    
    
//Observing where am I
    Serial.println("disconnected from server");
    
// remove data from incoming buffer
     client.flush();
    
//send reconnection command 
     if (client.connected()){
// if not first time send reconnection command       
       if (!FirstTimeFlag){
          client.print(ReconnectionCommand);
          Serial.println(ReconnectionCommand);
       }
    else{
// if first time request a name 
         client.print(FirstNameCommand);
         Serial.println(FirstNameCommand);
       }
     }

  }
}
//end of loop function

void read_data(){
  
  if (client.connected()){ 
// to read incoming bytes and get out when there are no bytes to read
    while((client.available())&&(i<12)) { 
//reads byte by byte from available data
       byte c = client.read();
       
// to see incoming messages in serial monitor
       Serial.println(c);
       
// put bytes that are read in a buffer
       buffer[i++]=c;
    }

  }

// only to go in if read data and buffer is full  
  if (i==11){
// 1st level check if format is okay 
   if ((buffer[0]==byte(46))&&(buffer[10]==byte(46))&&(buffer[2]==byte(44))&&(buffer[5]==byte(44))&&(buffer[8]==byte(44)) ){ 
//to test recived name is the same as the name i have   
     String test_name((char)buffer[3]);
     test_name +=(char)buffer[4];
     switch (buffer[1]){
// case 1(49 asci) recive my requested name  
       case 49:
         if (FirstTimeFlag){
//M is 77 in asci
           if (buffer[9]==77){
             FirstTimeFlag=false;  
             EEPROM.write(FirstTimeFlagLocation, FirstTimeFlag);
             EEPROM.write(NameByte1Location, buffer[3]);
             EEPROM.write(NameByte2Location, buffer[4]);
             String name((char)buffer[3]);
             name +=(char)buffer[4];
             Serial.println(name);
             format_commands();
           }
         }
         break;
// case 2(50 asci) do action      
         case 50:
           if (name==test_name){
            Action(); 
           }
         break;
// case 3 (51 asci)server request me to change name     
         case 51:
           if (name==test_name){
             EEPROM.write(NameByte1Location, buffer[6]);
             EEPROM.write(NameByte2Location, buffer[7]);
             String name((char)buffer[6]);
             name +=(char)buffer[7];       
           }
       break;
// report error to server     
       default:
         if(FirstTimeFlag)
           client.print(FirstNameCommand);
          else
           client.print(error_in_format);
     }  
   }else{
     client.print(error_in_format);
   }
       
// return buffer to 0  
  i=0;  
  }
}

void Action(){
  String user((char)buffer[6]);
  user +=(char)buffer[7]; 
  String ack ="113,";
  ack+=name;
  ack+=",";
  ack+=user;
  switch(buffer[9]){
    case byte(255):
      digitalWrite(lamp,HIGH);
      digitalWrite(latch,HIGH);
      ack+=",on,,";
      client.print(ack);
 // monitor
      Serial.print(ack);
    break;
    
    case byte(0):
      digitalWrite(lamp,LOW);
      digitalWrite(latch,HIGH);
      ack+=",off,,";
      client.print(ack);
 // monitor
      Serial.print(ack);      
    break;
    
    default:
      client.print(error_in_format);
  }
}


