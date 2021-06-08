obj-m:= ku_sense.o

KDIR:=~/linux-rpi/
ARM:= ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi-
PWD:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
	gcc -o $(obj-m:.o=_app) $(obj-m:.o=_app.c)
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
	rm $(obj-m:.o=_app)
scp:
	scp $(obj-m:.o=.ko) pi@10.10.10.12:~/
