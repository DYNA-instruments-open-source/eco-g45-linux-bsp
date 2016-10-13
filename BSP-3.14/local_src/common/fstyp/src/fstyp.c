/*
 * fstyp.c
 *
 * Copyright (C) 2000, 2001 Martin K. Petersen <mkp@mkp.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details. 
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "mount_guess_fstype.h"

int main (int argc, char **argv) 
{
	char *type = NULL;

	if (argc != 2) {
		fprintf(stdout, "Usage: fstyp <device>\n");
		exit (EXIT_FAILURE);
	}
	type = fstype (argv[1]);

	if (type) {
		fprintf (stdout, "%s\n", type);
		exit (EXIT_SUCCESS);
	}
	
	fprintf (stdout, "unknown\n");
	exit (EXIT_FAILURE);
}

