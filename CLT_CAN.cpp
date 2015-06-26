/* The purpose of this program is to read the temperature and send it over the CAN bus
   No information is displayed as this will be done by the receiving mbed */

/* Thermistor is a Vishay 2322 640 6153
   Tolerance = 5%
   Operating temperature: -40C to 125C */

/* The potential divider is formed by the thermistor and a 15kohm resistor and is supplied by 3.3V
   The voltage across the 15kohm resistor is measured */

#include "mbed.h"
#include "cmath"

/* Constants of thermistor
   Values used in A, B and C calculated using online Mega/MicroSquirt Calculator */
#define A 0.0009404849219921952
#define B 0.000239245801500490
#define C 0.00000012708217191349288

#define DEFAULT_SEND_RATE 1
#define CAN_ID 10
#define MESSAGE_LENGTH 5
#define FLOAT_BIT 0x81

DigitalOut led1(LED1);
DigitalOut led2(LED2);
Ticker ticker;
CAN can(p30, p29);
AnalogIn sensor(p20);

union fc {
        float f;
        char c[4];
};

//Calculates temperature from an analog value
float getTemperature()
{
    float analog_in = 0;
    float voltage_in = 0;
    float resistance = 0;
    float temperature = 0;


    analog_in = sensor.read();
    voltage_in = (3.3*analog_in); //Conversion from ADC value to actual voltage
    resistance = ((49500/voltage_in)-15000); //Potential Divider formula to get resistance of thermistor

    //Steinhart-Hart Equation for temperature from resistance, originally in Kelvin, -273 for Celsius
    temperature =( 1/(A+(B*log(resistance))+(C*log(resistance)*log(resistance)*log(resistance))))-273;

    return temperature; // Possible uncertainty of +/- 1 degrees
}

//Writes a message to the CAN bus
void send()
{
    fc temperature;
    temperature.f = getTemperature();
    char data[MESSAGE_LENGTH] = {FLOAT_BIT, temperature.c[0], temperature.c[1], temperature.c[2], temperature.c[3]};
    if (can.write(CANMessage(CAN_ID, data, MESSAGE_LENGTH))) {
        led1 = !led1;
    }
}

void setSendRate(float sendRate){
    ticker.attach(&send, sendRate);
}

void receivedMessage(CANMessage msg) {

    //If first byte is device's CAN ID then the next 4 bits are new send rate
    if (msg.data[0] == CAN_ID) {
        fc newSendRate;

        newSendRate.c[0] = msg.data[1];
        newSendRate.c[1] = msg.data[2];
        newSendRate.c[2] = msg.data[3];
        newSendRate.c[3] = msg.data[4];

        setSendRate(newSendRate.f);
    }

}


int main() {
    ticker.attach(&send, DEFAULT_SEND_RATE);
    CANMessage msg;
    while(1) {
        if (can.read(msg)) {
            led2 = !led2;
            receivedMessage(msg);
        }
        wait(0.001);
    }
}

