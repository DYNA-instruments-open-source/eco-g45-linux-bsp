/******************************************************************************/
/* Copyright (C) 2011 Garz&Fricke GmbH						 			      */
/*		No use or disclosure of this information in any form without	      */
/*		the written permission of the author							      */
/******************************************************************************/

/******************************************************************************/
/* INCLUDES													 			      */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>


/******************************************************************************/
/* DEFINES													 			      */
/******************************************************************************/
#define VERSION		"1.0"
#define SVN_REV		"$Revision: 920 $"

// The testcases are stored in a linked list
typedef struct tc_list_item_t
{
	char name[64];
	char command[128];
	int result;
	struct tc_list_t *next;
} tc_list_item;


/******************************************************************************/
/* GLOBAL VARIABLES											 			      */
/******************************************************************************/
int				number_of_testcases = 0;
int				testcases_failed = 0;
int				total_testcases_run = 0;
int				total_testcases_failed = 0;
int				runs = 1;

char			logfile[256]    = "logfile.txt";
char			ltp_path[256]   = "/opt/ltp";
char			configfile[256] = "planet_emv";
char			version[10]     = VERSION;

FILE			*p_logfile;
struct utsname	sysinfo;
struct timeval	start, end;

// We have three linked list: testcases, preparation, cleanup
tc_list_item	testcase_top, preparation_top, cleanup_top;
tc_list_item	*p_current_testcase = &testcase_top;
tc_list_item	*p_current_preparation = &preparation_top;
tc_list_item	*p_current_cleanup = &cleanup_top;


/******************************************************************************/
/* FUNCTIONS												 			      */
/******************************************************************************/
void usage()
{
	printf("Built-In-Test v%s\n", version);
	printf("Syntax: bit [-r <runs>] [-l <logfile>] [-h] <scriptfile>\n");
	printf("	-r	: number of runs (default: 1), 0 defines infinite\n");
	printf("	-l	: custom logfile name (default: logfile.txt)\n");
	printf("	-p	: path to LTP installation (default: /opt/ltp)\n");
	printf("	-h	: this help\n");
	exit(1);
}


void parse_arguments(int argc, char **argv)
{
	int c;
	char *filename;
	extern char *optarg;
	extern int optind, optopt, opterr;

	while ((c = getopt(argc, argv, ":l:r:p:h")) != -1)
	{
		switch(c)
		{
		case 'h':
			usage();
			break;
		case 'r':
			runs = atoi(optarg);
			break;
		case 'l':
			strcpy(logfile, optarg);
			break;
		case 'p':
			strcpy(ltp_path, optarg);
			break;
		case '?':
			usage();
			break;
		}
	}

	// Use the first parameter after getopts as scriptfile
	if (optind == argc)
		usage();
	strcpy(configfile, argv[optind]);

	// Increase OPTIND and run getopts again, so that the scriptfile
	// can be specified at any place within the parameter list.
	optind++;
	while ((c = getopt(argc, argv, ":l:r:p:h")) != -1)
	{
		switch(c)
		{
		case 'h':
			usage();
			break;
		case 'r':
			runs = atoi(optarg);
			break;
		case 'l':
			strcpy(logfile, optarg);
			break;
		case 'p':
			strcpy(ltp_path, optarg);
			break;
		case '?':
			usage();
			break;
		}
	}
}


int create_logfile()
{
	mkdir("results", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

	chdir("./results");

	p_logfile = fopen(logfile, "w");
	chdir("..");

	if (!p_logfile)
		return errno;

	return 0;
}


void close_logfile()
{
	close(p_logfile);
}


void log_header()
{
	time_t rawtime;
	struct tm *timeinfo;

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	fprintf(p_logfile, "Test Start Time: %s\n", asctime (timeinfo));
	fprintf(p_logfile, "---------------------------------------------\n");
	fprintf(p_logfile, "Testcase                       Result     Exit Value\n");
	fprintf(p_logfile, "--------                       ------     ----------\n");
}


void log_summary(int mseconds)
{
	fprintf(p_logfile, "\n-----------------------------------------------\n");
	fprintf(p_logfile, "Total Tests: %d\n", number_of_testcases);
	fprintf(p_logfile, "Total Failures: %d\n", testcases_failed);
	fprintf(p_logfile, "Kernel Version: %s\n", sysinfo.release);
	fprintf(p_logfile, "Machine Architecture: %s\n", sysinfo.machine);
	fprintf(p_logfile, "Hostname: %s\n\n", sysinfo.nodename);
	fprintf(p_logfile, "Elapsed time: %d.%d seconds\n\n\n", mseconds/1000, mseconds%1000/100);
}


void log_result(char *name, int value)
{
	char *result_string = "PASS";
	if (value != 0)
		result_string = "FAIL";
	fprintf(p_logfile, "%-30s %-10s %d\n", name, result_string, value);
}


int read_configfile()
{
	FILE *fp;
	static char line[200];
	static char *line_ptr, *command, *name;
	tc_list_item **pp_list_item = &p_current_testcase;

	chdir("./runtest");
	fp = fopen(configfile, "r");
	chdir("..");

	if (!fp)
		return errno;

	// Read configfile line by line
	while (fgets(line, sizeof(line), fp))
	{
		// Check for separation line
		if (line[0]=='#' && line[1]==' ')
		{
			// Switch to the according linked list
			if (line[2]=='P')
				pp_list_item = &p_current_preparation;
			if (line[2]=='T')
				pp_list_item = &p_current_testcase;
			if (line[2]=='C')
				pp_list_item = &p_current_cleanup;
		}

		// Skip comment and empty lines
		if (line[0]=='#' || line[0]==' ' || line[0]=='\n')
			continue;

		// Split line into name and command
		line_ptr = line;
		name = line_ptr;
		while (*line_ptr != ' ' && *line_ptr != '\t')
			line_ptr++;
		*(line_ptr++) = 0;
		while (*line_ptr == ' ' || *line_ptr == '\t')
			line_ptr++;
		command = line_ptr;

		// Store command in linked list
		strcpy((*pp_list_item)->name, name);
		strcpy((*pp_list_item)->command, command);
		(*pp_list_item)->next = calloc(1, sizeof(tc_list_item));
		*pp_list_item = (*pp_list_item)->next;

		if (*pp_list_item == p_current_testcase)
			number_of_testcases++;
	}
	fclose(fp);

	return 0;
}


void free_linked_lists()
{
	p_current_testcase = testcase_top.next;
	while (p_current_testcase != NULL)
	{
		tc_list_item *temp = p_current_testcase->next;
		free(p_current_testcase);
		p_current_testcase = temp;
	}
	p_current_preparation = preparation_top.next;
	while (p_current_preparation != NULL)
	{
		tc_list_item *temp = p_current_preparation->next;
		free(p_current_preparation);
		p_current_preparation = temp;
	}
	p_current_cleanup = cleanup_top.next;
	while (p_current_cleanup != NULL)
	{
		tc_list_item *temp = p_current_cleanup->next;
		free(p_current_cleanup);
		p_current_cleanup = temp;
	}
}


void run_cleanup()
{
	int ret;

	chdir("./testcases/bin");

	p_current_cleanup = &cleanup_top;
	while (p_current_cleanup->name[0] != 0)
	{
		ret = system(p_current_cleanup->command);
		p_current_cleanup = p_current_cleanup->next;
	}

	chdir("../..");
	exit(0);
}


void run_preparations()
{
	int ret;

	chdir("./testcases/bin");

	printf("Preparing tests...\n");
	p_current_preparation = &preparation_top;
	while (p_current_preparation->name[0] != 0)
	{
		ret = system(p_current_preparation->command);
		if (WEXITSTATUS(ret) != 0 && WEXITSTATUS(ret) != 255)
		{
			printf("\nStep \"%s\" reported an error(%d).\nPreparation failed!\n", p_current_preparation->name, WEXITSTATUS(ret));
			run_cleanup();
		}
		p_current_preparation = p_current_preparation->next;
	}

	chdir("../..");
}


void run_tests()
{
	long mseconds;
	int ret;

	chdir("./testcases/bin");
	log_header();
	gettimeofday(&start, NULL);

	// Run tests and log results
	p_current_testcase = &testcase_top;
	while (p_current_testcase->name[0] != 0)
	{
		ret = system(p_current_testcase->command);
		p_current_testcase->result = WEXITSTATUS(ret);
		log_result(p_current_testcase->name, p_current_testcase->result);
		if (p_current_testcase->result != 0)
			testcases_failed++;

		// Check for Ctrl+C
		if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT))
			run_cleanup();

		p_current_testcase = p_current_testcase->next;
	}

	gettimeofday(&end, NULL);
	mseconds = ((end.tv_sec  - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec)/1000.0) + 0.5;

	// Show summary of test results
	printf("\n----------------------------------------------------\n");
	printf("Testcase                       Result     Exit Value\n");
	printf("--------                       ------     ----------\n");
	p_current_testcase = &testcase_top;
	while (p_current_testcase->name[0] != 0)
	{
		char *result_string = "PASS";
		if (p_current_testcase->result != 0)
			result_string = "FAIL";
		printf("%-30s %-10s %d\n", p_current_testcase->name, result_string, p_current_testcase->result);
		p_current_testcase = p_current_testcase->next;
	}
	printf("----------------------------------------------------\n");
	printf("Elapsed time: %d.%d seconds\n", mseconds/1000, mseconds%1000/100);
	printf("----------------------------------------------------\n");

	log_summary(mseconds);
	chdir("../..");
}


int main(int argc, char **argv)
{
	int i;
	char path[512];
	char *oldpath;

	// Construct version string
	strcat(version, "r");
	strncat(version, strchr(SVN_REV, ' ')+1, strrchr(SVN_REV, ' ')-strchr(SVN_REV, ' ')-1);

	// Parse command line arguments
	parse_arguments(argc, argv);

	// Change working directory to the LTP root
	chdir(ltp_path);

	// Add current directory to the search PATH, so that
	// testcases can be executed without absolute path.
	oldpath = getenv("PATH");
	strcpy(path, oldpath);
	strcat(path, ":.");
	setenv("PATH", path, 1);

	// Read configfile and create a linked list of all testcases and preparation/cleanup steps
	if (read_configfile() != 0)
	{
		printf("ERROR: Could not read configfile \"%s\"\n", configfile);
		return -1;
	}

	// Display header
	printf("\n");
	printf("***************************************\n");
	printf("* Garz & Fricke - Linux Built-In-Test *\n");
	printf("***************************************\n");
	printf("Built-In-Test v%s\n", version);

	// Run preparations
	run_preparations();

	// Read system information for logfile summary
	uname(&sysinfo);

	// Create logfile
	if (create_logfile() != 0)
	{
		printf("ERROR: Could not create logfile \"%s\"\n", logfile);
		return -1;
	}

	// Run all tests in the list
	for (i = 0; i < runs || runs == 0; i++)
	{
		testcases_failed = 0;

		printf("\n***************************************\n");
		printf("* Status                              *\n");
		printf("***************************************\n");
		printf("*\n");
		printf("* This is run %d of %d\n", i+1, runs);
		printf("* Number of passed tests so far: %d\n", total_testcases_run-total_testcases_failed);
		printf("* Number of failed tests so far: %d\n", total_testcases_failed);
		printf("*\n");
		printf("***************************************\n\n");

		run_tests();

		total_testcases_run += number_of_testcases;
		total_testcases_failed += testcases_failed;
	}

	// Display summary
	printf("\n***************************************\n");
	printf("* Summary                             *\n");
	printf("***************************************\n");
	printf("*\n");
	printf("* Test runs: %d\n", runs);
	printf("* Totally executed tests: %d\n", total_testcases_run);
	printf("* Passed tests: %d\n", total_testcases_run-total_testcases_failed);
	printf("* Failed tests: %d\n", total_testcases_failed);
	printf("*\n");
	printf("***************************************\n");

	// Destroy the linked lists of testcases/preparations/cleanups
	free_linked_lists();

	// Close logfile
	close_logfile();

	// Run cleanup
	run_cleanup();

	return 0;
}