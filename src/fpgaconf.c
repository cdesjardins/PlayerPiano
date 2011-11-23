/*
    Copyright 2008 Chris Desjardins - cjd@chrisd.info

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
    A simple module to program the Pluto-3 fpga board from knjn.com over 
    the serial port from Linux.
*/

#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

struct termios oldtio;

void print_termios(struct termios tio)
{
    printf("iflag: %x\n", tio.c_iflag);
    printf("oflag: %x\n", tio.c_oflag);
    printf("cflag: %x\n", tio.c_cflag);
    printf("lflag: %x\n", tio.c_lflag);
}

int serial_init (char *devicename)
{
    int fd;
    struct termios newtio;

    printf ("Serial port: %s\n", devicename);
    fd = open (devicename, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) 
    {
        printf ("Error from serial_init(), %s\n", devicename);
        perror (devicename);
        return -1;
    }
    printf("Got serial\n");
    tcgetattr (fd, &oldtio);   // save current port settings 
    print_termios(oldtio);

    newtio.c_cflag =  B115200 | CS8 | CREAD;
    newtio.c_iflag = IGNBRK;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VMIN] = 1;
    newtio.c_cc[VTIME] = 0;
    tcflush (fd, TCIFLUSH);
    tcsetattr (fd, TCSANOW, &newtio);

    print_termios(newtio);
    
    return fd;
}

int serial_close (int fd)
{
    tcsetattr (fd, TCSANOW, &oldtio);
    close (fd);
    return 0;
}

/* 
 * udelay(1) takes forever... so here is a quicker 
 * delay function, which is fine so long as there
 * is nothing else going on in the system...
 * depending on the processor speed you may
 * need to increase this delay quite a bit.
 * myudelay(2500) works on my gumstix overo 
 * board, (200Mhz arm7) but on my x86 quad 
 * core 2.66Ghz I need udelay(200)...
 */ 
void myudelay(int delay)
{
    volatile int x = 0;
    while (x++ < delay) { }
}

void program_fpga(char *devicename, char *filename)
{
    FILE *fIn;
    char ch;
    int ret;
    int num_bytes = 0;
    int fd;

    fIn = fopen(filename, "rb");
    if (fIn)
    {
        fd = serial_init(devicename);
        if (fd > 0)
        {
            tcsendbreak(fd, 100);
            printf("sent break\n");
            while (fread(&ch, 1,1, fIn) == 1)
            {
                num_bytes++;
                ret = write(fd, &ch, 1);
                myudelay(2500);
            }
            fclose(fIn);
            close(fd);
            printf("sent: %i\n", num_bytes);
            serial_close(fd);
        }
    }
}
