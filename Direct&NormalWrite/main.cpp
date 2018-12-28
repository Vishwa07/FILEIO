#include<iostream>
#include<memory>
#include<ctime>
#include<cstring>
#include<cstdlib>
#include <string.h>
 #include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include<algorithm>
using namespace std;
using namespace std::chrono;

#define O_DIRECT_FILE 1
int main(int argc, char const *argv[])
{



	  if (argc < 4)
	   {
		   
        std::cerr << "Datapath,SizeToWriteInMB,iteration" << std::endl;
        return 1;
	   }

    std::string path(argv[1]);
	path = path + "/master.mdb";


	unsigned long size = 0;
	size = (unsigned long)atoi(argv[2]);
    size=size*1024UL*1024;
	
    
    int iteration=0;
	iteration = atoi(argv[3]);

   // cout<<"size to write is: "<<size;
    high_resolution_clock clock;
    auto startTime = clock.now();
    
    
        int fd = -1;

 /*char ch;
 cout<<"Attach to vtune:";
 cin>>ch;*/
#ifdef O_DIRECT_FILE
    if ((fd = open(path.c_str(),O_CREAT|O_RDWR|O_DIRECT, 0775)) == -1)
        {
            return 1;
        }
#else
    if ((fd = open(path.c_str(),O_CREAT|O_RDWR, 0775)) == -1)
        {
            return 1;
        }
#endif


//posix_fallocate(fd,0,iteration*size);
   // lseek(fd, mapSize-1, SEEK_SET);
    //write(fd, "", 1);
    void *buffer = NULL;
#ifdef O_DIRECT_FILE
    posix_memalign(&buffer,sysconf(_SC_PAGESIZE),size) ; 
#else
    buffer = calloc(size,1);
#endif

    //
    int rc;
    for(int i=0;i<iteration;i++)
    {
     //clock_t time1 = std::clock();
/*
On Linux, write(),read() (and similar system calls) will transfer at most 0x7ffff000 (2,147,479,552) bytes, 
returning the number of bytes actually transferred. 
(This is true on both 32-bit and 64-bit systems.)
*/
//for size more than 1.99 GB need to split the buffer , not yet implemented 
     rc= write(fd, buffer,size);
     //cout<<"wrote "<< rc <<" bytes"<<endl;
     if(rc<0)
     {

         cerr<<"error occured in wrting...";
         return 1;
     }

   // time1 = std::clock() - time1;
 //   cout<<"Total Time in ms:"<<((double)time1/CLOCKS_PER_SEC)*1000<<std::endl;


    }
    fsync(fd);
    free(buffer);
    buffer = NULL;
    close(fd);
    
    //time = std::clock() - time;
    //cout<<"Total Time in ms:"<<((double)time/CLOCKS_PER_SEC)*1000<<std::endl;
    auto endTime = clock.now();
    auto deltaT = endTime - startTime;
    auto totalTime = std::max(1L,duration_cast<milliseconds>(deltaT).count());
    
    long totalSize = (size * iteration)/(1024*1024);
    cout << "Wrote " << totalSize << "MB in " << totalTime << "ms, chunkSz = " << size/(1024*1024) << "MB, " <<"rate = " << (totalSize*1000)/totalTime << "MB/S" << std::endl;
    
    
    return 0;
}

