#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#define MAXPATHLEN 4096
struct filepack{
	FILE *ff;
	FILE *df;
};
void _recls(const char *p,FILE *fp,FILE *dfp){
	char b[MAXPATHLEN];
	DIR *dp=opendir(p);
	struct dirent *d;
	while((d=readdir(dp))!=NULL){
		if(d->d_type!=DT_DIR){
            fwrite(&(d->d_type),sizeof(unsigned char),1,fp);
            fwrite(&(d->d_ino),sizeof(ino_t),1,fp);
			fprintf(fp,"%s/%s\n",p,d->d_name);
			continue;
		}
		if(strcmp(d->d_name,".")==0||strcmp(d->d_name,"..")==0) continue;
		fputs(strcat(strcat(strcpy(b,p),"/"),d->d_name),dfp);
		fputc('\n',dfp);
		_recls(b,fp,dfp);
	}
	closedir(dp);
}
struct filepack mkrptree(const char *p,const char* f,const char *df){
	FILE *fp=fopen(f,"w+");
	FILE *dfp=fopen(df,"w+");
	DIR *dp=opendir(p);
	struct filepack fpk;
	if(!dp){
		closedir(dp);
		fpk.ff=NULL;
		fpk.df=NULL;
		return fpk;
	}
	closedir(dp);
	_recls(p,fp,dfp);
	fseek(fp,0,SEEK_SET);
	fseek(dfp,0,SEEK_SET);
	fpk.ff=fp;
	fpk.df=dfp;
	return fpk;
}
size_t flen(const char *f){
	FILE *fp=fopen(f,"a");
	size_t s=ftell(fp);
	fclose(fp);
	return s;
}
