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
int e1=62;
int e2=e1+1;
int e3=e1+2;

// Netowrk SSID & Password
char ssid[] = "mostafa";  
char pass[] = "01005381961";  
int status = WL_IDLE_STATUS;

// IP & portn number of server because TCP
IPAddress server(192,168,1,5);
int port=14;

// Initialize the client library
WiFiClient client;

// intialise first time flag
boolean ftime;
String name;
// reconnection global because used if disconnected or if not first time
String recon;
String watchdog;
byte buffer[10];
int i=0;
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
         String fn= "0,,,,,.";
         client.print(fn);
// just to observe
         Serial.println(fn);
         read_data();
       }
// if not first time just reconnectrion       
       else{
         get_name_from_mem();
         reconnection();
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
void reconnection(){
     recon="1,";
     watchdog="2,";
     String recon_2=",,,,.";
     recon+=name;
     recon+=recon_2;
     watchdog+=name;
     watchdog+=recon_2;   
}

void loop (){
  
  read_data();
///////////////////*************/////////////////////////////  
//  To be changed just to test new server
    x++;
  if(x==1000){
    client.print(watchdog);
    Serial.println(watchdog);
    x=0;
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
      client.print(recon);
      Serial.println(recon);
     }
    
//Try to reconnect
    client.connect(server, port);
  }
}

void read_data(){
  
  if (client.connected()){ 
// to read incoming bytes and get out when there are no bytes to read
    while((client.available())&&(i<11)) { 
//reads byte by byte from available data
       byte c = client.read();
       
// to see incoming messages in serial monitor
       //Serial.println(c);
       
// put bytes that are read in a buffer
       buffer[i++]=c;
    }
    
// return buffer to 0  
  i=0;
  
}
  
  
}
