#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

  
static  const  char *dirpath = "/home/gun/Documents";
char desc[100], command[100];
char fpath[100], fpath2[100];

void findPath(char* fpath, const char* path){
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);
}

void logFile(char* command, char* desc){
    char now[100];
    char level[30];
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = localtime(&rawtime);
    strftime(now, sizeof(now), "%y%m%d-%H:%M:%S::", info);
    if(strcmp(command, "RMDIR") == 0 || strcmp(command, "UNLINK") == 0){
        strcpy(level, "WARNING");
    }
    else{
        strcpy(level, "INFO");
    }
    char logLine[200];
    sprintf(logLine, "%s::%s%s::%s", level, now, command, desc);

    FILE* fp;
    fp = fopen("/home/gun/fs.log", "a");
    fprintf(fp, "%s\n", logLine);
    fclose(fp);
}

static  int  xmp_getattr(const char *path, struct stat *stbuf){
    int res;
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);
    res = lstat(fpath, stbuf);

    if (res == -1) return -errno;

    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,off_t offset, struct fuse_file_info *fi)
{
    findPath(fpath, path);
    int res = 0;
    DIR *dp;
    struct dirent *de;
    (void) offset;
    (void) fi;
    dp = opendir(fpath);

    if (dp == NULL) return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        res = (filler(buf, de->d_name, &st, 0));
        if(res!=0) break;
    }

    closedir(dp);
    return 0;
}


static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    findPath(fpath, path);
    
    int res = 0;
    int fd = 0 ;
    
    (void) fi;

    fd = open(fpath, O_RDONLY);

    if (fd == -1) return -errno;

    res = pread(fd, buf, size, offset);

    if (res == -1) res = -errno;

    close(fd);
    return res;
}


static int xmp_access(const char *path, int mask)
{
    findPath(fpath, path);
	int res;

	res = access(fpath, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	findPath(fpath, path); 

	int res;

	res = mkdir(fpath, mode);
	if (res == -1) return -errno;

    char temp[10] = "MKDIR";
    logFile(temp, fpath);  
	return 0;
}

static int xmp_unlink(const char *path)
{
    findPath(fpath, path); 

	int res;
	res = unlink(fpath);
	if (res == -1)
		return -errno;

    char temp[10] = "UNLINK";
    logFile(temp, fpath); 
	return 0;
}

static int xmp_rmdir(const char *path)
{
	findPath(fpath, path);

	int res;

	res = rmdir(fpath);
	if (res == -1) return -errno;

    char temp[10] = "RMDIR";
    logFile(temp, fpath);  
	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
    findPath(fpath, from);
    findPath(fpath2, to);

	int res;

	res = rename(fpath, fpath2);
	if (res == -1)
		return -errno;

    char temp[10] = "RENAME";
    strcat(fpath, "::");
    strcat(fpath, fpath2);
    logFile(temp, fpath);  
	return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
{
    findPath(fpath, path);
	int res;
	struct timeval tv[2];

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(fpath, tv);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
    findPath(fpath, path);

	int res;

	res = truncate(fpath, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	findPath(fpath, path);

	int res;

	res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	findPath(fpath, path);

	int fd;
	int res;

	(void) fi;
	fd = open(fpath, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}


static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
    findPath(fpath, path);
	int res;

	res = statvfs(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_create(const char* path, mode_t mode, struct fuse_file_info* fi) {

    findPath(fpath, path);

	(void) fi;

    int res;
    res = creat(fpath, mode);
    if(res == -1)
	return -errno;

    close(res);

    char temp[10] = "CREAT";
    logFile(temp, fpath);  

    return 0;
}

static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .read = xmp_read,
    
	.access = xmp_access,
	//.readlink	= xmp_readlink,
	//.mknod = xmp_mknod,
	.mkdir = xmp_mkdir,
	//.symlink = xmp_symlink,
	.unlink = xmp_unlink,
	.rmdir = xmp_rmdir,
	.rename = xmp_rename,
	//.link	= xmp_link,
	//.chmod = xmp_chmod,
	//.chown = xmp_chown,
	.truncate = xmp_truncate,
	.utimens = xmp_utimens,
	.open = xmp_open,
	.write = xmp_write,
	.statfs = xmp_statfs,
	.create = xmp_create,
	//.release = xmp_release,
	//.fsync = xmp_fsync,
};

  

int  main(int  argc, char *argv[])

{

umask(0);

return fuse_main(argc, argv, &xmp_oper, NULL);

}
