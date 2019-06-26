#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([fstyp], [0.1], [mkp@mkp.net])
AC_CONFIG_SRCDIR([src/linux_fs.h])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])
AC_STDC_HEADERS

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_INT32_T
AC_ISC_POSIX
AC_ARG_PROGRAM

# Checks for library functions.
AC_CHECK_FUNCS([getpagesize])
AC_CHECK_FUNCS(mkdir)

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
