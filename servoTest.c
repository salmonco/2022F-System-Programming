#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>

#define SERVO1 18
#define SERVO2 16

int main()
{
	// current preservation
	// 20 difference  = 180 degree(left-most, right-most)
	// 10 difference = 90 degree rotation
	// --> SERVO2 can move by 5/10 interval
	
	// --> SERVO1 can move by 3/13 interval
	wiringPiSetupGpio();
	
	pinMode(SERVO1, OUTPUT);
	pinMode(SERVO2, OUTPUT);
	
	softPwmCreate(SERVO1, 0, 200);
	softPwmCreate(SERVO2, 0, 200);
	
	softPwmWrite(SERVO1, 13);
	//softPwmWrite(SERVO2, 15);
	delay(600);
	
	
	softPwmWrite(SERVO1, 3);
	//softPwmWrite(SERVO2, 5);
	delay(600);
	
	softPwmWrite(SERVO1, 10);
	delay(600);
	
	return 0;
}
