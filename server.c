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
#define PIN  20
#define POUT 21
#define VALUE_MAX 40
#define DIRECTION_MAX 40

static int GPIOExport(int pin) {
#define BUFFER_MAX 3
  char buffer[BUFFER_MAX];
  ssize_t bytes_written;
  int fd;

  fd = open("/sys/class/gpio/export", O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open export for writing!\n");
    return (-1);
  }

  bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
  write(fd, buffer, bytes_written);
  close(fd);
  return (0);
}

static int GPIOUnexport(int pin) {
  char buffer[BUFFER_MAX];
  ssize_t bytes_written;
  int fd;

  fd = open("/sys/class/gpio/unexport", O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open unexport for writing!\n");
    return (-1);
  }

  bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
  write(fd, buffer, bytes_written);
  close(fd);
  return (0);
}

static int GPIODirection(int pin, int dir) {
  static const char s_directions_str[] = "in\0out";

  char path[DIRECTION_MAX];
  int fd;

  snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio direction for writing!\n");
    return (-1);
  }

  if (-1 ==
      write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
    fprintf(stderr, "Failed to set direction!\n");
    return (-1);
  }

  close(fd);
  return (0);
}

static int GPIORead(int pin) {
  char path[VALUE_MAX];
  char value_str[3];
  int fd;

  snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
  fd = open(path, O_RDONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio value for reading!\n");
    return (-1);
  }

  if (-1 == read(fd, value_str, 3)) {
    fprintf(stderr, "Failed to read value!\n");
    return (-1);
  }

  close(fd);

  return (atoi(value_str));
}

static int GPIOWrite(int pin, int value) {
  static const char s_values_str[] = "01";

  char path[VALUE_MAX];
  int fd;

  snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio value for writing!\n");
    return (-1);
  }

  if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
    fprintf(stderr, "Failed to write value!\n");
    return (-1);
  }

  close(fd);
  return (0);
}

void error_handling(char *message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}

int main(int argc, char *argv[]) {
    int state = 1;
    int prev_state = 1;
    int light = 0;

    int serv_sock,clnt_sock=-1;
    struct sockaddr_in serv_addr,clnt_addr;
    socklen_t clnt_addr_size;
    char msg[10];
    char msg1[10];
    char msg2[10];
    char msg3[10];
    
    //Enable GPIO pins
    if (-1 == GPIOExport(PIN) || -1 == GPIOExport(POUT))
        return(1);

    //Set GPIO directions
    if (-1 == GPIODirection(PIN, IN) || -1 == GPIODirection(POUT,OUT))
        return(2);

    if ( -1 == GPIOWrite(POUT,1))
        return(3);
    
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
            clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, 	&clnt_addr_size);
            if(clnt_sock == -1)
                error_handling("accept() error");   
        }
while(1)
    {           
        //state = GPIORead(PIN);
       // int  a = 10;
       // char* str;
       // str = itoa(a);
       /* if(prev_state == 0 && state == 1){
            light = (light+1)%2;
            snprintf(msg,2,"%d",light);
            write(clnt_sock, char, sizeof(msg));
            printf("msg = %s\n",msg);
        }*/
        
        snprintf(msg,5,"%d",500);
        snprintf(msg1,5,"%d",1000);
        snprintf(msg2,5,"%d",200);
        snprintf(msg3,5,"%d", 1023);
        write(clnt_sock,msg,sizeof(msg));
        write(clnt_sock,msg1,sizeof(msg1));
        write(clnt_sock,msg2,sizeof(msg2));
        //write(clnt_sock,msg2,sizeof(msg3));
        printf("Connect success\n");
        
        //prev_state = state;
        sleep(1);
    }

    close(clnt_sock);
    close(serv_sock);
    
    //Disable GPIO pins
    if (-1 == GPIOUnexport(PIN) || -1 == GPIOUnexport(POUT))
        return(4);

    return(0);
}
