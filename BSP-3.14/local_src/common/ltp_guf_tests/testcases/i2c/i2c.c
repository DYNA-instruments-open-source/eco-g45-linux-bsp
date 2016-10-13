/******************************************************************************/
/* Copyright (C) 2011 Garz&Fricke GmbH						 			      */
/*		No use or disclosure of this information in any form without	      */
/*		the written permission of the author							      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        i2c.c                                                         */
/*                                                                            */
/* Description: This is a test for i2c busses and devices.                    */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   i2c                                                           */
/*                                                                            */
/* Test Assertion                                                             */
/* & Strategy:  Random bytes will be written in two certain registers of an   */
/*              i2c slave and read back afterwards. If the values read are    */
/*              equal to the according written bytes, the test is passed.     */
/*              Otherwise the test is failed (even if only one read-write     */
/*              procedure fails).                                             */
/*                                                                            */
/* Author:      Phillip Durdaut <phillip.durdaut@garz-fricke.com>             */
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

/* Extern Global Variables */
extern int  Tst_count;             	/* counter for tst_xxx routines.         */
extern char *TESTDIR;              	/* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID     = "i2c            ";			/* test program identifier.              */
int  TST_TOTAL = 1;                 		/* total number of tests in this file.   */

int    fd;									/* file descriptor of the i2c device */
time_t t;									/* used for the random generator */

char *conf_device_name  = "/dev/i2c-0";		/* name of the standard device incl. /dev/ */
int  conf_slave_address = 0x50;				/* address of the i2c slave to talk with */
int	 conf_address_mode  = 16;				/* number of bits the i2c slave registers are addressed with */
int  conf_register_a    = 0x0020;			/* address of the test register A */
int  conf_register_b    = 0x0040;			/* address of the test register B */
int  conf_wait_us       = 10000;			/* microseconds to wait between writing and reading */
int  conf_quiet         = 0;				/* quiet mode */

	
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
			"Closing the i2c device failed. Error: %d, %s",
			conf_device_name, errno, strerror(errno));
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
	/* open the i2c device */
	fd = open(conf_device_name, O_RDWR);
	if (fd == -1) {
		tst_brkm(TBROK, cleanup,
			"Unable to open the i2c device %s. Error: %d, %s",
			conf_device_name, errno, strerror(errno));
	}
	
	/* set the address of the i2c slave */
	if (ioctl(fd, I2C_SLAVE_FORCE, conf_slave_address) < 0) {
		tst_brkm(TBROK, cleanup,
			"Unable to set the address of the i2c slave 0x%02X. Error: %d, %s",
			conf_slave_address, errno, strerror(errno));
	}

	/* initialize the random generator */
	time(&t);
    srand((int)t);
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
		bytes_count = ((conf_address_mode == 8) ? 2 : 3);
	}
	else {
		blocks_count = 2;
		bytes_count = ((conf_address_mode == 8) ? 1 : 2);
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
	if (conf_address_mode == 8) {		
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
	msgset.msgs[0].addr  = conf_slave_address;
	msgset.msgs[0].flags = I2C_BLOCK_WRITE;
	msgset.msgs[0].buf   = msg;
	msgset.msgs[0].len   = bytes_count;
	
	if (direction == I2C_BLOCK_READ) {
		/* configure the block that gets the data from slave to master */
		msgset.msgs[1].addr  = conf_slave_address;
		msgset.msgs[1].flags = I2C_BLOCK_READ;
		msgset.msgs[1].buf   = data;
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
/* Description: This function controls the write and the read commands to the */
/*              i2c slave. The procedure is repeated twice as two registers   */
/*              are tested.                                                   */
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
	char random_byte, received_byte;
	int current_register, i, failed = 0;
	
	for (i = 0; i < 2; i++) {
	
		current_register = (!i) ? conf_register_a : conf_register_b;		
		random_byte = rand() % 256;
		
		i2c_transfer(I2C_BLOCK_WRITE, &random_byte, current_register);
		if (!conf_quiet)
			tst_resm(TINFO,
				"Byte 0x%02X written to 0x%04X.", 
				random_byte, current_register);
		
		usleep(conf_wait_us);
		
		i2c_transfer(I2C_BLOCK_READ, &received_byte, current_register);		
		if (!conf_quiet)
			tst_resm(TINFO,
				"Byte 0x%02X read from 0x%04X.",
				received_byte, current_register);
		
		if (random_byte != received_byte)
			return -1;
	}
	
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
	printf("  -d s    Select the i2c bus by its device name s (default: /dev/i2c-0)\n");
	printf("  -s x    Select the i2c slave by its address x (default: 0x50)\n");
	printf("  -m x    Select whether the i2c slave registers are addressed by 8 or 16 bits (default: 16)\n");
	printf("  -a x    Select the address of the test register A (default: 0x0020)\n");
	printf("  -b x    Select the address of the test register B (default: 0x0040)\n");
	printf("  -w x    Select the time to wait between writing and reading in us (default: 10000)\n");
	printf("  -q      Suppress info messages\n");
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
	int opt_d = 0, opt_s = 0, opt_m = 0, opt_a = 0, opt_b = 0, opt_r = 0, opt_w = 0, opt_q = 0, ret = 0, lc;
	char *str_d, *str_s, *str_m, *str_a, *str_b, *str_r, *str_w, *msg;
	
	option_t options[] = {
		{"d:", &opt_d, &str_d},
		{"s:", &opt_s, &str_s},
		{"m:", &opt_m, &str_m},
		{"a:", &opt_a, &str_a},
		{"b:", &opt_b, &str_b},
		{"r:", &opt_r, &str_r},
		{"w:", &opt_w, &str_w},
		{"q", &opt_q, NULL},
		{NULL, NULL, NULL}
	};	

	/* parse options */
	if ((msg = parse_opts(argc, argv, options, &help)) != NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

	if (opt_d)
		conf_device_name = str_d;
	if (opt_s) {		
		if (sscanf(str_s, "0x%x", &conf_slave_address) != 1)
			tst_resm(TWARN,
				"Hex numbers should begin with '0x'. Using standard slave address 0x50 now.");
	}
	if (opt_m) {		
		if (sscanf(str_m, "%d", &conf_address_mode) != 1
		||	(conf_address_mode != 8 && conf_address_mode != 16)) {
			conf_address_mode = 16;
			tst_resm(TWARN,
				"Slave registers are addressed by either 8 or 16 bits. Using standard value 16 now.");
		}
	}
	if (opt_a) {		
		if (sscanf(str_a, "0x%x", &conf_register_a) != 1)
			tst_resm(TWARN,
				"Hex numbers should begin with '0x'. Using standard address 0x0020 for the test register A now.");
	}
	if (opt_b) {		
		if (sscanf(str_b, "0x%x", &conf_register_b) != 1)
			tst_resm(TWARN,
				"Hex numbers should begin with '0x'. Using standard address 0x0040 for the test register B now.");
	}
	if (opt_w) {		
		if (sscanf(str_w, "%d", &conf_wait_us) != 1)
			tst_resm(TWARN,
				"Something wrong with the time to wait parameter. Using standard value 10000.");
	}
	if (opt_q)
		conf_quiet = 1;
	
	/* do initialization */
	setup();

	/* start the test */
	tst_resm(TINFO,
		"Starting i2c test. (slave: 0x%02X   bus: %s   runs: %d)",
		conf_slave_address, conf_device_name, STD_LOOP_COUNT);

	/* perform the test */
	for (lc=0; TEST_LOOPING(lc) && ret == 0; lc++)
	{
		ret = test();
		if (!conf_quiet)
		{
			// In normal mode display each test result
			if (ret != 0)
				tst_resm(TFAIL, 
					"Received byte does not equal sent byte. Test failed.");
			else
				tst_resm(TPASS, 
					"Both received bytes are equal to the according sent byte. Test passed.");
		}
	}

	if (conf_quiet)
	{
		// In quiet mode only display summary
		if (ret != 0)
			tst_resm(TFAIL, 
				"Received byte was not equal to sent byte. Test failed.");
		else
			tst_resm(TPASS, 
				"All received bytes were equal to sent bytes. Test passed.");
	}

	cleanup();
	tst_exit();
}