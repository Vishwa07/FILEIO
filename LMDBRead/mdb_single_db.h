#include <vector>
#include "lmdb.h"
#include <cstdio>
#include<list>
#include <memory>

#define MAX_DBS 1
typedef int db_handle;

class mdb_single_db
{
private:
	bool isInitialized = false;
	MDB_env *mdbEnv = nullptr;
	int db_count = 0;
	MDB_dbi mdbDb[MAX_DBS];

public:
	enum open_flags
	{
		none = 0,
		allow_dupe_keys = 1
	};
	mdb_single_db ( );
	~mdb_single_db ( );

	void init_env ( const char *path, size_t maxMapSize );
	int init_db ( const char *db_name, open_flags flags = open_flags::none );
	void close ( );
	static int mdb_cmp_long(const MDB_val *a, const MDB_val *b);

	bool get ( unsigned long k, int &outData );
	bool get ( db_handle hdb, unsigned long k, int &outData );
	bool get ( unsigned long k, std::vector<unsigned char> &outData );
	bool get (std::list<unsigned long> k ,std::list<std::unique_ptr<unsigned char>>&outdata);
	bool get ( db_handle hdb, unsigned long k, std::vector<unsigned char> &outData );
	bool take_first ( unsigned long k, int &outData );
	bool take_first (db_handle hdb, unsigned k, int &outData );
	bool put_record ( unsigned long k, unsigned v );
	bool put_record ( db_handle hdb, unsigned long k, unsigned v );
	bool put_record ( unsigned long k, std::vector<unsigned char> &bits,int batchPerEntries );
	bool put_record ( db_handle hdb, unsigned long k, std::vector<unsigned char> &bits,int iteration );
	void delete_record ( unsigned long k );
	void delete_record ( db_handle hdb, unsigned long k );

private:
	void throw_error (int rc);
};

