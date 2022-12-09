#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#define IN  0
#define OUT 1
#define LOW  0
#define HIGH 1
#define POUT 17
#define PWM 2
#define VALUE_MAX_GPIO 30
#define VALUE_MAX_PWM 256
#define BUFFER_MAX 3
#define DIRECTION_MAX 45

static int GPIOExport(int pin) {
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;
	
	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open export for writing!\n");
		return(-1);
	}
	
	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}

static int GPIOUnexport(int pin) {
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;
	
	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open unexport for writing!\n");
		return(-1);
	}
	
	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}

static int GPIODirection(int pin, int dir) {
	static const char s_directions_str[] = "in\0out";
	
	char path[DIRECTION_MAX] = "/sys/class/gpio/gpio%d/direction";
	int fd;
	
	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
	
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio direction for writing!\n");
		return(-1);
	}
	
	if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
		fprintf(stderr, "Failed to set direction!\n");
		close(fd);
		return(-1);
	}
	
	close(fd);
	return(0);
}

static int GPIORead(int pin) {
	char path[VALUE_MAX_GPIO];
	char value_str[3];
	int fd;
	
	snprintf(path, VALUE_MAX_GPIO, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_RDONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio value for reading!\n");
		return(-1);
	}
	
	if (-1 == read(fd, value_str, 3)) {
		fprintf(stderr, "Failed to read value!\n");
		return(-1);
	}
	
	close(fd);
	
	return(atoi(value_str));
}

static int GPIOWrite(int pin, int value) {
	static const char s_values_str[] = "01";
	char path[VALUE_MAX_GPIO];
	int fd;
	
	snprintf(path, VALUE_MAX_GPIO, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio value for writing!\n");
		return(-1);
	}
	
	if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
		fprintf(stderr, "Failed to write value!\n");
		return(-1);
	}
	
	close(fd);
	return(0);
}

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

int main(int argc, char *argv[]) {
	int sock;
	struct sockaddr_in serv_addr;
	
	int errchk;
	char msg1[2];
	char msg2[2];
	
	char case_v[2];
	char case_h[2];
	int v, h;	// v = vertical, h = horizontal
	v = 500;
	h = 500;
	
	printf("9999999999\n");
	PWMExport(0);
	PWMExport(1);
	
	PWMWritePeriod(0, 20000000);
	PWMWritePeriod(1, 20000000); 
	
	PWMWriteDutyCycle(1, 0);
	PWMWriteDutyCycle(0, 0);
	
	PWMEnable(0);
	PWMEnable(1);
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
	while (1) {
		
		for(int i=0;i<1000;i++){
			PWMWriteDutyCycle(0, 1000*i);
			usleep(1000);
		}
		
		//printf("ddd\n");
		//errchk = read(sock, msg1, sizeof(msg1));
		//if(errchk==-1)
			//error_handling("read() error");
		//strcpy(case_v, msg1);
		//printf("%s\n", case_v);
		
		//errchk = read(sock, msg2, sizeof(msg2));
		//if(errchk==-1)
			//error_handling("read() error");	
		//strcpy(case_h, msg2);
		//printf("%s\n", case_h);
		
		
	//if(strcmp(case_v, "+")==0)
	//{
		//if (strcmp(case_h, "0") == 0)
		//{
			//printf("v=1, h=0\n");
			//PWMWriteDutyCycle(0, 100000);
			////NO HORIZONTAL MOVEMENT
		//}
		//else if (strcmp(case_h, "+") == 0)// '+' ASCII = 43
		//{	printf("v=1, h=1\n");
			//PWMWriteDutyCycle(0, 100000);
			//PWMWriteDutyCycle(1, 100000);
		//}
		//else if (strcmp(case_h, "-") == 0) // '-' ASCII = 45
		//{	printf("v=1, h=-1\n");
			//PWMWriteDutyCycle(0, 100000);
			//PWMWriteDutyCycle(1, 100000);
		//}
		//sleep(1);
	//}

	//else if(strcmp(case_v, "0")==0)
	//{
		//if (strcmp(case_h, "-") == 0)
		//{
			//printf("v=0, h=0\n");
			////NO VERTICAL & HORIZONTAL MOVEMENT
		//}
		//else if (strcmp(case_h, "+") == 0) // '+' ASCII = 43
		//{
			//printf("v=0, h=1\n");
			////NO VERTICAL MOVEMENT
			//PWMWriteDutyCycle(1, 100000);
		//}
		//else if (strcmp(case_h, "-") == 0) // '-' ASCII = 45
		//{	
			//printf("v=0, h=-1\n");
			////NO VERTICAL MOVEMENT
			//PWMWriteDutyCycle(1, 100000);
		//}
		//sleep(1);
	//}

	//else if(strcmp(case_v, "-")==0) 
	//{
		//if (strcmp(case_h, "0") == 0)
		//{
			//printf("v=-1, h=0\n");
			//PWMWriteDutyCycle(0, 10000);
			////NO HORIZONTAL MOVEMENT
		//}
		//else if (strcmp(case_h, "+") == 0) // '+' ASCII = 43
		//{	printf("v=-1, h=1\n");
			//PWMWriteDutyCycle(0, 10000);
			//PWMWriteDutyCycle(1, 10000);
		//}
		//else if (strcmp(case_h, "-") == 0) // '-' ASCII = 45
		//{	printf("v=-1, h=-1\n");
			//PWMWriteDutyCycle(0, 100000);
			//PWMWriteDutyCycle(1, 100000);
		//}
		sleep(1);
	}
	//}
	return 0;
}