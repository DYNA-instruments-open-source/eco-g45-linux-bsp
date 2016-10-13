/*
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FLASH_CONFIG_H_
#define FLASH_CONFIG_H_

#include <dirent.h>
#include "ezxml.h"

#define DEV_DIR "/dev/" // mind the trailing slash!
#define BASE_NAME "mtdblock"

/* The actual string stored in Flash containes the configuration data as well as
 * a 32bit CRC checksum over the content and the trailing \0.
 * This checksum is also terminated by a NULL character:
 *
 * |----------------tmp_str--------------------|
 * |---------*file_content:-----------|        |
 * |--------------XML--------------|\0|-CRC-|\0|
 * |<----strlen(file_content)----->|-1|<-4->|-1|
 *
 * Thus the total overhead to the string is 6 Byte.
 */
#define XML_OVERHEAD 6

/* read all block devices /dev/mtdblock* and obtain the current xml configuration,
 * perform a CRC check and store current content in **fisroot_str
 * **target points to the correct device for writing back
 */
int xml_read_from_mtd(char **fisroot_str, char **target);
/*
 * write **fisroot_str to /dev/mtdblock* (given by **target) and append a CRC
 * checksum
 * If no assume_yes given, prompt for affirmation
 */
int xml_write_to_mtd(char **fisroot_str, int assume_yes, char **target);

// ----------------------------------------------------------------
/*
 * helper functions:
 */

/* filter to match all mtdblock devices */
int match_dev_name(const struct dirent* entry);
/* get contents of /dev matching above function */
int get_dev_name(struct dirent ***dev_list, char *path);
/* get raw data from block device */
int read_block_device(char *dev_name, char **file_content);
/* write raw data to block device */
int write_block_device(char *dev_name, char **file_content );
#endif /* FLASH_CONFIG_H_ */
