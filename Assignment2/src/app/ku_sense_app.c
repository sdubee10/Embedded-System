#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include "ku_sense.h"

#define DEV_NAME "ku_sense_dev"


struct ku_data data;

int main(void)
{
	int fd, ret;

	data.switchPower = 0;

	fd = open("/dev/ku_sense_dev", O_RDWR);

	//count = 0;
	ret = 0;
	while(1)
	{
		ret = ioctl(fd, IOCTL_SWITCH, 0);
		
		if(ret % 4 == 1)
			ioctl(fd, IOCTL_MOTOR1, 0);
		else if (ret % 4 == 2)
			ioctl(fd, IOCTL_MOTOR2, 0);
		else if (ret % 4 == 3)
			ioctl(fd, IOCTL_MOTOR3, 0);
		else
			ret = 0;
	}
	close(fd);
}
