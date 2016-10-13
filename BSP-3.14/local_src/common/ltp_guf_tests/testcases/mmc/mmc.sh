#!/bin/sh
#******************************************************************************#
# Copyright (C) 2011 Garz&Fricke GmbH						 			       #
#		No use or disclosure of this information in any form without	       #
#		the written permission of the author							       #
#******************************************************************************#

#******************************************************************************#
#                                                                              #
#  File:        mmc.sh                                                         #
#                                                                              #
#  Description: This is a basic test for filesystem operations on MMC cards.   #
#                                                                              #
#  Total Tests: 1                                                              #
#                                                                              #
#  Test Name:   mmc                                                            #
#                                                                              #
#  Test Assertion                                                              #
#  & Strategy:  This script tests the basic functionality of an MMC card. It   #
#               first deletes all partitions from the card and then creates    #
#               a single new one. This partition is formatted with FAT32 and   #
#               then mounted into the filesystem. A textfile is created and    #
#               read back. If all the operations succeed the test is passed,   #
#               else it fails.                                                 #
#                                                                              #
#  Author:      Tim Jaacks <tim.jaacks@garz-fricke.com>                        #
#                                                                              #
#******************************************************************************#
export TCID="mmc            "	# Identifier of this testcase.

devnode="/dev/mmcblk0"
mountdir="/mnt/mmc"
test_string="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
fdisk_input=""
fstype="vfat"
mkfs="mkfs"
skip_format=0
quiet=0

#******************************************************************************#
# Function:     print_usage                                                    #
#******************************************************************************#
print_usage()
{
	echo "  -h      Show this help screen"
	echo "  -d dev  Device node for MMC cards (default: $devnode)"
	echo "  -t typ  Filesystem type (ext2, ext3, ext4, vfat, default: $fstype)"
	echo "  -f		Do not format disk, just mount it"
	echo "  -q      Suppress info messages"
}

#******************************************************************************#
# Function:     parse_options                                                  #
#                                                                              #
# Description:  - Parse the command line options                               #
#                                                                              #
# Return        - zero                                                         #
#******************************************************************************#
parse_options()
{
	while getopts hd:t:fq OPT; do
		case "$OPT" in
			h)
				print_usage
				exit 0
				;;
			d)
				devnode=$OPTARG
				;;
			t)
				fstype=$OPTARG
				;;
			f)
				skip_format=1
				;;
			q)
				quiet=1
				;;
			\?)
				# getopts issues an error message
				print_usage >&2
				exit 2
				;;
		esac
	done
	return 0
}

#******************************************************************************#
# Function:     setup                                                          #
#                                                                              #
# Description:  - Check if required commands exits                             #
#               - Export global variables                                      #
#               - Check if required config files exits                         #
#               - Create temporary files and directories                       #
#                                                                              #
# Return        - zero on success                                              #
#               - non zero on failure. return value from commands ($RC)        #
#******************************************************************************#
setup()
{
	export TST_TOTAL=1  # Total number of test cases in this file.

	# Set up LTPTMP (temporary directory used by the tests).
	LTPTMP=${TMP}                 # Temporary directory to create files, etc.
	export TST_COUNT=0            # Set up is initialized as test 0

	# Initialize cleanup function to execute on program exit.
	# This function will be called before the test program exits.
	trap "cleanup" 0

	# Initialize return code to zero.
	RC=0                # Exit values of system commands used

	# Check if an MMC is plugged in
	if [ ! -e $devnode ]
	then
		tst_brkm TBROK NULL "No MMC card plugged in. Cannot execute any tests."
		RC=2
		return $RC
	fi

	# Check if any partitions are mounted, if yes then unmount
	for partition in $(find "$devnode"p* 2>/dev/null)
	do
		if [ $(mount | grep $partition | wc -l) -ne 0 ]
		then
			umount "$partition"
		fi
	done

	# Create mount directory, if it does not exist
	mkdir -p $mountdir

	# Check if mkfs exists for the specified filesystem type
	if [ $skip_format -eq 0 ]
	then
		$mkfs.$fstype >/dev/null 2>/dev/null
		if [ $? -eq 127 ]
		then
			# If mkfs is not a built-in executable, check whether busysbox provides it
			mkfs="busybox mkfs"
			$mkfs.$fstype >/dev/null 2>/dev/null
			if [ $? -eq 127 ]
			then
				tst_brkm TBROK NULL "Creating a $fstype filesystem is not possible on this machine."
				RC=2
				return $RC
			fi
		fi
	fi

	return 0
}


#******************************************************************************#
# Function:     cleanup                                                        #
#                                                                              #
# Description   - remove temporary files and directories.                      #
#                                                                              #
# Return        - zero on success                                              #
#               - non zero on failure. return value from commands ($RC)        #
#******************************************************************************#
cleanup()
{
	return 0
}


#******************************************************************************#
# Function:     mmc                                                            #
#                                                                              #
# Description   - Test if partitions can be deleted, created and formatted,    #
#                 and files be written to and read from.                       #
#******************************************************************************#
mmc()
{
	TST_COUNT=1                    # Test case number.
	RC=0                           # return code from commands.

	# Print test assertion.
	if [ $quiet -eq 0 ]
	then
		tst_resm TINFO "Starting MMC test."
	fi

	if [ $skip_format -eq 0 ]
	then
		if [ $quiet -eq 0 ]
		then
			tst_resm TINFO "Deleting all partitions from MMC."
			tst_resm TINFO "Creating new partition."
		fi

		# Concatenate input string for fdisk
		for partition in $(find "$devnode"p* 2>/dev/null)
		do
			fdisk_input=$fdisk_input"d\n${partition#$devnode"p"}\n"
		done
		fdisk_input=$fdisk_input"n\np\n1\n\n\nw\n"

		# Call fdisk to remove all partitions and create a new one
		echo -e "$fdisk_input" | fdisk $devnode >/dev/null 2>/dev/null
		RC=$?
		if [ $RC -ne 0 ]
		then
			tst_res TFAIL $LTPTMP/tst_file.out \
			"fdisk returned an error."
			return $RC
		fi

		# Wait a short moment for the OS to recognize the new partition
		usleep 100000

		# If auto-mounting is activated, maybe the card has been mounted
		# after creating the partition. If so, unmount it.
		if [ $(mount | grep "$devnode"p1 | wc -l) -ne 0 ]
		then
			umount "$devnode"p1
		fi

		# Format new partition using specified filesystem
		if [ $quiet -eq 0 ]
		then
			tst_resm TINFO "Creating $fstype filesystem on new partition."
		fi
		$mkfs.$fstype $devnode"p1" >/dev/null 2>/dev/null
		RC=$?
		if [ $RC -ne 0 ]
		then
			tst_res TFAIL $LTPTMP/tst_file.out \
			"Could not create $fstype filesystem on MMC partition."
			return $RC
		fi
	fi

	# Mount partition to mount directory
	if [ $quiet -eq 0 ]
	then
		tst_resm TINFO "Mounting partition to $mountdir."
	fi
	mount $devnode"p1" $mountdir
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_res TFAIL $LTPTMP/tst_file.out \
		"Could not mount partition to $mountdir."
		return $RC
	fi

	# Create textfile, unmount and mount the MMC, then read file back
	if [ $quiet -eq 0 ]
	then
		tst_resm TINFO "Creating testfile."
	fi
	echo $test_string > $mountdir/mmc_test.txt
	umount $mountdir
	mount $devnode"p1" $mountdir
	readstring=$(cat $mountdir/mmc_test.txt 2>/dev/null)

	# Print test results and exit with appropriate value
	if [ ! -n "$readstring" ]
	then
		tst_res TFAIL $LTPTMP/tst_file.out \
		"Read/write test failed."
		RC=1
		return $RC
	fi
	if [ $readstring != $test_string ]
	then
		tst_res TFAIL $LTPTMP/tst_file.out \
		"Read/write test failed."
		RC=1
		return $RC
	else
		tst_resm TPASS \
		"Write and read to/from file was successful."
	fi
	return 0
}


#******************************************************************************#
# Function:     main                                                           #
#                                                                              #
# Description:  - Execute all tests, exit with test status.                    #
#                                                                              #
# Exit:         - zero on success                                              #
#               - non-zero on failure.                                         #
#******************************************************************************#
RC=0    # Return value from setup, and test functions.

parse_options $@

setup || exit $RC

mmc || exit $RC