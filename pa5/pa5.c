/*
Nicholas Clement

  gcc -Wall `pkg-config fuse --cflags` fusexmp.c -o fusexmp `pkg-config fuse --libs`

  Note: This implementation is largely stateless and does not maintain
        open file handels between open and release calls (fi->fh).
        Instead, files are opened and closed as necessary inside read(), write(),
        etc calls. As such, the functions that rely on maintaining file handles are
        not implmented (fgetattr(), etc). Those seeking a more efficient and
        more complete implementation may wish to add fi->fh support to minimize
        open() and close() calls and support fh dependent functions.
*/
//fusermount -u example to mount or unmount
//fusermount -u example

#define FUSE_USE_VERSION 28
#define HAVE_SETXATTR
#define ENCRYPT 1
#define DECRYPT 0
#define PASS -1
#define ENC_XATTR "user.pa5-encfs.encrypted"


/* Add namespace defintion for older kernels. Normally included in linux/xattr.h */
#ifndef XATTR_USER_PREFIX
#define XATTR_USER_PREFIX "user."
#define XATTR_USER_PREFIX_LEN (sizeof (XATTR_USER_PREFIX) - 1)
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <fuse.h>
#include "aes-crypt.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif


typedef struct {
    char *rootdir;
    char *key;
} inputParam;

static void newpath(char fpath[PATH_MAX], const char *path)
{

    inputParam *state = (inputParam *) (fuse_get_context()->private_data);
    strcpy(fpath, state->rootdir);
    strncat(fpath, path, PATH_MAX);
}


static int xmp_getattr(const char *path, struct stat *stbuf)
{
	//update path
	char fullpath[PATH_MAX];
  newpath(fullpath, path);
	int res;

	res = lstat(fullpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_access(const char *path, int mask)
{
	//update path
	char fullpath[PATH_MAX];
  newpath(fullpath, path);
	int res;

	res = access(fullpath, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	//update path
	char fullpath[PATH_MAX];
  newpath(fullpath, path);
	int res;

	res = readlink(fullpath, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}


static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	//update path
char fullpath[PATH_MAX];
newpath(fullpath, path);
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fullpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
 //update path
char fullpath[PATH_MAX];
newpath(fullpath, path);
	int res;

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(fullpath, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(fullpath, mode);
	else
		res = mknod(fullpath, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	//update path
	char fullpath[PATH_MAX];
	newpath(fullpath, path);
	int res;

	res = mkdir(fullpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_unlink(const char *path)
{
	//update path
	char fullpath[PATH_MAX];
  newpath(fullpath, path);
	int res;

	res = unlink(fullpath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
	//update path
	char fullpath[PATH_MAX];
  newpath(fullpath, path);
	int res;

	res = rmdir(fullpath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
	int res;

	res = symlink(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
	int res;

	res = rename(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_link(const char *from, const char *to)
{

	int res;

	res = link(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	//update path
	char fullpath[PATH_MAX];
  newpath(fullpath, path);
	int res;

	res = chmod(fullpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
	//update path
	char fullpath[PATH_MAX];
  newpath(fullpath, path);
	int res;

	res = lchown(fullpath, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	//update path
	char fullpath[PATH_MAX];
  newpath(fullpath, path);
	int res;

	res = truncate(fullpath, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	//update path
	char fullpath[PATH_MAX];
  newpath(fullpath, path);
	int res;
	struct timeval tv[2];

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(fullpath, tv);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	//update path
	char fullpath[PATH_MAX];
  newpath(fullpath, path);
	int res;

	res = open(fullpath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}
/* check file attributes for encryption, if encrypted
then decrypt and then read, if not encrypted then just read */
static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{

	int fd;
	int res;
    ssize_t valsize;
    char fullpath[PATH_MAX];
    FILE *fp, *tmp;

    newpath(fullpath, path);

    (void) fi;

    valsize = getxattr(fullpath, ENC_XATTR, NULL, 0);
    if (valsize >= 0) {
      	//create temp file for decryption
				//tempfile() auto removes itself after the funciton
				tmp = tmpfile();
        fp = fopen(fullpath, "rb");
				//decrypt for reading
        do_crypt(fp, tmp, DECRYPT, ((inputParam *) fuse_get_context()->private_data)->key);
				//set file pointer location
        fseek(tmp, 0, SEEK_END);
        size_t len = ftell(tmp);
        fseek(tmp, 0, SEEK_SET);
				//preform read operation
        res = fread(buf, 1, len, tmp);
        if (res == -1)
            return -errno;
				//close files after reading
        fclose(fp);
        fclose(tmp);

    }
		//for the case of the file not being encrypted
    else {
	    fd = open(fullpath, O_RDONLY);
	    if (fd == -1)
		    return -errno;

        res = pread(fd, buf, size, offset);
	    if (res == -1)
		    res = -errno;

    	close(fd);
    }

	return res;
}


/*check the file attributes to see if it is encrypted, if it is DECRYPT,
then update the file, and encrypt

if it is not encrypted then just write as if it is a normal file*/
static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{

	int fd;
	int res;
    ssize_t valsize;
    char fullpath[PATH_MAX];
    FILE *fp, *tmp;

    newpath(fullpath, path);

    (void) fi;
		//check the attributes on the file looking for encryption
    valsize = getxattr(fullpath, ENC_XATTR, NULL, 0);
		// in the case of the file being encrypted
    if (valsize >= 0) {
				//create a temp file, do_crypt has issues if reading/writing same file

				tmp = tmpfile();

				//open fp to read bytes and open tmppath to write bytes
        fp = fopen(fullpath, "rb+");
        //tmp = fopen(tmppath, "wb+");
				//set the file posistion indicator with fseek
        fseek(fp, 0, SEEK_SET);
				//time to decrypt
        do_crypt(fp, tmp, DECRYPT, ((inputParam *) fuse_get_context()->private_data)->key);
				//update file posistion indicator
        fseek(fp, 0, SEEK_SET);
				//preform desired write operation
        res = fwrite(buf, 1, size, tmp);
        if (res == -1)
            return -errno;

        fseek(tmp, 0, SEEK_SET);
				//ENCRYPT the file
        do_crypt(tmp, fp, ENCRYPT, ((inputParam *) fuse_get_context()->private_data)->key);
				//close files and remove the temp evidence
        fclose(fp);
        fclose(tmp);

    }
		//in the case of the file not being encrypted
    else {
	    fd = open(fullpath, O_WRONLY);
	    if (fd == -1)
		    return -errno;
				//write without decrypting or encrypting, do this to preserve file state
        res = pwrite(fd, buf, size, offset);
	    if (res == -1)
		    res = -errno;

    	close(fd);
    }

	return res;
}


static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	//update path
	char fullpath[PATH_MAX];
  newpath(fullpath, path);
	int res;

	res = statvfs(fullpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}
/*create a new file with the encrypted attribute*/
/*This is the only place where the encryted attribute is set, write simply matches the attribute on the given file*/
static int xmp_create(const char* path, mode_t mode, struct fuse_file_info* fi) {

    (void) fi;
		FILE *fp;
		//update path
		char fullpath[PATH_MAX];
	  newpath(fullpath, path);

    int res;
		//create file with given path and mode
    res = creat(fullpath, mode);
    if(res == -1)
			return -errno;
		//open that file and save to new file pointer
		fp = fdopen(res,"w");
    close(res);
		//encrypt the new file pointer using the key from the given parameter
		do_crypt(fp,fp,ENCRYPT,((inputParam *) fuse_get_context()->private_data)->key);

		fclose(fp);
		//check to make sure encrypt attribute was set
		if (setxattr(fullpath, ENC_XATTR, "true", 4, 0) == -1) {
        return -errno;
    }

    return 0;
}


static int xmp_release(const char *path, struct fuse_file_info *fi)
{

	(void) path;
	(void) fi;
	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{

	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

#ifdef HAVE_SETXATTR
static int xmp_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
	//update path
	char fullpath[PATH_MAX];
  newpath(fullpath, path);
	int res = lsetxattr(fullpath, name, value, size, flags);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
	//update path
	char fullpath[PATH_MAX];
	newpath(fullpath, path);
	int res = lgetxattr(fullpath, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
{
	//update path
	char fullpath[PATH_MAX];
  newpath(fullpath, path);
	int res = llistxattr(fullpath, list, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_removexattr(const char *path, const char *name)
{
	//update path
	char fullpath[PATH_MAX];
  newpath(fullpath, path);
	int res = lremovexattr(fullpath, name);
	if (res == -1)
		return -errno;
	return 0;
}
#endif /* HAVE_SETXATTR */

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.access		= xmp_access,
	.readlink	= xmp_readlink,
	.readdir	= xmp_readdir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.symlink	= xmp_symlink,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.link		= xmp_link,
	.chmod		= xmp_chmod,
	.chown		= xmp_chown,
	.truncate	= xmp_truncate,
	.utimens	= xmp_utimens,
	.open		= xmp_open,
	.read		= xmp_read,
	.write		= xmp_write,
	.statfs		= xmp_statfs,
	.create         = xmp_create,
	.release	= xmp_release,
	.fsync		= xmp_fsync,
#ifdef HAVE_SETXATTR
	.setxattr	= xmp_setxattr,
	.getxattr	= xmp_getxattr,
	.listxattr	= xmp_listxattr,
	.removexattr	= xmp_removexattr,
#endif
};

int main(int argc, char *argv[])
{

	umask(0);

    if (argc < 4) {
        fprintf(stderr, "Usage: %s %s \n", argv[0], "<Key> <Mirror Directory> <Mount Point>");
        return EXIT_FAILURE;
    }

    inputParam *encfs_data;

    encfs_data = malloc(sizeof(inputParam));

    encfs_data->key = argv[1];
    encfs_data->rootdir = realpath(argv[2], NULL);

	return fuse_main(argc - 2, argv + 2, &xmp_oper, encfs_data);

}
