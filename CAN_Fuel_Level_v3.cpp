/*
Liam Tracy 28/03/15
UGRacing Team 2 Electronics

CAN Fuel Level v1.3
Program for taking a Fuel Level sensor input every x seconds.
It takes an average of the data collected over a specified
amount of time and sends it out over CAN bus.


v1.3 Changelog:
Added CAN standard commands; eg. sends 0x80 to indicate sensor reading, followed by value

v1.2 Changelog:
This version uses a potential divider to get the input value. In this case it was
a 10k resistor fed from 3.3V with a 10k variable resistor going to ground.
Does not work well when one resistor is much larger than the other. Use 2 around the
same value for the best results.

Added error checking on input:
-Will round numbers bigger than 1.0 to 1.0
*/




#include "mbed.h"

#define CAN_ID 1           //Determines the CAN ID value
#define CAN_speed 100000     //Determines frequency of the CAN bus (Hz)
#define msg_length 2        //Length (in bytes) of the message being sent over CAN bus
#define PollFreq 1         //Determines the polling frequency for the fuel sensor (Hz)
#define SendFreq 5        //Determines how often the data should be sent over CAN (Hz)
#define Resistor_Div 10    //The value of the fixed resistor in the potential divider (kilohms)


//PINS
AnalogIn LevelIn (p20);
CAN CANout(p9, p10);    //CAN bus


//VARIABLES
int LevelVal = 0;      //Variable used to calculate the mean of the fuel level
int i=0;                // "i" is used as a counter for finding the average
char CANdata_array[1];       //Variable that is sent over the CAN bus
//int LevelError =0;      //Variable for checking if the fuel level data being read is correct. 0: No error, 1: Error
Ticker ticker;          //Ticker for calling the send function
CANMessage msg;


//TEST PINS
DigitalOut led1(LED1);  //For testing: LED to indicate when data is being read
DigitalOut led2(LED2);  //For testing: LED to indicate whether message is sent
DigitalOut led3(LED3);  //For testing: LED to indicate whether message is received

//Sums up all of the values retrieved from the AnalogIn value
void gather() {
    i++;                    //Add one to the counter
    
    /*
    //This is for when the variable resistor is connected to Vcc
    float resistance = floor(10*(Resistor_Div*(1-LevelIn))/LevelIn);    //Calculate the resistance of the sensor, turn it into a percentage and round it.
    */
    
    //This is for when the variable resistor is connected to ground
    float resistance = floor(10*(Resistor_Div*LevelIn)/(1-LevelIn));   //Calculate the resistance of the sensor, turn it into a percentage and round it.
    
    //Check if any component tolerance errors occurred, round if true
    if (resistance > 100) {
        resistance = 100;
    }

    LevelVal=(LevelVal+resistance);
    //printf("Analog value %d: %e \r\n", i, resistance);      //For testing: terminal output (AnalogIn value)
    //printf("'LevelVal' value %d: %d \r\n", i, LevelVal);              //For testing: terminal output (Current sum of AnalogIn values)
    led1 = !led1;
}


//Sends out the average fuel level value over CAN
void send_msg() {
    if(CANout.write(CANMessage(CAN_ID, CANdata_array, msg_length))) {      //Send CAN message
        //printf("Data type: %d\r\n", CANdata_array[0]);                   //For testing: terminal output (received data)
        //printf("Data sent: %d\r\n", CANdata_array[1]);                   //For testing: terminal output (received data)
        led2 = !led2;
    }
}


//Main loop
int main() {
    //CANout.frequency(CAN_speed);            //Set the CAN frequency of TX
    ticker.attach(&gather, PollFreq);       //Gather the fuel level value at the polling rate
    while(1) {
        if(i >= SendFreq) {
            CANdata_array[0]=0x80;           //Set the data to be of type sensor data
            CANdata_array[1]=(LevelVal/i);   //Find the average fuel level for the defined time length
            send_msg();          //Send the data over CAN bus
            LevelVal = 0;           //Reset the average value
            i=0;                    //Reset the counter
        }
    }
}