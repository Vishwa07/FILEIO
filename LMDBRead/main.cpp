#include<iostream>
#include<ctime>
#include <sys/types.h>
#include <sys/stat.h>   
#include<vector>
#include <string.h>
#include<cstdlib>
#include <tbb/tbb.h>
#include <vector>
#include "mdb_single_db.h"
#include<algorithm>

using namespace tbb;


int main(int argc, char * argv[])
{
    if (argc < 4)
	   {
        std::cerr << "path,sizeinMB,numberofImage"<< std::endl;
        return 1;

       }
	std::string path(argv[1]);
	path = path + "/master.mdb";


	int numberofImage=0;
	numberofImage = atoi(argv[3]);

    int size=0;
	size = atoi(argv[2]);
	char ch;
	std::cout<<"attach to vtune and press any char ";
	std::cin>>ch;
	mdb_single_db masterDb;
	//60GB map size 
	size_t mapsize = 60UL*1024*1024*1024;
	masterDb.init_env(path.c_str(),mapsize);
    //parallel_for(blocked_range<size_t>(0,n), ApplyFoo(a));
	std::list<std::unique_ptr<unsigned char>> l;
    std::list<unsigned long> keys;
    for(int i=1;i<=numberofImage;i++)
    {
		int val = rand()%1999;
        keys.push_back(val);
    }

	
	clock_t time = std::clock();
    masterDb.get(keys,l);
	int count =0;
	/*for(auto iterator=l.begin();iterator!=l.end();iterator++)
	{
	
		unsigned char *p = (*iterator).get();
		if(p==NULL)
		{
			std::cout<<"error occurred retrieving ";
			continue;
		
		}
		int size =0;
		for(int i=0;p[i]!=NULL;i++)
		{
			size++;
		}
		std::cout<<size<<"\n";

	   	size = 0;
	}*/
	time = std::clock() - time;
	std::cout<<"Total Time in ms:"<<((double)time/CLOCKS_PER_SEC)*1000<<std::endl;

	masterDb.close();

 return 0;   
}