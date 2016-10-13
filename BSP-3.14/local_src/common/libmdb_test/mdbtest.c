/************************************************************************/
/* Copyright (C) 2012 Garz&Fricke GmbH					 				*/
/*		No use or disclosure of this information in any form without	*/
/*		the written permission of the author							*/
/************************************************************************/

/************************************************************************/
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*  This is a test tool for MDB. It attempts to connect to a cashless   */
/*  device on ttymxc1 and sends configuration data to it. On success,   */
/*  the response is printed on the standard output. ttymxc1 has to      */
/*  support MARK and SPACE parity, otherwise the communication will     */
/*  fail. This test has been tried with Garz & Fricke KarL² on VINCELL  */
/*  with MDB master option.                                             */
/*																		*/
/************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <libmdb.h>

#define TTY_DEVICE		"ttymxc1"
#define MDB_ADDRESS		0x10
#define MDB_MAX_LENGTH	36
#define RETRIES			10


int main(int argc, char **argv)
{
	unsigned char reset_data[] = { 0x00 };
	unsigned char poll_data[] = { 0x02 };
	unsigned char config_data[] = { 0x01, 0x00, 0x83, 0x10, 0x02, 0x01 };
	unsigned char get_id_data[] = { 0x07, 0x00, 0x47, 0x55, 0x46, 0x30,
									0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
									0x30, 0x30, 0x30, 0x30, 0x30, 0x32,
									0x30, 0x30, 0x32, 0x33, 0x32, 0x31,
									0x20, 0x00, 0x20, 0x4B, 0x6F, 0x03 };
	unsigned char recv_data[MDB_MAX_LENGTH];
	int mdb_handle, recv_len, ret, i;
	struct mdb_settings settings = {
		.size = sizeof(struct mdb_settings),
		.reset_time_ms = DEFAULT_RESET_TIME_MS,
		.recv_timeout_ms = 15,
	};

	printf("Opening %s for MDB\n", TTY_DEVICE);
	ret = mdb_open(&mdb_handle, TTY_DEVICE);
	if (ret)
	{
		printf("Could not open %s for MDB (error %d)\n", TTY_DEVICE, ret);
		return 1;
	}

	printf("Setting MDB configuration\n");
	ret = mdb_set_settings(mdb_handle, &settings);
	if (ret)
	{
		printf("Could not set MDB configuration\n");
		goto error;
	}

	printf("Asserting wakeup line\n");
	ret = mdb_wakeup(mdb_handle, 1);
	if (ret)
	{
		printf("Could not assert wakeup line (error %d)\n", ret);
		goto error;
	}

	printf("Resetting the MDB bus\n");
	ret = mdb_reset(mdb_handle);
	if (ret)
	{
		printf("Could not reset the MDB bus (error %d)\n", ret);
		goto error;
	}

	printf("Sending reset command to client\n");
	ret = -1;
	for (i = 0; i < RETRIES; i++)
	{
		recv_len = MDB_MAX_LENGTH;
		ret = mdb_transaction(mdb_handle, MDB_ADDRESS, reset_data,
								sizeof(reset_data), recv_data, &recv_len);
		if (ret == 0)
		{
			printf("Received %d bytes from client: ", recv_len);
			int j;
			for (j = 0; j < recv_len; j++)
			{
				printf("0x%02x ", recv_data[j]);
			}
			printf("\n");
			break;
		}
		usleep(100*1000);
	}
	if (i == RETRIES)
	{
		printf("MDB transceive error (error %d).\n", ret);
		goto error;
	}

	printf("Sending poll command to client\n");
	ret = -1;
	for (i = 0; i < RETRIES; i++)
	{
		recv_len = MDB_MAX_LENGTH;
		ret = mdb_transaction(mdb_handle, MDB_ADDRESS, poll_data,
								sizeof(poll_data), recv_data, &recv_len);
		if (ret == 0)
		{
			printf("Received %d bytes from client: ", recv_len);
			int j;
			for (j = 0; j < recv_len; j++)
			{
				printf("0x%02x ", recv_data[j]);
			}
			printf("\n");
			break;
		}
		usleep(100*1000);
	}
	if (i == RETRIES)
	{
		printf("MDB transceive error (error %d).\n", ret);
		goto error;
	}

	printf("Sending config command to client\n");
	ret = -1;
	for (i = 0; i < RETRIES; i++)
	{
		recv_len = MDB_MAX_LENGTH;
		ret = mdb_transaction(mdb_handle, MDB_ADDRESS, config_data,
								sizeof(config_data), recv_data, &recv_len);
		if (ret == 0)
		{
			printf("Received %d bytes from client: ", recv_len);
			int j;
			for (j = 0; j < recv_len; j++)
			{
				printf("0x%02x ", recv_data[j]);
			}
			printf("\n");
			break;
		}
		usleep(100*1000);
	}
	if (i == RETRIES)
	{
		printf("MDB transceive error (error %d).\n", ret);
		goto error;
	}

	printf("Sending get ID command to client\n");
	ret = -1;
	for (i = 0; i < RETRIES; i++)
	{
		recv_len = MDB_MAX_LENGTH;
		ret = mdb_transaction(mdb_handle, MDB_ADDRESS, get_id_data,
								sizeof(get_id_data), recv_data, &recv_len);
		if (ret == 0)
		{
			printf("Received %d bytes from client: ", recv_len);
			int j;
			for (j = 0; j < recv_len; j++)
			{
				printf("0x%02x ", recv_data[j]);
			}
			printf("\n");
			break;
		}
		usleep(100*1000);
	}
	if (i == RETRIES)
	{
		printf("MDB transceive error (error %d).\n", ret);
		goto error;
	}

	mdb_wakeup(mdb_handle, 0);
	mdb_close(mdb_handle);
	return 0;

error:
	mdb_wakeup(mdb_handle, 0);
	mdb_close(mdb_handle);
	return 1;
}
