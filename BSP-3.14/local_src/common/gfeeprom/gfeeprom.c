// gfeeprom.c : Defines the entry point for the application.
//

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/i2c/gfeeprom.h>

#include "Version.h"

#define _QUOTEME(x) #x
#define QUOTEME(x) _QUOTEME(x)

#define GF_SVN_VERSION "$Revision: 754 $"
#define TOOL_NAME "gfeeprom"

#define STANDARD_EEPROM_FILE_NAME "bus0eepromA0"
#define STANDARD_EEPROM_PATH "/dev/gfeeprom"
#define STANDARD_EEPROM_FILE STANDARD_EEPROM_PATH "/" STANDARD_EEPROM_FILE_NAME

#define GFEEPROM_OP_CLEAN 			"--clean"
#define GFEEPROM_OP_SHOW 			"--show"
#define GFEEPROM_OP_PROGRAM 		"--program"
#define GFEEPROM_OP_HELP 			"--help"
#define GFEEPROM_OP_DEVICE			"-d"
#define GFEEPROM_OP_FULL_CONTENT	"-fc"
#define GFEEPROM_OP_EEPROM_EA		"-ea"
#define GFEEPROM_OP_BIN_DUMP		"-bd"
#define GFEEPROM_OP_BIN_DUMP_TO_FILE "-df"
#define GFEEPROM_OP_BIN_SRC_FILE	"-sf"
#define GFEEPROM_OP_BIN_SRC_STRING	"-ss"

#define DUMP_FULL_CONTENT			0xFFFFFFFF

#define PRINTF(s) printf("%s\n", s)

static const char wszHelp[] =
{
	TOOL_NAME " version " MAJOR_VERSION "." MINOR_VERSION " [" GF_SVN_VERSION "] (" __DATE__ ") <grimm@garz-fricke.com> \r\n"
	"usage:\r\n"
	"  " GFEEPROM_OP_CLEAN " [" GFEEPROM_OP_DEVICE " <device file>] [" GFEEPROM_OP_FULL_CONTENT "]\r\n"
	"                                                         Resets an EEPROM content to empty.\r\n"
	"                                                           Use " GFEEPROM_OP_DEVICE " for not using\r\n"
	"                                                             the standard file '" STANDARD_EEPROM_FILE "'.\r\n"
	"                                                           Use " GFEEPROM_OP_FULL_CONTENT " for cleaning\r\n"
	"                                                             the full eeprom. If not set only all areas above\r\n"
	"                                                             area id " QUOTEME(GFEEPROM_MIN_AREA_TYPE_NUMBER) " will be removed.\r\n"
	"  " GFEEPROM_OP_SHOW " [" GFEEPROM_OP_DEVICE " <device file>] [" GFEEPROM_OP_FULL_CONTENT "] [" GFEEPROM_OP_BIN_DUMP "] [" GFEEPROM_OP_BIN_DUMP_TO_FILE " <filen name>] [" GFEEPROM_OP_EEPROM_EA " <area number>]\r\n"
	"                                                         Shows the available EEPROMs equiped on platform.\r\n"
	"                                                           Use " GFEEPROM_OP_DEVICE " for showing the\r\n"
	"                                                             GuF content of a specific EEPROM device.\r\n"
	"                                                           Use " GFEEPROM_OP_FULL_CONTENT " for showing\r\n"
	"                                                             the full content of a specific EEPROM device.\r\n"
	"                                                           Use " GFEEPROM_OP_BIN_DUMP_TO_FILE " for writing\r\n"
	"                                                             the full content of a specific EEPROM device to the specified file.\r\n"
	"                                                           Use " GFEEPROM_OP_BIN_DUMP " for dumping\r\n"
	"                                                             the binary content of the EEPROM area.\r\n"
	"                                                           Use " GFEEPROM_OP_EEPROM_EA " for getting \r\n"
	"                                                             information of a specific EEPROM area.\r\n"
	"  " GFEEPROM_OP_PROGRAM " [" GFEEPROM_OP_DEVICE " <device file>] [" GFEEPROM_OP_EEPROM_EA " <area number>] [" GFEEPROM_OP_BIN_SRC_FILE " <source file>] [" GFEEPROM_OP_BIN_SRC_STRING " <string>]\r\n"
	"                                                         Programs the GuF area or a custom area. If the erea is present\r\n"
	"                                                         the are will overwritten if not the area will be created\r\n"
	"                                                           Use " GFEEPROM_OP_DEVICE " for showing the\r\n"
	"                                                             GuF content of a specific EEPROM device.\r\n"
	"                                                           Use " GFEEPROM_OP_EEPROM_EA " for setting \r\n"
	"                                                             information of this EEPROM area.\r\n"
	"                                                           Use " GFEEPROM_OP_BIN_SRC_FILE " for loading \r\n"
	"                                                             the EEPROM area content from this file.\r\n"
	"                                                           Use " GFEEPROM_OP_BIN_SRC_STRING " for using \r\n"
	"                                                             the following command line string as raw data packet.\r\n"
	"  " GFEEPROM_OP_HELP "\r\n"
	"                                                         Print this help.\r\n"
};

static void printUsage(void)
{
	PRINTF(wszHelp);
}

static int clean(char* device_name, bool full_content)
{
	int hDev;
	int ret;
	struct gfeeprom_access ea;

	hDev = open(device_name, O_RDWR);
	if(hDev < 0)
	{
		printf(TOOL_NAME " couldn't open device %s because of error %u.\r\n", device_name, errno);
		return -errno;
	}

	ea.size_of_struct = sizeof(struct gfeeprom_access);
	ea.access_type = full_content ? EEPROM_ACCESS_TYPE_CLEAN : EEPROM_ACCESS_TYPE_DELETE_ALL;
	ea.force_write = false;
	ea.data = NULL;
	ea.data_length = 0;
	ea.verify_buffer = NULL;
	ea.verify_buffer_length = 0;

	ret = ioctl(hDev, GFEEPROM_IOCTL_ACCESS_EEPROM_AREA, &ea);
	if (ret < 0)
	{
		printf(TOOL_NAME " couldn't %s the EEPROM. Error: 0x%X\r\n", full_content ? "clean" : "remove the custom areas from" , errno);
		close(hDev);
		return ret;
	}
	close(hDev);
	printf(TOOL_NAME " successful %s the EEPROM %s.\r\n", full_content ? "cleaned" : "removed the custom areas from", device_name);
	return ERROR_SUCCESS;
}

static int show_area(int hDev, u32 eara_number, bool bin_dump, char* dump_file)
{
	ssize_t read_bytes;
	char* data_ptr;
	u16 length_buffer;

	read_bytes = pread(hDev, &length_buffer, sizeof(u16), eara_number);
	if ((-1 == read_bytes) && (EFBIG == errno) && length_buffer)
	{
		printf("    Found Area: %u [%u Byte%s] %s\r\n", eara_number, length_buffer, (length_buffer > 1) ? "s" : "", (bin_dump && !dump_file) ? "dumping binary:" : "");
		if (bin_dump)
		{
			int i, j;

			data_ptr = (char*)malloc(sizeof(char)*length_buffer);
			if (!data_ptr)
			{
				PRINTF("      Failed allocating enough RAM for EEPROM area.\r\n");
				return -ENOMEM;
			}
			read_bytes = pread(hDev, data_ptr, sizeof(char)*length_buffer, eara_number);
			if (read_bytes != length_buffer)
			{
				free(data_ptr);
				printf("      Failed reading the complete EEPROM area. Only %u byte%s read.\r\n", read_bytes, (read_bytes > 1) ? "s" : "");
				return -EPIPE;
			}
			if (dump_file)
			{
				int hdump;

				hdump = open(dump_file, O_WRONLY | O_CREAT | O_EXCL);
				if(hdump >= 0)
				{
					if (write(hdump, data_ptr, length_buffer) < 0)
						printf("      Failed writing dump file '%s'. Error %u.\r\n", dump_file, errno);
					else
						printf("      Successful dumped EEPROM area to file '%s'.\r\n", dump_file);
					close(hdump);
				}
				else
					printf("      Failed opening dump file '%s'. Error %u.\r\n", dump_file, errno);
			}
			else
			{
				for (i = 0; i < (length_buffer>>4); i++)
				{
					printf("      0x%04X : 0x%02X 0x%02X 0x%02X 0x%02X  0x%02X 0x%02X 0x%02X 0x%02X  0x%02X 0x%02X 0x%02X 0x%02X  0x%02X 0x%02X 0x%02X 0x%02X\r\n",
						(i<<4),
						data_ptr[(i<<4)], data_ptr[(i<<4)+1], data_ptr[(i<<4)+2], data_ptr[(i<<4)+3],
						data_ptr[(i<<4)+4], data_ptr[(i<<4)+5], data_ptr[(i<<4)+6], data_ptr[(i<<4)+7],
						data_ptr[(i<<4)+8], data_ptr[(i<<4)+9], data_ptr[(i<<4)+10], data_ptr[(i<<4)+11],
						data_ptr[(i<<4)+12], data_ptr[(i<<4)+13], data_ptr[(i<<4)+14], data_ptr[(i<<4)+15]);
				}
				if (length_buffer & 0xF)
				{
					printf("      0x%04X :", (i<<4));
					for (j = 0; j < (length_buffer & 0xF); j++)
					{
						if (j && !(j & 0x3))
							printf(" ");
						printf(" 0x%02X", data_ptr[(i<<4)+j]);
					}
					printf("\r\n");
				}
			}
			free(data_ptr);
		}
		return ERROR_SUCCESS;
	}
	return -ENXIO;
}

static int show_eeprom(char* szFileName, u32 eara_number, bool bin_dump, char* dump_file)
{
	int hDev;
	ssize_t read_bytes;
    struct dirent *ep;
    struct gfeeprom_hw_normal_version_data hw_info;

	hDev = open(szFileName, O_RDONLY);
	if(hDev < 0)
	{
		printf(" failed to open EEPROM '%s' with error: %d.\r\n", szFileName, errno);
		return hDev;
	}

	if (!eara_number || (DUMP_FULL_CONTENT == eara_number))
	{
		read_bytes = pread(hDev, &hw_info, sizeof(struct gfeeprom_hw_normal_version_data), EEPROM_TYPE_EHW);
		if(read_bytes <= sizeof(struct gfeeprom_hw_small_version_data))
		{
			printf(" Reading hardware info from EEPROM '%s' failed.\r\n", szFileName);
		}
		else
		{
			if (sizeof(struct gfeeprom_hw_normal_version_data) == hw_info.size_of_struct)
			{
				printf("  EEPROM: %s -> component: '%s' | article number(s): '%s' | serial number(s): '%s' | major: '%s' | minor: '%s' | comment: '%s'\r\n",
					szFileName,
					hw_info.component,
					hw_info.article_number,
					hw_info.serial_numbers,
					hw_info.major,
					hw_info.minor,
					hw_info.comment);
			}
			else if (sizeof(struct gfeeprom_hw_small_version_data) == hw_info.size_of_struct)
			{
				struct gfeeprom_hw_small_version_data* hw_info_small = (struct gfeeprom_hw_small_version_data*)&hw_info;

				printf("  EEPROM: %s -> component: '%s' | article number(s): '%s' | major: '%s' | minor: '%s' | comment: '%s'\r\n",
					szFileName,
					hw_info_small->component,
					hw_info_small->article_number,
					hw_info_small->major,
					hw_info_small->minor,
					hw_info_small->comment);
			}
			else
				printf("  Skipping EEPROM: %s because the hardware info struct size isn't matching.\r\n", szFileName);
		}
	}

	if (DUMP_FULL_CONTENT == eara_number)
	{
		int i;
		u16 areas = 0;

		if (ioctl(hDev, GFEEPROM_IOCTL_GET_CUSTOM_AREA_COUNT, &areas, sizeof(u16)) < 0)
		{
			PRINTF("  Failed getting area count scanning manually.\r\n");
			PRINTF("  Dumping custom area(s) of EEPROM content:\r\n");
			for (i = EEPROM_TYPE_EHW+1; i < 0xFFFF; i++)
				show_area(hDev, i, bin_dump, NULL);
		}
		else if (areas)
		{
			struct gfeeprom_area_array param;
			u16* used_area_list = NULL;

			used_area_list = malloc(sizeof(u16)*areas);
			if (!used_area_list)
			{
				PRINTF("     Failed allocating enough RAM for EEPROM area list.\r\n");
				close(hDev);
				return ERROR_NOT_ENOUGH_MEMORY;
			}
			param.size_of_struct = sizeof(struct gfeeprom_area_array);
			param.elements = areas;
			param.areas = used_area_list;
			if (ioctl(hDev, GFEEPROM_IOCTL_GET_USED_CUSTOM_AREAS, &param, sizeof(struct gfeeprom_area_array)) < 0)
			{
				printf("     failed to open retrieve used area list with error: %d.\r\n", errno);
				close(hDev);
				free(used_area_list);
				return ERROR_INVALID_DATA;
			}
			printf("  Dumping %u custom area%s of EEPROM content:\r\n", areas, (areas > 1) ? L"s" : L"");
			for (i = 0; i < areas; i++)
				show_area(hDev, used_area_list[i], bin_dump, NULL);
			free(used_area_list);
		}
		else
		{
			PRINTF("  No additional custom area stored on EEPROM.\r\n");
		}
	}
	else if (eara_number)
	{
		if (show_area(hDev, eara_number, bin_dump, dump_file) < 0)
			printf("  No custom area %u found on EEPROM.\r\n", eara_number);
	}
	close(hDev);

	return ERROR_SUCCESS;
}

static int show_all(void)
{
	int hDev;
	ssize_t read_bytes;
    DIR *dp;
    struct dirent *ep;
    struct gfeeprom_hw_normal_version_data hw_info;
    char file_name_buffer[256];

	dp = opendir(STANDARD_EEPROM_PATH);
	if (dp == NULL)
	{
		printf(TOOL_NAME ", couldn't open eeprom path %s\r\n", STANDARD_EEPROM_PATH);
		return ERROR_PATH_NOT_FOUND;
	}

	PRINTF(TOOL_NAME " the following EEPROMs are present in the system:\r\n");
	while (ep = readdir(dp))
	{
		if ((ep->d_name[0] != '\0') && strcmp(ep->d_name, ".") && strcmp(ep->d_name, ".."))
		{
			strcpy(file_name_buffer, STANDARD_EEPROM_PATH);
			strcat(file_name_buffer, "/");
			strcat(file_name_buffer, ep->d_name);
			printf("Try to open EEPROM: %s ...", file_name_buffer);
			hDev = open(file_name_buffer, O_RDONLY);
			if(hDev < 0)
			{
				printf(" failed with error: %d.\r\n", errno);
				continue;
			}
			PRINTF(" successful.");

			printf("reading hardware info from EEPROM: %s...", file_name_buffer);
			read_bytes = pread(hDev, &hw_info, sizeof(struct gfeeprom_hw_normal_version_data), EEPROM_TYPE_EHW);
			if(read_bytes <= sizeof(struct gfeeprom_hw_small_version_data))
			{
				PRINTF(" failed.");
				close(hDev);
				continue;
			}
			PRINTF(" successful.");

			if (sizeof(struct gfeeprom_hw_normal_version_data) == hw_info.size_of_struct)
			{
				printf("  EEPROM: %s -> component: '%s' | article number(s): '%s' | serial number(s): '%s' | major: '%s' | minor: '%s' | comment: '%s'\r\n",
					file_name_buffer,
					hw_info.component,
					hw_info.article_number,
					hw_info.serial_numbers,
					hw_info.major,
					hw_info.minor,
					hw_info.comment);
			}
			else if (sizeof(struct gfeeprom_hw_small_version_data) == hw_info.size_of_struct)
			{
				struct gfeeprom_hw_small_version_data* hw_info_small = (struct gfeeprom_hw_small_version_data*)&hw_info;

				printf("  EEPROM: %s -> component: '%s' | article number(s): '%s' | major: '%s' | minor: '%s' | comment: '%s'\r\n",
					file_name_buffer,
					hw_info_small->component,
					hw_info_small->article_number,
					hw_info_small->major,
					hw_info_small->minor,
					hw_info_small->comment);
			}
			else
				printf("  Skipping EEPROM: %s because the hardware info struct size isn't matching.\r\n", file_name_buffer);
			close(hDev);
		}
	}

	closedir (dp);
	return ERROR_SUCCESS;
}

static int program_guf_hw(char* data_file)
{
	int								hFile = -1;
	int 	  						hDev = -1;
	struct stat						f_stat;
	int	 							dwFileSize = 0;
	char							szBuffer[GFV_COMMENT_LENGTH+1];
	char* 							szEEPROMConfig;
	char* 							szToken;
	u_int32_t 						dwBytesRead;
	u_int32_t  						uiLineNr;
	struct gfeeprom_program_area	hw_info;
	bool							bInEEPROMSection = false;
	u_int32_t						uiEEPROMCounter = 0;

	hFile = open(data_file, O_RDWR);
	if(hFile < 0)
	{
		printf(TOOL_NAME " error opening EEPROM config file %s. (0x%X)\r\n", data_file, errno);
		return -5;
	}

	dwFileSize = fstat(hFile, &f_stat);
	if (-1 == dwFileSize)
	{
		PRINTF(TOOL_NAME " error couldn't retrieve file attributes.\r\n");
		close(hFile);
		return -5;
	}

	szEEPROMConfig = (char*) malloc(f_stat.st_size);
	if (!szEEPROMConfig)
	{
		PRINTF(TOOL_NAME " error couldn't allocate memory.\r\n");
		close(hFile);
		return -5;
	}

	if(!read(hFile, szEEPROMConfig, sizeof(char)*f_stat.st_size))
	{
		printf(TOOL_NAME " error reading EEPROM config file %s. (0x%X)\r\n", data_file, errno);
		close(hFile);
		return -6;
	}

	szToken = strtok(szEEPROMConfig, "\r\n" );
	uiLineNr = 0;
	while( szToken != NULL )
	{
		uiLineNr++;

		if(strstr(szToken, "[BEGIN]") == szToken)
		{
			bInEEPROMSection = true;
			hw_info.size_of_struct = sizeof(struct gfeeprom_program_area);
			hw_info.verify = false;
			hw_info.force_write = false;
			hw_info.hw_version.size_of_struct = sizeof(struct gfv_hw_version_data);
			hw_info.hw_version.major[0] = '\0';
			hw_info.hw_version.minor[0] = '\0';
			hw_info.hw_version.article_number[0] = '\0';
			hw_info.hw_version.serial_numbers[0] = '\0';
			hw_info.hw_version.component[0] = '\0';
			hw_info.hw_version.comment[0] = '\0';
			hw_info.hw_version.address_of_component = 0xFFFFFFFF;
			hw_info.hw_version.phys_mem_size = 0;
		}
		else if (strstr(szToken, "[END]") == szToken)
		{
			if (bInEEPROMSection)
			{
				bInEEPROMSection = false;

				if(hDev >= 0)
				{
					if (ioctl(hDev, GFEEPROM_IOCTL_PROGRAM_HW_INFO, &hw_info, sizeof(struct gfeeprom_program_area)) < 0)
					{
						printf(TOOL_NAME " EEPROM %d: error couldn't write the EEPROM information. Error: 0x%X\r\n", ++uiEEPROMCounter, errno);
					}
					else
					{
						printf(TOOL_NAME " EEPROM %d: data has been flashed successful.\r\n\r\n", ++uiEEPROMCounter);
					}
					close(hDev);
				}
			}
			else
			{
				printf(TOOL_NAME " error encountered [END] before begin at line %d\r\n", uiLineNr);
				close(hFile);
				return -7;
			}
		}
		else if(strstr(szToken, "Device") == szToken)
		{
			if (bInEEPROMSection)
			{
				if(1 != sscanf(szToken, "Device = %" QUOTEME(GFV_COMMENT_LENGTH) "s", szBuffer))
				{
					printf(TOOL_NAME " error in EEPROM config file line %d\r\n", uiLineNr);
					close(hFile);
					return -7;
				}
				hDev = open(szBuffer, O_RDWR);
				if(hDev < 0)
					printf(TOOL_NAME " couldn't open device %s.\r\n", szBuffer);
			}
		}
		else if(strstr(szToken, "Major Version") == szToken)
		{
			if (bInEEPROMSection)
			{
				if(1 != sscanf(szToken, "Major Version = %" QUOTEME(GFV_NUMBER_LENGTH) "s", hw_info.hw_version.major))
				{
					printf(TOOL_NAME " error in EEPROM config file line %d\r\n", uiLineNr);
					close(hFile);
					return -7;
				}
			}
		}
		else if(strstr(szToken, "Minor Version") == szToken)
		{
			if (bInEEPROMSection)
			{
				if(1 != sscanf(szToken, "Minor Version = %" QUOTEME(GFV_NUMBER_LENGTH) "s", hw_info.hw_version.minor))
				{
					printf(TOOL_NAME " error in EEPROM config file line %d\r\n", uiLineNr);
					close(hFile);
					return -7;
				}
			}
		}
		else if(strstr(szToken, "Article Number") == szToken)
		{
			if (bInEEPROMSection)
			{
				if(1 != sscanf(szToken, "Article Number = %" QUOTEME(GFV_ARTICLE_NUMBER_LENGTH) "s", hw_info.hw_version.article_number))
				{
					printf(TOOL_NAME " error in EEPROM config file line %d\r\n", uiLineNr);
					close(hFile);
					return -7;
				}
			}
		}
		else if(strstr(szToken, "Serial Number") == szToken)
		{
			if (bInEEPROMSection)
			{
				if(1 != sscanf(szToken, "Serial Number = %" QUOTEME(GFV_SERIAL_NUMBER_LENGTH) "s", hw_info.hw_version.serial_numbers))
				{
					printf(TOOL_NAME " error in EEPROM config file line %d\r\n", uiLineNr);
					close(hFile);
					return -7;
				}
			}
		}
		else if(strstr(szToken, "Component") == szToken)
		{
			if (bInEEPROMSection)
			{
				if(1 != sscanf(szToken, "Component = %" QUOTEME(GFV_COMPONENT_LENGTH) """[^\n]*1[\n]", hw_info.hw_version.component))
				{
					printf(TOOL_NAME " error in EEPROM config file line %d\r\n", uiLineNr);
					close(hFile);
					return -7;
				}
			}
		}
		else if(strstr(szToken, "Comment") == szToken)
		{
			if (bInEEPROMSection)
			{
				if(1 != sscanf(szToken, "Comment = %" QUOTEME(GFV_COMMENT_LENGTH) """[^\n]*1[\n]", hw_info.hw_version.comment) )
				{
					printf(TOOL_NAME " error in EEPROM config file line %d\r\n", uiLineNr);
					close(hFile);
					return -7;
				}
			}
		}
		else if(strstr(szToken, "Physical Address of component") == szToken)
		{
			if (bInEEPROMSection)
			{
				if(1 != sscanf(szToken, "Physical Address of component = 0x%x", &(hw_info.hw_version.address_of_component)))
				{
					printf(TOOL_NAME " error in EEPROM config file line %d\r\n", uiLineNr);
					close(hFile);
					return -7;
				}
			}
		}
		else if(strstr(szToken, "Physical memory size of component") == szToken)
		{
			if (bInEEPROMSection)
			{
				if(1 != sscanf(szToken, "Physical memory size of component = 0x%x", &(hw_info.hw_version.phys_mem_size)))
				{
					printf(TOOL_NAME " error in EEPROM config file line %d\r\n", uiLineNr);
					close(hFile);
					return -7;
				}
			}
		}
		else if(strstr(szToken, "Verify") == szToken)
		{
			if (bInEEPROMSection)
			{
				if(1 != sscanf(szToken, "Verify = 0x%x", &(hw_info.verify)))
				{
					printf(TOOL_NAME " error in EEPROM config file line %d\r\n", uiLineNr);
					close(hFile);
					return -7;
				}
			}
		}
		else if(strstr(szToken, "Force write") == szToken)
		{
			if (bInEEPROMSection)
			{
				if(1 != sscanf(szToken, "Force write = 0x%x", &(hw_info.force_write)))
				{
					printf(TOOL_NAME " error in EEPROM config file line %d\r\n", uiLineNr);
					close(hFile);
					return -7;
				}
			}
		}
		szToken = strtok(NULL, "\r\n" );
	}
	close(hFile);
	return ERROR_SUCCESS;
}

static int program(char* device_name, u32 area_number, char* data_file, char* data_string)
{
	int 	  hDev;
	char*	  data = NULL;
	size_t	  size = 0;
	bool	  mem_allocated = false;

	hDev = open(device_name, O_RDWR);
	if(hDev < 0)
	{
		printf(TOOL_NAME " couldn't open device %s.\r\n", device_name);
		return hDev;
	}

	if (data_file)
	{
		int 	  		hData_file, file_size;
		struct stat		f_stat;


		hData_file = open(data_file, O_RDONLY);
		if(hData_file < 0)
		{
			printf(TOOL_NAME " error opening source file %s. (0x%X)\r\n", data_file, errno);
			close(hDev);
			return -errno;
		}

		file_size = fstat(hData_file, &f_stat);
		if (-1 == file_size)
		{
			PRINTF(TOOL_NAME " error couldn't retrieve file attributes.\r\n");
			close(hData_file);
			close(hDev);
			return -errno;
		}

		data = (char*) malloc(sizeof(char)*f_stat.st_size);
		if (!data)
		{
			PRINTF(TOOL_NAME " error couldn't allocate memory.\r\n");
			close(hData_file);
			close(hDev);
			return -ENOMEM;
		}

		file_size = read(hData_file, data, sizeof(char)*f_stat.st_size);
		if(file_size < 0)
		{
			printf(TOOL_NAME " error reading source file %s. (0x%X)\r\n", data_file, errno);
			close(hData_file);
			close(hDev);
			return -errno;
		}
		close(hData_file);
		size = sizeof(char)*f_stat.st_size;
		mem_allocated = true;
	}
	else if (data_string)
	{
		data = data_string;
		size = strlen(data_string);
	}
	else
	{
		printf(TOOL_NAME " failed programming device %s, no source specified.\r\n", device_name);
		close(hDev);
		return -EINVAL;
	}

	if (-1 == pwrite(hDev, data, size, area_number))
	{
		printf(TOOL_NAME " failed writing area %u to device %s with error %u.\r\n", area_number, device_name, errno);
		close(hDev);
		return -errno;
	}
	close(hDev);
	return ERROR_SUCCESS;
}

int main(int argc, char** argv)
{
	u_int32_t dwReturn = 0;
	u_int32_t counter;
	char* 	  device = STANDARD_EEPROM_FILE;
	int 	  hDev;

	if(argc < 2)
	{
		printUsage();
		return 0;
	}

	if(!strcmp(argv[1], GFEEPROM_OP_CLEAN))
	{
		bool full_content = false;

		for (counter = 1; counter < argc; counter++)
		{
			if (!strcmp(argv[counter], GFEEPROM_OP_DEVICE) && (counter+1 < argc))
				device = argv[++counter];
			else if (!strcmp(argv[counter], GFEEPROM_OP_FULL_CONTENT))
				full_content = true;
		}
		dwReturn = clean(device, full_content);
	}
	else if(!strcmp(argv[1], GFEEPROM_OP_SHOW))
	{
		bool	show_all_elements = true;
		bool	bin_dump = false;
		char*	file_name = NULL;
		u32 	eara_number = 0;

		for (counter = 1; counter < argc; counter++)
		{
			if (!strcmp(argv[counter], GFEEPROM_OP_DEVICE) && (counter+1 < argc))
			{
				device = argv[++counter];
				show_all_elements = false;
			}
			else if (!strcmp(argv[counter], GFEEPROM_OP_EEPROM_EA) && (counter+1 < argc))
			{
				eara_number = atol(argv[++counter]);
			}
			else if (!strcmp(argv[counter], GFEEPROM_OP_BIN_DUMP))
			{
				bin_dump = true;
			}
			else if (!strcmp(argv[counter], GFEEPROM_OP_BIN_DUMP_TO_FILE) && (counter+1 < argc))
			{
				file_name = argv[++counter];
				bin_dump = true;
			}
			else if (!strcmp(argv[counter], GFEEPROM_OP_FULL_CONTENT))
			{
				eara_number = DUMP_FULL_CONTENT;
			}
		}
		if (show_all_elements)
			dwReturn = show_all();
		else
			dwReturn = show_eeprom(device, eara_number, bin_dump, file_name);
	}
	else if (!strcmp(argv[1], GFEEPROM_OP_HELP))
	{
		printUsage();
	}
	else if (!strcmp(argv[1], GFEEPROM_OP_PROGRAM))
	{
		u32 	eara_number = EEPROM_TYPE_EHW;
		char*	src_file = NULL;
		char*	src_string = NULL;

		for (counter = 1; counter < argc; counter++)
		{
			if (!strcmp(argv[counter], GFEEPROM_OP_DEVICE) && (counter+1 < argc))
			{
				device = argv[++counter];
			}
			else if (!strcmp(argv[counter], GFEEPROM_OP_EEPROM_EA) && (counter+1 < argc))
			{
				eara_number = atol(argv[++counter]);
			}
			else if (!strcmp(argv[counter], GFEEPROM_OP_BIN_SRC_FILE) && (counter+1 < argc))
			{
				src_file = argv[++counter];
			}
			else if (!strcmp(argv[counter], GFEEPROM_OP_BIN_SRC_STRING) && (counter+1 < argc))
			{
				src_string = argv[++counter];
			}
		}
		if (EEPROM_TYPE_EHW == eara_number)
			dwReturn = program_guf_hw(src_file);
		else
		{
			dwReturn = program(device, eara_number, src_file, src_string);
			if (ERROR_SUCCESS == dwReturn)
				PRINTF(TOOL_NAME " programming finished successful.\r\n");
		}
	}
	else
	{
		printUsage();
		dwReturn = -1;
	}

	close(hDev);
	return dwReturn;
}
