#include <wiringPi.h>
#include <softPwm.h>
#define SERVO1 18
#define SERVO2 16

int main(){
    wiringPiSetupGpio();
    pinMode(SERVO1,OUTPUT);
    pinMode(SERVO2,OUTPUT);
    softPwmCreate(SERVO1,0,200);
    sofoPwmCreate(SERVO2,0,200);

    for(;;){
        softPwmWrite(SERVO1,10);
        softPwmWrite(SERVO2,10);
        delay(500);
        softPwmWrite(SERVO1,15);
        softPwmWrite(SERVO2,15);
        daley(500)
        softPwmWrite(SERVO1,20);
        softPwmWrite(SERVO2,20);
        delay(500);
    }
    return 0;
}