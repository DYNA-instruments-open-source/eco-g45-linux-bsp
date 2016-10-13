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

#ifndef MDB_H
#define MDB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define DEFAULT_RESET_TIME_MS		100
#define DEFAULT_RECV_TIMEOUT_MS		5
#define MAX_MDB_MESSAGE_LENGTH		36

#define MDB_ACK						0x00
#define MDB_NACK					0xFF

struct mdb_settings
{
	unsigned int size;				/* size of this struct */
	unsigned int reset_time_ms;		/* MDB reset time */
	unsigned int recv_timeout_ms;	/* timeout between received bytes */
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
int mdb_open(int *handle, char* tty_dev);

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
int mdb_close(int handle);

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
int mdb_reset(int handle);

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
					int send_len, unsigned char *precv_data, int *precv_len);

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
int mdb_wakeup(int handle, bool wakeup);

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
int mdb_get_settings(int handle, struct mdb_settings *psettings);

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
int mdb_set_settings(int handle, struct mdb_settings *psettings);

#ifdef __cplusplus
}
#endif

#endif // MDB_H
