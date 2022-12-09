#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h> /* Needed for SPI port */
#include <sys/ioctl.h> /* Needed for SPI port */
#include <linux/spi/spidev.h> /* Needed for SPI port */

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

/*
 * (SGL/DIF = 1, D2=D1=D0=0)
 */
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
	int serv_sock,clnt_sock=-1;
	struct sockaddr_in serv_addr,clnt_addr;
	socklen_t clnt_addr_size;
    
	char msg1[10];
	char msg2[10];
	char msg3[10];
	char msg4[10];
	char msg5[10];
	char msg6[10];
	int str_len1;
	int str_len2;
	int str_len3;
	int str_len4;
	
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
	
	int v;
	int h;

	if(argc!=2){
		printf("Usage : %s <port>\n",argv[0]);
	}
    
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1)
		error_handling("socket() error");

	memset(&serv_addr, 0 , sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
		error_handling("bind() error");

	if(listen(serv_sock,5) == -1)
		error_handling("listen() error");

	if(clnt_sock<0){           
		clnt_addr_size = sizeof(clnt_addr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
		if(clnt_sock == -1)
		    error_handling("accept() error");   
	}
	
	int fd = open(DEVICE, O_RDWR);
	if (fd <= 0) {
		printf("Device %s not found\n", DEVICE);
		return -1;
	}
	
	if (prepare(fd) == -1) {
		return -1;
	}

	while(1) {
		top_left = readadc(fd, 0);
		top_right = readadc(fd, 2);
		bottom_left = readadc(fd, 4);
		bottom_right = readadc(fd, 6);
		
		printf("light (channel 0) : %u\n", top_left);
		printf("light (channel 2) : %u\n", top_right);
		printf("light (channel 4) : %u\n", bottom_left);
		printf("light (channel 6) : %u\n", bottom_right);
		
		/*top_left = 200;
		top_right = 600;
		bottom_left = 400;
		bottom_right = 800;*/
		
		/*snprintf(msg1,10,"%d",top_left);
		snprintf(msg2,10,"%d",top_right);
		snprintf(msg3,10,"%d",bottom_left);
		snprintf(msg4,10,"%d",bottom_right);*/
		/*write(clnt_sock, msg1, sizeof(msg1));*/
		
		str_len1 = read(clnt_sock, msg1, sizeof(msg1));
		str_len2 = read(clnt_sock, msg2, sizeof(msg2));
		str_len3 = read(clnt_sock, msg3, sizeof(msg3));
		str_len4 = read(clnt_sock, msg4, sizeof(msg4));
		if(str_len1 == -1)
		    error_handling("read() error");
		if(str_len2 == -1)
		    error_handling("read() error");
		if(str_len3 == -1)
		    error_handling("read() error");
		if(str_len4 == -1)
		    error_handling("read() error");
		
		printf("Receive message1 from Client : %s\n", msg1);
		printf("Receive message2 from Client : %s\n", msg2);
		printf("Receive message3 from Client : %s\n", msg3);
		printf("Receive message4 from Client : %s\n", msg4);
		
		top_middle = atoi(msg1);
		middle_left = atoi(msg2);
		middle_right = atoi(msg3);
		bottom_middle = atoi(msg4);
		
		/*top_middle = 100;
		middle_left = 400;
		middle_right = 300;
		bottom_middle = 200;*/
		
		printf("top_middle : %d\n",top_middle);
		printf("middle_left : %d\n",middle_left);
		printf("middle_right : %d\n",middle_right);
		printf("bottom_middle : %d\n",bottom_middle);
		
		/* compute average */
		avg_top = (top_left + top_middle + top_right) / 3;
		avg_bottom = (bottom_left + bottom_middle + bottom_right) / 3;
		avg_left = (top_left + middle_left + bottom_left) / 3;
		avg_right = (top_right + middle_right + bottom_right) / 3;
		printf("avg_top : %d\n", avg_top);
		printf("avg_bottom : %d\n", avg_bottom);
		printf("avg_left : %d\n", avg_left);
		printf("avg_right : %d\n", avg_right);
		
		axis_h = avg_top - avg_bottom;
		axis_v = avg_right - avg_left;
		printf("axis_h : %d\n", axis_h);
		printf("axis_v : %d\n", axis_v);
		if (axis_h > 0) {
			printf("axis_h > 0\n");
			h = 1;
		} else if (axis_h < 0) {
			printf("axis_h < 0\n");
			h = -1;
		} else {
			printf("axis_h = 0\n");
			h = 0;
		}
		
		if (axis_v > 0) {
			printf("axis_v > 0\n");
			v = 1;
		} else if (axis_v < 0) {
			printf("axis_v < 0\n");
			v = -1;
		} else {
			printf("axis_v = 0\n");
			v = 0;
		}
		
		/* send (v, h) to servo_client */
		snprintf(msg5,10,"%d",v);
		snprintf(msg6,10,"%d",h);
		write(clnt_sock, msg5, sizeof(msg5));
		write(clnt_sock, msg6, sizeof(msg6));
		
		sleep(1);
	}
	close(fd);
	close(clnt_sock);

	return(0);
}
