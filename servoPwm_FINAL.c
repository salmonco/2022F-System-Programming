#include <stdio.h>
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

#define SERVO2 13   // x(volt O)
#define SERVO1 18   // y(volt X)

#define VALUE_MAX_PWM 256
#define DIRECTION_MAX 45
#define BUFFER_MAX 3

void error_handling(char *message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}

static int PWMExport(int pwmnum) {
    char buffer[BUFFER_MAX];
    int bytes_written;
    int fd;
    
    fd = open("/sys/class/pwm/pwmchip0/unexport", O_WRONLY);
    if (-1 == fd) {
	fprintf(stderr, "Failed to open in unexport!\n");
	return(-1);
    }
    
    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pwmnum);
    write(fd, buffer, bytes_written);
    close(fd);
    
    sleep(1);
    fd = open("/sys/class/pwm/pwmchip0/export", O_WRONLY);
    if (-1 == fd) {
	fprintf(stderr, "Failed to open in export!\n");
	return(-1);
    }
    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pwmnum);
    write(fd, buffer, bytes_written);
    close(fd);
    sleep(1);
    return(0);
}

static int PWMEnable(int pwmnum) {
    static const char s_unenable_str[] = "0";
    static const char s_enable_str[] = "1";
    
    char path[DIRECTION_MAX];
    int fd;
    
    snprintf(path, DIRECTION_MAX, "/sys/class/pwm/pwmchip0/pwm%d/enable", pwmnum);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
	fprintf(stderr, "Failed to open in enable!\n");
	return(-1);
    }
    
    write(fd, s_unenable_str, strlen(s_unenable_str));
    close(fd);
    
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
	fprintf(stderr, "Failed to open in enable!\n");
	return(-1);
    }
    
    write(fd, s_enable_str, strlen(s_enable_str));
    close(fd);
    return(0);
}

static int PWMWritePeriod(int pwmnum, int value) {
    char s_values_str[VALUE_MAX_PWM];
    char path[VALUE_MAX_PWM];
    int fd, byte;
    
    snprintf(path, VALUE_MAX_PWM, "/sys/class/pwm/pwmchip0/pwm%d/period", pwmnum);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
	fprintf(stderr, "Failed to open in period!\n");
	return(-1);
    }
    
    byte = snprintf(s_values_str, VALUE_MAX_PWM, "%d", value);
    
    if (-1 == write(fd, s_values_str, byte)) {
	fprintf(stderr, "Failed to write value in period!\n");
	close(fd);
	return(-1);
    }
    
    close(fd);
    return(0);
}

static int PWMWriteDutyCycle(int pwmnum, int value) {
    char s_values_str[VALUE_MAX_PWM];
    char path[VALUE_MAX_PWM];
    int fd, byte;
    
    snprintf(path, VALUE_MAX_PWM, "/sys/class/pwm/pwmchip0/pwm%d/duty_cycle", pwmnum);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
	fprintf(stderr, "Failed to open in duty_cycle!\n");
	return(-1);
    }
    
    byte = snprintf(s_values_str, VALUE_MAX_PWM, "%d", value);
    
    if (-1 == write(fd, s_values_str, byte)) {
	fprintf(stderr, "Failed to write value in duty_cycle!\n");
	close(fd);
	return(-1);
    }
    
    close(fd);
    return(0);
}

int sock;
struct sockaddr_in serv_addr;
int str1, str2;
char msg1[2];
char msg2[2];

void *read_thd() {
    while (1) {
	str1 = read(sock, msg1, sizeof(msg1)); // x
	str2 = read(sock, msg2, sizeof(msg2)); // y
	if(str1==-1)
	    error_handling("read() error");
	if(str2==-1)
	    error_handling("read() error");
	
	printf("msg1: %s\n", msg1);
	printf("msg2: %s\n", msg2);
	
	sleep(1);
    }
}

#define INIT 20
#define GAP 5
int h;
int v;

void *servo_x_thd() {
    while (1) {
        if(!strcmp(msg1,"+"))
        {   
	    for(int i=h*1000;i<(h+GAP)*1000;i++){
		PWMWriteDutyCycle(0, i*100);
		usleep(300);
	    }
	    h+=GAP;
        }

        if(!strcmp(msg1,"-"))
        {
	    for(int i=h*1000;i>(h-GAP)*1000;i--){
		PWMWriteDutyCycle(0, i*100);
		usleep(300);
	    }
	    h-=GAP;
        }
        printf("One loop of servo X End--------------\n");
	sleep(1);
    }
}

void *servo_y_thd() {
    while (1) {
        if(!strcmp(msg2,"+"))
        {   
	    for(int i=v*1000;i<(v+GAP)*1000;i++){
		PWMWriteDutyCycle(1, i*100);
		usleep(300);
	    }
            v+=GAP;
        }

        if(!strcmp(msg2,"-"))
        {
	    for(int i=v*1000;i>(v-GAP)*1000;i--){
		PWMWriteDutyCycle(1, i*100);
		usleep(300);
	    }
            v-=GAP;
        }
        printf("One loop of servo Y End--------------\n");
	sleep(1);
    }
}

int main(int argc, char *argv[]){
    pthread_t p_thread[3];
    int thr_id;
    int status;
    
    PWMExport(0); // pwm0 is gpio18
    PWMExport(1);
    
    PWMWritePeriod(0, 20000000);
    PWMWritePeriod(1, 20000000); 
    
    PWMWriteDutyCycle(0, 0);
    PWMWriteDutyCycle(1, 0);
    
    PWMEnable(0);
    PWMEnable(1);

    if(argc!=3){
        printf("Usage : %s <IP> <port>\n",argv[0]);
        exit(1);
    }
    
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1)
	error_handling("socket() error");
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));  

    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
	    error_handling("connect() error");
	    
    /* init servo position */
    for(int i=0;i<INIT*1000;i++){
	PWMWriteDutyCycle(0, i*100);
	PWMWriteDutyCycle(1, i*100);
	usleep(300);
    }
    v = INIT;
    h = INIT;
    
    /* thread */
    thr_id = pthread_create(&p_thread[0], NULL, read_thd, NULL);
    if (thr_id < 0) {
	    perror("thread create error : ");
	    exit(0);
    }
    
    thr_id = pthread_create(&p_thread[1], NULL, servo_x_thd, NULL);
    if (thr_id < 0) {
	    perror("thread create error : ");
	    exit(0);
    }
    
    thr_id = pthread_create(&p_thread[2], NULL, servo_y_thd, NULL);
    if (thr_id < 0) {
	    perror("thread create error : ");
	    exit(0);
    }
    
    pthread_join(p_thread[0], (void **)&status);
    pthread_join(p_thread[1], (void **)&status);
    pthread_join(p_thread[2], (void **)&status);
    
    close(sock);
    
    return 0;
}
