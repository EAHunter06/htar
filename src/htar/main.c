#include "comp.h"
#define EXIT(n)                 \
	free(arr);          	    \
	fclose(f.ff);               \
	fclose(f.df);               \
	remove("___htar_tmp___");   \
	remove("___htar_dtmp___");  \
	fclose(fp);                 \
	return n
int main(int argc,char **argv){
	if(argc!=3&&argc!=4){
		puts("Usage: htar <FILENAME> <FOLDER>");
		return 1;
	}
	struct filepack f=mkrptree(argv[2],"___htar_tmp___","___htar_dtmp___");
	if(!f.ff||!f.df){
		printf("Error: Path(%s) not found\n",argv[2]);
		remove("___htar_tmp___");
		remove("___htar_dtmp___");
		return 1;
	}

	char buf[MAXPATHLEN],lbuf[MAXPATHLEN]={0};
	char *tp=buf+strlen(argv[2])+1,*ltp=lbuf+strlen(argv[2])+1;
	FILE *fp=fopen(argv[1],"w"),*af;
	struct stat st;
	int c;
	unsigned char type;
	ino_t inode,ino;
	size_t cnt=0,sz=0,lc=0,lcb,*arr=malloc(128*sizeof(size_t)),arrs=128*sizeof(size_t),sp,i,ap=0;
	_Bool sf=0;

	do{
		fgets(buf,MAXPATHLEN-1,f.df);
		if(feof(f.df)) break;
		cnt++;
	}while(1);
	fseek(f.df,0,SEEK_SET);
	fwrite(&cnt,sizeof(size_t),1,fp);;

	do{
		fgets(buf,MAXPATHLEN-1,f.df);
		if(feof(f.df)) break;
		fputs(tp,fp);
		tp[strlen(tp)-1]='\0';
		stat(buf,&st);
		fwrite(&(st.st_mode),sizeof(mode_t),1,fp);
		fwrite(&(st.st_atim),sizeof(struct timespec),1,fp);
		fwrite(&(st.st_mtim),sizeof(struct timespec),1,fp);
	}while(1);

	do{
        memset(buf,0,MAXPATHLEN);
        if(lbuf[0]) memset(lbuf,0,MAXPATHLEN);
        lc++;
        fread(&type,sizeof(unsigned char),1,f.ff);
        fread(&inode,sizeof(ino_t),1,f.ff);
		fgets(buf,MAXPATHLEN-1,f.ff);
		if(feof(f.ff)) break;
		for(i=ap;i<sz;i++) if(arr[i]==lc){
            sf=1;
			ap++;
            continue;
        }
		if(sf){
            sf=0;
            continue;
		}
		tp[strlen(tp)-1]='\0';
		lstat(buf,&st);

		fwrite(&(st.st_nlink),sizeof(nlink_t),1,fp);
        fputs(tp,fp);
        fputc('\n',fp);
		if(st.st_nlink!=1){
            sp=ftell(f.ff);
            lcb=lc;
            for(i=1;i<st.st_nlink;){
                lc++;
                fseek(f.ff,sizeof(unsigned char),SEEK_CUR);
                fread(&ino,sizeof(ino_t),1,f.ff);
                fgets(lbuf,MAXPATHLEN-1,f.ff);
                if(ino==inode){
                    i++;
                    arr[sz++]=lc;
                    if(sz>arrs){
                        arrs+=64*sizeof(size_t);
                        arr=realloc(arr,arrs);
                    }
                    fputs(ltp,fp);
                }
            }
            lc=lcb;
            fseek(f.ff,sp,SEEK_SET);
		}

		fwrite(&type,sizeof(unsigned char),1,fp);
		fwrite(&(st.st_mode),sizeof(mode_t),1,fp);
		fwrite(&(st.st_atim),sizeof(struct timespec),1,fp);
		fwrite(&(st.st_mtim),sizeof(struct timespec),1,fp);

		if(type==DT_REG){
            cnt=flen(buf);
            fwrite(&cnt,sizeof(size_t),1,fp);
            af=fopen(buf,"r");
            do{
                c=fgetc(af);
                if(c==EOF) break;
                fputc(c,fp);
            }while(1);
            fclose(af);
		}else if(type==DT_LNK){
            if(lbuf[0]) memset(lbuf,0,MAXPATHLEN);
            cnt=readlink(buf,lbuf,MAXPATHLEN-1);
            fwrite(&cnt,sizeof(size_t),1,fp);
            fputs(lbuf,fp);
		}else{
            puts("Error: In htar files; there can be only files, directories, hard links and symbolic links.");
            EXIT(1);
		}
	}while(1);
	EXIT(0);
}
