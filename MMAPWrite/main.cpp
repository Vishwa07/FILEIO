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
using namespace std;

using namespace std::chrono;


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

    high_resolution_clock clock;
    auto startTime = clock.now();
    int fd = -1;
    if ((fd = open(path.c_str(),O_CREAT|O_RDWR, 0775)) == -1)
        {
            return 1;
        }

    //we need to see if can do this at the beginning 
    unsigned long mapSize = size*iteration;
    lseek(fd, mapSize-1, SEEK_SET);
    write(fd, "", 1);
    
    cout<<"started writing ";
    void *ptr = mmap64(NULL,mapSize,PROT_WRITE|PROT_READ, MAP_SHARED|MAP_POPULATE,fd,0);
    madvise(ptr,mapSize,MADV_SEQUENTIAL|MADV_WILLNEED);
    void *header = ptr;
    void *buff = NULL;
    buff = calloc(size,1); 

    clock_t time = std::clock();
    unsigned long offset=0;
    for(int i=0;i<iteration;i++)
    {
        clock_t time1 = std::clock();

        memcpy(ptr,buff,size);
         
        ptr=ptr+size;
        time1 = std::clock() - time1;
        cout<<"Time for " <<i+1<<" in ms:"<<((double)time1/CLOCKS_PER_SEC)*1000<<std::endl;

    }
   
   
   
    int rc=msync(header,mapSize,MS_SYNC);
    if(rc!=0)
    {
        cout<<"sync failed"<<rc;
    }

    rc = munmap(header,mapSize);

    if(rc!=0)
    {
        cout<<"unmap failed"<<rc;
    }
    
    header = NULL;
    close(fd);
    auto endTime = clock.now();
    auto deltaT = endTime - startTime;
    auto totalTime = std::max(1L,duration_cast<milliseconds>(deltaT).count());
    
    long totalSize = (size * iteration)/(1024*1024);
    cout << "read " << totalSize << "MB in " << totalTime << "ms, chunkSz = " << size/(1024*1024) << "MB, " <<"rate = " << (totalSize*1000)/totalTime << "MB/S" << std::endl;
    
    return 0;
}
