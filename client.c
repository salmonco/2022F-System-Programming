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

#define VALUE_MAX 30

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
	char path[VALUE_MAX];
	char value_str[3];
	int fd;
	
	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
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
	char path[VALUE_MAX];
	int fd;
	
	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
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

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    char msg1[10];
    char msg2[10];
    char msg3[10];
    char msg4[10];
    char on[2]="1";
    int str_len1;
    int str_len2;
    int str_len3;
    int str_len4;
    int light = 0;
    
    if(argc!=3){
        printf("Usage : %s <IP> <port>\n",argv[0]);
        exit(1);
    }

    //Enable GPIO pins
    if (-1 == GPIOExport(POUT))
        return(1);

    //Set GPIO directions
    if (-1 == GPIODirection(POUT, OUT))
        return(2);
        sock = socket(PF_INET, SOCK_STREAM, 0);
        if(sock == -1)
            error_handling("socket() error");
        
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
        serv_addr.sin_port = htons(atoi(argv[2]));  
        
        if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
            error_handling("connect() error");

//int repeat = 3;
int top_left = 0;
int top_middle = 0;
int top_right = 0;
int middle_left = 0;
int middle_right = 0;
int bottom_left = 0;
int bottom_right = 0;
int bottom_middle = 0;

int avg_top = 0;
int avg_bottom = 0;
int avg_left = 0;
int avg_right = 0;

int axis_h = 0;
int axis_v = 0;

 while(1) {                    
        /*str_len1 = read(sock, msg1, sizeof(msg1));
	str_len2 = read(sock, msg2, sizeof(msg2));
	str_len3 = read(sock, msg3, sizeof(msg3));
	str_len4 = read(sock, msg4, sizeof(msg4));
        if(str_len1 == -1)
            error_handling("read() error");
	if(str_len2 == -1)
            error_handling("read() error");
	if(str_len3 == -1)
            error_handling("read() error");
	if(str_len4 == -1)
            error_handling("read() error");
	
        printf("Receive message1 from Server : %s\n", msg1);
	printf("Receive message2 from Server : %s\n", msg2);
	printf("Receive message3 from Server : %s\n", msg3);
	printf("Receive message4 from Server : %s\n", msg4);
	
	top_middle = atoi(msg1);
	middle_left = atoi(msg2);
	middle_right = atoi(msg3);
	bottom_middle = atoi(msg4);*/
	top_middle = 100;
	middle_left = 400;
	middle_right = 300;
	bottom_middle = 200;
	printf("top_middle : %d\n",top_middle);
	printf("middle_left : %d\n",middle_left);
	printf("middle_right : %d\n",middle_right);
	printf("bottom_middle : %d\n",bottom_middle);
	
	top_left = 200;
	top_right = 600;
	bottom_left = 400;
	bottom_right = 800;
	
	//average
	avg_top = (top_left + top_middle + top_right) / 3;
	avg_bottom = (bottom_left + bottom_middle + bottom_right) / 3;
	avg_left = (top_left + middle_left + bottom_left) / 3;
	avg_right = (top_right + middle_right + bottom_right) / 3;
	printf("avg_top : %d\n", avg_top);
	printf("avg_bottom : %d\n", avg_bottom);
	printf("avg_left : %d\n", avg_left);
	printf("avg_right : %d\n", avg_right);
	
	axis_h = avg_top - avg_bottom;
	axis_v = avg_left - avg_right;
	printf("axis_h : %d\n", axis_h);
	printf("axis_v : %d\n", axis_v);
	if (axis_h > 0) {
	} else if (axis_h < 0) {
	} else {
	}
	
	if (axis_v > 0) {
	} else if (axis_v < 0 ) {
	} else {
	}
	
        if(strncmp(on,msg1,1)==0)
            light = 1;
        else
            light = 0;
	
        GPIOWrite(POUT, light);
        //repeat = repeat -1;
	break;
    }
    close(sock);
    //Disable GPIO pins
    if (-1 == GPIOUnexport(POUT))
        return(4);

    return(0);
}
