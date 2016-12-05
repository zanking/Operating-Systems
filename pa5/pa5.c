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

#define EN_PARAMS ((encryptParameters*) fuse_get_context()->private_data)

typedef struct{
	char * directory;
	char * key;
}encryptParameters;

/*function that returns path of directory
accesses private struct to get the directory*/

char* get_path(const char *path) {
	int len = strlen(path) + strlen(EN_PARAMS->directory) + 1;
	char* ret = malloc(sizeof(char) * len);
	strcpy(ret, EN_PARAMS->directory);
	strcat(ret, path);
	return ret;
}

static int xmp_getattr(const char *tpath, struct stat *stbuf)
{
	char * path = get_path(tpath);
	int res;

	res = lstat(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_access(const char *tpath, int mask)
{
	char * path = get_path(tpath);
	int res;

	res = access(path, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readlink(const char *tpath, char *buf, size_t size)
{
	char * path = get_path(tpath);
	int res;

	res = readlink(path, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}


static int xmp_readdir(const char *tpath, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	char * path = get_path(tpath);
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(path);
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

static int xmp_mknod(const char *tpath, mode_t mode, dev_t rdev)
{
	char * path = get_path(tpath);
	int res;

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(path, mode);
	else
		res = mknod(path, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mkdir(const char *tpath, mode_t mode)
{
	char * path = get_path(tpath);
	int res;

	res = mkdir(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_unlink(const char *tpath)
{
	char * path = get_path(tpath);
	int res;

	res = unlink(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *tpath)
{
	char * path = get_path(tpath);
	int res;

	res = rmdir(path);
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

static int xmp_chmod(const char *tpath, mode_t mode)
{
	char * path = get_path(tpath);
	int res;

	res = chmod(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chown(const char *tpath, uid_t uid, gid_t gid)
{
	char * path = get_path(tpath);
	int res;

	res = lchown(path, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *tpath, off_t size)
{
	char * path = get_path(tpath);
	int res;

	res = truncate(path, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_utimens(const char *tpath, const struct timespec ts[2])
{
	char * path = get_path(tpath);
	int res;
	struct timeval tv[2];

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(path, tv);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_open(const char *tpath, struct fuse_file_info *fi)
{
	char * path = get_path(tpath);
	int res;

	res = open(path, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_read(const char *tpath, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int fd;
	int res;
    ssize_t sizes;
    FILE *fp, *tmp;

    char *fullpath = get_path(tpath);

    (void) fi;
		//handle encryption
    sizes = checkEncrypted(fullpath);
    if (sizes == 1) {
        char* tmppath;
        tmppath = malloc(sizeof(char)*(strlen(fullpath) + strlen(".temp") + 1));
        tmppath[0] = '\0';
        strcat(tmppath, fullpath);
        strcat(tmppath, ".temp");

        tmp = fopen(tmppath, "wb+");
        fp = fopen(fullpath, "rb");

        do_crypt(fp, tmp, 0, EN_PARAMS->key);

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
		//handle the case of the file not being encrypted
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


static int xmp_write(const char *tpath, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;
    ssize_t valsize;
    char* fullpath = get_path(tpath);
    FILE *fp, *tmp;

    (void) fi;

    valsize = checkEncrypted(fullpath);
    if (valsize != 0) {
			printf("Encrypted");
        char* tmppath;
        tmppath = malloc(sizeof(char)*(strlen(fullpath) + strlen(".temp") + 1));
        tmppath[0] = '\0';
        strcat(tmppath, fullpath);
        strcat(tmppath, ".temp");

        fp = fopen(fullpath, "rb+");
        tmp = fopen(tmppath, "wb+");

        //fseek(fp, 0, SEEK_SET);
        do_crypt(fp, tmp, 0, EN_PARAMS->key);
        fseek(fp, 0, SEEK_SET);

        res = fwrite(buf, 1, size, tmp);

        if (res == -1){
            return -errno;}

        fseek(tmp, 0, SEEK_SET);
        do_crypt(tmp, fp, 1, EN_PARAMS->key);

				addEncryptAttrib(fullpath);

        fclose(fp);
        fclose(tmp);
        remove(tmppath);


}
else {

			 char* tmppath;
			 tmppath = malloc(sizeof(char)*(strlen(fullpath) + strlen(".temp") + 1));
			 tmppath[0] = '\0';
			 strcat(tmppath, fullpath);
			 strcat(tmppath, ".temp");

			 fp = fopen(fullpath, "rb+");
			 tmp = fopen(tmppath, "wb+");
			 res = fwrite(buf, 1, size, tmp);

			 if (res == -1){
					 return -errno;}

			fseek(tmp, 0, SEEK_SET);
	 		do_crypt(tmp, fp, 1, EN_PARAMS->key);

			addEncryptAttrib(fullpath);

	 		fclose(fp);
	 		fclose(tmp);
	 		remove(tmppath);
	 }
	 return res;

}

static int xmp_statfs(const char *tpath, struct statvfs *stbuf)
{
	char * path = get_path(tpath);
	int res;

	res = statvfs(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_create(const char* tpath, mode_t mode, struct fuse_file_info* fi) {

    (void) fi;
		FILE *fp;
		char * fullpath = get_path(tpath);

    int res;
    res = creat(fullpath, mode);
    if(res == -1)
			return -errno;
		fp = fdopen(res,"w");
    close(res);

		do_crypt(fp,fp,0,EN_PARAMS->key);

		fclose(fp);

    return 0;
}


static int xmp_release(const char *tpath, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
	char * path = get_path(tpath);
	(void) path;
	(void) fi;
	return 0;
}

static int xmp_fsync(const char *tpath, int isdatasync,
		     struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
	char * path = get_path(tpath);

	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}
//Check encryption attribute on file
int checkEncrypted(const char *path) {

	int temp;
	char attir[5];
	getxattr(path, "user.pa5-encfs.encrypted", attir, sizeof(char)*5);
	if (strcmp(attir, "true") == 0){
		return 1;
	}
	else{
		return 0;
	}
	//
	// valsize = getxattr(argv[3], tmpstr, tmpval, valsize);
	// if(valsize < 0){
	//     if(errno == ENOATTR){
	// 	fprintf(stdout, "No %s attribute set on %s\n", tmpstr, argv[3]);
	// 	free(tmpval);
	// 	free(tmpstr);
	// 	return EXIT_SUCCESS;
	//     }
	//     else{
	// 	perror("getxattr error");
	// 	fprintf(stderr, "path  = %s\n", argv[3]);
	// 	fprintf(stderr, "name  = %s\n", tmpstr);
	// 	fprintf(stderr, "value = %s\n", tmpval);
	// 	fprintf(stderr, "size  = %zd\n", valsize);
	// 	exit(EXIT_FAILURE);
	//     }
}
//add encryption attribute to file
void addEncryptAttrib(const char *path){
	char *tmpstr = NULL;
	char *trues = "true";
	char *argv2 = "pa5-encfs.encrypted";
	// int temp;
	// int attributeReturn = setxattr(path, "pa5-encfs.encrypted","true", (sizeof(char)*20), 0);
	// temp = attributeReturn == 0;
	// return temp;

	/* Append necessary 'user.' namespace prefix to beginning of name string */
	tmpstr = malloc(strlen(path) + XATTR_USER_PREFIX_LEN + 1);
	if(!tmpstr){
	    perror("malloc of 'tmpstr' error");
	    exit(EXIT_FAILURE);
	}
	strcpy(tmpstr, XATTR_USER_PREFIX);
	strcat(tmpstr, argv2);
	/* Set attribute */
	if(setxattr(path, tmpstr, trues, strlen(trues), 0)){
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
static int xmp_setxattr(const char *tpath, const char *name, const char *value,
			size_t size, int flags)
{
	char * path = get_path(tpath);
	int res = lsetxattr(path, name, value, size, flags);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_getxattr(const char *tpath, const char *name, char *value,
			size_t size)
{
		char * path = get_path(tpath);
	int res = lgetxattr(path, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_listxattr(const char *tpath, char *list, size_t size)
{
	char * path = get_path(tpath);
	int res = llistxattr(path, list, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_removexattr(const char *tpath, const char *name)
{
	char * path = get_path(tpath);
	int res = lremovexattr(path, name);
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
	if ( argc < 4 ) /* argc should be 4 for correct execution */
	 {

			 fprintf(stderr,"not enough arguments\n" );
	 }

	 encryptParameters *data = NULL;

	 data = malloc(sizeof(encryptParameters));

	 if (data == NULL){ //check to make sure data exists
		 printf(stdout, "Error allocating ");
	 }



	 data-> directory = realpath(argv[argc-2], NULL);
	 data -> key = argv[1];
	// encryptParameters -> directory = argv[2];
	 argv[1] = argv[3];
	 argv[3] = NULL;
	 argv[2] = NULL;

	 argc = argc-2;


	return fuse_main(argc, argv, &xmp_oper, data);
}
