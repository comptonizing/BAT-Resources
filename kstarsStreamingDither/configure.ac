AC_INIT([kstarsStreamingDither], [0.1], [philipp@whyisitso.de])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_MACRO_DIRS([m4])

AC_PROG_CC
AC_PROG_CXX

AM_PROG_AR

LT_PREREQ([2.4.6])
LT_INIT
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

CXXFLAGS="$CXXFLAGS -g -pedantic -W -Wall"
LDFLAGS="$LDFLAGS -g -W -Wall"

# Enable debugging
AC_ARG_ENABLE([debugging], AC_HELP_STRING([--enable-debugging], [Enable debugging statements]), [CXXFLAGS="-D ISDEBUG $CXXFLAGS"], [])

AC_ARG_WITH(gtk3,
	    [AC_HELP_STRING([--with-gtk3=<gtk3 path>], [Alternative path to the gtk3 library])],
	    gtk3_dir=$withval,
	    gtk3_dir=notgiven)

AS_IF([ [[ "$gtk3_dir" == "yes" ]] ],
      [AC_MSG_ERROR([you must write something behind --with-gtk3 ...])]
      )
AS_IF([ [[ "$gtk3_dir" != "notgiven" ]] ],
      [AC_MSG_NOTICE([Adding location $gtk3_dir to search path])]
      CXXFLAGS="$CXXFLAGS -I$gtk3_dir/include"
      LDFLAGS="$LDFLAGS -L$gtk3_dir/lib -Wl,--rpath=$gtk3_dir/lib"
      export PKG_CONFIG_PATH=$gtk3_dir/lib/pkgconfig
      )

# LIBRARIES
AC_HEADER_STDC
AC_CHECK_HEADERS([math.h string.h stdlib.h])

AC_SEARCH_LIBS(sin, m, [], [AC_MSG_ERROR([No math library found!])], [])
AC_SEARCH_LIBS(gtk_application_new, gtk-3, [], [AC_MSG_ERROR([gtk-3 library not found!])])
AC_SEARCH_LIBS([_ZN3Gtk11Application3runEv], [gtkmm-3.0], [], [AC_MSG_ERROR([gtkmm-3.0 not found!])], [])

AM_PATH_GTK_3_0([3.0.0],,AC_MSG_ERROR([Gtk+ 3.0.0 or higher required.]))
PP=$(pkg-config --cflags-only-I --cflags-only-other gtk+-3.0)
CXXFLAGS="${CXXFLAGS} ${PP}"

AC_SUBST([CXXFLAGS], [$CXXFLAGS])
AC_SUBST([LDFLAGS], [$LDFLAGS])

AC_CONFIG_FILES([Makefile
		 src/Makefile])

AC_OUTPUT
