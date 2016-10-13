/*
 * mdb - Library for using the Multi-Drop Bus (MDB) over a MARK/SPACE
 *       parity supporting TTY serial interface.
 *
 * Copyright (C) 2012 Tim Jaacks <tim.jaacks@garz-fricke.com>
 *
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <asm/ioctls.h>
#include "libmdb.h"

/*
 * The TTY layer and the driver need some time until the characters to be
 * sent are actually written to the transmit FIFO. This time has to be added
 * to the set timeout value (mdb_settings.recv_timeout_ms). The value of this
 * additional time has been measured experimentially.
 */
#define TTY_TIMEOUT_MS	18

struct mdb_handle
{
	char *tty_dev;
	int tty_fd;
	struct mdb_settings settings;
};

/*****************************************************************
 *Function name	: mdb_open
*/
/**
 * @brief	Opens the given serial port and configure it for MDB.
 *			This function returns a handle in the first parameter,
 *			which is needed for all further library calls.
 *
 * @param	handle		Pointer to variable to store handle into
 * @param	tty_dev		String containing the TTY device to use (e.g. "ttymxc1")
 *
 * @return	error code
 * @retval	0			Success
 * @retval	-EINVAL		Invalid parameter
 * @retval	-EIO		TTY port could not be opened or configured
*/
int mdb_open(int *handle, char *tty_dev)
{
	char tty_dev_node[30];
	int ret = 0;
	struct termios tio;
	struct mdb_handle *pmdb;

	if (!handle || !tty_dev)
		return -EINVAL;

	pmdb = (struct mdb_handle *) malloc(sizeof(struct mdb_handle));
	if (!pmdb)
		return -EIO;

	pmdb->settings.size = sizeof(struct mdb_settings);
	pmdb->settings.reset_time_ms = DEFAULT_RESET_TIME_MS;
	pmdb->settings.recv_timeout_ms = DEFAULT_RECV_TIMEOUT_MS;

	/* Open serial port */
	sprintf(tty_dev_node, "/dev/%s", tty_dev);
	pmdb->tty_fd = open(tty_dev_node, O_RDWR | O_NONBLOCK);
	if (pmdb->tty_fd < 0) {
		ret = -EIO;
		goto open_failed;
	}
	pmdb->tty_dev = tty_dev;

	/* Set serial port configuration */
	tcgetattr(pmdb->tty_fd, &tio);
	cfsetospeed(&tio, B9600);
	cfsetispeed(&tio, B9600);
	tio.c_cflag &= ~(CSIZE | CSTOPB); // 8 data bits, 1 stop bit
	tio.c_cflag |= CS8;
	tio.c_cflag |= CREAD | PARENB | CMSPAR;
	tio.c_lflag = 0;
	tio.c_iflag |= INPCK | PARMRK | ICRNL;
	tio.c_iflag &= ~IGNPAR;
	tio.c_cc[VMIN] = 0;		// Non-blocking read:
	tio.c_cc[VTIME] = 0;	// read() will return immediately
	ret = tcsetattr(pmdb->tty_fd, TCSANOW, &tio);

	if(ret) {
		ret = -EIO;
		goto open_failed;
	}

	*handle = (int)pmdb;
	return 0;

open_failed:
	free(pmdb);
	return ret;
}

/*****************************************************************
 *Function name	: mdb_close
*/
/**
 * @brief	Closes the serial port and frees all resources.
 *
 * @param	handle		Handle identifying caller (returned by mdb_open)
 *
 * @return	error code
 * @retval	0			Success
 * @retval	-EINVAL		Invalid handle
*/
int mdb_close(int handle)
{
	struct mdb_handle *pmdb = (struct mdb_handle*) handle;

	if (!pmdb)
		return -EINVAL;

	close(pmdb->tty_fd);
	free(pmdb);

	return 0;
}

/*****************************************************************
 *Function name	: mdb_reset
*/
/**
 * @brief	Resets the MDB bus by holding the TX-line active for
 *			the configured reset time (default 100ms).
 *
 * @param	handle		Handle identifying caller (returned by mdb_open)
 *
 * @return	error code
 * @retval	0			Success
 * @retval	-EINVAL		Invalid handle
*/
int mdb_reset(int handle)
{
	struct mdb_handle *pmdb = (struct mdb_handle*) handle;

	if (!pmdb)
		return -EINVAL;

	tcsendbreak(pmdb->tty_fd, pmdb->settings.reset_time_ms);
	return 0;
}

/*****************************************************************
 *Function name	: mdb_transaction
*/
/**
 * @brief	Sends an MDB message to the given address, including CHK
 *			calculation. Waits for a reply within the configured timeout
 *			(default 5ms) and returns it to the caller. Automatically
 *			verifies the reply and sends ACK or NACK to the client,
 *			depending on whether CHK was OK or not OK.
 *
 * @param	handle		Handle identifying caller (returned by mdb_open)
 * @param	adr			Address of the MDB client to communicate with
 * @param	psend_data	Pointer to the data to be sent to the client
 * @param	send_len	Length of the data to be sent (1..36)
 * @param	precv_data	Pointer to the memory where the received data shall be stored
 * @param	precv_len	Pointer to the length of the receive array. After returning,
 *						pRecvLen contains the length of the received data (1..36).
 *
 * @return	error code
 * @retval	0			Success
 * @retval	-EINVAL		Invalid handle or parameter pointers are NULL or send_len
 *						is not within 1..36
 * @retval	-EIO		TTY error
 * @retval	-ETIMEDOUT	Waiting for reply timed out
 * @retval	-ECOMM		Received CHK did not match the sum of the received bytes
 * @retval	-EPROTO		Reply did not have the last byte marked with 9th bit
*/
int mdb_transaction(int handle, unsigned char adr, unsigned char *psend_data,
					int send_len, unsigned char *precv_data, int *precv_len)
{
	struct mdb_handle *pmdb = (struct mdb_handle*) handle;
	struct termios tio;
	unsigned char buffer[MAX_MDB_MESSAGE_LENGTH+3];
	unsigned char chk = 0, *buf_ptr, *recv_ptr;
	int bytes, ret;
	fd_set fds;
	struct timeval tv;

	if (!pmdb || !psend_data || !precv_data || !precv_len ||
			send_len < 1 || send_len > MAX_MDB_MESSAGE_LENGTH ||
			*precv_len > MAX_MDB_MESSAGE_LENGTH)
		return -EINVAL;

	/*
	 * The first data byte gets merged with the address byte
	 * (address is only 5 bits)
	 */
	buffer[0] = adr | psend_data[0];

	/* The following bytes are data bytes, copy them directly */
	memcpy(&buffer[1], &psend_data[1], send_len-1);

	/* Last byte is the checksum */
	buf_ptr = buffer;
	while (buf_ptr < buffer + send_len)
		chk += *(buf_ptr++);
	*buf_ptr = chk;

	/*
	 * Configure serial port for the first byte. This one has the 9th
	 * bit set, so we have to enable MARK parity.
	 */
	tcgetattr(pmdb->tty_fd, &tio);
	tio.c_cflag |= PARODD; // CMSPAR | PARODD = MARK parity
	ret = tcsetattr(pmdb->tty_fd, TCSANOW, &tio);
	if(ret)
		return -EIO;

	/* Empty input and output buffers */
	tcflush(pmdb->tty_fd, TCIOFLUSH);

	/* Send first byte */
	bytes = write(pmdb->tty_fd, buffer, 1);
	if (bytes != 1)
		return -EIO;

	/*
	 * Configure serial port for the remaining bytes. These have the 9th
	 * bit unset, so we have to enable SPACE parity.
	 */
	tio.c_cflag &= ~PARODD; // CMSPAR & ~PARODD = SPACE parity
	ret = tcsetattr(pmdb->tty_fd, TCSANOW, &tio);
	if(ret)
		return -EIO;

	/* Send remaining bytes */
	bytes = write(pmdb->tty_fd, &buffer[1], send_len);
	if (bytes != send_len)
		return -EIO;

	/* Receive reply */
	FD_ZERO(&fds);
	FD_SET(pmdb->tty_fd, &fds);
	tv.tv_sec = 0;
	/*
	 * For the first read we define a timeout consisting of the following
	 * components:
	 *  1. TTY and driver layer delay (see above)
	 *  2. Time for each written character to be physically transmitted
	 *  3. Configured receive timeout
	 */
	tv.tv_usec = TTY_TIMEOUT_MS * 1000
				+ send_len * 11 * 10000/96		// 11 bits, baud rate 9600
				+ 1000 * pmdb->settings.recv_timeout_ms;
	bytes = 0;
	ret = select(pmdb->tty_fd + 1, &fds, NULL, NULL, &tv);
	if(ret < 0)
		return -EIO;
	while (ret > 0 && bytes < sizeof(buffer))
	{
		ret = read(pmdb->tty_fd, buffer + bytes, sizeof(buffer)-bytes);
		if (ret >= 0)
			bytes += ret;
		else
			return -EIO;
		FD_ZERO(&fds);
		FD_SET(pmdb->tty_fd, &fds);
		tv.tv_usec = 1000 * pmdb->settings.recv_timeout_ms;
		/*
		 * For all further reads the timeout is only the configured inter-
		 * character receive timeout.
		 */
		ret = select(pmdb->tty_fd + 1, &fds, NULL, NULL, &tv);
		if(ret < 0)
			return -EIO;
	}

	/* Evaluate reply */
	
	/* Check for length = 0 */
	if (bytes == 0)
	{
		*precv_len = 0;
		return -ETIMEDOUT;
	}

	/*
	 * Check for length = 1
	 * Parity error bytes get preceeded by a byte sequence of 0xFF and 0x00. We
	 * have to check whether the reply consists of only a single byte, in which
	 * case we should have read three bytes: 0xFF, 0x00 and the data byte.
	 */
	if (bytes == 3 && buffer[0] == 0xff && buffer[1] == 0x00)
	{
		/* If the data byte is not ACK or NACK, an error has occured */
		if (buffer[2] == MDB_ACK || buffer[2] == MDB_NACK)
		{
			*precv_len = 1;
			precv_data[0] = buffer[2];
			return 0;
		}
		*precv_len = 0;
		return -EIO;
	}

	/*
	 * In all other cases calculate the checksum, check which byte is the last
	 * one, and copy all bytes into the receive buffer.
	 */
	buf_ptr = buffer;
	recv_ptr = precv_data;
	chk = 0;
	while (buf_ptr < buffer + bytes)
	{
		if (buf_ptr + 2 < buffer + bytes &&
			*buf_ptr == 0xff && *(buf_ptr+1) == 0x00)
		{
			/*
			 * A parity error occured on this byte, which means that the 9th
			 * bit was set. This is supposed to be the last byte of the reply,
			 * containing the checksum.
			 */
			*precv_len = buf_ptr - buffer;
			if (*(buf_ptr+2) == chk)
			{
				/*
				 * Last byte was correctly marked with 9th bit set and CHK
				 * matched the sum of all received bytes. We can send an ACK
				 * and return successfully.
				 */
				buffer[0] = MDB_ACK;
				write(pmdb->tty_fd, buffer, 1);
				return 0;
			}
			else
			{
				/*
				 * CHK did not match the sum of the received bytes. Send NACK
				 * to client and return with an error.
				 */
				buffer[0] = MDB_NACK;
				write(pmdb->tty_fd, buffer, 1);
				return -ECOMM;
			}
		}
		chk += *buf_ptr;
		*(precv_data++) = *(buf_ptr++);
	}

	/*
	 * If execution comes to this point, data has been received,
	 * but there was no end-byte marked with the 9th bit set.
	 */
	*precv_len = bytes;
	return -EPROTO;
}

/*****************************************************************
 *Function name	: mdb_wakeup
*/
/**
 * @brief	Sets the wakeup line on the MDB interface
 *
 * @param	handle		Handle identifying caller (returned by mdb_open)
 * @param	wakeup		true -> sets the line to GND
 *                      false -> sets the line to VCC
 *
 * @return	error code
 * @retval	0			Success
 * @retval	-EINVAL		Invalid handle
 * @retval	-EIO		Could not set line to specified level
*/
int mdb_wakeup(int handle, bool wakeup)
{
	struct mdb_handle *pmdb = (struct mdb_handle*) handle;
	int wakeup_fd, bytes_written;
	char wakeup_dev_node[70];
	char *data = wakeup ? "1\n" : "0\n";

	if (!pmdb)
		return -EINVAL;

	/*
	 * The wakeup GPIO pin should be exported to sysfs by the board's init
	 * routine. It should have placed a link to the pin under the device's
	 * sysfs folder, called "mdb_wakup_out". We try to open it here. If the
	 * link does not exist, we cannot access the wakeup line.
	 */
	sprintf(wakeup_dev_node, "/sys/class/tty/%s/device/mdb_wakeup_out/value",
					pmdb->tty_dev);
	wakeup_fd = open(wakeup_dev_node, O_RDWR | O_NONBLOCK);
	if (wakeup_fd < 0)
		return -EIO;

	bytes_written = write(wakeup_fd, data, strlen(data)+1);
	if (bytes_written != strlen(data)+1)
		return -EINVAL;

	return 0;
}

/*****************************************************************
 *Function name	: mdb_get_settings
*/
/**
 * @brief	Gets the current configuration of the MDB device
 *
 * @param	handle		Handle identifiying caller (returned by mdb_open)
 * @param	psettings	Pointer to mdb_settings struct with initialized size parameter
 *
 * @return	error code
 * @retval	0			Success
 * @retval	-EINVAL		Invalid handle or invalid psettings pointer
 * @retval	-EMSGSIZE	The size parameter does not match with the library
*/
int mdb_get_settings(int handle, struct mdb_settings *psettings)
{
	struct mdb_handle *pmdb = (struct mdb_handle*) handle;

	if (!pmdb || !psettings)
		return -EINVAL;

	if (sizeof(struct mdb_settings) != psettings->size)
		return -EMSGSIZE;

	memcpy(psettings, &pmdb->settings, sizeof(struct mdb_settings));
	return 0;
}

/*****************************************************************
 *Function name	: mdb_set_settings
*/
/**
 * @brief	Sets the configuration of the MDB device
 *
 * @param	handle		Handle identifiying caller (returned by mdb_open)
 * @param	psettings	Pointer to mdb_settings struct with initialized size parameter
 *
 * @return	error code
 * @retval	0			Success
 * @retval	-EINVAL		Invalid handle or invalid psettings pointer
 * @retval	-EMSGSIZE	The size parameter does not match with the library
*/
int mdb_set_settings(int handle, struct mdb_settings *psettings)
{
	struct mdb_handle *pmdb = (struct mdb_handle*) handle;

	if (!pmdb || !psettings)
		return -EINVAL;

	if (sizeof(struct mdb_settings) != psettings->size)
		return -EMSGSIZE;

	memcpy(&pmdb->settings, psettings, sizeof(struct mdb_settings));
	return 0;
}
