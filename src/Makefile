OUTNAME=playerpiano
FILELIST=playerpiano.c fpgaconf.c

all: $(FILELIST)
	arm-none-linux-gnueabi-gcc $^ -o $(OUTNAME) -Wall -I /home/chrisd/source/linux-omap-2.6/include

clean:
	-rm $(OUTNAME)

copy:
	sudo cp $(OUTNAME) /media/$(DISK)/home/root
