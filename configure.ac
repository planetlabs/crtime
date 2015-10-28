AC_INIT([crtime], [0.02], [brian@planet.com])
AM_INIT_AUTOMAKE
AC_PROG_CC
AC_CHECK_HEADER(ext2fs/ext2fs.h, , [AC_MSG_ERROR([Couldn't find ext2fs.h])])
AC_CHECK_LIB(ext2fs, ext2fs_open)
AC_CHECK_LIB(ext2fs, ext2fs_read_inode_full)
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
