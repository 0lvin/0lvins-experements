/* -*- coding: utf-8 -*-
  Copyright (C) 2007  Denis Pauk

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA*/
#include "config.h"
#define NoNStoP
#define _XOPEN_SOURCE 1
#define _XOPEN_SOURCE_EXTENDED 1
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <ftw.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/types.h>
#include <pwd.h>
/*antivirus*/
#include <clamav.h>
#define CANWRITE 0
#define CANREAD  1
#define NOTCAN   2
char PROCDIR[]="/proc/";
char SYSDIR[]="/sys/";
/*изменение группы и пользователяна другую менее привелигированную позволяет уберечь ситему от возможных проблем чаязанных со сбоями системы, так как менеепривелигированный может испортить меньше*/
char USER[]="clamav";
volatile sig_atomic_t signal_flag=0;
int can_read_data_base=NOTCAN;
struct cl_node *root = NULL;
int basefile=-1;
int errorseek=0;
int changedfiles=0;
size_t size_files=0;
int files=0;
int files_errors=0;
int viruses_count=0;
size_t size_hash=0;
int not_scaned=0;
struct cl_limits limits;
int max_file_size=0;
struct hashtable
{
/*defice+inode=file*/
	dev_t device;
	ino_t inode;
	int a;
	int b;
	int c;
	int d;
};
/*блокировка файла при записи*/
void lock_W(int fd)
{
	struct flock lock;
	lock.l_type= F_WRLCK;
	lock.l_whence= SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	fcntl(fd,F_SETLKW,&lock);
}
/*блокировка файла при чтении(мягкая позволяет другим потокам(процессам) считывать из файла информацию)*/
void lock_R(int fd)
{
	struct flock lock;
	lock.l_type= F_RDLCK;
	lock.l_whence= SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	fcntl(fd,F_SETLKW,&lock);
}
/*убрать блокировку файла*/
void unlock(int fd)
{
	struct flock lock;
	lock.l_type= F_UNLCK;
	lock.l_whence= SEEK_SET;
	lock.l_start=0;
	lock.l_len=0;
	fcntl(fd,F_SETLKW,&lock);
}
/*virus scan*/
int scan(const char* file)
{
const char *virname;
int ret=CL_CLEAN;
int fd;
unsigned long int blocks = 0;
	if((fd = open(file, O_RDONLY)) == -1) {
		files_errors++;
		return CL_CLEAN;
    	}else
        {
		files++;
	}
	/* scan descriptor */
	if((ret = cl_scandesc(fd, &virname, &blocks, root, &limits, CL_SCAN_STDOPT)) == CL_VIRUS)
	{
		viruses_count++;
		printf("\tVirus %s (%ld bytes). ", virname,blocks*CL_COUNT_PRECISION);
	}
	else {
		printf("\tNo Virus (%ld bytes). ",blocks*CL_COUNT_PRECISION);
	}
	close(fd);
	size_files+=blocks;
	return ret;
}
int loadhash(const char* file,struct hashtable * hash)
{
int begin_pos=lseek(basefile,0,SEEK_CUR);
int cur_pos=begin_pos;
ino_t inode;
dev_t device;
struct stat statbuf;
int desk=-1;
	if(can_read_data_base==NOTCAN)
		return !CL_CLEAN;
	if(hash==NULL)
		return !CL_CLEAN;
	desk=open(file,O_RDONLY);
		if(desk<0)
			return !CL_CLEAN;
		if(fstat(desk,&statbuf)==-1)
			return !CL_CLEAN;
		inode=statbuf.st_ino;
		device=statbuf.st_dev;
	close(desk);
	if(hash==NULL)
		return !CL_CLEAN;
	{
		while(read(basefile,hash,sizeof(struct hashtable))>0)
		{
			if(inode==hash->inode)
				if(device==hash->device)
				{
					return CL_CLEAN;
				}
		}
	}while(errno==EINTR);
	errorseek++;
	lseek(basefile,0,SEEK_SET);
	cur_pos=0;
	{
		while(read(basefile,hash,sizeof(struct hashtable))>0)
		{
			cur_pos+=sizeof(struct hashtable);
			if(inode==hash->inode)
				if(device==hash->device)
				{
					return CL_CLEAN;
				}
			if(begin_pos<cur_pos)
				return !CL_CLEAN;
		}
	}while(errno==EINTR);
return !CL_CLEAN;
}
int savehash(struct hashtable * hash)
{
struct hashtable temp;
int begin_pos=lseek(basefile,0,SEEK_CUR);
int cur_pos=begin_pos;
	if(can_read_data_base!=CANWRITE)
		return !CL_CLEAN;
	{
		while(read(basefile,&temp,sizeof(struct hashtable))>0)
		{
			if(hash->inode==temp.inode)
				if(hash->device==temp.device)
				{
					lseek(basefile,-sizeof(struct hashtable),SEEK_CUR);
					write(basefile,hash,sizeof(struct hashtable));			
					break;
				}
		}
	}while(errno==EINTR);

	lseek(basefile,0,SEEK_SET);
	cur_pos=0;
	{
		while(read(basefile,&temp,sizeof(struct hashtable))>0)
		{
			cur_pos+=sizeof(struct hashtable);
			if(temp.inode==hash->inode)
				if(temp.device==hash->device)
				{
					lseek(basefile,-sizeof(struct hashtable),SEEK_CUR);
					break;
				}
			if(begin_pos<cur_pos)
				{
					lseek(basefile,0,SEEK_END);
					break;
				}
		}
	}while(errno==EINTR);
	{
		write(basefile,hash,sizeof(struct hashtable));
	}while(errno==EINTR);
	return CL_CLEAN;
}

/*ganerate hash!*/
int genhash(const char* file,struct hashtable * hash)
{
int desk=-1;
int buff[4];
struct stat statbuf;
	if(hash==NULL)
	{
		printf("\tCan't open file:%s ", strerror(errno));
		return !CL_CLEAN;
	};
	desk=open(file,O_RDONLY);
	if(desk<0)
		return !CL_CLEAN;
	if(fstat(desk,&statbuf)==-1)
		return !CL_CLEAN;
	hash->inode=statbuf.st_ino;
	hash->device=statbuf.st_dev;
	hash->a=statbuf.st_size;
	hash->b=statbuf.st_mode;
	hash->c=statbuf.st_uid;
	hash->d=statbuf.st_gid;
	size_hash+=statbuf.st_size;
	buff[0]=0;
	buff[1]=0;
	buff[2]=0;
	buff[3]=0;
	{
		while(read(desk,buff,16)>0)
		{
			hash->a+=buff[0]^buff[3];
			hash->b+=buff[1]^buff[0];
			hash->c+=buff[2]^buff[1];
			hash->d+=buff[3]^buff[2];
			buff[0]=statbuf.st_size;
			buff[1]=statbuf.st_mode;
			buff[2]=statbuf.st_dev;
			buff[3]=statbuf.st_ino;				
		}
	}while(errno==EINTR);
	close(desk);
	return CL_CLEAN;
}

/*this function check hash for file*/
int checkhash(const char* file,struct hashtable ** hash)
{
struct hashtable *hashload=NULL;
	if(*hash==NULL)
	{
		*hash=(struct hashtable *)calloc(1,sizeof(struct hashtable));
		if(*hash==NULL)
			return !CL_CLEAN;
	};
	hashload=(struct hashtable *)calloc(1,sizeof(struct hashtable));
	if(hashload==NULL)
		return !CL_CLEAN;
	if(genhash(file,*hash)!=CL_CLEAN)
		return !CL_CLEAN;
	loadhash(file,hashload);
	if(memcmp(hashload,*hash,sizeof(struct hashtable))!=0)
	{
		free(hashload);
		return !CL_CLEAN;
	}
	free(hashload);
	return CL_CLEAN;
}
/*ganerate hash for file and save!*/
int makehash(const char* file,struct hashtable ** hash)
{
	if(*hash==NULL)
	{
		*hash=(struct hashtable *)calloc(1,sizeof(struct hashtable));
		if(*hash==NULL)
			return !CL_CLEAN;
		if(genhash(file,*hash)!=CL_CLEAN)
			return !CL_CLEAN;
	};
/*	printf("%08x %08x %80x %08x",hash->a,hash->b,hash->c,hash->d);*/
	savehash(*hash);
	free(*hash);
	*hash=NULL;
	return !CL_CLEAN;
}
/*this function calls for all files*/
int work(const char *file, const struct stat *sb, int flag, struct FTW *s)
{
struct hashtable *hash=NULL;
int ret=CL_CLEAN;
	if(signal_flag!=0)
		return !CL_CLEAN;
	if(strncmp(file,PROCDIR,strlen(PROCDIR))!=0)
	if(strncmp(file,SYSDIR,strlen(SYSDIR))!=0)
	if(sb->st_size<max_file_size)
	if(flag==FTW_F)
	if(sb->st_size!=0)
		if(S_ISREG(sb->st_mode))
		{
			printf("%s ",file);
			if(checkhash(file,&hash)!=CL_CLEAN)
			{
				changedfiles++;
				ret=scan(file);
				if(ret==CL_CLEAN)
				{
					makehash(file,&hash);
				}
				else
				{
					printf("Error: %s\n" ,cl_perror(ret));
					#ifdef NoNStoP
					ret=CL_CLEAN;
					#endif
				}
			}
			else
				printf("\tCheck Hash");
			if(hash!=NULL)
				free(hash);
			printf("\n");
			return ret;
		};
	not_scaned++;
	printf("%s\tNot Scaned\n",file);
	return ret;
}
int signalhandle(int sig)
{
static const char byby[]="I recive Signal ^C\n";
	write(1,byby,strlen(byby));
	signal_flag=1;
	return 0;
}
int initsignal()
{
struct sigaction sa;
	sa.sa_handler = signalhandle;
	sigemptyset(&sa.sa_mask);
	return sigaction(SIGINT,&sa,NULL);
}
int main()
{
char dir[PATH_MAX];
/*antivirus*/
int ds1=0, ds2=0, dms1=0, dms2=0, ret=CL_CLEAN;
unsigned int sigs = 0;
long double mbV;/*for virus*/
long double mbH;/*for hash*/
struct timeval t1, t2;
struct passwd * user=getpwnam(USER);
	if(user!=NULL)
	{
		printf("Change user to userid=%d usergid=%d\n",user->pw_uid,user->pw_gid);
		if(setgid(user->pw_gid)!=-1)
			printf("change group to %s\n",USER);
		else
			printf("change group not can\n");
		if(setuid(user->pw_uid)!=-1)
			printf("change user to %s\n",USER);
		else
			printf("Can't change user\n");
	};
	basefile=open("/filehash.database",O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
	if(basefile==-1)
	{
		printf("Can Write in This Dir!\n");
		basefile=open("/filehash.database",O_RDONLY);
		if(basefile==-1)
		{
			printf("Can Read in This Dir!\n");
			can_read_data_base=NOTCAN;
		}else
		{
			can_read_data_base=CANREAD;
			lock_R(basefile);
		}
	}else
	{
		can_read_data_base=CANWRITE;
		lock_W(basefile);
	}
	initsignal();
	{
		gettimeofday(&t1,NULL);
		printf("Load virus database. Please Wait!\n");
		/* load all available databases from default directory */
		if((ret = cl_loaddbdir(cl_retdbdir(), &root, &sigs))) {
			printf("cl_loaddbdir: %s\n", cl_perror(ret));
			return 2;
		}
		printf("Loaded %d signatures. %s \n", sigs,__func__);
		/* set up archive limits */
		memset(&limits, 0, sizeof(struct cl_limits));
		limits.maxfiles = 1000; /* max files */
		limits.maxfilesize = 10 * 1048576; /* maximum archived file size == 10 Mb */
		limits.maxreclevel = 5; /* maximum recursion level */
		limits.maxmailrec = 64; /* maximum mail recursion level */
		limits.maxratio = 200; /* maximum compression ratio */
		limits.archivememlim = 0; /* disable memory limit for bzip2 scanner */
		/* build engine */
		if((ret = cl_build(root))) {
			printf("Database initialization error: %s\n", cl_strerror(ret));;
			cl_free(root);
			return 2;
		}
		max_file_size=1000 * 1048576;
		gettimeofday(&t2,NULL);
		ds1 = t2.tv_sec - t1.tv_sec;
		dms1 = t2.tv_usec - t1.tv_usec;
		ds1 -= (dms1 < 0) ? (1):(0);
		dms1 += (dms1 < 0) ? (1000000):(0);
	}	
	getcwd(dir,sizeof(dir));
	{
		gettimeofday(&t1,NULL);
		ret=nftw(dir,work,1000,FTW_PHYS);
		cl_free(root);
		gettimeofday(&t2,NULL);
		ds2 = t2.tv_sec - t1.tv_sec;
		dms2 = t2.tv_usec - t1.tv_usec;
		ds2 -= (dms2 < 0) ? (1):(0);
		dms2 += (dms2 < 0) ? (1000000):(0);
	}
	if(can_read_data_base!=NOTCAN)
		unlock(basefile);
	close(basefile);
	mbV = (double)size_files * (CL_COUNT_PRECISION / 1024) / 1024.0;
	mbH = (double)size_hash / 1024 / 1024 ;
	printf("|----------------------------- SCAN SUMMARY -----------------------------|\n");
	printf("Hash system can read or write to table?\t:");
	switch(can_read_data_base)
	{
	case CANWRITE:
		printf("Write And Read!\n");
		break;
	case CANREAD:
		printf("Only Read!\n");
		break;
	case NOTCAN:
		printf("Can't Write and Read!\n");
		break;
	default:
		printf("Unknow error with open Dir!\n");
		break;
	};
	printf("Use antivirus version in file tree walk\t:%s\n",cl_retver());
	printf("Known viruses in file tree walk\t\t:%d\n",sigs);
	printf("Size files in file tree walk\t\t:%2.2Lf Mb\n",mbV);
	printf("Size files in file tree walk\t\t:%d bytes\n",size_files*CL_COUNT_PRECISION);
	printf("Size hashed in file tree walk\t\t:%d bytes\n",size_hash);
	printf("Size hashed in file tree walk\t\t:%2.2Lf Mb\n",mbH);
	printf("Viruses in file tree walk\t\t:%d\n",viruses_count);
	printf("Files in file tree walk\t\t\t:%d\n",files);
	printf("Errors with files in file tree walk\t:%d\n",files_errors);
	printf("Not Scaned Files in file tree walk\t:%d\n",not_scaned);
	printf("Time for load virus data base \t\t:%d.%3.3d sec (%d m %d s)\n",
		ds1, dms1/1000, ds1/60, ds1%60);
	printf("Scan time\t\t\t\t:%d.%3.3d sec (%d m %d s)\n",
		ds2, dms2/1000, ds2/60, ds2%60);
	printf("Error with position in file tree walk\t:%d\n",errorseek);
	printf("Changed files in file tree walk\t\t:%d\n",changedfiles);
	printf("|----------------------------- END  SUMMARY -----------------------------|\n");
	return (ret == CL_VIRUS ? 1 : 0);
}
