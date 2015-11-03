/*
 * Derived from debugfs.c in e2fsprogs and inspired by:
 * http://unix.stackexchange.com/questions/50177/birth-is-empty-on-ext4/50184
 *
 * Copyright (C) 1993 Theodore Ts'o.  This file may be redistributed
 * under the terms of the GNU Public License.
 *
 * Modifications by Robert Sanders <gt8134b@prism.gatech.edu>
 *
 * To build:
 * apt-get install e2fslibs-dev # or similar
 * gcc -o crtime crtime.c -lext2fs
 *
 * To let non-root users get creation times of files:
 * chown root ./crtime
 * chmod u+s ./crtime
 *
 * To run:
 * target=/path/to/some/file
 * fs=$(df "${target}"  | tail -1 | awk '{print $1}');
 * ./crtime ${fs} ${target}
 */
#include <ext2fs/ext2fs.h>
#include <stdio.h>
#include <sys/stat.h>

ext2_filsys current_fs;

static int open_filesystem(char *device)
{
    int retval;
    io_manager io_ptr = unix_io_manager;

    retval = ext2fs_open(device, 0, 0, 0, io_ptr, &current_fs);
    if (retval) {
        fprintf(stderr, "Error while opening filesystem: %d\n", retval);
        return retval;
    }
    return 0;
}

static int get_inode(char *f, ext2_ino_t *inode) {

    struct stat sb;

    if (stat(f, &sb) == -1) {
        perror("stat");
        return errno;
    }

    if ((sizeof(sb.st_ino) > sizeof(ext2_ino_t)) &&
        (sb.st_ino > 0xffffffff)) {
        fprintf(stderr,
                "Inode of %s is bigger than what e2fslibs supports\n",
                f);
        return -1;
    }
    memcpy((void *)inode, (void *)&sb.st_ino, sizeof(ext2_ino_t));
    return 0;
}

/* http://unix.stackexchange.com/questions/32795/what-is-the-maximum-allowed-filename-and-folder-size-with-ecryptfs */
#define MAX_PATH 4096
#define DF_COMMAND "/bin/df --output=source "

/* NB: caller must call free on **fs on success */
int get_fs_name(char *f, char **fs) {

    FILE *fp = NULL;
    char cmd[MAX_PATH + sizeof(DF_COMMAND) + 1];
    int retval;
    int l;

    *fs = NULL;

    retval = snprintf(cmd, sizeof(cmd), DF_COMMAND"%s", f);
    if (retval >= sizeof(cmd) - 1) {
        fprintf(stderr, "%s too long!", f);
        goto fail;
    }

    fp = popen(cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to run df command\n");
        goto fail;
    }

    /* leave space for newline and space for null termination */
    *fs = malloc(MAX_PATH + 2);
    if (*fs == NULL) {
        fprintf(stderr, "Failed to allocate memory for df output\n");
        goto fail;
    }

    /* throw away the header */
    if (fgets(*fs, MAX_PATH, fp) == NULL) {
        fprintf(stderr, "Failed to read df header\n");
        goto fail;
    }

    if (fgets(*fs, MAX_PATH, fp) == NULL) {
        fprintf(stderr, "failed to read data from df\n");
        goto fail;
    }

    l = strlen(*fs);
    if ((*fs)[l - 1] != '\n') {
        fprintf(stderr, "Did not find newline in df output.\n");
        goto fail;
    }
    (*fs)[l - 1] = 0;

    pclose(fp);
    return 0;

fail:
    if (*fs)
        free(*fs);
    if (fp)
        pclose(fp);
    return -1;
}

int main(int argc, char *argv[]) {

    ext2_ino_t inode;
    struct ext2_inode *inode_buf;
    struct ext2_inode_large *large_inode;
    int retval;
    char *fs;

    if (argc != 2) {
        fprintf(stderr, "Usage: crtime <file>\n");
        return -1;
    }

    retval = get_inode(argv[1], &inode);
    if (retval) {
        return retval;
    }

    retval = get_fs_name(argv[1], &fs);
    if (retval) {
        return retval;
    }

    retval = open_filesystem(fs);
    if (retval) {
        free(fs);
        return retval;
    }

    inode_buf = (struct ext2_inode *)malloc(EXT2_INODE_SIZE(current_fs->super));
    if (!inode_buf) {
        free(fs);
        fprintf(stderr, "Failed to allocate memory\n");
        return -1;
    }

    retval = ext2fs_read_inode_full(current_fs, inode, inode_buf, EXT2_INODE_SIZE(current_fs->super));
    if (retval) {
        fprintf(stderr, "Failed to read inode\n");
        free(fs);
        free(inode_buf);
        return retval;
    }

    if (EXT2_INODE_SIZE(current_fs->super) <= EXT2_GOOD_OLD_INODE_SIZE) {
        free(fs);
        free(inode_buf);
        fprintf(stderr, "Create time unavailable");
        return -1;
    }

    large_inode = (struct ext2_inode_large *)inode_buf;
    if (large_inode->i_extra_isize < 24) {
        free(fs);
        free(inode_buf);
        fprintf(stderr, "Create time unavailable");
        return -1;
    }

    printf("%d\n", large_inode->i_crtime);

    free(fs);
    free(inode_buf);
    ext2fs_close(current_fs);
    return 0;
}
