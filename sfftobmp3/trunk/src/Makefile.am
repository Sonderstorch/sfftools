## Makefile.am -- Process this file with automake to produce Makefile.in

AM_CPPFLAGS         =  -I$(top_builddir) -I$(top_srcdir)
bin_PROGRAMS = sfftobmp
sfftobmp_SOURCES = my_getopt.c cmdline.cpp codes.cpp common.cpp decoder.cpp input.cpp main.cpp output.cpp
sfftobmp_LDADD = @TIFF_LIB@ @JPEG_LIB@ -lboost_filesystem -lboost_system
