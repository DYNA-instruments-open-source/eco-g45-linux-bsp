#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/types.h>
#define _GNU_SOURCE /* Must be defined to enable getopt_long functinality. */
#include <getopt.h>

#define TRUE 1
#define FALSE 0

#define FAIL -1
#define PASS 0

#define BYTE_ACCESS 0
#define WORD_ACCESS 1
#define DWORD_ACCESS 2

#define LOG(fmt, arg...) \
if (!silent) \
	printf(fmt, ## arg);
unsigned int silent = FALSE;

/* Helper fuctions. */
static int write_and_check_eeprom(char* eeprom_path, __s8 *testdata, unsigned int offset,
						size_t size, __s32 iterations)
{
	int result = FAIL;
	int fd = -1;
	size_t n = 0;
	size_t bytes_left = 0;
	__s8 *readback;
	unsigned int i = 0;
	unsigned int j = 0;

	if(iterations > 1) {
		LOG("Iterations: %d\n", iterations);
	}
	readback = (__s8*)malloc(size);
	if(readback == NULL) {
		perror("ERROR malloc readback memory");
		result = FAIL;
		goto out;
	}
	for(i = 0; i < iterations; i++) {
		/* Write. */
		fd = open(eeprom_path, O_RDWR);
		if(fd == -1) {
			perror("ERROR open EEPROM Sysfs bin file");
			if(iterations > 1) {
				LOG("Iteration %d FAILED\n", i + 1);
			}
			result = FAIL;
			goto out;
		}
		n = 0;
		bytes_left = size;
		do {
			n = write(fd, (const void*)((size_t)testdata-n), bytes_left);
			if(n > 0) {
				bytes_left = bytes_left - n;
			} else if(n == 1) {
				perror("ERROR write testdata to EEPROM");
				if(iterations > 1) {
					LOG("Iteration %d FAILED\n", i + 1);
				}
				result = FAIL;
				goto out;
			} else {
				/* EOF (bytes_left == 0), nothing to do */
			}
		} while(bytes_left != 0);
		close(fd);
		/* Read back. */
		fd = open(eeprom_path, O_RDWR);
		if(fd == -1) {
			perror("ERROR open EEPROM Sysfs bin file: ");
			if(iterations > 1) {
				LOG("Iteration %d FAILED\n", i + 1);
			}
			result = FAIL;
			goto out;
		}
		n = 0;
		bytes_left = size;
		do {
			n = read(fd, (void*)((size_t)readback-n), bytes_left);
			if(n > 0) {
				bytes_left = bytes_left - n;
			} else if(n == 1) {
				perror("ERROR write testdata to EEPROM");
				if(iterations > 1) {
					LOG("Iteration %d FAILED\n", i + 1);
				}
				result = FAIL;
				goto out;
			} else {
				/* EOF (bytes_left == 0), nothing to do */
			}
		} while(bytes_left != 0);
		for(j = 0; j < size; j++) {
			if(readback[j] != testdata[j]) {
				LOG("ERROR: Compare error  at offset %d: Expected value: \
							0x%2x Read value: 0x%2x !!!\n", j, testdata[j],
							readback[j]);
			if(iterations > 1) {
				LOG("Iteration %d FAILED\n", i + 1);
			}
				result = FAIL;
				goto out;
			}
		}
		close(fd);
			if(iterations > 1) {
				LOG("Iteration %d OK\n", i + 1);
			}
	}
	/* Everything OK. */
	result = PASS;
	return result;
out:
	close(fd);
	free(readback);
	
	return result;
}

static int write_and_check_eeprom_single(char* eeprom_path, void* testdata,
	unsigned int offset, int acc_type)
{
	int result = FAIL;
	int fd = -1;
	size_t n = 0;
	__s8 test_s8 = 0;
	__s8 read_s8 = 0;
	__s16 test_s16 = 0;
	__s16 read_s16 = 0;
	__s32 test_s32 = 0;
	__s32 read_s32 = 0;

	if(acc_type == BYTE_ACCESS) {
		test_s8 = *((__s8*)testdata);
	} else if(acc_type == WORD_ACCESS) {
		test_s16 = *((__s16*)testdata);
	} else if(acc_type == DWORD_ACCESS) {
		test_s32 = *((__s32*)testdata);
	} else {
		LOG("Unknown access type.\n");
		result = FAIL;
		goto out;
	}	
	/* Write one byte. */
	fd = open(eeprom_path, O_RDWR);
	if(fd == -1) {
		perror("ERROR open EEPROM Sysfs bin file: ");
		result = FAIL;
		goto out;
	}	
	if(lseek(fd, (off_t)offset, SEEK_SET) != offset) {
		perror("ERROR seek on EEPROM Sysfs bin file");
		result = FAIL;
		goto out;
	}		
	if(acc_type == BYTE_ACCESS) {
		n = write(fd, (const void*)&test_s8, sizeof(__s8));
		if(n != sizeof(__s8)) {
			perror("ERROR write testbyte to EEPROM");
			result = FAIL;
			goto out;
		}
	} else if(acc_type == WORD_ACCESS) {
		n = write(fd, (const void*)&test_s16, sizeof(__s16));
		if(n != sizeof(__s16)) {
			perror("ERROR write testword to EEPROM");
			result = FAIL;
			goto out;
		}
	} else if(acc_type == DWORD_ACCESS) {
		n = write(fd, (const void*)&test_s32, sizeof(__s32));
		if(n != sizeof(__s32)) {
			perror("ERROR write testdword to EEPROM");
			result = FAIL;
			goto out;
		}
	} else {
		LOG("Unknown access type.\n");
		result = FAIL;
		goto out;
	}			
	close(fd);
	/* Read back this byte. */
	fd = open(eeprom_path, O_RDWR);
	if(fd == -1) {
		perror("ERROR open EEPROM Sysfs bin file");
		result = FAIL;
		goto out;
	}
	if(lseek(fd, (off_t)offset, SEEK_SET) != offset) {
		perror("ERROR seek on EEPROM Sysfs bin file");
		result = FAIL;
		goto out;
	}		
	if(acc_type == BYTE_ACCESS) {
		n = read(fd, &read_s8, sizeof(__s8));
		if(n != sizeof(__s8)) {
			perror("ERROR read testbyte from EEPROM");
			result = FAIL;
			goto out;
		}
	} else if(acc_type == WORD_ACCESS) {
		n = read(fd, &read_s16, sizeof(__s16));
		if(n != sizeof(__s16)) {
			perror("ERROR read testword from EEPROM");
			result = FAIL;
			goto out;
		}
	} else if(acc_type == DWORD_ACCESS) {
		n = read(fd, &read_s32, sizeof(__s32));
		if(n != sizeof(__s32)) {
			perror("ERROR read testdword from EEPROM");
			result = FAIL;
			goto out;
		}
	} else {
		LOG("Unknown access type.\n");
		result = FAIL;
		goto out;
	}	
	if(read_s8 != test_s8) {
		LOG("ERROR: Compare error  at offset %d: Expected value: 0x%2x Read value: 0x%2x !!!\n", offset, test_s8, read_s8);
		result = FAIL;
		goto out;
	}
	close(fd);
	/* Everything OK. */
	result = PASS;
	return result;
out:
	if(fd > -1) {
		close(fd);
	}
	return result;
}

/* Testcases */
/* Testcase 1: Set the whole EEPROM memory to 0x00, readback and check. */
static int testcase_1_reset_whole_eeprom(char* eeprom_path, size_t size,
									__s32 iterations)
{
	int result = FAIL;
	__s8 *testdata;

	LOG(
"-----------------------------------------------------------------------------\
--\n"
	);
	LOG(
"Testcase 1 : Set the whole EEPROM memory to 0x00, readback and check.\n"
	);
	testdata = (__s8*)malloc(size);
	if(testdata == NULL) {
		LOG("Readback memory could not be allocated\n");
		return FAIL;
	}
	memset((void*)testdata, 0x00, size);
	result = write_and_check_eeprom(eeprom_path,  testdata, (unsigned int)0, 
			size, iterations);
	free(testdata);
	
	LOG(
"-----------------------------------------------------------------------------\
--\n"
);

	return result;
}

/* Testcase 2: Set the whole EEPROM memory to 0xFF, readback and check. */
static int testcase_2_set_whole_eeprom(char* eeprom_path, size_t size,
									__s32 iterations)
{
	int result = FAIL;
	__s8 *testdata;
	
	LOG(
"-----------------------------------------------------------------------------\
--\n"
	);
	LOG(
"Testcase 2 : Set the whole EEPROM memory to 0xFF, readback and check.\n"
	);
	testdata = (__s8*)malloc(size);
	if(testdata == NULL) {
		LOG("Readback memory could not be allocated\n");
		return FAIL;
	}
	memset((void*)testdata, 0xFF, size);
	result = write_and_check_eeprom(eeprom_path,  testdata, (unsigned int)0, 
			size, iterations);
	free(testdata);
	
	LOG(
"-----------------------------------------------------------------------------\
--\n"
);

	return result;
}

/* Testcase 3: Set the whole EEPROM memory to random data, readback and check. */
static int testcase_3_set_whole_eeprom_random(char* eeprom_path, size_t size,
									unsigned int iterations)
{
	int result = FAIL;
	__s8 *testdata;
	unsigned int seed = 0;
	unsigned int i = 0;
	LOG(
"-----------------------------------------------------------------------------\
--\n"
	);
	LOG(
"Testcase 3 : Set the whole EEPROM memory to random data, readback and check.\n"
	);
	testdata = (__s8*)malloc(size);
	if(testdata == NULL) {
		LOG("Readback memory could not be allocated\n");
		return FAIL;
	}
	seed = (__s32)time(NULL);
	srand(seed);	
	for(i = 0; i < size; i++) {
		testdata[i] = (__s8)((double)rand()/(double)RAND_MAX * UCHAR_MAX);
	}
	result = write_and_check_eeprom(eeprom_path,  testdata, (unsigned int)0, 
			size, iterations);
	free(testdata);
	
	LOG(
"-----------------------------------------------------------------------------\
--\n"
);

	return result;
}

/* Testcase 4: Set the whole EEPROM memory to random data with single byte
               access, readback and check. */
static int testcase_4_whole_eeprom_single_bytes(char* eeprom_path, size_t size,
									unsigned int iterations)
{
	int result = FAIL;
	__s8 test_s8 = 0;
	unsigned int seed = 0;
	unsigned int i = 0;
	unsigned int j = 0;

	LOG(
"-----------------------------------------------------------------------------\
--\n"
	);
	LOG(
"Testcase 4 : Set the whole EEPROM memory to random data with single byte \
access\n"
	);
	LOG("             readback and check.\n");
	if(iterations > 1) {
		LOG("Iterations: %d\n", iterations);
	}
	seed = (__s32)time(NULL);
	srand(seed);	
	for(i = 0; i < iterations; i++) {
		for(j = 0; j < (unsigned int)size; j++) {
			test_s8 = (__s8)((double)rand()/(double)RAND_MAX * UCHAR_MAX);
			result = write_and_check_eeprom_single(eeprom_path, &test_s8, j,
													BYTE_ACCESS);
			if(result == FAIL) {
				LOG("Iteration %d FAILED\n", i + 1);
				goto out;
			}		
		}
		if(iterations > 1) {
			LOG("Iteration %d OK\n", i + 1);
		}
	}
out:
	LOG(
"-----------------------------------------------------------------------------\
--\n"
);

	return result;
}

/* Testcase 5: Set the whole EEPROM memory to random data with single word
               access, readback and check. */
static int testcase_5_whole_eeprom_single_words(char* eeprom_path, size_t size,
									unsigned int iterations)
{
	int result = FAIL;
	__s16 test_s16 = 0;
	unsigned int seed = 0;
	unsigned int i = 0;
	unsigned int j = 0;

	LOG(
"-----------------------------------------------------------------------------\
--\n"
	);
	LOG(
"Testcase 5 : Set the whole EEPROM memory to random data with single word \
access\n"
	);
	LOG("             readback and check.\n");
	if(iterations > 1) {
		LOG("Iterations: %d\n", iterations);
	}
	seed = (__s32)time(NULL);
	srand(seed);	
	for(i = 0; i < iterations; i++) {
		for(j = 0; j < (unsigned int)size; j=j+2) {
			test_s16 = (__s16)((double)rand()/(double)RAND_MAX * USHRT_MAX);
			result = write_and_check_eeprom_single(eeprom_path, &test_s16, j,
													WORD_ACCESS);
			if(result == FAIL) {
				LOG("Iteration %d FAILED\n", i + 1);
				goto out;
			}	
		}
	if(iterations > 1) {
		LOG("Iteration %d OK\n", i + 1);
	}
	}
out:
	LOG(
"-----------------------------------------------------------------------------\
--\n"
);

	return result;
}

/* Testcase 6: Set the whole EEPROM memory to random data with single dword
               access, readback and check. */
static int testcase_6_whole_eeprom_single_dwords(char* eeprom_path, size_t size,
									unsigned int iterations)
{
	int result = FAIL;
	__s32 test_s32 = 0;
	unsigned int seed = 0;
	unsigned int i = 0;
	unsigned int j = 0;

	LOG(
"-----------------------------------------------------------------------------\
--\n"
	);
	LOG(
"Testcase 6 : Set the whole EEPROM memory to random data with single dword\n"
	);
	LOG("             access readback and check.\n");
	if(iterations > 1) {
		LOG("Iterations: %d\n", iterations);	
	}
	seed = (__s32)time(NULL);
	srand(seed);	
	for(i = 0; i < iterations; i++) {
		for(j = 0; j < (unsigned int)size; j=j+4) {
			test_s32 = (__s32)((double)rand()/(double)RAND_MAX * UINT_MAX);
			result = write_and_check_eeprom_single(eeprom_path, &test_s32, j,
													BYTE_ACCESS);
			if(result == FAIL) {
				LOG("Iteration %d FAILED\n", i + 1);
				goto out;
			}		
		}
		if(iterations > 1) {
			LOG("Iteration %d OK\n", i + 1);
		}
	}
out:
	LOG(
"-----------------------------------------------------------------------------\
--\n"
);

	return result;
}

/* Testcase 7: Write accross EEPROM page boundaries, readback and check. */
static int testcase_7_cross_page_boundaries(char* eeprom_path, size_t size,
													size_t page_size,												
													unsigned int iterations)
{
	int result = FAIL;
	__s8 *testdata;
	unsigned int seed = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int offset = 0;

	LOG(
"-----------------------------------------------------------------------------\
--\n"
	);
	LOG(
"Testcase 7: Write accross EEPROM page boundaries, readback and check.\n"
	);
	LOG("             readback and check.\n");
	if(iterations > 1) {
		LOG("Iterations: %d\n", iterations);
	}
	testdata = (__s8*)malloc(page_size);
	if(testdata == NULL) {
		LOG("Readback memory could not be allocated\n");
		return FAIL;
	}
	seed = (__s32)time(NULL);
	srand(seed);	
	for(i = 0; i < iterations; i++) {
		for(j = 0; j < page_size; j++) {
			testdata[j] = (__s8)((double)rand()/(double)RAND_MAX * UCHAR_MAX);
		}
		for(offset = page_size/2; offset < size-page_size-1; offset += page_size) {
			result = write_and_check_eeprom(eeprom_path,  testdata, (unsigned int)0, 
					page_size, 1);
			if(result == FAIL) {
				LOG("Iteration %d FAILED\n", i + 1);
				goto out;
			}		
		}
		if(iterations > 1) {
			LOG("Iteration %d OK\n", i + 1);
		}
	}
out:
	free(testdata);	
	LOG(
"-----------------------------------------------------------------------------\
--\n"
);
	return result;
}

/* Testcase 8: Write to random byte offsets in the EEPROM, readback and 
               check. */
static int testcase_8_random_byte_offsets(char* eeprom_path, size_t size,
									unsigned int iterations)
{
	int result = FAIL;
	unsigned int seed = 0;
	unsigned int offset = 0;
	__s8 test_s8 = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	
	LOG(
"-----------------------------------------------------------------------------\
--\n"
	);
	LOG(
"Testcase 8: Write to random byte offsets in the EEPROM, readback and \
check.\n");
	LOG(
"-----------------------------------------------------------------------------\
--\n"
);
	if(iterations > 1) {
		LOG("Iterations: %d\n", iterations);
	}
	LOG("Accesses per iteration: %d\n", size);	
	seed = (__s32)time(NULL);
	srand(seed);	
	for(i = 0; i < iterations; i++) {
		/* Use the possible number of offsets as count. */
		for(j = 0; j<size; j++) {
			offset = (unsigned)((double)rand()/(double)RAND_MAX * (size-1));
			test_s8 = (__s8)((double)rand()/(double)RAND_MAX * UCHAR_MAX);
			result = write_and_check_eeprom_single(eeprom_path, &test_s8, offset,
													BYTE_ACCESS);
			if(result == FAIL) {
				LOG("Iteration %d FAILED\n", i + 1);
				goto out;
			}		
		}
		if(iterations > 1) {
			LOG("Iteration %d OK\n", i + 1);
		}
	}
out:
	return result;
}

/* Testcase 9: Write to random word offsets in the EEPROM, readback and 
               check. */
static int testcase_9_random_word_offsets(char* eeprom_path, size_t size,
									unsigned int iterations)
{
	int result = FAIL;
	unsigned int seed = 0;
	unsigned int offset = 0;
	unsigned int tmp = 0;
	__s16 test_s16 = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	
	LOG(
"-----------------------------------------------------------------------------\
--\n"
	);
	LOG(
"Testcase 9: Write to random word offsets in the EEPROM, readback and \
check.\n");
	LOG(
"-----------------------------------------------------------------------------\
--\n"
);
	if(iterations > 1) {
		LOG("Iterations: %d\n", iterations);
	}
	LOG("Accesses per iteration: %d\n", size/2);	
	seed = (__s32)time(NULL);
	srand(seed);	
	for(i = 0; i < iterations; i++) {
		/* Use the possible number of offsets as count. */
		for(j = 0; j < size/2; j++) {
			offset = (unsigned)((double)rand()/(double)RAND_MAX * (size-1));
			tmp = offset >> 1;
			offset = tmp << 1;
			test_s16 = (__s16)((double)rand()/(double)RAND_MAX * USHRT_MAX);
			result = write_and_check_eeprom_single(eeprom_path, &test_s16, offset,
													WORD_ACCESS);
			if(result == FAIL) {
				LOG("Iteration %d FAILED\n", i + 1);
				goto out;
			}		
		}
		if(iterations > 1) {
			LOG("Iteration %d OK\n", i + 1);
		}
	}
out:
	return result;
}

/* Testcase 10: Write to random dword offsets in the EEPROM, readback and 
               check. */
static int testcase_10_random_dword_offsets(char* eeprom_path, size_t size,
									unsigned int iterations)
{
	int result = FAIL;
	unsigned int seed = 0;
	unsigned int offset = 0;
	unsigned int tmp = 0;
	__s32 test_s32 = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	
	LOG(
"-----------------------------------------------------------------------------\
--\n"
	);
	LOG(
"Testcase 10: Write to random dword offsets in the EEPROM, readback \
check.\n");
	LOG(
"-----------------------------------------------------------------------------\
--\n"
);
	if(iterations > 1) {
		LOG("Iterations: %d\n", iterations);
	}
	LOG("Accesses per iteration: %d\n", size/(sizeof(__s32)));	
	seed = (__s32)time(NULL);
	srand(seed);	
	for(i = 0; i < iterations; i++) {
		/* Use the possible number of offsets as count. */
		for(j = 0; j < size/(sizeof(__s32)); j++) {
			offset = (unsigned)((double)rand()/(double)RAND_MAX * (size-1));
			tmp = offset >> 2;
			offset = tmp << 2;
			test_s32 = (__s32)((double)rand()/(double)RAND_MAX * UINT_MAX);
			result = write_and_check_eeprom_single(eeprom_path, &test_s32, offset,
													DWORD_ACCESS);
			if(result == FAIL) {
				LOG("Iteration %d FAILED\n", i + 1);
				goto out;
			}		
		}
		if(iterations > 1) {
			LOG("Iteration %d OK\n", i + 1);
		}
	}
out:
	return result;
}

/* Testcase 11: Write words to random byte offsets in the EEPROM, readback and 
               check. */
static int testcase_11_random_byte_offsets_word(char* eeprom_path, size_t size,
									unsigned int iterations)
{
	int result = FAIL;
	unsigned int seed = 0;
	unsigned int offset = 0;
	__s16 test_s16 = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	
	LOG(
"-----------------------------------------------------------------------------\
--\n"
	);
	LOG(
"Testcase 11: Write words to random byte offsets in the EEPROM, readback\n\
and check.\n");
	LOG(
"-----------------------------------------------------------------------------\
--\n"
);
	if(iterations > 1) {
		LOG("Iterations: %d\n", iterations);
	}
	LOG("Accesses per iteration: %d\n", size/(sizeof(__s16)));	
	seed = (__s32)time(NULL);
	srand(seed);	
	for(i = 0; i < iterations; i++) {
		/* Use the possible number of offsets as count. */
		for(j = 0; j < size/(sizeof(__s16)); j++) {
			offset = (unsigned)((double)rand()/(double)RAND_MAX * (size-1));  
			if(offset > size-sizeof(__s16)) {
				offset = size-sizeof(__s16);
			}
			test_s16 = (__s16)((double)rand()/(double)RAND_MAX * USHRT_MAX);
			result = write_and_check_eeprom_single(eeprom_path, &test_s16, offset,
													WORD_ACCESS);
			if(result == FAIL) {
				LOG("Iteration %d FAILED\n", i + 1);
				goto out;
			}		
		}
		if(iterations > 1) {
			LOG("Iteration %d OK\n", i + 1);
		}
	}
out:
	return result;
}

/* Testcase 12: Write dwords to random byte offsets in the EEPROM, readback and 
               check. */
static int testcase_12_random_byte_offsets_dword(char* eeprom_path, size_t size,
									unsigned int iterations)
{
	int result = FAIL;
	unsigned int seed = 0;
	unsigned int offset = 0;
	__s16 test_s32 = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	
	LOG(
"-----------------------------------------------------------------------------\
--\n"
	);
	LOG(
"Testcase 12: Write dwords to random byte offsets in the EEPROM, readback\n\
and check.\n");
	LOG(
"-----------------------------------------------------------------------------\
--\n"
);
	if(iterations > 1) {
		LOG("Iterations: %d\n", iterations);
	}
	LOG("Accesses per iteration: %d\n", size/(sizeof(__s32)));	
	seed = (__s32)time(NULL);
	srand(seed);	
	for(i = 0; i < iterations; i++) {
		/* Use the possible number of offsets as count. */
		for(j = 0; j < size/(sizeof(__s32)); j++) {
			offset = (unsigned)((double)rand()/(double)RAND_MAX * (size-1));  
			if(offset > size-sizeof(__s32)) {
				offset = size-sizeof(__s32);
			}
			test_s32 = (__s32)((double)rand()/(double)RAND_MAX * UINT_MAX);
			result = write_and_check_eeprom_single(eeprom_path, &test_s32, offset,
													DWORD_ACCESS);
			if(result == FAIL) {
				LOG("Iteration %d FAILED\n", i + 1);
				goto out;
			}		
		}
		if(iterations > 1) {
			LOG("Iteration %d OK\n", i + 1);
		}
	}
out:
	return result;
}

/* Testcase 13: Write dwords to random word offsets in the EEPROM, readback and 
               check. */
static int testcase_13_random_word_offsets_dword(char* eeprom_path, size_t size,
									unsigned int iterations)
{
	int result = FAIL;
	unsigned int seed = 0;
	unsigned int offset = 0;
	unsigned int tmp = 0;
	__s32 test_s32 = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	
	LOG(
"-----------------------------------------------------------------------------\
--\n"
	);
	LOG(
"Testcase 13: Write dwords to random word offsets in the EEPROM, readback\n\
and check.\n");
	LOG(
"-----------------------------------------------------------------------------\
--\n"
);
	if(iterations > 1) {
		LOG("Iterations: %d\n", iterations);
	}
	LOG("Accesses per iteration: %d\n", size/(sizeof(__s32)));	
	seed = (__s32)time(NULL);
	srand(seed);	
	for(i = 0; i < iterations; i++) {
		/* Use the possible number of offsets as count. */
		for(j = 0; j < size/(sizeof(__s32)); j++) {
			offset = (unsigned)((double)rand()/(double)RAND_MAX * (size-1)); 
			tmp = offset >> 1;
			offset = tmp << 1;

			if(offset > size-sizeof(__s32)) {
				offset = size-sizeof(__s32);
			}
			test_s32 = (__s32)((double)rand()/(double)RAND_MAX * UINT_MAX);
			result = write_and_check_eeprom_single(eeprom_path, &test_s32, offset,
													DWORD_ACCESS);
			if(result == FAIL) {
				LOG("Iteration %d FAILED\n", i + 1);
				goto out;
			}		
		}
		if(iterations > 1) {
			LOG("Iteration %d OK\n", i + 1);
		}
	}
out:
	return result;
}

static void show_help(void)
{
	LOG(
"Usage: eepromtest [OPTIONS]\n"
"Executes the specified EEPROM test.\n"
"\n"
"  -a, --address           I2C address of the EEPROM (e.g. 0x57).\n"
"  -t, --testcase          Tescase to run:\n"
"                          all: Run all testcases.\n"
"                          reset_all: Sets the whole contents of the EEPROM to\
 0x00.\n"
"                          set_all: Sets the whole contents of the EEPROM to\
 0xFF.\n"
"                          set_all_random: Sets the whole contents of the\
 EEPROM to random bytes.\n"
"                          all_single_bytes: Sets the whole contents of the\
 EEPROM to random bytes with single byte accesses.\n"
"                          all_single_words: Sets the whole contents of the\
 EEPROM to random bytes with single word accesses.\n"
"                          all_single_dwords: Sets the whole contents of the\
 EEPROM to random bytes with single dword accesses.\n"
"                          cross_page_boundaries: Writes random data accross\
 EEPROM page boundaries.\n"
"                          random_byte_offsets: Writes to random byte offsets\
 in the EEPROM.\n"
"                          random_word_offsets: Writes to random word offsets\
 in the EEPROM.\n"
"                          random_dword_offsets: Writes to random dword\
offsets in the EEPROM.\n"
"                          random_byte_offsets_word: Writes words to random byte\
offsets in the EEPROM.\n"
"                          random_byte_offsets_dword: Writes dwords to random byte\
offsets in the EEPROM.\n"
"                          random_word_offsets_dword: Writes dwords to random word\
offsets in the EEPROM.\n"
"  -s, --silent            Do not write any output to stdout.\n"
"  -h, --help              Show this help and exit.\n"
	);
	return;
}

int main(int argc, char *argv[])
{
	int opt;
	struct option longopts[] = {
		{"help"      , 0 /* no arg */ , NULL, 'h' /* getopt_long ret 'h' */},
		{"silent"    , 0 /* no arg */ , NULL, 's' /* getopt_long ret 's' */},
		{"testcase"  , 1 /* has arg */, NULL, 't' /* getopt_long ret 't' */},
		{"iterations", 1 /* has arg */, NULL, 'i' /* getopt_long ret 'i' */},
		{"address"   , 1 /* has arg */, NULL, 'a' /* getopt_long ret 'a' */},
		{0, 0, 0, 0}  /* Array always terminates with a NULL option. */
	};
	
	unsigned int i2c_address;
	unsigned int i2c_address_set = FALSE;
	unsigned int iterations = 0;
	unsigned int iterations_set = FALSE;
	char testcase[128];
	unsigned int testcase_set = FALSE;
	char eeprom_path[128];
	struct stat eeprom_st;
	char eeprom_type[32];
	unsigned int eeprom_byte_size = 0;
	unsigned int eeprom_page_size = 0;
	unsigned int eeprom_num_pages = 0;
	int fd = -1;
	char path[128];
	size_t n = 0;
	int result = FAIL;
	unsigned int i = 0;
	
	/* Option parsing. */
	/* Reamark: the shortopts must match the return values from
	            the longopts. A colon in the shortopts means has arg. */
	while((opt = getopt_long(argc, argv, ":hst:i:a:", longopts, NULL)) != -1) {
		switch(opt) {
		case 'h':
			show_help();
			return PASS;
			break;
		case 's':
			silent = TRUE;
			break;
		case 't':
			strcpy(testcase, optarg);
			testcase_set = TRUE;
			break;
		case 'i':
			iterations = strtol (optarg, NULL, 0);
			if (iterations < 1) {
				LOG("Iteration count must be bigger that '0'.\n");
				show_help();
				return FAIL;
			} else {
				iterations_set = TRUE;
			}
			break;
		case 'a':
			if (sscanf(optarg, "0x%x", &i2c_address) != 1) {
				LOG("Hex numbers must begin with '0x'.\n");
				show_help();
				return FAIL;
			} else {
				if(i2c_address > 0xFF) {
					LOG("I2C address must not be bigger than '0xFF'.\n");
					return FAIL;
				} else {
					i2c_address_set = TRUE;
					sprintf(eeprom_path, "/sys/bus/i2c/devices/0-00%2x/", i2c_address);
					strcpy(path, eeprom_path);
					strcat(path, "eeprom");
					if(stat(path, &eeprom_st) == -1) {
						LOG("Cannot find EEPROM in path '%s'\n", eeprom_path);
						return FAIL;
					}
				}
			}
			break;
		case ':':
			LOG("Option needs a value.\n");
			show_help();
			return FAIL;
			break;
		case '?':
			LOG("Unknow option: %c\n", optopt);
			show_help();
			return FAIL;
			break;
		default:
			show_help();
			return FAIL;
		break;
		}
	}
	
	if(!testcase_set) {
		LOG("Testcase must be specified e.g. '-t reset_all' or '--testcase reset_all'.\n");
		show_help();
		return FAIL;
	}
	if(!i2c_address_set) {
		LOG("I2C adress must be specified e.g. '-a 0x57' or '--address 0x57'.\n");
		show_help();
		return FAIL;
	}
	if(!iterations_set) {
		LOG("Iterations are not specified like e.g. '-i 5' or '--iterations 5'.\n");
		LOG("Using 1 iteration by default.\n");
		iterations = 1;
	}

	/* Parse eeprom type from sysfs and set eeprom data */
	strcpy(path, eeprom_path);
	strcat(path, "name");
	fd = open(path, O_RDONLY);
	if(fd == -1) {
		LOG("Could open EEPROM I2C 'name' sysfs entry.\n");
		return FAIL;
	}
	n = read(fd, (void*)eeprom_type, sizeof(eeprom_type));
	if(n < 5) { /* lowest eeprom name size */
		LOG("Could not read EEPROM type from sysfs.\n");
		close(fd);
		return FAIL;
	}
	/* Type name does not terminate with '\0'*/
	for(i = 0; i<sizeof(eeprom_type); i++){
		if((eeprom_type[i] < '0' || eeprom_type[i] > '9')
			&& (eeprom_type[i] < 'A' || eeprom_type[i] > 'Z')
			&& (eeprom_type[i] < 'a' || eeprom_type[i] > 'z')) {
			eeprom_type[i] = '\0';
			break;
		}
	}
	close(fd);
	/* TODO: Add here more EEPROM types. */
	if(!strcmp("24c32", eeprom_type)) {
		eeprom_byte_size = 4096;
		eeprom_page_size = 128;
		eeprom_num_pages = 32;
	} else {
		LOG("Unknown EEPROM type: %s\n", eeprom_type);
		return FAIL;
	}

	
	LOG(
"================================== EPROM-Test ===============================\
==\n"
	);
	LOG("EEPROM type           : %s\n", eeprom_type);
	LOG("EEPROM size           : %d bytes\n", eeprom_byte_size);
	LOG("EEPROM page size      : %d bytes\n", eeprom_page_size);
	LOG("EEPROM number of pages: %d\n", eeprom_num_pages);

	strcpy(path, eeprom_path);
	strcat(path, "eeprom");

	/* Execute the specified testcase. */
	if(!strcmp(testcase, "reset_all")) {
		result = testcase_1_reset_whole_eeprom(path, eeprom_byte_size,
												iterations);
	} else if(!strcmp(testcase, "set_all")) {
		result = testcase_2_set_whole_eeprom(path, eeprom_byte_size,
												iterations);
	} else if(!strcmp(testcase, "set_all_random")) {
		result = testcase_3_set_whole_eeprom_random(path, eeprom_byte_size,
												iterations);
	} else if(!strcmp(testcase, "all_single_bytes")) {
		result = testcase_4_whole_eeprom_single_bytes(path, eeprom_byte_size,
												iterations);
	} else if(!strcmp(testcase, "all_single_words")) {
		result = testcase_5_whole_eeprom_single_words(path, eeprom_byte_size,
												iterations);
	} else if(!strcmp(testcase, "all_single_dwords")) {
		result = testcase_6_whole_eeprom_single_dwords(path, eeprom_byte_size,
												iterations);
	} else if(!strcmp(testcase, "cross_page_boundaries")) {
		result = testcase_7_cross_page_boundaries(path, eeprom_byte_size,
												eeprom_page_size,												
												iterations);	
	} else if(!strcmp(testcase, "random_byte_offsets")) {
		result = testcase_8_random_byte_offsets(path, eeprom_byte_size,
												iterations);	
	} else if(!strcmp(testcase, "random_word_offsets")) {
		result = testcase_9_random_word_offsets(path, eeprom_byte_size,
												iterations);
	} else if(!strcmp(testcase, "random_dword_offsets")) {
		result = testcase_10_random_dword_offsets(path, eeprom_byte_size,
												iterations);
	} else if(!strcmp(testcase, "random_byte_offsets_word")) {
		result = testcase_11_random_byte_offsets_word(path, eeprom_byte_size,
												iterations);
	} else if(!strcmp(testcase, "random_byte_offsets_dword")) {
		result = testcase_12_random_byte_offsets_dword(path, eeprom_byte_size,
												iterations);
	} else if(!strcmp(testcase, "random_word_offsets_dword")) {
		result = testcase_13_random_word_offsets_dword(path, eeprom_byte_size,
												iterations);												
	} else if(!strcmp(testcase, "all")) {
		for(i = 0; i< iterations; i++) {
			if(iterations > 1) {
				LOG("Overall iterations: %d\n", iterations);
			}
			result = testcase_1_reset_whole_eeprom(path, eeprom_byte_size,
													iterations);
			if(result == FAIL) {
				LOG("Overall iteration %d FAILED\n", i + 1);
				break;
			}
			result = testcase_2_set_whole_eeprom(path, eeprom_byte_size,
													1);
			if(result == FAIL) {
				LOG("Overall iteration %d FAILED\n", i + 1);
				break;
			}
			result = testcase_3_set_whole_eeprom_random(path, eeprom_byte_size,
													1);
			if(result == FAIL) {
				LOG("Overall iteration %d FAILED\n", i + 1);
				break;
			}
			result = testcase_4_whole_eeprom_single_bytes(path, eeprom_byte_size,
													1);
			if(result == FAIL) {
				LOG("Overall iteration %d FAILED\n", i + 1);
				break;
			}
			result = testcase_5_whole_eeprom_single_words(path, eeprom_byte_size,
													1);
			if(result == FAIL) {
				LOG("Overall iteration %d FAILED\n", i + 1);
				break;
			}
			result = testcase_6_whole_eeprom_single_dwords(path, eeprom_byte_size,
													1);
			if(result == FAIL) {
				LOG("Overall iteration %d FAILED\n", i + 1);
				break;
			}
			result = testcase_7_cross_page_boundaries(path, eeprom_byte_size,
													eeprom_page_size,												
													1);												
			if(result == FAIL) {
				LOG("Overall iteration %d FAILED\n", i + 1);
				break;
			}
			result = testcase_8_random_byte_offsets(path, eeprom_byte_size,
												iterations);	
			if(result == FAIL) {
				LOG("Overall iteration %d FAILED\n", i + 1);
				break;
			}
			result = testcase_9_random_word_offsets(path, eeprom_byte_size,
												iterations);	
			if(result == FAIL) {
				LOG("Overall iteration %d FAILED\n", i + 1);
				break;
			}
			result = testcase_10_random_dword_offsets(path, eeprom_byte_size,
												iterations);	
			if(result == FAIL) {
				LOG("Overall iteration %d FAILED\n", i + 1);
				break;
			}
			result = testcase_11_random_byte_offsets_word(path, eeprom_byte_size,
												iterations);
			if(result == FAIL) {
				LOG("Overall iteration %d FAILED\n", i + 1);
				break;
			}
			result = testcase_12_random_byte_offsets_dword(path, eeprom_byte_size,
												iterations);
			if(result == FAIL) {
				LOG("Overall iteration %d FAILED\n", i + 1);
				break;
			}
			result = testcase_13_random_word_offsets_dword(path, eeprom_byte_size,
												iterations);
			if(result == FAIL) {
				LOG("Overall iteration %d FAILED\n", i + 1);
				break;
			}

			if(iterations > 1) {
				LOG("Overall teration %d OK\n", i + 1);
			}
		}
	} else {
		LOG("Unknown testcase '%s'\n", testcase);
		result = FAIL;
	}

	if(result == FAIL) {
		LOG("Test result: FAIL\n");
	} else {
		LOG("Test result: PASS\n");
	}
	LOG(
"=============================================================================\
==\n"
	);

    return result;
}
