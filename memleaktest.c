


#define _GNU_SOURCE
#include<link.h>
#include <dlfcn.h>
#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>

int flag=1;

#if 0
void* nmalloc(size_t size){
    void* p=malloc(size);
    printf("nmalloc:%p,size:%ld\n",p,size);
    return p;
}

void nfree(void* ptr){
    printf("nfree:%p\n",ptr);
    return free(ptr);
}


#else
#endif

#if 0
void* nmalloc(size_t size){
    void* p=malloc(size);

    char buff[128]={0};
    snprintf(buff,128,"./mem/%p.mem",p);

    FILE* fp=fopen(buff,"w");
    if(!fp){
        free(p);
        return NULL;
    }
    fprintf(fp,"[+]addr:%p,size:%ld\n",p,size);
    fflush(fp);
    fclose(fp);
    //printf("nmalloc:%p,size:%ld\n",p,size);
    return p;
}

void nfree(void* ptr){
    //printf("nfree:%p\n",ptr);
    char buff[128]={0};
    snprintf(buff,128,"./mem/%p.mem",ptr);
    if(unlink(buff)<0){
        printf("double free:%p",ptr);
        return;
    }
    return free(ptr);
}

#else

#endif
#if 1

void* nmalloc(size_t size,const char* filename,int line){
    void* p=malloc(size);
    if(flag){
        char buff[128]={0};
        snprintf(buff,128,"./mem/%p.mem",p);

        FILE* fp=fopen(buff,"w");
        if(!fp){
            free(p);
            return NULL;
        }
        fprintf(fp,"[+]%s:%d,addr:%p,size:%ld\n",filename,line,p,size);
        fflush(fp);
        fclose(fp);
    }
    //printf("nmalloc:%p,size:%ld\n",p,size);
    return p;
}

void nfree(void* ptr){
    //printf("nfree:%p\n",ptr);
    if(flag){
        char buff[128]={0};
        snprintf(buff,128,"./mem/%p.mem",ptr);
        if(unlink(buff)<0){
            printf("double free:%p",ptr);
            return;
        }
    }
    return free(ptr);
}
#define malloc(size) nmalloc(size,__FILE__,__LINE__)
#define free(ptr) nfree(ptr)

#else


//hook

typedef void *(*malloc_t)(size_t size);
malloc_t malloc_f=NULL;
typedef void (*free_t)(void* ptr);
free_t free_f=NULL;

int enable_malloc=1;
int enable_free=1;

void *ConvertToELF(void *addr) {

	Dl_info info;
	struct link_map *link;
	
	dladdr1(addr, &info, (void **)&link, RTLD_DL_LINKMAP);

	return (void *)((size_t)addr - link->l_addr);
}


void* malloc(size_t size){
     void* p=NULL;
     if(enable_malloc){
        enable_malloc=0;
        p=malloc_f(size);
        void* finder=__builtin_return_address(0);
        char buff[128] = {0};
		sprintf(buff, "./mem/%p.mem", p);

		FILE *fp = fopen(buff, "w");
		if (!fp) {
			free(p);
			return NULL;
		}

		//fprintf(fp, "[+]%p, addr: %p, size: %ld\n", caller, p, size);
		fprintf(fp, "[+]%p, addr: %p, size: %ld\n", ConvertToELF(finder), p, size);
		fflush(fp);
		
		enable_malloc = 1;
		
	} else {
		p = malloc_f(size);
	}
        return p;

}

void free(void* ptr){
    if (enable_free) {
		enable_free = 0;

		char buff[128] = {0};
		snprintf(buff, 128, "./mem/%p.mem", ptr);

		if (unlink(buff) < 0) {
			printf("double free: %p", ptr);
			return ;
		}

		free_f(ptr);

		enable_free = 1;
	} else {
		free_f(ptr);
	}

	return ;
}

void init_hook(void){
    if(!malloc_f){
        malloc_f=(malloc_t)dlsym(RTLD_NEXT, "malloc");
    }
    if(!free_f){
        free_f=(free_t)dlsym(RTLD_NEXT,"free");
    }
}


#endif



int main(){

    //init_hook();

    void *p1=malloc(5);
    void *p2=malloc(10);
    void *p3=malloc(15);

    free(p1);
    free(p3);

    
    
}