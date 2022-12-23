#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <stdint.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define IN  0
#define OUT 1
#define LOW  0
#define HIGH 1
#define POUT 17

#define VALUE_MAX 30

#define BUFFER_MAX 3
#define DIRECTION_MAX 45
#define ARRAY_SIZE(array) sizeof(array) / sizeof(array[0])

static const char *DEVICE = "/dev/spidev0.0";
static uint8_t MODE = SPI_MODE_0;
static uint8_t BITS = 8;
static uint32_t CLOCK = 1000000;
static uint16_t DELAY = 5;

/*
 * Ensure all settings are correct for the ADC
 */
static int prepare(int fd) {
	if (ioctl(fd, SPI_IOC_WR_MODE, &MODE) == -1) {
		perror("Can't set MODE");
		return -1;
	}
	
	if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &BITS) == -1) {
		perror("Can't set number of BITS");
		return -1;
	}
	
	if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &CLOCK) == -1) {
		perror("Can't set write CLOCK");
		return -1;
	}
	
	if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &CLOCK) == -1) {
		perror("Can't set read CLOCK");
		return -1;
	}
	
	return 0;
}

/*
 * (SGL/DIF = 0, D2=D1=D0=0)
 */
uint8_t control_bits_differential(uint8_t channel) {
	return (channel & 7) << 4;
}

uint8_t control_bits(uint8_t channel) {
	return 0x8 | control_bits_differential(channel);
}

/*
 * Given a prep'd descriptor, and an ADC channel, fetch the
 * raw ADC value for the given channel.
 */
int readadc(int fd, uint8_t channel) {
	uint8_t tx[] = {1, control_bits(channel), 0};
	uint8_t rx[3];
	
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = DELAY,
		.speed_hz = CLOCK,
		.bits_per_word = BITS,
	};
	
	if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) == 1) {
		perror("IO Error");
		abort();
	}
	
	return ((rx[1] << 8) & 0x300) | (rx[2] & 0xFF);
}


void error_handling(char *message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sock;
    int serv_sock;
    struct sockaddr_in serv_addr;
    char msg1[10];
    char msg2[10];
    char msg3[10];
    char msg4[10];
    char msg5[10];
    char msg6[10];
    char on[2]="1";
    int str_len1;
    int str_len2;
    int str_len3;
    int str_len4;
    int light = 0;
    
    int repeat = 100;
	
	int fd = open(DEVICE, O_RDWR);
	if (fd <= 0) {
		printf("Device %s not found\n", DEVICE);
		return -1;
	}
	
	if (prepare(fd) == -1) {
		return -1;
	}
    
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

int i =0;


 while(1) {                    
	
	
	printf("-----write-----\n");
	
	top_middle = readadc(fd,0);
	printf("top_middle: %u\n", top_middle);
	
	
	middle_left = readadc(fd,2);
	printf("middle_left: %u\n", middle_left);
	
	
	middle_right = readadc(fd,4);
	printf("middle_right: %u\n", middle_right);
	
	
	bottom_middle = readadc(fd,6);
	printf("bottom_middle:%u\n", bottom_middle);
	
	
	snprintf(msg1,10, "%d",top_middle);
	snprintf(msg2,10, "%d",middle_left);
	snprintf(msg3,10, "%d",middle_right);
	snprintf(msg4,10, "%d",bottom_middle);
	
	
	
	
	write(sock,msg1,sizeof(msg1));
	write(sock,msg2,sizeof(msg2));
	write(sock,msg3,sizeof(msg3));
	write(sock,msg4,sizeof(msg4));
	
	
	/*printf("-----read-----");
	str_len1 = read(sock, msg5, sizeof(msg1));
	str_len2 = read(sock, msg6, sizeof(msg2));

        if(str_len1 == -1)
            error_handling("read() error");
	if(str_len2 == -1)
            error_handling("read() error");
        printf("Receive message1 from Server : %s\n", msg5);
	printf("Receive message2 from Server : %s\n", msg6);/*
	
	
	/*top_middle = atoi(msg1);
	middle_left = atoi(msg2);
	middle_right = atoi(msg3);
	bottom_middle = atoi(msg4);*/
	
	
	
	
	sleep(1);
    }
    close(sock);
    //Disable GPIO pins

    return(0);
}
