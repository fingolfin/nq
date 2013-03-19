dnl ##
dnl ## Process this file with autoconf to produce a configure script.
dnl ##
AC_PREREQ([2.68])
AC_INIT([ANU Nilpotent Quotient Program], [2.4dev], [support@gap-system.org], [nq])
AC_CONFIG_AUX_DIR(cnf)
AC_CONFIG_SRCDIR([src/nq.c])
AC_CONFIG_HEADER(src/config.h:src/config.hin)

dnl ##
dnl ## determine the system name
dnl ##
AC_CANONICAL_HOST

dnl ##
dnl ## C is the language
dnl ##
AC_LANG([C])

dnl ##
dnl ## Checks for programs.
dnl ##
AC_PROG_AWK
AC_PROG_CC
AC_PROG_MAKE_SET

dnl ##
dnl ## Checks for system header files.
dnl ##
AC_CHECK_HEADERS([stdlib.h string.h sys/time.h])


dnl ##
dnl ## Locate the GAP root dir
dnl ##
dnl ## If no directory is specified, then this defaults to ../..
# TODO: Automatically expand relative paths to absolute ones... this would simplify the code below
# considerably. Also, it's a first step towards building out-of-tree
AC_ARG_WITH([gap-root],
	[AS_HELP_STRING([--with-gap-root=PATH],
		[directory where GAP is installed @<:@default=../..@:>@])],
	[GAP_ROOT="$with_gap_root" ; GAP_ROOT_real="$with_gap_root"; GAP_ROOT_display="$with_gap_root"],
	[GAP_ROOT='${top_srcdir}/../..' ; GAP_ROOT_real="${srcdir}/../.."; GAP_ROOT_display=../..])
AC_MSG_CHECKING([path to GAP installation])

# Try and turn GAP_ROOT_real into an absolute path
GAP_ROOT_real=`cd $GAP_ROOT_real && pwd`

if test -d $GAP_ROOT_real ; then
	AC_MSG_RESULT([$GAP_ROOT_display])
else
	AC_MSG_ERROR([$GAP_ROOT_display is not a directory])
fi
AC_SUBST(GAP_ROOT)


dnl ##
dnl ## Check whether to use 32bit or 64bit ABI
dnl ##
AC_CHECK_SIZEOF(void *, 4)

AC_ARG_VAR(ABI, [Set this equal to 32 or 64 to build nq
		 in 32- or 64-bit mode. The default value
		 for this option is determined by testing the behaviour of your
		 compiler, so should be 32 on a 32-bit system and 64 on one
		 which is 64-bit.])

AC_MSG_CHECKING([ABI bit size])
if test "x$ABI" = "x" ;  then
  if test $ac_cv_sizeof_void_p = 8; then
    ABI="64"
  else
    ABI="32"
  fi
  ABI_CFLAGS=""
elif test "$ABI" = "64"; then
  if test $ac_cv_sizeof_void_p = 4; then
    AC_MSG_ERROR([ ABI=64 cannot be used on a 32-bit system.
                   Please replace this value with 32 and retry configure.
                   If you think this error is wrong, email
                   support@gap-system.org with full details.])
  else
    ABI_CFLAGS="-m64"
  fi
elif test "$ABI" = "32"; then
  ABI_CFLAGS="-m32"
else
  AC_MSG_ERROR([ $ABI is not a supported value for ABI. Please use ABI=64
                 or 32, or leave it unset.])
fi

AC_MSG_RESULT([$ABI])

AC_SUBST(ABI)
AC_SUBST(ABI_CFLAGS)


dnl ##
dnl ## Check which GAP configuration to use
dnl ##
AC_ARG_VAR(CONFIGNAME,[Supply a (meaningful) name for the configuration you are building.
    This name will be appended to the architecture-dependent named
    sub-directory of bin/. Allows for multiple configurations side by
    side. Default value is "defaultXX" where "XX" is 32 or 64.])

AC_MSG_CHECKING([GAP config name])
if test "x$CONFIGNAME" = "x"; then
  CONFIGNAME="default${ABI}"
fi
SYSINFO_GAP="$GAP_ROOT_real/sysinfo.gap-$CONFIGNAME"
if ! test -f $SYSINFO_GAP ; then
	# Fall back to sysinfo.gap, to support GAP 4.4
	SYSINFO_GAP="$GAP_ROOT_real/sysinfo.gap"
fi
if test -f $SYSINFO_GAP ; then
	AC_MSG_RESULT([$CONFIGNAME])
	#AC_SUBST(CONFIGNAME)
else
	AC_MSG_ERROR([sysinfo.gap-$GAP_CONFIG not found])
fi


dnl ##
dnl ## Parse the GAParch from sysinfo.gap
dnl ##

AC_MSG_CHECKING([for active GAP architecture])
GAPARCH=`fgrep "GAParch=" $SYSINFO_GAP | sed -e 's/^GAParch=\(.*\)$/\1/'`
if test "x$GAPARCH" = "x" ; then
	AC_MSG_ERROR([GAParch not found in $GAP_ROOT_display/sysinfo.gap])
else
	AC_MSG_RESULT([$GAPARCH])
fi
AC_SUBST(GAPARCH)

dnl ##
dnl ## Check for GMP
dnl ## If the user specified a specific GMP, try to use that.
dnl ## Otherwise, first try to use the GMP provided by GAP, or
dnl ## any GMP version installed globally by the user.
dnl ##

GMP_CPPFLAGS=
GMP_LIBS=

AC_ARG_WITH([gmp],
	[AS_HELP_STRING([--with-gmp=PATH],
	  [ Use GMP library.
        If the argument you supply is "yes" or <empty>, then the version of GMP bundled with GAP will be used (default).
        If the argument is "system" that means the library is reachable with the standard
        search path "/usr" or "/usr/local".
        Otherwise you give the <path> to the directory which contains the library.
        [[default=yes]]
    ])],
  [], [ with_gmp=yes ]
)

if test "x$with_gmp" = "xno" ; then
	AC_MSG_ERROR([GMP is required, cannot build without it])
elif test "x$with_gmp" = "xsystem" ; then
	# Try using system GMP
	GMP_CPPFLAGS=""
	GMP_LIBS="-lgmp"
else
	# Try using static linked GMP in the specified location
	if test "x$with_gmp" = "xyes" ; then
		# Try to use GAP's GMP, if available
		GMP_HOME="$GAP_ROOT_real/bin/$GAPARCH/extern/gmp"
	else
		GMP_HOME="$with_gmp"
	fi;
	if test -d ${GMP_HOME}/include && test -f ${GMP_HOME}/lib/libgmp.a ; then
		GMP_CPPFLAGS="-I${GMP_HOME}/include"
		GMP_LIBS="${GMP_HOME}/lib/libgmp.a"
	elif test "x$with_gmp" = "xyes" ; then
		# fallback to trying system wide GMP
		GMP_CPPFLAGS=""
		GMP_LIBS="-lgmp"
	else
		AC_MSG_ERROR([Could not locate libgmp.a in the specified location])
	fi
fi;

nq_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $GMP_CPPFLAGS"
nq_save_LIBS="$LIBS"
LIBS="$LIBS $GMP_LIBS"

AC_CHECK_HEADER( [gmp.h],
	[
	# TODO: Disable linker check for now: It causes problems on Linux, because
	# libgmp.a is in the linker command line before the test C file. On the long
	# run, this should be re-enabled, though perhaps in a different form.
	AC_MSG_CHECKING([whether linking against GMP works])
	AC_TRY_LINK(
		[#include <gmp.h>],
		[__gmpz_init(0);] ,
		[AC_MSG_RESULT([yes])],
		[AC_MSG_ERROR([no])]
		)
	],
	[ AC_MSG_ERROR([Could not locate the GMP include file gmp.h.]) ]
	)

# restore FLAGS
CPPFLAGS="$nq_save_CPPFLAGS"
LIBS="$nq_save_LIBS"

AC_SUBST(GMP_CPPFLAGS)
AC_SUBST(GMP_LIBS)


dnl ##
dnl ## Checks for typedefs, structures, and compiler characteristics.
dnl ##
AC_TYPE_LONG_LONG_INT

dnl ##
dnl ## Checks for library functions.
dnl ##
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([sbrk getrusage])

dnl ##
dnl ## Finally, generate the Makefiles and output everything
dnl ##
AC_CONFIG_FILES([Makefile doc/Makefile examples/Makefile src/Makefile])
AC_OUTPUT