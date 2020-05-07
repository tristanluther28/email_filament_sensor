/*
 * Filename: wemos_filament_sense.ino
 * Author: Tristan Luther
 * Date: 3/23/2020
 * Purpose: Wemos alert user of empty 3D printer.
 */

/*************** Libraries ********************/
#include "Arduino.h" //Tinfoil hat

/************** Macros/Globals ****************/

bool activeMode = false; //State machine variable for determining if we are looking for positive edge (filament loaded) or negative edge (filament gone)
uint8_t filamentSwitchPin = 4; //Pin that the filament switch is hooked up to (Wemos pin D2)
bool currentStatus = false; //Used for debouncing of switch
bool lastStatus = false; //Used for debouncing of switch

/***************** Functions ******************/

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
      Serial.println("Filament Gone: Sounding alarm!");
      //We are moving out of active mode
      activeMode = false;
    }
    //Otherwise the current status becomes the previous
    lastStatus = currentStatus;
 }

}
