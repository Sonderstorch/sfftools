#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(sfftobmp, 2.3, peter.schaefer@gmx.de)
AC_CONFIG_AUX_DIR(config)

AM_INIT_AUTOMAKE(sff, 2.3,peter.schaefer@gmx.de)

AC_CONFIG_SRCDIR([src/main.cpp])
AM_CONFIG_HEADER([src/config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

AC_CHECK_HEADER(tiffio.h,AC_DEFINE( HAVE_LIBTIFF,1, LibTIFF),AC_MSG_ERROR(tiffio.h not found))
AC_CHECK_LIB(tiff,TIFFOpen,[ TIFF_LIB="-ltiff" ],AC_MSG_ERROR(You need libtiff))
AC_SUBST(TIFF_LIB)

AC_CHECK_HEADER(jpeglib.h,AC_DEFINE(HAVE_LIBJPEG,1, LibJPEG),AC_MSG_ERROR(jpeglib.h not found))
AC_CHECK_LIB(jpeg, jpeg_destroy_decompress,[ JPEG_LIB="-ljpeg" ],AC_MSG_ERROR(You need libjpeg))
AC_SUBST(JPEG_LIB)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h utime.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_UTIME_NULL
AC_CHECK_FUNCS([memset strdup strrchr utime])
AC_OUTPUT(Makefile src/Makefile)

