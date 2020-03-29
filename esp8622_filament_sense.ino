/*
 * Filename: wemos_filament_sense.ino
 * Author: Tristan Luther
 * Date: 3/23/2020
 * Purpose: Email Client to alert user of empty 3D printer.
 */

/*************** Libraries ********************/
#include "Arduino.h"
#include <EMailSender.h>
#include <ESP8266WiFi.h>

/************** Macros/Globals ****************/

//Email using Gmail use port 465 (SSL) and SMTP Server smtp.gmail.com
//The sending account needs the less secure app option https://myaccount.google.com/lesssecureapps?pli=1
#define emailSenderAccount    "SENDER_EMAIL_USERNAME" //Username of the account sending the email
#define emailSenderPassword   "SENDER_EMAIL_PASSWORD" //Password of the account sending the email
#define emailRecipient        "RECIEVER_EMAIL_USERNAME" //Who is receiving the email
#define smtpServer            "smtp.gmail.com" //Location of SMTP being used
#define smtpServerPort        465 //gmail SSL SMTP Port number
#define emailSubject          "Lulzbot TAZ6: Moarstruder | Out of Filament" //Subject line of the email to be sent
#define emailMessage          "Lulzbot TAZ6: Moarstruder has run out of fillament (filament sensor has been triggered).\n- Wemos D1 Mini Filament Sensor" //Message in the email to be sent

//The two variables below must be replaced with local network credentials
const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASSWORD";

//Sending data object contains config and data to send
EMailSender emailSend(emailSenderAccount, emailSenderPassword);
//The deafult values though the EMailSender class expect a SSL STMP Gmail connection/account
//Can be changed with the commands below:
//EMailSender emailSend(emailSenderAccount, emailSenderPassword, emailSenderAccount, smtpServer, smtpServerPort);

bool activeMode = false; //State machine variable for determining if we are looking for positive edge (filament loaded) or negative edge (filament gone)
uint8_t filamentSwitchPin = 4; //Pin that the filament switch is hooked up to (Wemos pin D2)
bool currentStatus = false; //Used for debouncing of switch
bool lastStatus = false; //Used for debouncing of switch

/***************** Functions ******************/

//Function to connect to WiFi with the given AP SSID & Password
void WiFiConnect(){
  Serial.println(); //Clear the line
  Serial.print("Connecting"); //Print the status of the WiFi connection
  //Create the WiFi connection 
  WiFi.begin(ssid, password);
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

//Prepare the email to be sent
void sendEmail(){
  //Connect to the SMTP server
  EMailSender::EMailMessage message; //Delcare struct for SMTP
  //Modify the SMTP Struct based on the settings given in the Macros
  message.subject = emailSubject;
  message.message = emailMessage;
  //Fill the email response struct from the send
  EMailSender::Response resp = emailSend.send(emailRecipient, message);
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
