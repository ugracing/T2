#include "mbed.h"

#include <string>
#include "N5110.h"

Serial pc(USBTX, USBRX);
Ticker ticker;
Timeout pollChange;
DigitalOut led1(LED1);
DigitalOut led2(LED2);
CAN can1(p30, p29);

char counter = 0;
char temp;
float x;
unsigned int engineTemp = 0;
unsigned int mph = 0;

void setTempPoll(){
    //Set the temp sensor to poll at a new frequency, def in us.
    union{
        char c[4];
        float f;
    };
    f = 1.0;
    char send[5];
    send[0] = 0x01; send[1] = c[0]; send[2] = c[1]; send[3] = c[2]; send[4] = c[3];
    if(can1.write(CANMessage(500, send, 5))){ pc.printf("polling rate changed"); }
}

void CANRecivedMessage(CANMessage msg){
        //This is an ungodly union to convert from the characters to whatever you need.
        union{
            char c[4];
            float f;
            int i;
            long l;
        };
        
        pc.printf("Message recieved of length %d :  ", msg.len);
        //Print all the characters from the message in order.
        for(int i = 0; i < msg.len; i++){
            pc.printf("%d, ", msg.data[i]);
        }
        
        //fill the union
        for(int i = 1; (i < 5) && (i < msg.len); i++){
            c[i - 1] = msg.data[i];
        }
        //Print the data type defined by the first character and convery the characters to that type and display. 
        //The bitwise AND seperates the character to give us only the first two digits.
        switch(msg.data[0] & 0x03){
            case 0:
                //message is a character
                pc.printf("character: %d ", c[0]);
                break;
            case 1:
                //message is a float
                pc.printf("float: %f ", f);
                break;
            case 2:
                //message is an int
                pc.printf("integer: %i ", i);
                break;
            case 3:
                //message is a long
                pc.printf("long: %l ", l);
                break;
        }
        pc.printf("from ID %d\n\r", msg.id);
        pc.printf("Reed errors:%d\n\r", can1.rderror());
}


int main() {
    pc.baud(115200);
    led2 = 1;
    CANMessage msg;
    
    //Set a timer to change the polling rate for the temp sensor after 10 seconds. See if it actually works and all that...
    pollChange.attach(setTempPoll, 10.0);

    wait(1);
    
    while(1) {
        led1 = !led1;
        if(can1.read(msg)) {
            CANRecivedMessage(msg);
            led2 = !led2;
            
            if(msg.id == 10){engineTemp = msg.data[0];}
            if(msg.id == 700){mph = msg.data[0];}        
 
        }
    }
}