#include "mbed.h"

#include <string>
#include "N5110.h"

Serial pc(USBTX, USBRX);
//      VCC,SCE,RST,D/C,MOSI,SCLK,LED
N5110 lcd(p11,p13,p14,p12,p5,p7,p21);
Ticker ticker;
Timeout pollChange;
DigitalOut led1(LED1);
DigitalOut led2(LED2);
CAN can1(p30, p29);

char counter = 0;
char temp;
float x;
char lcdContrast = 0x19;
bool screenBuffer[84][48];
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
        //pc.printf("Message received: %d from ID:%d\n\r", msg.data[0], msg.id);
        pc.printf("Message recieved of length %d : ", msg.len);
        for(int i = 0; i < msg.len; i++){
            pc.printf("%d ", msg.data[i]);
        }
        pc.printf("from ID %d\n\r", msg.id);
        pc.printf("Reed errors:%d\n\r", can1.rderror());
}


int main() {
    lcd.init();
    pc.baud(115200);
    led2 = 1;
    CANMessage msg;
    pollChange.attach(setTempPoll, 10.0);

    wait(1);
    while(1) {
        led1 = !led1;
        if(can1.read(msg)) {
            CANRecivedMessage(msg);
            led2 = !led2;
            
            // these are default settings so not strictly needed
            lcd.normalMode();      // normal colour mode
            lcd.setBrightness(0.5); // put LED backlight on 50%
            
            if(msg.id == 10){engineTemp = msg.data[0];}
            if(msg.id == 700){mph = msg.data[0];}
            
            char buffer[14];
            memset(&buffer, 0, 14);
            lcd.clear();
            int length = sprintf(buffer,"engine: %d", engineTemp);
            lcd.printString(buffer,0,0);
            sprintf(buffer,"speed: %dmph", mph);
            lcd.printString(buffer,0,1);
        
 
        }
    }
}