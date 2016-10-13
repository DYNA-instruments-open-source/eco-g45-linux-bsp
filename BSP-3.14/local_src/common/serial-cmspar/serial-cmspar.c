/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 * Should you need to contact me, the author, you can do so either by
 * e-mail - mail your message to <vojtech@ucw.cz>, or by paper mail:
 * Vojtech Pavlik, Simunkova 1594, Prague 8, 182 00 Czech Republic
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>

#define SERIAL_PORT "/dev/ttymxc1"

struct termios tio_backup;
int ttyfd;

void sigint()
{
	signal(SIGINT,sigint);
	/* Restore serial port configuration before exiting */
	tcsetattr(ttyfd, TCSANOW, &tio_backup);
	close(ttyfd);
	exit(0);
}

int main(int argc, char **argv)
{
	struct termios tio;
	char c;
	int next_character_red = 0;
	int last_character_ff = 0;
	char teststring[] = "Hello World!";

	/* Register signal function for Ctrl+C */
	signal(SIGINT,sigint);

	/* Open serial port */
	ttyfd = open(SERIAL_PORT, O_RDWR | O_NONBLOCK);

	/* Get current port configuration in order to restore it later */
	tcgetattr(ttyfd, &tio_backup);

	/* Configure serial port */
	tcgetattr(ttyfd, &tio);
	cfsetospeed(&tio, B115200);
	cfsetispeed(&tio, B115200);
	tio.c_cflag &= ~(CSIZE | CSTOPB); // 8 data bits, 1 stop bit
	tio.c_cflag |= CS8;
	tio.c_cflag |= CREAD | PARENB | PARODD | CMSPAR; // CMSPAR | PARODD = MARK parity
	tio.c_lflag = 0;
	tio.c_iflag |= INPCK | PARMRK | ICRNL;
	tio.c_iflag &= ~IGNPAR;
	tio.c_cc[VMIN] = 0;
	tio.c_cc[VTIME] = 10;
	tcsetattr(ttyfd, TCSANOW, &tio);

	/* Send initial test string */
	write(ttyfd, teststring, sizeof(teststring));

	printf("Listening for characters on %s (115000 baud, 8M1)... ", SERIAL_PORT);
	printf("Press Ctrl+C to exit\n");

	/*
	 * Now listen on that port and echo received characters.
	 * Furhtermore, print the received characters on the debug console.
	 * If a parity error occurs, print the character with red background.
	 */
	while (1)
	{
		if (read(ttyfd, &c, 1) > 0)
		{
			if (c == 13)
				c = 10;
			if (c == 0xff)
			{
				last_character_ff = 1;
			}
			else if (c == 0 && last_character_ff)
			{
				next_character_red = 1;
				last_character_ff = 0;
			}
			else
			{
				write(ttyfd, &c, 1);
				if (next_character_red)
					printf("\033[0;41m\033[1;37m%c\033[0m", c);
				else
					printf("%c", c);
				fflush(stdout);
				next_character_red = 0;
				last_character_ff = 0;
			}
		}
	}
}
