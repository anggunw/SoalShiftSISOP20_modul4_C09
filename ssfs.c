#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>


static  const  char *dirpath = "/home/farrelmt/Documents";
char desc[100], command[100];
char fpath[100], fpath2[100];
int key = 10;
int act = 0;

void findPath(char* fpath, const char* path);
void logFile(char* command, char* desc);
void encrypt(char *fpath);
void decrypt(char *fpath);
void syncdir(char *fpath, char *fpath2, int modtime);
void dblog(char *fpath);
void checkdirname(const char *fpath);

static  int  xmp_getattr(const char *path, struct stat *stbuf)
{
    int res;
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);
    res = lstat(fpath, stbuf);

    if (res == -1) return -errno;

    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
  off_t offset, struct fuse_file_info *fi)
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
        checkdirname(fpath);
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        res = (filler(buf, de->d_name, &st, 0));
        if(res!=0) break;
    }

    closedir(dp);
    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
  struct fuse_file_info *fi)
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

static int xmp_create(const char* path, mode_t mode, struct fuse_file_info* fi)
{

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

static struct fuse_operations xmp_oper =
{
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

void findPath(char* fpath, const char* path)
{
    if(strcmp(path,"/") == 0){
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dirpath,path);
}

void logFile(char* command, char* desc)
{
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
    fp = fopen("/home/farrelmt/fs.log", "a");
    fprintf(fp, "%s\n", logLine);
    fclose(fp);
}

void encrypt(char *fpath) //encrypt blm selesai
{
  char ckey[] = "9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO";
  char *nencv1;
  nencv1 = strstr(fpath, "encv1_");
  nencv1 = strstr(nencv1, "/");
  const char strr = '.';
  char *buff;
  buff = strrchr(nencv1, strr);
  int lenenc, lenbuff, lencon;
  lenenc = strlen(nencv1);
  lenbuff = strlen(buff);
  lencon = lenenc - lenbuff;

  for(int i = 0; i < lencon; i++)
  {
    for(int j = 0; j < strlen(ckey); j++)
    {
        if((nencv1[i] == ckey[j]) && nencv[i] != '/')
        {
          int plus = j + key;
          plus = plus % strlen(ckey);
          nencv1[i] = ckey[plus];
          break;
        }
    }
  }

  act = 0;
}

void decrypt(char *fpath) //decrypt blm selesai
{
  char ckey[] = "Oj|DN_PwG,8hnl]cpz-J3$tTE}:#{*ZfV~4o)d7'Ir%b.M0&eSUCXBiQKH^!Fs+?yR2Y5`q6xagvmL[1WA@uk(9";
  char *nencv1;
  nencv1 = strstr(fpath, "encv1_");
  nencv1 = strstr(nencv1, "/");
  const char strr = '.';
  char *buff;
  buff = strrchr(nencv1, strr);
  int lenenc, lenbuff, lencon;
  lenenc = strlen(nencv1);
  lenbuff = strlen(buff);
  lencon = lenenc - lenbuff;

  for(int i = 0; i < lencon; i++)
  {
    for(int j = 0; j < strlen(ckey); j++)
    {
        if((nencv1[i] == ckey[j]) && nencv[i] != '/')
        {
          int plus = j + key;
          plus = plus % strlen(ckey);
          nencv1[i] = ckey[plus];
          break;
        }
    }
  }

  act = -1;
}

void dblog(char *fpath) //database log kosong
{
  //setiap mkdir rename tercatat ke database/log berupa file
}

void checkdirname(const char *fpath)
{
  //get parent directory
  //findPath(fpath, path);
  char *nencv1, *nencv3;
  nencv1 = strstr(fpath, "encv1_");
  nencv3 = strstr(fpath, "sync_");

  //encrypt
  if(nencv1)
    act = 1;

  //sync_
  else if(nencv3)
    act = 3;

  else
    act = 0;


}

void syncdir(char *fpath, char *fpath2, float modtime)
{
  //cel dir & file
  if(strcmp(fpath, fpath2) == 0)
  {
    //get modified time
    if(modtime < 0.1 && modtime > -0.1)
    {
      //sync
    }
  }

  else
  {
      //never sync again if above is violated
  }
}
