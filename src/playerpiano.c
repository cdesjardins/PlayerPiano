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
    2 Stepper motors
    4 h-bridges
    2 servos
    1 gumstix overo
    1 pluto-3 cyclone-II fpga (knjn.com)
    1 piano
    1 ATX power supply
    1 usb to serial converter
    plus a few gears, springs, belts, aluminum, nuts, bolts, wires, and legos... 

    The end result is a PlayerPiano, with the skill of a 1st grader. 

    This code is not the final PlayerPiano project, but it does all of
    the setup and I/O necessary to interface with the FPGA implement the 
    PlayerPiano with the above hard hooked together just right.
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PLAYERPIANO_STEPPER_A   0
#define PLAYERPIANO_STEPPER_B   1
#define PLAYERPIANO_SERVO_A     2
#define PLAYERPIANO_SERVO_B     3

void program_fpga(char *devicename, char *filename);

typedef struct 
{
    int gpio;
    FILE* file;
} t_gpio_data;

t_gpio_data g_gpio_data[] =
{
    {174, 0},
    {146, 0},
    {184, 0},
    {170, 0},
    {151, 0},
    {173, 0},
    {172, 0},
    {171, 0},
};

t_gpio_data g_gpio_clock = {163, 0};

/*
** Setup the GPIOs on the gumstix overo
*/
void export_gpio(t_gpio_data *gpio_data)
{
    FILE* fExport;
    char filename[64];

    fExport = fopen("/sys/class/gpio/export", "w");
    if (fExport)
    {
        fprintf(fExport, "%i", gpio_data->gpio);
        fclose(fExport);
        sprintf(filename, "/sys/class/gpio/gpio%i/direction", gpio_data->gpio);
        gpio_data->file = fopen(filename, "w");
        if (gpio_data->file == NULL)
        {
            printf("Unable to open %s\n", filename);
        }
    }
    else
    {
        printf("Unable to open export\n");
    }
}

/*
** Write a single GPIO
*/
void write_bit(t_gpio_data *gpio_data, int sig)
{
    fprintf(gpio_data->file, "%s", (sig & 0x01) ? "high" : "low");
    fflush(gpio_data->file);
    usleep(10);
}

/*
** Write 8 GPIOs, and then hit the clock GPIO
*/
void write_byte(unsigned char data)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        write_bit(&g_gpio_data[i], data >> i);
    }
    write_bit(&g_gpio_clock, 1);
    write_bit(&g_gpio_clock, 0);
}

/*
** The FPGA has 4 registers 16 bit registers implemented in it:
** 2 registers to control the 2 stepper motors, and
** 2 registers to control the 2 servos.
** This function will write any of the registers in the FPGA.
*/
void write_register(int reg, int data)
{
    write_byte(reg & 0xff);
    write_byte((data >> 0) & 0xff);
    write_byte((data >> 8) & 0xff);
}

/*
** This Function is a very simple method to read command line params
*/
int get_arg(int argc, char *argv[], char *arg)
{
    int i;
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], arg) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/*
** Main will program the FPGA over the serial port, and then
** it will make the steppers and servos dance around.
*/
int main(int argc, char *argv[])
{
    int i;
    for (i = 0; i < (sizeof(g_gpio_data) / sizeof(g_gpio_data[0])); i++)
    {
        export_gpio(&g_gpio_data[i]);
    }
    export_gpio(&g_gpio_clock);
    
    if (get_arg(argc, argv, "skipfpga") == 0)
    {
        write_byte(0);
        program_fpga("/dev/ttyUSB0", "playerpiano.rbf");
    }
    for (i = 0; i < 10; i++)
    {
        if (i & 1)
        {
            write_register(PLAYERPIANO_STEPPER_A, 1000);
            write_register(PLAYERPIANO_STEPPER_B, 0);
            write_register(PLAYERPIANO_SERVO_A, 0xcd14);
            write_register(PLAYERPIANO_SERVO_B, 0x2710);
        }
        else
        {
            write_register(PLAYERPIANO_STEPPER_A, 0);
            write_register(PLAYERPIANO_STEPPER_B, 1000);
            write_register(PLAYERPIANO_SERVO_A, 0x2710);
            write_register(PLAYERPIANO_SERVO_B, 0xcd14);
        }
        sleep(1);
    }
    return 0;
}
