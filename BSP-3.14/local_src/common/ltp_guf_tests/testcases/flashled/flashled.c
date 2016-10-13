/******************************************************************************/
/* Copyright (C) 2011 Garz&Fricke GmbH						 			      */
/*		No use or disclosure of this information in any form without	      */
/*		the written permission of the author							      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        flashled.c                                                    */
/*                                                                            */
/* Description: This is a test for the flshled.                               */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   flsahled                                                      */
/*                                                                            */
/* Test Assertion                                                             */
/* & Strategy:  TODO                                                          */
/* Author:      Carsten Behling <carsten.behling@garz-fricke.com>             */
/*                                                                            */
/******************************************************************************/

/* Standard Include Files */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <sys/ioctl.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"

/* Local Defines */
#if !defined(TRUE) && !defined(FALSE)
#define TRUE  1
#define FALSE 0
#endif

#define I2C_BLOCK_WRITE	0
#define I2C_BLOCK_READ	I2C_M_RD

/* GPIOs */
#define LED_1 0x30
#define LED_2 0x31

/* Registers */
#define GENERAL_PURPOSE 0x0010
#define TORCH_CURRENT 	0x00A0

/* Types */
typedef enum {
    TC_15,
    TC_30,
    TC_40,
    TC_50,
    TC_65,
    TC_80,
    TC_90,
    TC_100,
    TC_110,
    TC_120,
    TC_130,
    TC_140,
    TC_150,
    TC_160,
    TC_170,
    TC_180,
} torch_current_t;

typedef enum {
	M_SHUTDOWN,
	M_TORCH,
	M_FLASH,
	M_RESERVED_0,
	M_INDICATOR,
	M_RESERVED_1,
	M_RESERVED_2,
	M_INHIBIT_STROBE,
} flash_mode_t;

/* Extern Global Variables */
extern int  Tst_count;             	/* counter for tst_xxx routines.         */
extern char *TESTDIR;              	/* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID     = "flashled       ";			/* test program identifier.              */
int  TST_TOTAL = 1;                 		/* total number of tests in this file.   */

int    fd;									/* file descriptor of the i2c device */

char *device_name  = "/dev/i2c-0";		/* name of the standard device incl. /dev/ */
int  slave_address = 0x53;				/* address of the i2c slave to talk with */
int	 address_mode  = 8;				/* number of bits the i2c slave registers are addressed with */

flash_mode_t conf_flash_mode = M_TORCH; /* default: torch mode */
unsigned int conf_selected_led = LED_1; /* default D1 */
torch_current_t conf_torch_current = TC_15; /* default: 50 mA */

/* Extern Global Functions */
/******************************************************************************/
/*                                                                            */
/* Function:    cleanup                                                       */
/*                                                                            */
/* Description: Performs all one time clean up for this test on successful    */
/*              completion,  premature exit or  failure. Closes all temporary */
/*              files, removes all temporary directories exits the test with  */
/*              appropriate return code by calling tst_exit() function.       */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits calling tst_exit(). Non '0' return code.   */
/*              On success - Exits calling tst_exit(). With '0' return code.  */
/*                                                                            */
/******************************************************************************/
extern void cleanup()
{
	/* close the i2c device */
	if (close(fd) == -1) {
		tst_resm(TWARN,
			"Closing the flashled device %s failed. Error: %d, %s",
			device_name, errno, strerror(errno));
	}

	tst_exit();
}


/* Local Functions */
/******************************************************************************/
/*                                                                            */
/* Function:    setup                                                         */
/*                                                                            */
/* Description: Performs all one time setup for this test. This function is   */
/*              typically used to capture signals, create temporary dirs      */
/*              and temporary files that may be used in the course of this    */
/*              test.                                                         */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits by calling cleanup().                      */
/*              On success - Nothing.                                         */
/*                                                                            */
/******************************************************************************/
void setup()
{
	/* Setup  the GPIO pins */
	system("echo 34 > /sys/class/gpio/export");
	system("echo 35 > /sys/class/gpio/export");

	system("echo out > /sys/class/gpio/gpio34/direction");
	system("echo out > /sys/class/gpio/gpio35/direction");

	fd = open("/sys/class/gpio/gpio34/value", O_RDWR);
	if (fd == -1) {
		tst_brkm(TBROK, cleanup,
			"Unable to open the GPIO34. Error: %d, %s",
			errno, strerror(errno));
	}
	write(fd, &conf_selected_led, 1);
	close(fd);

	/* open the i2c device */
	fd = open(device_name, O_RDWR);
	if (fd == -1) {
		tst_brkm(TBROK, cleanup,
			"Unable to open the i2c device %s. Error: %d, %s",
			device_name, errno, strerror(errno));
	}

	/* set the address of the i2c slave */
	if (ioctl(fd, I2C_SLAVE_FORCE, slave_address) < 0) {
		tst_brkm(TBROK, cleanup,
			"Unable to set the address of the i2c slave 0x%02X. Error: %d, %s",
			slave_address, errno, strerror(errno));
	}
}


/******************************************************************************/
/*                                                                            */
/* Function:    i2c_transfer                                                  */
/*                                                                            */
/* Description: Performs an i2c transfer.                                     */
/*                                                                            */
/* Input:       direction - I2C_BLOCK_WRITE or I2C_BLOCK_READ.                */
/*              data - Points on the data to write to the i2c slave or to the */
/*                     data read from the i2c slave.                          */
/*              memory_address - The internal address of the i2c slave where  */
/*                               data should be written to or read from.      */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits by calling cleanup().                      */
/*              On success - Nothing.                                         */
/*                                                                            */
/******************************************************************************/
void i2c_transfer(int direction, char *data, int memory_address)
{
	struct i2c_rdwr_ioctl_data msgset;
	int	blocks_count;
	int	bytes_count;
	char memory_address_msb = (memory_address >> 8) & 0x00FF;
	char memory_address_lsb = memory_address & 0x00FF;

	if (direction == I2C_BLOCK_WRITE) {
		blocks_count = 1;
		bytes_count = ((address_mode == 8) ? 2 : 3);
	}
	else {
		blocks_count = 2;
		bytes_count = ((address_mode == 8) ? 1 : 2);
	}

	msgset.nmsgs = blocks_count;
	msgset.msgs  = (struct i2c_msg *)malloc(blocks_count * sizeof(struct i2c_msg));
	if (msgset.msgs == NULL) {
		tst_brkm(TBROK, cleanup,
			"Unable to allocate memory.");
	}

	char *msg = (char *)malloc(bytes_count * sizeof(char));
	if (msg == NULL) {
		free(msgset.msgs);
		tst_brkm(TBROK, cleanup,
			"Unable to allocate memory.");
	}

	/* fill message with bytes (at first the memory address and after that the data) */
	if (address_mode == 8) {
		*msg = memory_address_lsb;

		if (direction == I2C_BLOCK_WRITE)
			*(msg+1) = *data;
	}
	else {
		*msg = memory_address_msb;
		*(msg+1) = memory_address_msb;

		if (direction == I2C_BLOCK_WRITE)
			*(msg+2) = *data;
	}

	/* configure the block that contains the above message */
	msgset.msgs[0].addr  = slave_address;
	msgset.msgs[0].flags = I2C_BLOCK_WRITE;
	msgset.msgs[0].buf   = (void*)msg;
	msgset.msgs[0].len   = bytes_count;

	if (direction == I2C_BLOCK_READ) {
		/* configure the block that gets the data from slave to master */
		msgset.msgs[1].addr  = slave_address;
		msgset.msgs[1].flags = I2C_BLOCK_READ;
		msgset.msgs[1].buf   = (void*)data;
		msgset.msgs[1].len   = 1;
	}

	/* send the i2c message */
	if (ioctl(fd, I2C_RDWR, &msgset) < 0) {
		free(msgset.msgs);
		free(msg);
		tst_brkm(TBROK, cleanup,
			"Unable to transfer bytes. Error: %d, %s",
			errno, strerror(errno));
	}

	free(msgset.msgs);
	free(msg);
}


/******************************************************************************/
/*                                                                            */
/* Function:    test                                                          */
/*                                                                            */
/* Description: TODO                                                          */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - returns -1.                                      */
/*              On success - returns 0.                                       */
/*                                                                            */
/******************************************************************************/
int test()
{
	char value = (char)conf_flash_mode;

	i2c_transfer(I2C_BLOCK_WRITE, &value, GENERAL_PURPOSE);
	value = (char)conf_torch_current;
	usleep(10000);
	i2c_transfer(I2C_BLOCK_WRITE, &value, TORCH_CURRENT);

	return 0;
}


/******************************************************************************/
/*                                                                            */
/* Function:    help                                                          */
/*                                                                            */
/* Description: This function is called when the test is started with         */
/*              parameter -h. It displays all parameter options specifically  */
/*              available for this test.                                      */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      Nothing.                                                      */
/*                                                                            */
/******************************************************************************/
void help()
{
	printf("  -m x    Select the flashmode [torch_on|torch_off] (default: torch_on)\n");
	printf("  -l x    Select the led [1|2] (default: 1)\n");
	printf("  -d x    Select the drive current in mA (default: 15)\n");
}


/******************************************************************************/
/*                                                                            */
/* Function:    main                                                          */
/*                                                                            */
/* Description: Entry point to this test-case. It parses all the command line */
/*              inputs, calls the global setup and executes the test. It logs */
/*              the test status and results appropriately using the LTP API's */
/*              On successful completion or premature failure, cleanup() func */
/*              is called and test exits with an appropriate return code.     */
/*                                                                            */
/* Input:       Command line parameters (see help())                          */
/*                                                                            */
/* Exit:        On failure - Exits by calling cleanup().                      */
/*              On success - exits with 0 exit value.                         */
/*                                                                            */
/******************************************************************************/
int main(int argc, char **argv)
{
	int opt_m = 0, opt_d = 0, opt_l = 0, ret = 0;
	char *str_m, *str_d, *str_l, *msg;

	option_t options[] = {
		{"m:", &opt_m, &str_m},
		{"l:", &opt_l, &str_l},
		{"d:", &opt_d, &str_d},
		{NULL, NULL, NULL}
	};

	/* parse options */
	if((msg = parse_opts(argc, argv, options, &help)) != NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

	if(opt_m) {
		if(0 == strcmp(str_m, "torch_on"))
			conf_flash_mode = M_TORCH;
		else if(0 == strcmp(str_m, "torch_off"))
			conf_flash_mode = M_SHUTDOWN;
		else
			conf_flash_mode = M_TORCH;
	}

	if(opt_l) {
		if(0 == strcmp(str_l, "1"))
			conf_selected_led = LED_1;
		else if(0 == strcmp(str_l, "2"))
			conf_selected_led = LED_2;
		else
			conf_selected_led = LED_1;
	}

	if(opt_d) {
		if(0 == strcmp(str_d, "15"))
			conf_torch_current = TC_15;
		else if(0 == strcmp(str_d, "30"))
			conf_torch_current = TC_30;
		else if(0 == strcmp(str_d, "40"))
			conf_torch_current = TC_40;
		else if(0 == strcmp(str_d, "50"))
			conf_torch_current = TC_50;
		else if(0 == strcmp(str_d, "65"))
			conf_torch_current = TC_65;
		else if(0 == strcmp(str_d, "80"))
			conf_torch_current = TC_80;
		else if(0 == strcmp(str_d, "100"))
			conf_torch_current = TC_100;
		else if(0 == strcmp(str_d, "110"))
			conf_torch_current = TC_110;
		else if(0 == strcmp(str_d, "120"))
			conf_torch_current = TC_120;
		else if(0 == strcmp(str_d, "130"))
			conf_torch_current = TC_130;
		else if(0 == strcmp(str_d, "140"))
			conf_torch_current = TC_140;
		else if(0 == strcmp(str_d, "150"))
			conf_torch_current = TC_150;
		else if(0 == strcmp(str_d, "160"))
			conf_torch_current = TC_160;
		else if(0 == strcmp(str_d, "170"))
			conf_torch_current = TC_170;
		else if(0 == strcmp(str_d, "180"))
			conf_torch_current = TC_180;
		else
			conf_torch_current = TC_15;
	}

	/* do initialization */
	setup();

	/* start the test */
	ret = test();

	cleanup();
	tst_exit();
}