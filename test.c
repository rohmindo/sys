#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <time.h>
#include <arpa/inet.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/sysmacros.h>

#define SMOKE_DEV_PATH         "/dev/smoke_dev"
#define BUZZER_DEV_PATH		"/dev/buzzer_dev"
#define HYDRO_DEV_PATH		"/dev/hydro_dev"

#define IOCTL_HYDRO_MAGIC_NUMBER	'h'

#define INTERVAL         50000
#define BUFF_SIZE	 50

typedef struct{
	int hydro_int;
	int hydro_double;
	int temp_int;
	int temp_double;
}HYDRO;

#define IOCTL_CMD_HYDRO			_IOR(IOCTL_HYDRO_MAGIC_NUMBER, 0, HYDRO)
int main(void)
{
   int client_socket;
   struct sockaddr_in server_addr;
   char buff[BUFF_SIZE+5] = "Humidity:";
   char buff2[BUFF_SIZE+5] = "Temperature";

   client_socket = socket(PF_INET, SOCK_STREAM, 0);
   if(client_socket == -1) {
	   printf("socket fail\n");
	   exit(1);
   }

   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(4015);
   server_addr.sin_addr.s_addr = inet_addr("192.168.0.9");

   if(connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
	   printf("connect error\n");
	   return 1;
   }
  
   int smoke_dev, smoke_data;
   int buzzer_dev, buzzer_data = 400;
   int hydro_dev;
   HYDRO hydro_info;
   char Temp[20], Hum[20]; 
   smoke_dev = open(SMOKE_DEV_PATH, O_RDONLY);
   buzzer_dev = open(BUZZER_DEV_PATH, O_RDWR);
   hydro_dev = open(HYDRO_DEV_PATH, O_RDONLY);
   
   if(smoke_dev < 0) {
	   printf("fail to open smoke detection sensor device\n");
	   return 1;
   }

   if(buzzer_dev < 0) {
	   printf("fail to open buzzer sensor device\n");
	   return 1;
   }

   if(hydro_dev < 0) {
	   printf("fail to open hydro sensor device\n");
	   return 1;
   }

   int count = 0;
   
   

   while(1) {
	   read(smoke_dev, &smoke_data, sizeof(int));
	   printf("data : %d\n", smoke_data);
	   if(smoke_data > 600) {
		   write(buzzer_dev, &buzzer_data, sizeof(int)*2);
	   }
	   sleep(1);
	   while(1) {
	       ioctl(hydro_dev, IOCTL_CMD_HYDRO, &hydro_info);
	       printf("temperature is %d.%d%%, humidity is %d.%d%%\n",hydro_info.temp_int, hydro_info.temp_double, hydro_info.hydro_int, hydro_info.hydro_double);

	       if(hydro_info.temp_double > 100)
		    hydro_info.temp_double /= 100;
	       else if(hydro_info.temp_double > 10)
		       hydro_info.temp_double /= 10;

	       if(hydro_info.hydro_double > 100)
		       hydro_info.hydro_double /= 100;
	       else if(hydro_info.hydro_double > 10)
		       hydro_info.hydro_double /= 10;

	       sprintf(Temp, "%d.%dC", hydro_info.temp_int, hydro_info.temp_double);
	       sprintf(Hum, "%d.%d%%", hydro_info.hydro_int, hydro_info.hydro_double);      
	       printf("%s %s\n", Temp, Hum);
	       strcat(buff, Hum);
	       strcat(buff2, Temp);
	       write(client_socket, buff2, strlen(buff2) + 1);
	       write(client_socket, buff, strlen(buff) + 1);
	       printf("%s %s\n", Temp, Hum);
	       sleep(1);
	       if(hydro_info.temp_int != 0)
		  break;
	    }
   }
   
   close(client_socket);
   close(smoke_dev);
   close(buzzer_dev);
   close(hydro_dev);
   
   return 0;
}
