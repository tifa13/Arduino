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

//places in EEPROM to save first time flag(intially OXFF), name bit 1 and bit 2
int e1=83;
int e2=e1+1;
int e3=e1+2;

// Netowrk SSID & Password
char ssid[] = "mostafa";  
char pass[] = "01005381961";  
int status = WL_IDLE_STATUS;

// IP & portn number of server because TCP
IPAddress server(192,168,1,3);
int port=14;

// Initialize the client library
WiFiClient client;

// intialise first time flag
boolean ftime;
String name;
// reconnection global because used if disconnected or if not first time
String fn;
String recon;
String watchdog;
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
       ftime=EEPROM.read(e1);
// if first time send 
       if(ftime){
         fn= "0,,,,,.";
         client.print(fn);
// just to observe
         Serial.println(fn);
         read_data();
       }
// if not first time just reconnectrion       
       else{
         get_name_from_mem();
         format_commands();
         client.print(recon);
// just to observe 
         Serial.println(recon);
       }
     }
  }
}

// getting name from memory if not first time
void get_name_from_mem(){
     char name_1;
     name_1=EEPROM.read(e2);
     name=(String)name_1;
     name_1=EEPROM.read(e3);
     name+=name_1;       
}

// in a seprate function because used more then one time in different places
void format_commands(){
     recon="1,";
     watchdog="2,";
     error_in_format="9,";
     String recon_2=",,,,.";
     recon+=name;
     recon+=recon_2;
     watchdog+=name;
     watchdog+=recon_2; 
     error_in_format+=name;
     error_in_format+=recon_2;  
}

void loop (){
  
 read_data();
 
 if (!ftime){ 
///////////////////*************/////////////////////////////  
//  To be changed just to test new server 
    x++;
  if(x==1000){
    client.print(watchdog);
    Serial.println(watchdog);
    x=0;
  }
 } 
//////////////////****************///////////////////////////  
  
// check if dissconnected from server
  while (!(client.connected())) {
//Observing where am I
    Serial.println("disconnected from server");
    
// remove data from incoming buffer
     client.flush();
    
//send reconnection command 
     if (client.connected()){
// if not first time send reconnection command       
       if (!ftime){
          client.print(recon);
          Serial.println(recon);
       }else{
// if first time request a name 
         client.print(fn);
         Serial.println(fn);
       }
     }
    
//Try to reconnect
    client.connect(server, port);
  }
}

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
    
// return buffer to 0  
  i=0;  
  }
 // 1st level check if formate is okay 
 if ((buffer[0]==byte(46))&&(buffer[10]==byte(46))&&(buffer[2]==byte(44))&&(buffer[5]==byte(44))&&(buffer[8]==byte(44)) ){ 
//to test recived name is the same as the name i have   
   String test_name((char)buffer[3]);
   test_name +=(char)buffer[4];
   switch (buffer[1]){
 // case 1(49 asci) recive my requested name  
     case 49:
       if (ftime){
//M is 77 in asci
         if (buffer[9]==77){
           ftime=false;  
           EEPROM.write(e1, ftime);
           EEPROM.write(e2, buffer[3]);
           EEPROM.write(e3, buffer[4]);
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
           EEPROM.write(e2, buffer[6]);
           EEPROM.write(e3, buffer[7]);
           String name((char)buffer[6]);
           name +=(char)buffer[7];       
         }
     break;
// report error to server     
     default:
       if(ftime)
         client.print(fn);
        else
         client.print(error_in_format);
   }  
 }else{
   client.print(error_in_format);
 }
}

void Action(){
  String user((char)buffer[6]);
  user +=(char)buffer[7]; 
  String ack ="3,";
  ack+=name;
  ack+=",";
  ack+=user;
  switch(buffer[9]){
    case byte(255):
      digitalWrite(lamp,HIGH);
      digitalWrite(latch,HIGH);
      ack+=",on,,.";
      client.print(ack);
 // monitor
      Serial.print(ack);
    break;
    
    case byte(0):
      digitalWrite(lamp,LOW);
      digitalWrite(latch,HIGH);
      ack+=",off,,.";
      client.print(ack);
 // monitor
      Serial.print(ack);      
    break;
    
    default:
      client.print(error_in_format);
  }
}


