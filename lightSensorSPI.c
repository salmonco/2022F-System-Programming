#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h> //Needed for SPI port
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //Needed for SPI port
#include <time.h>
#include <stdint.h>
#include <getopt.h>
#include <sys/ioctl.h> //Needed for SPI port
#include <linux/types.h>
#include <linux/spi/spidev.h> //Needed for SPI port

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

int main(int argc, char *argv[]) {
	int repeat = 100;
	
	int fd = open(DEVICE, O_RDWR);
	if (fd <= 0) {
		printf("Device %s not found\n", DEVICE);
		return -1;
	}
	
	if (prepare(fd) == -1) {
		return -1;
	}

	while(1) {
		printf("light (channel 0) : %u\n", readadc(fd, 0));
		printf("light (channel 2) : %u\n", readadc(fd, 2));
		printf("light (channel 4) : %u\n", readadc(fd, 4));
		printf("light (channel 6) : %u\n", readadc(fd, 6));
		usleep(1000000);
	}

	close(fd);
	
	printf("complete\n");
	
	return(0);
}
