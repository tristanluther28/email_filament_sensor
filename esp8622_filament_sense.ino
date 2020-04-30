/*
 * Filename: wemos_filament_sense.ino
 * Author: Tristan Luther
 * Date: 3/23/2020
 * Purpose: Email Client to alert user of empty 3D printer.
 */

/*************** Libraries ********************/
#include "Arduino.h" //Tinfoil hat
#include <EMailSender.h> //Email Sending Class
#include <ESP8266WiFi.h> //802.11 Managment
#include <ESP8266WebServer.h>
/* 
   encoded with  xxd -i index.html > buffer.h
   unsigned char needed to be changed do const char
   null-terminate the end of the array (0x00)
   save to flash mem with: ICACHE_RODATA_ATTR
*/
#include "buffer.h" //Contains HTML page

/************** Macros/Globals ****************/

//Email using Gmail use port 465 (SSL) and SMTP Server smtp.gmail.com
//The sending account needs the less secure app option https://myaccount.google.com/lesssecureapps?pli=1

//The two variables below set the SSID and Password for the ESP Access Point to fill out the form.
#define APSSID "WemosFilamentSensor"
#define APPSK  "wem0sfun!"

//Access Point SSID & Password
const char *ssid = APSSID;
const char *password = APPSK;

//Client Network Credentials (will be filled out in the html form)
String clissid;
String clipassword;

ESP8266WebServer server(80); //Server on port 80

//Variables below will contain the data form the users GET request form. This will be used in the client mode
String emailSenderAccount; //Username of the account sending the email
String emailSenderPassword; //Password of the account sending the email
String emailRecipient; //Who is receiving the email
String smtpServer; //Location of SMTP being used
String smtpServerPort; //gmail SSL SMTP Port number
String emailSubject; //Subject line of the email to be sent
String emailMessage; //Message in the email to be sent
char charBufferOne[512]; //Buffer for converting from string
char charBufferTwo[512]; //Buffer for converting from string
char charBufferThree[512]; //Buffer for converting from string
char charBufferFour[512]; //Buffer for converting from string
char charBufferFive[512]; //Buffer for converting from string
char charBufferSix[512]; //Buffer for converting from string
char charBufferSeven[512]; //Buffer for converting from string

bool clientMode = false; //Variable for if we should be in either client or access point mode
bool activeMode = false; //State machine variable for determining if we are looking for positive edge (filament loaded) or negative edge (filament gone)
uint8_t filamentSwitchPin = 4; //Pin that the filament switch is hooked up to (Wemos pin D2)
bool currentStatus = false; //Used for debouncing of switch
bool lastStatus = false; //Used for debouncing of switch

/***************** Functions ******************/

//Send the HTML page to the user when someone connects
void handleRoot() {
 server.send(200, "text/html", index_html); //Send web page
}

//Handler for when the user submits the form
void handleForm() {
 clissid = server.arg("ssid"); 
 clipassword = server.arg("wifipass");
 emailSenderAccount = server.arg("emailsen");
 emailSenderPassword = server.arg("senpass");
 emailRecipient = server.arg("emailrec");
 smtpServer = server.arg("serveraddr");
 smtpServerPort = server.arg("port");
 emailSubject = server.arg("sub"); 
 emailMessage = server.arg("bod");

 Serial.println("Recieved from the user:");
 Serial.print("SSID: ");
 Serial.println(clissid);
 Serial.print("Password: ");
 Serial.println(clipassword);
 Serial.print("Sender Account: ");
 Serial.println(emailSenderAccount);
 Serial.print("Sender Account Password: ");
 Serial.println(emailSenderPassword);
 Serial.print("Recipient Account: ");
 Serial.println(emailRecipient);
 Serial.print("SMTP Server: ");
 Serial.println(smtpServer);
 Serial.print("SMTP Port: ");
 Serial.println(smtpServerPort);
 Serial.print("Email Subject Line: ");
 Serial.println(emailSubject);
 Serial.print("Email Body: ");
 Serial.println(emailMessage);
 
 String s = "<h1>Succesfully Configured! Please disconnect</h1>";
 server.send(200, "text/html", s); //Send web page

 //Put the device is clientMode
 clientMode = true;
 Serial.println("Leaving AP Mode/Switching to Client Mode");
}

//Function to set-up the Wemos Access Point
void WiFiAccessPoint(){
  WiFi.mode(WIFI_AP); //Switching to AP mode
  WiFi.softAP(ssid, password); //Initializing the AP with ssid and password. This will create a WPA2-PSK secured AP
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("");
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  WiFi.begin(); //Begin transactions
  return;
}

//Function to connect to WiFi with the given AP SSID & Password
void WiFiConnect(){
  server.close();
  WiFi.softAPdisconnect(); //Disconnect from the previous mode
  WiFi.disconnect(); //Disconnect from pre-existing connections
  WiFi.mode(WIFI_STA);
  Serial.println(); //Clear the line
  Serial.print("Connecting"); //Print the status of the WiFi connection
  //Create the WiFi connection 
  WiFi.begin(clissid, clipassword);
  //Until connected, wait with prints to the terminal
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  //Print the status of the WiFi connection
  Serial.println();
  Serial.println("WiFi connected.");
  Serial.println();
  return;
}

//Parses the string to convert it to a unsigned 16-bit integer
uint16_t Parse(String str){
  char buff[64];
  str.toCharArray(buff, 64);
  uint16_t num = (uint16_t)strtol(buff, NULL, 10);
  return num;
}

//Prepare the email to be sent
void sendEmail(){
  uint16_t port = Parse(smtpServerPort);
  Serial.print("Using Port: ");
  Serial.println(port);
  //Sending data object contains config and data to send
  emailSenderAccount.toCharArray(charBufferOne, 512);
  emailSenderPassword.toCharArray(charBufferTwo, 512);
  smtpServer.toCharArray(charBufferThree, 512);
  smtpServerPort.toCharArray(charBufferFour, 512);
  emailSubject.toCharArray(charBufferFive, 512);
  emailMessage.toCharArray(charBufferSix, 512);
  emailRecipient.toCharArray(charBufferSeven, 512);
  //EMailSender emailSend(charBufferOne, charBufferTwo);
  //The deafult values though the EMailSender class expect a SSL STMP Gmail connection/account
  //Can be changed with the commands below:
  EMailSender emailSend(charBufferOne, charBufferTwo, charBufferOne, charBufferThree, port);
  //Connect to the SMTP server
  EMailSender::EMailMessage message; //Delcare struct for SMTP
  //Modify the SMTP Struct based on the settings given in the Macros
  message.subject = charBufferFive;
  message.message = charBufferSix;
  //Fill the email response struct from the send
  EMailSender::Response resp = emailSend.send(charBufferSeven, message);
  //Update the debug
  Serial.println("Sending status: ");
  //Print to debug terminal
  Serial.println(resp.status);
  Serial.println(resp.code);
  Serial.println(resp.desc);
  return;
}

//Software debouncing function
bool debounceRead(bool last){
  //Read the current state of the button
  bool current = digitalRead(filamentSwitchPin);
  //Check if the last recorded state is the same as the current
  if(last != current){
    //Delay for 5ms to let the bouncing stop
    delay(5);
    //Read the actual state
    current = digitalRead(filamentSwitchPin);
  }
  //Return the status
  return current;
}

/******************* Set-up *******************/
void setup(){
  Serial.begin(115200); //Initialize UART connection at 115200 baud rate for debugging
  
  //Set-up the pinouts
  pinMode(INPUT, filamentSwitchPin);
  //Begin access point mode
  WiFiAccessPoint();

  //Function to handle the user first signing on
  server.on("/", handleRoot);
  //Form to handle the user's submit
  server.on("/recieve", handleForm);
  //Begin the webserver
  server.begin();  
  //While we are still waiting for the GET Request to set-up email/wifi, wait here
  while(!clientMode){
    //Handle the clients requests while they are still in AP mode
    server.handleClient();
  }
  
  //Set-up the WiFi connection
  WiFiConnect();
}

/******************* Loop *********************/
void loop(){
 //If the button is low then wait for it to be high
 if(!activeMode){
    //Serial.println("We are in: Non-active Mode");
    //This block is the non-active mode. The non-active mode waits for a logic 1 to enter the active mode
    //Software Debouncing
    currentStatus = debounceRead(filamentSwitchPin);
    //If we just detected a positive edge
    if(currentStatus == HIGH && lastStatus == LOW){
      //We are moving into active mode
      activeMode = true;
      //Turn the buzzer off
      Serial.println("Filament Loaded: Turning off alarm!");
    }
    //Otherwise the current status becomes the previous
    lastStatus = currentStatus;
 }
 else{
    //This block is the active mode. The filament has been loaded and will send the email if a logic 0 is detected
    //Serial.println("We are in: Active Mode");
    //Software Debouncing
    currentStatus = debounceRead(filamentSwitchPin);
    //If we just detected a negative edge
    if(currentStatus == LOW && lastStatus == HIGH){
      //Sound the alarm by sending the email
      Serial.println("Filament Gone: Sending the email/sounding alarm!");
      sendEmail();
      //We are moving out of active mode
      activeMode = false;
    }
    //Otherwise the current status becomes the previous
    lastStatus = currentStatus;
 }

}
