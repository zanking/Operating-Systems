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

// #define EN_PARAMS ((encryptParameters*) fuse_get_context()->private_data)
//
// typedef struct{
// 	char * directory;
// 	char * key;
// }encryptParameters;

typedef struct {
    char *rootdir;
    char *key;
} inputParam;

static void encfs_fullpath(char fpath[PATH_MAX], const char *path)
{

    inputParam *state = (inputParam *) (fuse_get_context()->private_data);
    strcpy(fpath, state->rootdir);
    strncat(fpath, path, PATH_MAX);
}

/*function that returns path of directory
accesses private struct to get the directory*/

// char* get_path(const char *path) {
// 	int len = strlen(path) + strlen(EN_PARAMS->directory) + 1;
// 	char* ret = malloc(sizeof(char) * len);
// 	strcpy(ret, EN_PARAMS->directory);
// 	strcat(ret, path);
// 	return ret;
// }

static int xmp_getattr(const char *path, struct stat *stbuf)
{
	//update path
	char fullpath[PATH_MAX];
  encfs_fullpath(fullpath, path);
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
  encfs_fullpath(fullpath, path);
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
  encfs_fullpath(fullpath, path);
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
encfs_fullpath(fullpath, path);
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
encfs_fullpath(fullpath, path);
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
	encfs_fullpath(fullpath, path);
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
  encfs_fullpath(fullpath, path);
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
  encfs_fullpath(fullpath, path);
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
  encfs_fullpath(fullpath, path);
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
  encfs_fullpath(fullpath, path);
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
  encfs_fullpath(fullpath, path);
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
  encfs_fullpath(fullpath, path);
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
  encfs_fullpath(fullpath, path);
	int res;

	res = open(fullpath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{

	int fd;
	int res;
    ssize_t valsize;
    char fullpath[PATH_MAX];
    FILE *fp, *tmp;

    encfs_fullpath(fullpath, path);

    (void) fi;

    valsize = getxattr(fullpath, ENC_XATTR, NULL, 0);
    if (valsize >= 0) {
        char* tmppath;
        tmppath = malloc(sizeof(char)*(strlen(fullpath) + strlen(".temp") + 1));
        tmppath[0] = '\0';
        strcat(tmppath, fullpath);
        strcat(tmppath, ".temp");

        tmp = fopen(tmppath, "wb+");
        fp = fopen(fullpath, "rb");

        do_crypt(fp, tmp, DECRYPT, ((inputParam *) fuse_get_context()->private_data)->key);

        fseek(tmp, 0, SEEK_END);
        size_t len = ftell(tmp);
        fseek(tmp, 0, SEEK_SET);

        res = fread(buf, 1, len, tmp);
        if (res == -1)
            return -errno;

        fclose(fp);
        fclose(tmp);
        remove(tmppath);
    }
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


static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{

	int fd;
	int res;
    ssize_t valsize;
    char fullpath[PATH_MAX];
    FILE *fp, *tmp;

    encfs_fullpath(fullpath, path);

    (void) fi;

    valsize = getxattr(fullpath, ENC_XATTR, NULL, 0);
    if (valsize >= 0) {
        char* tmppath;
        tmppath = malloc(sizeof(char)*(strlen(fullpath) + strlen(".temp") + 1));
        tmppath[0] = '\0';
        strcat(tmppath, fullpath);
        strcat(tmppath, ".temp");

        fp = fopen(fullpath, "rb+");
        tmp = fopen(tmppath, "wb+");

        fseek(fp, 0, SEEK_SET);

        do_crypt(fp, tmp, DECRYPT, ((inputParam *) fuse_get_context()->private_data)->key);

        fseek(fp, 0, SEEK_SET);

        res = fwrite(buf, 1, size, tmp);
        if (res == -1)
            return -errno;

        fseek(tmp, 0, SEEK_SET);

        do_crypt(tmp, fp, ENCRYPT, ((inputParam *) fuse_get_context()->private_data)->key);

        fclose(fp);
        fclose(tmp);
        remove(tmppath);
    }
    else {
	    fd = open(fullpath, O_WRONLY);
	    if (fd == -1)
		    return -errno;

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
  encfs_fullpath(fullpath, path);
	int res;

	res = statvfs(fullpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_create(const char* path, mode_t mode, struct fuse_file_info* fi) {

    (void) fi;
		FILE *fp;
		//update path
		char fullpath[PATH_MAX];
	  encfs_fullpath(fullpath, path);

    int res;
    res = creat(fullpath, mode);
    if(res == -1)
			return -errno;
		fp = fdopen(res,"w");
    close(res);

		do_crypt(fp,fp,ENCRYPT,((inputParam *) fuse_get_context()->private_data)->key);

		fclose(fp);

		if (setxattr(fullpath, ENC_XATTR, "true", 4, 0) == -1) {
        return -errno;
    }

    return 0;
}


static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
		 //update path
		// 	char fullpath[PATH_MAX];
	  //  encfs_fullpath(fullpath, path);
	(void) path;
	(void) fi;
	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
		 //update path
		// 	char fullpath[PATH_MAX];
	  //  encfs_fullpath(fullpath, path);

	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}
//Check encryption attribute on file
int checkEncrypted(const char *path) {

	char fullpath[PATH_MAX];
 encfs_fullpath(fullpath, path);
	char attir[5];
	getxattr(fullpath, "user.pa5-encfs.encrypted", attir, sizeof(char)*5);
	if (strcmp(attir, "true") == 0){
		return 1;
	}
	else{
		return 0;
	}
}

//add encryption attribute to file
void addEncryptAttrib(const char *path){
	char *tmpstr = NULL;
	char *trues = "true";
	char *argv2 = "pa5-encfs.encrypted";

	/* Append necessary 'user.' namespace prefix to beginning of name string */
	tmpstr = malloc(strlen(path) + XATTR_USER_PREFIX_LEN + 1);
	if(!tmpstr){
	    perror("malloc of 'tmpstr' error");
	    exit(EXIT_FAILURE);
	}
	strcpy(tmpstr, XATTR_USER_PREFIX);
	strcat(tmpstr, argv2);
	/* Set attribute */
	if(setxattr(path, "user.pa5-encfs.encrypted", "true", strlen(trues), 0)){
	    perror("setxattr error");
	    fprintf(stderr, "path  = %s\n", path);
	    fprintf(stderr, "name  = %s\n", tmpstr);
	    fprintf(stderr, "value = %s\n", trues);
	    fprintf(stderr, "size  = %zd\n", strlen(trues));
	    exit(EXIT_FAILURE);
	}
	/* Cleanup */
	free(tmpstr);
}

#ifdef HAVE_SETXATTR
static int xmp_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
	//update path
	char fullpath[PATH_MAX];
  encfs_fullpath(fullpath, path);
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
	encfs_fullpath(fullpath, path);
	int res = lgetxattr(fullpath, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
{
	//update path
	char fullpath[PATH_MAX];
  encfs_fullpath(fullpath, path);
	int res = llistxattr(fullpath, list, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_removexattr(const char *path, const char *name)
{
	//update path
	char fullpath[PATH_MAX];
  encfs_fullpath(fullpath, path);
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
