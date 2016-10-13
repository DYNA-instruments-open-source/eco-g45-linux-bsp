#!/bin/sh
#******************************************************************************#
# Copyright (C) 2011 Garz&Fricke GmbH						 			       #
#		No use or disclosure of this information in any form without	       #
#		the written permission of the author							       #
#******************************************************************************#

#******************************************************************************#
#                                                                              #
#  File:        mstick.sh                                                      #
#                                                                              #
#  Description: This is a basic test for USB memory sticks.                    #
#                                                                              #
#  Total Tests: 1                                                              #
#                                                                              #
#  Test Name:   mstick                                                         #
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
export TCID="mstick         "	# Identifier of this testcase.

devnode="/dev/sd"
mountdir="/mnt/mstick"
test_string="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
number_of_sticks=1
quiet=0

#******************************************************************************#
# Function:     print_usage                                                    #
#******************************************************************************#
print_usage()
{
	echo "  -h      Show this help screen"
	echo "  -d dev  Device node for memory sticks (default: $devnode)"
	echo "  -n x    Number of memory sticks present in the system (default: 1)"
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
	while getopts hd:n:q OPT; do
		case "$OPT" in
			h)
				print_usage
				exit 0
				;;
			d)
				devnode=$OPTARG
				;;
			n)
				number_of_sticks=$OPTARG
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
	export TST_TOTAL=$number_of_sticks  # Total number of test cases in this file.

	# Set up LTPTMP (temporary directory used by the tests).
	LTPTMP=${TMP}       # Temporary directory to create files, etc.
	export TST_COUNT=0  # Set up is initialized as test 0

	# Initialize cleanup function to execute on program exit.
	# This function will be called before the test program exits.
	trap "cleanup" 0

	# Initialize return code to zero.
	RC=0                # Exit values of system commands used

	# Check if any partitions are mounted, if yes then unmount
	for partition in $(find "$devnode"?? 2>/dev/null)
	do
		if [ $(mount | grep $partition | wc -l) -ne 0 ]
		then
			umount $partition
		fi
	done

	# Create mount directories, if they do not exist
	count=1
	for stick in $(find "$devnode"? 2>/dev/null)
	do
		mkdir -p $mountdir$count
		count=$(($count+1))
	done

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
# Function:     mstick                                                         #
#                                                                              #
# Description   - Test if partitions can be deleted, created and formatted,    #
#                 and files be written to and read from.                       #
#******************************************************************************#
mstick()
{
	TST_COUNT=1                    # Test case number.
	RC=0                           # return code from commands.

	# Print test assertion.
	if [ $quiet -eq 0 ]
	then
		tst_resm TINFO "Starting USB stick test"
		tst_resm TINFO "Check if sticks are plugged in"
	fi

	# Check if sticks are present in the device table
	if [ $(find "$devnode"? 2>/dev/null | wc -l) -ne $number_of_sticks ]
	then
		tst_res TFAIL $LTPTMP/tst_file.out \
		"Number of memory sticks present in the device table was not as expected ($number_of_sticks)."
		RC=2
		return $RC
	fi

	if [ $quiet -eq 0 ]
	then
		tst_resm TINFO "As expected, "$number_of_sticks" stick(s) are present in the device table."
	fi

	# Repeat the following steps for all sticks
	for thisdevnode in $(find "$devnode"? 2>/dev/null)
	do
		# Check if there is a first partition present on the stick.
		# If not, create one.
		if [ ! -e $thisdevnode"1" ]
		then
			tst_resm TINFO "No partition present on $thisdevnode, creating one now"
			echo -e "n\np\n1\n\n\nw\n" | fdisk $thisdevnode >/dev/null 2>/dev/null
			RC=$?
			if [ $RC -ne 0 ]
			then
				tst_res TFAIL $LTPTMP/tst_file.out \
				"fdisk returned an error."
				return $RC
			fi
			# Wait a short moment for the OS to recognize the new partition
			usleep 100000

			# If auto-mounting is activated, maybe the stick has been mounted
			# after creating the partition. If so, unmount it.
			if [ $(mount | grep "$thisdevnode"1 | wc -l) -ne 0 ]
			then
				umount "$thisdevnode"1
			fi
		fi

		# Mount first partition to mount directory
		if [ $quiet -eq 0 ]
		then
			tst_resm TINFO "Mounting $thisdevnode"1" at $mountdir$TST_COUNT"
		fi
		mount $thisdevnode"1" $mountdir$TST_COUNT > /dev/null 2>/dev/null
		RC=$?
		if [ $RC -ne 0 ]
		then
			# Mounting failed. This might be the case because the
			# partition is not formatted yet. We try to format it.
			tst_resm TINFO "Mounting failed. Creating FAT32 filesystem on partition $thisdevnode"1"."
			/sbin/mkfs.vfat $thisdevnode"1" >/dev/null 2>/dev/null
			RC=$?
			if [ $RC -ne 0 ]
			then
				tst_res TFAIL $LTPTMP/tst_file.out \
				"Could not create FAT32 filesystem on partition."
				return $RC
			fi
			# Now, try mounting again
			mount $thisdevnode"1" $mountdir$TST_COUNT > /dev/null 2>/dev/null
			RC=$?
			if [ $RC -ne 0 ]
			then
				tst_res TFAIL $LTPTMP/tst_file.out \
				"Could not mount partition to $mountdir."
				return $RC
			fi
			tst_resm TINFO "Successfully mounted."
		fi

		# Create textfile, unmount and mount the stick, then read file back
		if [ $quiet -eq 0 ]
		then
			tst_resm TINFO "Creating testfile"
		fi
		echo $test_string > $mountdir$TST_COUNT/mstick_test.txt
		umount $mountdir$TST_COUNT
		mount $thisdevnode"1" $mountdir$TST_COUNT
		readstring=$(cat $mountdir$TST_COUNT/mstick_test.txt 2>/dev/null)

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
			"Write and read to/from file was successful on $thisdevnode."
		fi

		# Increase test number
		TST_COUNT=$(($TST_COUNT+1))
	done

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

mstick || exit $RC