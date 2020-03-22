#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#define MAXPATHLEN 4096
int main(int argc,char **argv){
    if(argc!=3||(strcmp(argv[1],"-u")==0&&strcmp(argv[1],"-p")==0)){
        puts("Usage: unhtar -[u|p] <FILENAME>\n\t-u: Extract file into current directory.\n\t-p: Extract file into _<FILENAME> named directory.");
        return 1;
    }
    FILE *fp=fopen(argv[2],"r");
    if(!fp){
        printf("Error: File(%s) not found.\n",argv[2]);
        return 1;
    }

    unsigned char type;
    size_t sz,i;
    char wbuf[MAXPATHLEN]={0},*rbuf=wbuf,wbuf2[MAXPATHLEN]={0},*rbuf2=wbuf2;
    mode_t md;
    struct timespec time[2];
    int rmax=MAXPATHLEN-1,fd;
    nlink_t lcnt;
    FILE *ap;
    long pos,pos2;

    if(argv[1][1]=='p'){
        wbuf[0]='_';
        wbuf2[0]='_';
        strcat(strcat(wbuf,argv[2]),"/");
        strcat(strcat(wbuf2,argv[2]),"/");
        rbuf+=strlen(wbuf);
        rbuf2+=strlen(wbuf);
        mkdir(wbuf,0755);
        rmax-=strlen(wbuf);
    }

    fread(&sz,sizeof(size_t),1,fp);
    for(i=0;i<sz;i++){
        memset(rbuf,0,rmax);
        fgets(rbuf,rmax,fp);
        rbuf[strlen(rbuf)-1]='\0';

        fread(&md,sizeof(mode_t),1,fp);
        fread(time,sizeof(struct timespec),2,fp);

        mkdir(wbuf,md);
        fd=open(wbuf,O_DIRECTORY|O_RDONLY);
        futimens(fd,time);
        close(fd);
    }

    do{
        fread(&lcnt,sizeof(nlink_t),1,fp);
        if(feof(fp)) break;
        memset(rbuf,0,rmax);
        fgets(rbuf,rmax,fp);
        rbuf[strlen(rbuf)-1]='\0';

        pos=ftell(fp);
        for(i=1;i<lcnt;i++) fgets(rbuf2,rmax,fp);

        fread(&type,sizeof(unsigned char),1,fp);
        fread(&md,sizeof(mode_t),1,fp);
        fread(time,sizeof(struct timespec),2,fp);
        fread(&sz,sizeof(size_t),1,fp);

        if(type==DT_REG){
            ap=fopen(wbuf,"w");
            for(i=0;i<sz;i++) fputc(fgetc(fp),ap);
            fclose(ap);
        }else{
            memset(rbuf2,0,rmax);
            fread(rbuf2,sizeof(char),sz,fp);
            symlink(rbuf2,wbuf);
        }
        fd=open(wbuf,O_RDONLY);
        futimens(fd,time);
        close(fd);

        pos2=ftell(fp);
        fseek(fp,pos,SEEK_SET);
        for(i=1;i<lcnt;i++){
            memset(rbuf2,0,rmax);
            fgets(rbuf2,rmax,fp);
            rbuf2[strlen(rbuf2)-1]='\0';
            link(wbuf,wbuf2);
        }
        fseek(fp,pos2,SEEK_SET);

    }while(1);

    fclose(fp);
    return 0;
}
