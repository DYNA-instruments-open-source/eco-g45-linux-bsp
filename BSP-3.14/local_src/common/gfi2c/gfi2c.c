// gfi2c.c : Defines the entry point for the application.
//

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "Version.h"

#define _QUOTEME(x) #x
#define QUOTEME(x) _QUOTEME(x)

#define GF_SVN_VERSION "$Revision: 595 $"
#define TOOL_NAME "gfi2c"

#define DEFAULT_I2C_BUS		"/dev/i2c-0"
#define DEFAULT_WIDTH		8
#define DEFAULT_NUM			1
#define DEFAULT_DELAY		0
#define DEFAULT_ADDRESS		0x00
#define DEFAULT_REGISTER	0x00
#define DEFAULT_DATA		-1
#define DEFAULT_VERBOSITY	0
#define DEFAULT_MASK		0
#define DEFAULT_SINGLE		0
#define DEFAULT_IGN_ACK		0
#define DEFAULT_STOP_ERR	0

#define GFI2C_OP_READ 			"--read"
#define GFI2C_OP_WRITE 		    "--write"
#define GFI2C_OP_TEST 		    "--test"
#define GFI2C_OP_HELP 			"--help"
#define GFI2C_OPTION_NUM		"-n"
#define GFI2C_OPTION_DELAY		"-d"
#define GFI2C_OPTION_WIDTH		"-w"
#define GFI2C_OPTION_REGWIDTH	"-r"
#define GFI2C_OPTION_SINGLE		"-s"
#define GFI2C_OPTION_IGN_ACK	"-i"
#define GFI2C_OPTION_STOP_ERR	"-e"
#define GFI2C_OPTION_VERBOSE	"-v"

#define PRINTF(s) printf("%s\n", s)

static const char wszHelp[] =
{
	TOOL_NAME " version " MAJOR_VERSION "." MINOR_VERSION " [" GF_SVN_VERSION "] (" __DATE__ ") <westerburg@garz-fricke.com> \n"
	"usage:\n"
	"  " GFI2C_OP_READ " <i2c-bus device file> <i2c device address> <register address> [<data>]\n"
	"      Reads the specified register from the specified I2C device.\n"
	"      If the optional <data>-value is specified, read data is compared against this value.\n"
	"  " GFI2C_OP_WRITE " <i2c-bus device file> <i2c device address> <register address> <data>\n"
	"      Writes the given data to the specified register of the specified I2C device.\n"
	"  " GFI2C_OP_TEST " <i2c-bus device file> <i2c device address> <addr1> <mask1> <addr2> <mask2>\n"
	"      Perform read/write test using the two specified register addresses and data-masks\n"
	"  " GFI2C_OP_HELP "\n"
	"      Print this help.\n"
	"Read and write operations support the following options:\n"
	"  " GFI2C_OPTION_WIDTH " <width> : use 8 or 16 bit wide accesses.\n"
	"  " GFI2C_OPTION_REGWIDTH " <width> : use 8 or 16 bit wide register addresses.\n"
	"  " GFI2C_OPTION_NUM   " <num>   : perform <num> number of (identical) accesses.\n"
	"  " GFI2C_OPTION_DELAY " <msecs> : wait <msecs> milliseconds between successive accesses.\n"
	"  " GFI2C_OPTION_SINGLE " : perform single I2C accesses.\n"
	"  " GFI2C_OPTION_IGN_ACK " : ignore acknowledge bit.\n"
	"  " GFI2C_OPTION_STOP_ERR " : stop on error.\n"
	"  " GFI2C_OPTION_VERBOSE " : increase verbosity of logging output.\n"
};

static int op_verbosity	= DEFAULT_VERBOSITY;
static int op_width		= DEFAULT_WIDTH;
static int op_regwidth	= DEFAULT_WIDTH;
static int op_num		= DEFAULT_NUM;
static int op_delay 	= DEFAULT_DELAY;
static int op_address	= DEFAULT_ADDRESS;
static int op_reg1		= DEFAULT_REGISTER;
static int op_reg2		= DEFAULT_REGISTER;
static int op_data1		= DEFAULT_DATA;
static int op_data2		= DEFAULT_DATA;
static int op_mask1		= DEFAULT_MASK;
static int op_mask2		= DEFAULT_MASK;
static int op_single	= DEFAULT_SINGLE;
static int op_ignack	= DEFAULT_IGN_ACK;
static int op_stoperr	= DEFAULT_STOP_ERR;
static char* device 	= DEFAULT_I2C_BUS;

static int bytesXmit = 0;
static int bytesOverhead = 0;
static int errors = 0;


static void printUsage(void)
{
	PRINTF(wszHelp);
}

static int read_i2c_byte_data(int file, int reg)
{
	unsigned char buf[2];
	unsigned char data;
	struct i2c_msg msgs[2];
	struct i2c_rdwr_ioctl_data msgset;
	int err;

	if (reg & 0x80000000)
	{
		buf[0] = (reg >> 8) & 0xff;
		buf[1] = reg & 0xff;
		msgs[0].len = 2;
	}
	else
	{
		buf[0] = reg & 0xff;
		msgs[0].len = 1;
	}

	msgs[0].addr = op_address;
	msgs[0].buf = buf;
	msgs[0].flags = 0;
	if (op_ignack)
		msgs[0].flags |= I2C_M_IGNORE_NAK;

	msgs[1].addr = op_address;
	msgs[1].len = 1;
	msgs[1].buf = &data;
	msgs[1].flags = I2C_M_RD;
	if (!op_single)
		msgs[1].flags |= I2C_M_NOSTART;
	if (op_ignack)
		msgs[1].flags |= I2C_M_IGNORE_NAK;

	msgset.nmsgs = 2;
	msgset.msgs = msgs;

	err = ioctl(file, I2C_RDWR, &msgset);
	if (err >= 0)
	{
		bytesXmit += 1; 	// one byte usefull data;
		bytesOverhead += (2 + ((reg & 0x80000000) ? 1 : 0));
		err = data;
	}

	return err;
}

static int write_i2c_byte_data(int file, int reg, int data)
{
	unsigned char buf[3];
	struct i2c_msg msg;
	struct i2c_rdwr_ioctl_data msgset;
	int err;

	if (reg & 0x80000000)
	{
		buf[0] = (reg >> 8) & 0xff;
		buf[1] = reg & 0xff;
		buf[2] = data & 0xff;
		msg.len = 3;
	}
	else
	{
		buf[0] = reg & 0xff;
		buf[1] = data & 0xff;
		msg.len = 2;
	}

	msg.addr = op_address;
	msg.buf = buf;
	msg.flags = 0;
	if (op_ignack)
		msg.flags |= I2C_M_IGNORE_NAK;

	msgset.nmsgs = 1;
	msgset.msgs = &msg;

	err = ioctl(file, I2C_RDWR, &msgset);
	if (err >= 0)
	{
		bytesXmit += 1; 	// one byte usefull data;
		bytesOverhead += (2 + ((reg & 0x80000000) ? 1 : 0));
	}

	return err;
}

int read_i2c_word_data(int file, int reg)
{
	unsigned char buf[2];
	unsigned char data[2];
	struct i2c_msg msgs[2];
	struct i2c_rdwr_ioctl_data msgset;
	int err;

	if (reg & 0x80000000)
	{
		buf[0] = (reg >> 8) & 0xff;
		buf[1] = reg & 0xff;
		msgs[0].len = 2;
	}
	else
	{
		buf[0] = reg & 0xff;
		msgs[0].len = 1;
	}

	msgs[0].addr = op_address;
	msgs[0].buf = buf;
	msgs[0].flags = 0;
	if (op_ignack)
		msgs[0].flags |= I2C_M_IGNORE_NAK;

	msgs[1].addr = op_address;
	msgs[1].len = 2;
	msgs[1].buf = data;
	msgs[1].flags = I2C_M_RD;
	if (!op_single)
		msgs[1].flags |= I2C_M_NOSTART;
	if (op_ignack)
		msgs[1].flags |= I2C_M_IGNORE_NAK;

	msgset.nmsgs = 2;
	msgset.msgs = msgs;

	err = ioctl(file, I2C_RDWR, &msgset);
	if (err >= 0)
	{
		bytesXmit += 2; 	// one byte usefull data;
		bytesOverhead += (2 + ((reg & 0x80000000) ? 1 : 0));
		err = (data[0] << 8) | data[1];
	}

	return err;
}

static int write_i2c_word_data(int file, int reg, int data)
{
	unsigned char buf[4];
	struct i2c_msg msg;
	struct i2c_rdwr_ioctl_data msgset;
	int err;

	if (reg & 0x80000000)
	{
		buf[0] = (reg >> 8) & 0xff;
		buf[1] = reg & 0xff;
		buf[2] = (data >> 8) & 0xff;
		buf[3] = data & 0xff;
		msg.len = 4;
	}
	else
	{
		buf[0] = reg & 0xff;
		buf[1] = (data >> 8) & 0xff;
		buf[2] = data & 0xff;
		msg.len = 3;
	}

	msg.addr = op_address;
	msg.buf = buf;
	msg.flags = 0;
	if (op_ignack)
		msg.flags |= I2C_M_IGNORE_NAK;

	msgset.nmsgs = 1;
	msgset.msgs = &msg;

	err = ioctl(file, I2C_RDWR, &msgset);
	if (err >= 0)
	{
		bytesXmit += 2; 	// one byte usefull data;
		bytesOverhead += (2 + ((reg & 0x80000000) ? 1 : 0));
	}

	return err;
}


int main(int argc, char** argv)
{
	int opCounter = 0;
	int hDev = -1;
	int err = 0;
	int clockStart = 0;

	if ((argc < 3) || (!strcmp(argv[1], GFI2C_OP_HELP)))
	{
		printUsage();
		return 0;
	}
	else if (!strcmp(argv[1], GFI2C_OP_READ))
	{
		if (argc < 5)
		{
			printUsage();
			return -1;
		}
		device = argv[2];
		op_address = strtol(argv[3], NULL, 0);
		op_reg1 = strtol(argv[4], NULL, 0);
		opCounter = 5;

		if ((argc > 5) && argv[5][0] != '-')
		{
			op_data1 = strtol(argv[5], NULL, 0);
			opCounter = 6;
		}
	}
	else if (!strcmp(argv[1], GFI2C_OP_WRITE))
	{
		if (argc < 6)
		{
			printUsage();
			return -1;
		}
		device = argv[2];
		op_address = strtol(argv[3], NULL, 0);
		op_reg1 = strtol(argv[4], NULL, 0);
		op_data1 = strtol(argv[5], NULL, 0);
		opCounter = 6;
	}
	else if (!strcmp(argv[1], GFI2C_OP_TEST))
	{
		if (argc < 8)
		{
			printUsage();
			return -1;
		}
		device = argv[2];
		op_address = strtol(argv[3], NULL, 0);
		op_reg1 = strtol(argv[4], NULL, 0);
		op_mask1 = strtol(argv[5], NULL, 0);
		op_reg2 = strtol(argv[6], NULL, 0);
		op_mask2 = strtol(argv[7], NULL, 0);
		opCounter = 8;
	}
	else
	{
		printUsage();
		return -1;
	}

	for (; opCounter < argc; opCounter++)
	{
		if (!strcmp(argv[opCounter], GFI2C_OPTION_NUM) && (opCounter+1 < argc))
			op_num = strtol(argv[++opCounter], NULL, 0);
		else if (!strcmp(argv[opCounter], GFI2C_OPTION_DELAY) && (opCounter+1 < argc))
			op_delay = strtol(argv[++opCounter], NULL, 0);
		else if (!strcmp(argv[opCounter], GFI2C_OPTION_WIDTH) && (opCounter+1 < argc))
			op_width = strtol(argv[++opCounter], NULL, 0);
		else if (!strcmp(argv[opCounter], GFI2C_OPTION_REGWIDTH) && (opCounter+1 < argc))
			op_regwidth = strtol(argv[++opCounter], NULL, 0);
		else if (!strcmp(argv[opCounter], GFI2C_OPTION_SINGLE))
			op_single = 1;
		else if (!strcmp(argv[opCounter], GFI2C_OPTION_IGN_ACK))
			op_ignack = 1;
		else if (!strcmp(argv[opCounter], GFI2C_OPTION_STOP_ERR))
			op_stoperr = 1;
		else if (!strcmp(argv[opCounter], GFI2C_OPTION_VERBOSE))
			++op_verbosity;
	}

	if ((op_width != 8) && (op_width != 16))
	{
		fprintf(stderr, "Unsupported data-width: %u\n", op_width);
		err = -1;
		goto done;
	}

	if ((op_regwidth != 8) && (op_regwidth != 16))
	{
		fprintf(stderr, "Unsupported register-address width: %u\n", op_regwidth);
		err = -1;
		goto done;
	}

	if (op_regwidth == 16)
	{
		op_reg1 |= 0x80000000;
		op_reg2 |= 0x80000000;
	}

	hDev = open(device, O_RDWR);
	if (hDev < 0)
	{
		fprintf(stderr, "Error opening I2C-bus \"%s\" (%d)\n", device, errno);
		err = -2;
		goto done;
	}

	if (op_mask1 == DEFAULT_MASK)
		op_mask1 = (op_width == 8) ? 0xff : 0xffff;
	if (op_mask2 == DEFAULT_MASK)
		op_mask2 = (op_width == 8) ? 0xff : 0xffff;

	clockStart = clock();
	if (op_width == 8)
	{
		if (!strcmp(argv[1], GFI2C_OP_WRITE))
			for (opCounter = 0; opCounter < op_num; opCounter++)
			{
				err = write_i2c_byte_data(hDev, op_reg1, op_data1);
				if ((op_verbosity > 1) || (err < 0))
					printf("wb 0x%02x@0x%02x : 0x%02x (%d)\n", op_address, (op_reg1 & 0xffff), op_data1, (err < 0) ? errno : 0);
				if (err < 0)
				{
					++errors;
					if (op_stoperr)
						break;
				}
				if (op_delay)
					usleep(op_delay*1000);
			}
		else if (!strcmp(argv[1], GFI2C_OP_READ))
			for (opCounter = 0; opCounter < op_num; opCounter++)
			{
				err = read_i2c_byte_data(hDev, op_reg1);
				if ((op_verbosity > 1) || (err < 0) ||
					((op_data1 != -1) && (err != op_data1)))
					printf("rb 0x%02x@0x%02x : 0x%02x (%d)\n", op_address, (op_reg1 & 0xffff), err, (err < 0) ? errno : 0);
				if ((err < 0) || ((op_data1 != -1) && (err != op_data1)))
				{
					++errors;
					if (op_stoperr)
						break;
				}
				if (op_delay)
					usleep(op_delay*1000);
			}
		else if (!strcmp(argv[1], GFI2C_OP_TEST))
		{
			for (opCounter = 0; opCounter < op_num; opCounter++)
			{
				op_data1 = rand() & op_mask1;
				op_data2 = rand() & op_mask2;

				err = write_i2c_byte_data(hDev, op_reg1, op_data1);
				if ((op_verbosity > 1) || (err < 0))
					printf("wb 0x%02x@0x%02x : 0x%02x (%d)\n", op_address, (op_reg1 & 0xffff), op_data1, (err < 0) ? errno : 0);
				if (err < 0)
				{
					++errors;
					if (op_stoperr)
						break;
				}

				err = write_i2c_byte_data(hDev, op_reg2, op_data2);
				if ((op_verbosity > 1) || (err < 0))
					printf("wb 0x%02x@0x%02x : 0x%02x (%d)\n", op_address, (op_reg2 & 0xffff), op_data2, (err < 0) ? errno : 0);
				if (err < 0)
				{
					++errors;
					if (op_stoperr)
						break;
				}

				err = read_i2c_byte_data(hDev, op_reg1);
				if ((op_verbosity > 1) || (err < 0) || ((err & op_mask1) != op_data1))
					printf("rb 0x%02x@0x%02x : 0x%02x (%d)\n", op_address, (op_reg1 & 0xffff), (err & op_mask1), (err < 0) ? errno : 0);
				if ((err < 0) || ((err & op_mask1) != op_data1))
				{
					++errors;
					if (op_stoperr)
						break;
				}

				err = read_i2c_byte_data(hDev, op_reg2);
				if ((op_verbosity > 1) || (err < 0) || ((err & op_mask2) != op_data2))
					printf("rb 0x%02x@0x%02x : 0x%02x (%d)\n", op_address, (op_reg2 & 0xffff), (err & op_mask2), (err < 0) ? errno : 0);
				if ((err < 0) || ((err & op_mask2) != op_data2))
				{
					++errors;
					if (op_stoperr)
						break;
				}

				if (op_delay)
					usleep(op_delay*1000);
			}
		}
	}
	else
	{
		if (!strcmp(argv[1], GFI2C_OP_WRITE))
			for (opCounter = 0; opCounter < op_num; opCounter++)
			{
				err = write_i2c_word_data(hDev, op_reg1, op_data1);
				if ((op_verbosity > 1) || (err < 0))
					printf("ww 0x%02x@0x%02x : 0x%04x (%d)\n", op_address, (op_reg1 & 0xffff), op_data1, (err < 0) ? errno : 0);
				if (err < 0)
				{
					++errors;
					if (op_stoperr)
						break;
				}
				if (op_delay)
					usleep(op_delay*1000);
			}
		else if (!strcmp(argv[1], GFI2C_OP_READ))
			for (opCounter = 0; opCounter < op_num; opCounter++)
			{
				err = read_i2c_word_data(hDev, op_reg1);
				if ((op_verbosity > 1) || (err < 0) ||
					((op_data1 != -1) && (err != op_data1)))
					printf("rw 0x%02x@0x%02x : 0x%04x (%d)\n", op_address, (op_reg1 & 0xffff), err, (err < 0) ? errno : 0);
				if ((err < 0) || ((op_data1 != -1) && (err != op_data1)))
				{
					++errors;
					if (op_stoperr)
						break;
				}
				if (op_delay)
					usleep(op_delay*1000);
			}
		else if (!strcmp(argv[1], GFI2C_OP_TEST))
			for (opCounter = 0; opCounter < op_num; opCounter++)
			{
				op_data1 = rand() & op_mask1;
				op_data2 = rand() & op_mask2;

				err = write_i2c_word_data(hDev, op_reg1, op_data1);
				if ((op_verbosity > 1) || (err < 0))
					printf("ww 0x%02x@0x%02x : 0x%04x (%d)\n", op_address, (op_reg1 & 0xffff), op_data1, (err < 0) ? errno : 0);
				if (err < 0)
				{
					++errors;
					if (op_stoperr)
						break;
				}

				err = write_i2c_word_data(hDev, op_reg2, op_data2);
				if ((op_verbosity > 1) || (err < 0))
					printf("ww 0x%02x@0x%02x : 0x%04x (%d)\n", op_address, (op_reg2 & 0xffff), op_data2, (err < 0) ? errno : 0);
				if (err < 0)
				{
					++errors;
					if (op_stoperr)
						break;
				}

				err = read_i2c_word_data(hDev, op_reg1);
				if ((op_verbosity > 1) || (err < 0) || ((err & op_mask1) != op_data1))
					printf("rw 0x%02x@0x%02x : 0x%04x (%d)\n", op_address, (op_reg1 & 0xffff), (err & op_mask1), (err < 0) ? errno : 0);
				if ((err < 0) || ((err & op_mask1) != op_data1))
				{
					++errors;
					if (op_stoperr)
						break;
				}

				err = read_i2c_word_data(hDev, op_reg2);
				if ((op_verbosity > 1) || (err < 0) || ((err & op_mask2) != op_data2))
					printf("rw 0x%02x@0x%02x : 0x%04x (%d)\n", op_address, (op_reg2 & 0xffff), (err & op_mask2), (err < 0) ? errno : 0);
				if ((err < 0) || ((err & op_mask2) != op_data2))
				{
					++errors;
					if (op_stoperr)
						break;
				}

				if (op_delay)
					usleep(op_delay*1000);
			}
	}
	if (op_verbosity > 0)
	{
		int clockDiff = clock() - clockStart;
		printf("0x%02x %lds : %d bps total (%d bps overhead, %d bps data), %d errors\n",
			op_address, clockDiff / CLOCKS_PER_SEC,
			(int)(((long long)(bytesXmit + bytesOverhead)*CLOCKS_PER_SEC) / clockDiff),
			(int)(((long long)bytesOverhead*CLOCKS_PER_SEC) / clockDiff),
			(int)(((long long)bytesXmit*CLOCKS_PER_SEC) / clockDiff), errors);
	}

done:
	if (hDev >= 0)
		close(hDev);

	return err;
}
