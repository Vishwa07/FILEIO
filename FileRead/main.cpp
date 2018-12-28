#include<iostream>
#include<memory>
#include<cstdlib>
#include<string.h>
#include <sys/mman.h>
#include <stdio.h>
#include<fcntl.h>
#include<ctime>
#include<map>
#include <unistd.h>
#include<vector>
#include <chrono>
#include <sys/stat.h>

#include <algorithm>
using namespace std;
using namespace std::chrono;


//#define MMAP 1
#define O_DIRECT_FILE
int main(int argc, char const *argv[])
{
    

    struct stat sb;

	  if (argc < 4)
	   {
		   
        std::cerr << "Datapath,SizeToWriteInMB,iteration,1/0 for Seq/Ran" << std::endl;
        return 1;
	   }
	
    std::string path(argv[1]);
	path = path + "/master.mdb";
	unsigned long size = 0;
	size = (unsigned long)atoi(argv[2]);
    size=size*1024UL*1024;
	int iteration=0;
	iteration = atoi(argv[3]);
    int fd = -1;

     high_resolution_clock clock;
    auto startTime = clock.now();

    if ((fd = open(path.c_str(),O_CREAT|O_RDONLY|O_DIRECT, 0775)) == -1)
        {
            return 1;
        }
        int rc = fstat(fd,&sb);
      #ifdef MMAP  
        unsigned long mapSize = sb.st_size;
        unsigned char *ptr = reinterpret_cast<unsigned char*> (mmap64(NULL,mapSize,PROT_READ, MAP_SHARED,fd,0));
        
        //cache of 256 Gb is almost filling up and swap process comes and the read runs at very slow rate 
        //madvise(ptr,mapSize,MADV_SEQUENTIAL|MADV_WILLNEED);

        unsigned char * header = ptr;
        void *buff = NULL;
        buff = calloc(size,1); 


        clock_t time = std::clock();
        unsigned long offset=0;
        for(int i=0;i<iteration;i++)
            {
                //clock_t time1 = std::clock();

                memcpy(buff,ptr,size);
                ptr=ptr+size;
               // time1 = std::clock() - time1;
                //cout<<"Time for " <<i+1<<" in ms:"<<((double)time1/CLOCKS_PER_SEC)*1000<<std::endl;

            } 
   
         rc = munmap(header,mapSize);
         ptr= NULL; 
         header = NULL;
        if(rc!=0)
        {
            cout<<"unmap failed"<<rc;
        }
        #else
        //posix_fadvise(fd,0,sb.st_size,POSIX_FADV_SEQUENTIAL|POSIX_FADV_WILLNEED);
        int bytes_read=0;
        void *buff = NULL;
       #ifdef O_DIRECT_FILE
            posix_memalign(&buff,sysconf(_SC_PAGESIZE),size) ; 
        #else
            buff = calloc(size,1);
         #endif
        for(int i=0;i<iteration;i++)
                    {
                        //clock_t time1 = std::clock();
                        /*
On Linux, write(),read() (and similar system calls) will transfer at most 0x7ffff000 (2,147,479,552) bytes, 
returning the number of bytes actually transferred. 
(This is true on both 32-bit and 64-bit systems.)
*/
//for size more than (2GB minus 4KB) need to split the buffer , not yet implemented 
                        bytes_read = read(fd, buff, size);
                        if(size<2147479552L && bytes_read!=size)
                        {
                            
                            cout<<"Error reading ..."<< i+1<<" Read only "<<bytes_read <<endl;

                        }
                        //time1 = std::clock() - time1;
                        //cout<<"Time for " <<i+1<<" in ms:"<<((double)time1/CLOCKS_PER_SEC)*1000<<std::endl;

                    } 
   

        #endif
    
    close(fd);
    auto endTime = clock.now();
    auto deltaT = endTime - startTime;
    auto totalTime = std::max(1L,duration_cast<milliseconds>(deltaT).count());
    
    long totalSize = (size * iteration)/(1024*1024);
    cout << "read " << totalSize << "MB in " << totalTime << "ms, chunkSz = " << size/(1024*1024) << "MB, " <<"rate = " << (totalSize*1000)/totalTime << "MB/S" << std::endl;
    
    
    return 0;
}
