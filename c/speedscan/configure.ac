#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(SpeedScan , 0.5.3, 0lvin@ukr.net)
AC_CONFIG_SRCDIR([src/speedscan.c])
#gen makefiles
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([clamav], [cl_scandesc])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h limits.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_C_VOLATILE

# Checks for library functions.
AC_FUNC_MEMCMP
AC_CHECK_FUNCS([getcwd gettimeofday memset strerror])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT

