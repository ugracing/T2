#include "mbed.h"
 
Serial pc(USBTX, USBRX);
Ticker tickerTemp;
Ticker tickerSpeed;
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
CAN can1(p9, p10);
AnalogIn temp_pot(p20);
AnalogIn speed_pot(p19);
char counter = 0;
 
void sendTemp() {
    union{
        char c[4];
        float f;
    };
    f = temp_pot;
    //The message is defined as as info, reading, float. 
    char outArray[5] = {0x81, c[0], c[1], c[2], c[3]};
    //printf("send()\n\r");
    //printf("%f\n\r", x);
    if(can1.write(CANMessage(500, outArray, 5))) {
        //printf("wloop()\n\r");
        counter++;
        //printf("Message sent: %d\n\r", temp);
        //printf("Send errors: %d\n\r", can1.tderror());
    } 
    led1 = !led1;
}

void sendSpeed(){
    char mph[2] = {0x80, (int)(160*speed_pot)};
    can1.write(CANMessage(700, mph, 2));
}

void CANCommandTemp(CANMessage msg){
    //first byte details what command
    //Example: change polling rate for temp, first byte should be 1
    //The second and third byte should be the new rate, in float form detailing seconds.
    switch(msg.data[0]){
        case 1:
            union{
                char c[4];
                float f;
            };
            c[0] = msg.data[1]; c[1] = msg.data[2]; c[2] = msg.data[3]; c[3] = msg.data[4];
            //DEBUG
            pc.printf("new polling rate for temp: %f\n\r", f);
            tickerTemp.attach(&sendTemp, f);            
    }
}
 
int main() {
    pc.baud(9600);
    CANMessage msg;
    tickerTemp.attach(&sendTemp, 0.25);
    wait(1);
    tickerSpeed.attach(&sendSpeed, 0.25);
    while(1){
        if(can1.read(msg)){
            if(msg.id == 500){
                CANCommandTemp(msg);
            } else if(msg.id == 700){
                
            }
        }
    }
}