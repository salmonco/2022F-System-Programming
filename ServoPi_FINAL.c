#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include <pthread.h>

#define LOW 0
#define HIGH 1
#define IN  0
#define OUT 1
//#define SERVO 1
#define SERVO1 18   // vertical
#define SERVO2 16   // horizontal
#define POUT 17

void error_handling(char *message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}


//void servoPlusControl(int gap)
//{
    //softPwmCreate(SERVO1, 0, 200);
    //digitalWrite(SERVO1, LOW);	// voltage = 0
    //for(int i=0;i<4;i++){
    //softPwmWrite(SERVO1, 15+gap);
    //delay(1000);
    //}
	//return;
//}

//void servoMinusControl(int gap)
//{
    //softPwmCreate(SERVO1, 0, 200);
    //digitalWrite(SERVO1, LOW);
    //for(int i=0;i<4;i++){
	//softPwmWrite(SERVO1, 15-gap);
	//delay(1000);
    //}
	//return;
//}

int sock;
struct sockaddr_in serv_addr;
int str1, str2;
char msg1[2];
char msg2[2];

void *read_thd() {
    while (1) {
	printf("ddd\n");
	str1 = read(sock, msg1, sizeof(msg1));
	str2 = read(sock, msg2, sizeof(msg2));
	if(str1==-1)
	    error_handling("read() error");
	if(str2==-1)
	    error_handling("read() error");
	
	printf("msg1Read: %s\n", msg1);
	printf("msg2Read: %s\n", msg2);
	
	sleep(1);
    }
}

#define START 15
int gap = 1;

int v = START;
int h = START;
int dir=1;

void *servo_thd() {
    while (1) {
	
	printf("msg1Servo: %s\n", msg1);
	printf("msg2Servo: %s\n", msg2);
	
	// Test1
	if((v<5||v>25)&&(h<5||h>25)) 
	{ gap*=-1;
	    printf("print in reverse way\n");
	}
	// Test2
	// if(v<5){for(;v=15;){v++;})
	// if(v>25){for(;v=15;){v--;}}
	// if(h<5){for(;h=15;){h++;}}
	// if(h>25){for(;h=15;){h--;}}

        // if "+" move +2 degree, if "-" move -2 degree, if "0" do not move 
        // degree 2 = 5/45
        if(!strcmp(msg1,"+")&&!strcmp(msg2,"+")) // (+,+)
        {   
            v+=gap;
            h+=gap;
            softPwmWrite(SERVO1, v);
            softPwmWrite(SERVO2, h);
            delay(500);
        }
	    
        if(!strcmp(msg1,"+")&&!strcmp(msg2,"0")) // (+,0)
        {	    
            v+=gap;
            softPwmWrite(SERVO1, v);
	        //
	        delay(500);
        }

        if(!strcmp(msg1,"+")&&!strcmp(msg2,"-")) // (+,-)
        {
            v+=gap;
            h-=gap;
            softPwmWrite(SERVO1,v);
	        softPwmWrite(SERVO2,h);
	        delay(500);
        }

        if(!strcmp(msg1,"0")&&!strcmp(msg2,"+")) // (0,+)
        {
            h+=gap;
            //
            softPwmWrite(SERVO2, h);
            delay(500);
        }

        if(!strcmp(msg1,"0")&&!strcmp(msg2,"0")) // (0,0)
        {
            //
            delay(500);
        }

        if(!strcmp(msg1,"0")&&!strcmp(msg2,"-")) // (0,-)
        {
            h-=gap;
            //
            softPwmWrite(SERVO2, h);
            delay(500);
        }
        
        if(!strcmp(msg1,"-")&&!strcmp(msg2,"+")) // (-,+)
        {
            v-=gap;
            h+=gap;
            softPwmWrite(SERVO1,v);
	        softPwmWrite(SERVO2,h);
	        delay(500);
        }

        if(!strcmp(msg1,"-")&&!strcmp(msg2,"0")) // (-,0)
        {
            v-=gap;
            softPwmWrite(SERVO1,v);
	        // 
	        delay(500);
        }

        if(!strcmp(msg1,"-")&&!strcmp(msg2,"-")) // (-,-)
        {
            v-=gap;
            h-=gap;
            softPwmWrite(SERVO1,v);
	        softPwmWrite(SERVO2,h);
	        delay(500);
        }

        printf("One loop of servo End------------------------\n");
	sleep(1);
    }
    
    
}


int main(int argc, char *argv[]){
    
    /*int sock;
    struct sockaddr_in serv_addr;
    int str1, str2;
    char msg1[2];
    char msg2[2];*/
    
    pthread_t p_thread[2];
    int thr_id;
    int status;
    
    /* servo new test start */
    wiringPiSetupGpio();
    pinMode(SERVO1,OUTPUT);
    pinMode(SERVO2,OUTPUT);
    softPwmCreate(SERVO1,0,200);
    softPwmWrite(SERVO1, 15);
    softPwmCreate(SERVO2,0,200);
    softPwmWrite(SERVO2, 15);
    /* finished */

    printf("0000000\n");
    if(argc!=3){
        printf("Usage : %s <IP> <port>\n",argv[0]);
        exit(1);
    }
    printf("11111111111\n");
    
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1)
	error_handling("socket() error");
    printf("22222\n");
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));  
    printf("3333333333333\n");
    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
	    error_handling("connect() error");

    printf("555555555555\n");
		
    int i, j, vflag, hflag;
    
    /*if(wiringPiSetup()==-1)
    {
	return -1;
    }*/
    
    //while (1) {
	/* read msg */
	/*printf("ddd\n");
	str1 = read(sock, msg1, sizeof(msg1));
	if(str1==-1)
	    error_handling("read() error");
			
	printf("msg1: %s\n", msg1);
		
	str2 = read(sock, msg2, sizeof(msg2));
	if(str2==-1)
	    error_handling("read() error");	
				
	printf("msg2: %s\n", msg2);*/
	
	/* servo */
	/*for(;;){
	    softPwmWrite(SERVO1,10);
	    softPwmWrite(SERVO2,10);
	    delay(500);
	    softPwmWrite(SERVO1,15);
	    softPwmWrite(SERVO2,15);
	    delay(500);
	    softPwmWrite(SERVO1,20);
	    softPwmWrite(SERVO2,20);
	    delay(500);
	}*/
	
	//sleep(1);
    //}
    
    /*while(1)
    {
	printf("ddd\n");
	str1 = read(sock, msg1, sizeof(msg1));
	if(str1==-1)
	    error_handling("read() error");
			
	printf("msg1: %s", msg1);
		
	str2 = read(sock, msg2, sizeof(msg2));
	if(str2==-1)
	    error_handling("read() error");	
				
	printf("msg2: %s\n", msg2);
		
	// currently, servo motor prints direction of axis-x (horizontal)
	if(!strcmp(msg2, "+"))
	{
	    servoPlusControl(4);
	}
	
	else if(!strcmp(msg2, "-"))
	{
	    servoMinusControl(4);
	}
	
	printf("finished horizontal move\n------------------------------");
	
	if(!strcmp(msg1, "+"))
	{
	    servoPlusControl(2);
	}
	
	else if(!strcmp(msg1, "-"))
	{
	    servoMinusControl(2);
	}
	
    }*/
    
    /* thread */
    thr_id = pthread_create(&p_thread[0], NULL, read_thd, NULL);
    if (thr_id < 0) {
	    perror("thread create error : ");
	    exit(0);
    }
    
    thr_id = pthread_create(&p_thread[1], NULL, servo_thd, NULL);
    if (thr_id < 0) {
	    perror("thread create error : ");
	    exit(0);
    }
    
    pthread_join(p_thread[1], (void **)&status);
    pthread_join(p_thread[0], (void **)&status);
    
    close(sock);
    
    return 0;
  }
