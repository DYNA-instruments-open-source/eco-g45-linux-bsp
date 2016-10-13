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

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include "crc32.h"
#include "ezxml.h"
#include "flash_config.h"

#define DEBUG

#undef DEBUG
#ifdef DEBUG
#define printf_d(args...) printf(args)
#else
#define	printf_d(args...) /* nothing */
#endif

/* filter to match all mtdblock devices */
int match_dev_name( const struct dirent *entry )
{
	if ( strstr(entry->d_name, BASE_NAME) == NULL)
		return 0;
	return 1;
}

/* get contents of /dev matching above function */
int get_dev_name(struct dirent ***dev_list, char *path) {
	int ret=0;

	ret=scandir(path, dev_list, match_dev_name, alphasort);
    if (ret < 0)
    	return 0;

    return ret;
}

/* get raw data from block device */
int read_block_device(char *dev_name, char **file_content )
{
	FILE *input = NULL;
	int len;

	// Get file length
	input = fopen(dev_name, "rb");
	fseek(input, 0L, SEEK_END);
	len = ftell(input);
	rewind(input);

	*file_content = malloc(5);
	if ( ! *file_content ) {
		perror("Insufficient memory\n");
		fclose(input);
		return -ENOMEM;
	}
	// check for <?xml tag
	fread(*file_content, 5, 1, input);
	if ( ! strstr(*file_content, "<?xml") )
		return 1; // return without an error, checking later for insufficient data

	// read file content into memory
	free(*file_content);
	*file_content = malloc(len);
	if ( ! *file_content ) {
		perror("Insufficient memory\n");
		fclose(input);
		return -ENOMEM;
	}
	rewind(input);
	fread(*file_content, len, 1, input);
	// check for valid string
	if ( strlen(*file_content) >= len ) {
		perror("Invalid data\n");
		fclose(input);
		return -1;
	}

	fclose(input);
	return 0;
}

/* write raw data to block device */
int write_block_device(char *dev_name, char **file_content )
{
	FILE *output = NULL;
	int len = strlen(*file_content) + XML_OVERHEAD;
	int file_len=0;

	printf_d("Writing to block device\n");
	// Get file length
	output = fopen(dev_name, "r+b");
	fseek(output, 0L, SEEK_END);
	file_len = ftell(output);
	rewind(output);

	if (len > file_len) {
		printf("Insufficient space left on device: %s \n", dev_name);
		fclose(output);
		return -ENOSPC;
	}

	// perform write
	fwrite(*file_content, len, 1, output);

	fclose(output);
	return 0;
}

/* read all block devices /dev/mtdblock* and obtain the current xml configuration,
 * perform a CRC check and store current content in **fisroot_str
 * **target points to the correct device for writing back
 */
int xml_read_from_mtd(char **fisroot_str, char **target)
{
	ezxml_t fis_root = NULL; 	// struct to hold temporary FIS XML
	char *fis_root_str = NULL; 	// corresponding string
	ezxml_t fis_red_root = NULL;// struct to hold temporary Redundant FIS XML
	char *fis_red_root_str = NULL;			// corresponding string
	ezxml_t xml_flash = NULL;

	struct dirent **dev_list=NULL; // list of block devices
	char *xml_content = NULL;	// string from block devices
	/* counter: */
	int n=0;
	int i=0;
	int ret=0;					// used for return values
	int len=0;					// various lengths
	/* fis generations */
	int fis_gen = 0;
	int fis_red_gen=0;
	/* CRC sums */
	unsigned long crcsum;
	unsigned long crcsum_calc;
	/* device names */
	char *tmp_dev = NULL;
	char *fis_dev = NULL;
	char *fis_red_dev = NULL;
	/* ezxml error code*/

	printf_d("Reading data from Flash...\n");
	n=get_dev_name(&dev_list, DEV_DIR);
	if (n == 0) {
        printf("Error getting block devices\n");
        return ENODEV;
	} else if (n < 2) {
        printf("Insufficient block devices\n");
        free(dev_list);
        return ENODEV;
    }

	while (n--) {
		tmp_dev = malloc(strlen(DEV_DIR)+strlen(dev_list[n]->d_name)+1);
		tmp_dev = strcpy(tmp_dev, DEV_DIR);
		tmp_dev = strcat(tmp_dev, dev_list[n]->d_name);
		ret = read_block_device(tmp_dev, &xml_content);
		if ( ! ret ) {
			printf_d(" %s", dev_list[n]->d_name);
			len=strlen(xml_content);
			// calculate and check crc sum
			// We have to do four byte-accesses because on ARM9, DWORD-
			// access is only possible on 32bit-aligned addresses.
			crcsum=*(xml_content+len+1) + (*(xml_content+len+2) << 8)
				+ (*(xml_content+len+3) << 16) + (*(xml_content+len+4) << 24);
			crcsum_calc=crc32((unsigned char *)xml_content, len+1);
			if ( crcsum != crcsum_calc) {
				printf("( CRC sums did not match: 0x%08x vs 0x%08x) \n",
						(unsigned int)crcsum, (unsigned int)crcsum_calc);
				goto out;
			}
			// Parse string into XML_structure
			if ( i ) {
				fis_red_root_str=strdup(xml_content);
				fis_red_dev=strdup(tmp_dev);
			} else {
				i++;
				fis_root_str=strdup(xml_content);
				fis_dev=strdup(tmp_dev);
			}
			printf_d("( ok ) \n");
		} else if ( ret < 0 ) { // error
			if ( xml_content )
				free(xml_content);
			return ret;
		}
out:
		if ( xml_content )
			free(xml_content);
		if (tmp_dev)
			free(tmp_dev);
	}
	if (!fis_root_str) {
		perror("No valid XML-configuration found!\n");
		free(dev_list);
		return 1;
	}
	free(dev_list);

	// reset return value
	ret=0;
	/* set correct flash generation and choose target device:
	 *  - if only one working XML-partition:
	 *  	use its content, generation and pathname
	 *  - if two working XML-partitions:
	 *  	find partition with lower generation and use it as target
	 *  	use content and generation of the other partition
	 */
	if (fis_red_root_str && fis_root_str) {
		printf_d(" Found two valid FIS partitions, checking flash generation\n");
		// FIS
		len=strlen(fis_root_str);
		fis_root=ezxml_parse_str(fis_root_str, len);

		xml_flash = ezxml_child(fis_root, "flash");
		if (!xml_flash) {
			printf("Error: No <flash> tag found! \n");
			ret = 1;
			goto clean;
		}
		fis_gen = strtoul(ezxml_attr(xml_flash, "generation"), NULL, 0);
		printf_d("   fis generation = %i\n", fis_gen);

		// Redundant FIS
		len=strlen(fis_red_root_str);
		fis_red_root=ezxml_parse_str(fis_red_root_str, len);

		xml_flash = ezxml_child(fis_red_root, "flash");
		if (!xml_flash) {
			printf("Error: No <flash> tag found! \n");
			ret = 1;
			goto clean;
		}
		fis_red_gen = strtoul(ezxml_attr(xml_flash, "generation"), NULL, 0);
		printf_d("   fis red generation = %i\n", fis_red_gen);

		// compare generations
		if ( fis_gen > fis_red_gen) {
			*target=strdup(fis_red_dev);
			*fisroot_str=ezxml_toxml(fis_root);
		} else {
			*target=strdup(fis_dev);
//			*fisroot_str=strdup(fis_red_root_str);
			*fisroot_str=ezxml_toxml(fis_red_root);
		}
	} else {
		perror(" Just one valid XML-configuration found!\n");
		*target=strdup(fis_dev);
		*fisroot_str=ezxml_toxml(fis_root);
	}

	printf_d("Data ready\n");
	// clean up
clean:
	if (fis_root)
		ezxml_free(fis_root);
	if (fis_red_root)
		ezxml_free(fis_red_root);

	if (fis_root_str)
		free(fis_root_str);
	if (fis_red_root_str)
		free(fis_red_root_str);

	if (fis_dev)
		free(fis_dev);
	if (fis_red_dev)
		free(fis_red_dev);

	return ret;
}

/*
 * write **fisroot_str to /dev/mtdblock* (given by **target) and append a CRC
 * checksum
 * If no assume_yes given, prompt for affirmation
 */
int xml_write_to_mtd(char **fisroot_str, int assume_yes, char **target)
{
	int ret=0;
	int len=0;
	char *tmp_str = NULL;
	char *affirm = NULL;
	unsigned long checksum;

	if ( (! *fisroot_str ) || ( ! * target ) ) {
		perror("Error writing to device: Insufficient Data\n");
		return 1;
	}
	if ( ! assume_yes ) {
		printf("Update RedBoot non-volatile configuration - continue (y/n)? ");
		ret=scanf("%a[a-z]", &affirm);
		if (ret == 1) {
			if ( strncmp(affirm, "y", 1) )
				return 1;
		} else {
			return 1;
		}
		ret = 0;
		if ( affirm )
			free(affirm);
	}

	// append CRC sum
	printf_d("calculating CRC checksum...");
	len = strlen(*fisroot_str);
	tmp_str = malloc(len + XML_OVERHEAD);
	strcpy(tmp_str, *fisroot_str);

	checksum = crc32((unsigned char *)*fisroot_str, len + 1);
	// We have to do four byte-accesses because on ARM9, DWORD-
	// access is only possible on 32bit-aligned addresses.
	*(tmp_str + len + 1) = checksum & 0xFF;
	*(tmp_str + len + 2) = (checksum >> 8) & 0xFF;
	*(tmp_str + len + 3) = (checksum >> 16) & 0xFF;
	*(tmp_str + len + 4) = (checksum >> 24) & 0xFF;
	printf_d("checksum: 0x%08x\n", checksum);
	tmp_str[len + sizeof checksum + 1] = '\0';
	printf_d("done.\n");

	ret = write_block_device(*target, &tmp_str);
	free(tmp_str);
	return ret;
}
