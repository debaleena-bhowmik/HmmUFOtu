dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([HmmUFOtu], [1.3.2])
LT_INIT([shared static])

AC_CANONICAL_SYSTEM
AC_CANONICAL_HOST

AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_INT8_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_INT32_T
AC_PROG_LIBTOOL
AC_PROG_RANLIB
#LT_INIT
AC_CHECK_HEADERS([stdint.h])
AC_CHECK_HEADER_STDBOOL

AM_INIT_AUTOMAKE()

#AC_LANG(C++)
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AC_CONFIG_MACRO_DIR([m4])

AC_CHECK_FUNCS([sqrt isnan floor])
AC_CHECK_FUNCS([strerror])

#AC_FUNC_MALLOC
# auxinary features
AX_CHECK_ZLIB()
AX_BOOST_BASE([1.50.0])
AX_BOOST_IOSTREAMS()

# conditional packages
AC_ARG_WITH([libjsoncpp],
            [AS_HELP_STRING([--with-libjsoncpp],
                            [support generating .jplace and other JSON files @<:@default=check@:>@]
                            )],
            [],
  	        [with_libjsoncpp=check]
  	        )
  	        
AS_CASE(["$with_libjsoncpp"],
        [yes],
        [AC_CHECK_LIB([jsoncpp], [main], [], [])], # yes check
        [no], [], # no, not check
        [AC_CHECK_LIB([jsoncpp], [main], [], [])] # default check
       )
AM_CONDITIONAL([HAVE_LIBJSONCPP], [test "$with_libjsoncpp" != no -a "$ac_cv_lib_jsoncpp_main" = yes])

AC_CONFIG_FILES([Makefile
				 data/Makefile
				 src/Makefile
				 test/Makefile])

AC_CONFIG_SUBDIRS([
                   src/util
                   src/math
                   src/libcds
                   src/libdivsufsort])
AC_OPENMP

AC_OUTPUT
