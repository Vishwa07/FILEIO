#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <iostream>
#include <stdexcept>
#include "mdb_single_db.h"



int mdb_single_db::mdb_cmp_long(const MDB_val *a, const MDB_val *b)
{
	return (*(unsigned long *)a->mv_data < *(unsigned long *)b->mv_data) ? -1 :
		*(unsigned long *)a->mv_data > *(unsigned long *)b->mv_data;
}


mdb_single_db::mdb_single_db ( )
{
}

mdb_single_db::~mdb_single_db ( )
{
	close ( );
}

void mdb_single_db::close ( )
{
	//TODO: need fence here
	if (mdbEnv && isInitialized)
	{
		for (auto i=0; i<db_count; i++)
			mdb_dbi_close(mdbEnv, mdbDb[i]);
		mdb_env_close(mdbEnv);

		mdbEnv = nullptr;
		isInitialized = false;
	}
}

void mdb_single_db::init_env(const char *path, size_t maxMapSize )
{
	MDB_env *env = nullptr;

	if (isInitialized == false && mdb_env_create(&env) == 0)
	{
		// Set environment parameters
		// Maximum size of each file
		mdb_env_set_mapsize(env, maxMapSize);
		// Maximum number of "databases" inside the environment
		// We only have 2 (env-db) for our use-case (for master).
		// The segment db's only use the default (one).
		mdb_env_set_maxdbs(env, MAX_DBS);

		// Create the data file, no sub-directories so files are created
		// directly.
		// MDB_WRITEMAP - use writeable memory (+p)
		// MDB_NOSYNC - don't flush system buffers when committing a transaction (+p, -d)
		// MDB_MAPASYNC - Use async flush to disk (+p, -d)
//		auto rc = mdb_env_open(env, path, MDB_WRITEMAP|MDB_NOSUBDIR, 0);
//		auto rc = mdb_env_open(env, path, MDB_WRITEMAP|MDB_NOSUBDIR|MDB_NOSYNC|MDB_MAPASYNC, 0);
		auto rc = mdb_env_open(env, path, MDB_WRITEMAP|MDB_NOSUBDIR|MDB_NOSYNC|MDB_NOMETASYNC, 0666);
		if (rc != 0)
		{
			// Error, environment was never opened so no need to close
			throw_error(rc);
		}

		// lmdb environment is now valid, save it since init_db uses it too
		mdbEnv = env;
		isInitialized = true;
		// Create the default database (inside the environment)
		auto default_pos = init_db(nullptr);
	}
}

int mdb_single_db::init_db ( const char * db_name, open_flags flags )
{
	int db_pos = 0;

	// LMDB scheme is to have an environment containing one or more
	// DB's inside (the one big file).
	if (db_count < MAX_DBS)
	{
		MDB_txn *txn; 
		auto rc = mdb_txn_begin(mdbEnv, nullptr, 0, &txn);
		if (rc != 0)
		{
			// Failed, close the environment
			mdb_env_close(mdbEnv);

			throw_error(rc);
		}

		MDB_dbi db;
		unsigned int mdb_flags;
		if (flags & open_flags::allow_dupe_keys)
		{
			mdb_flags = MDB_CREATE | MDB_DUPSORT | MDB_INTEGERDUP;
		}
		else
		{
			// mdb_flags = MDB_CREATE | MDB_INTEGERKEY - lmdb limitation, cannot have integer key in maindb if using named db's(?);
			mdb_flags = MDB_CREATE;
		}

		rc = mdb_dbi_open(txn, db_name, mdb_flags, &db); 
		if (rc != 0)
		{
			// Failed, close the environment and
			// abort the transaction
			mdb_txn_abort(txn);
			//mdb_env_close(env);

			throw_error(rc);
		}

		MDB_cmp_func *fun = &mdb_cmp_long;
		rc = mdb_set_compare(txn,db,fun);

		if (rc != 0)
		{
			throw_error(rc);
		}

		// Commit the open/create
		rc = mdb_txn_commit(txn);
		if (rc != 0)
		{
			throw_error(rc);
		}

		// All good, save the values
		mdbDb[db_count] = db;
		db_pos = db_count;
		db_count++;
	}
	else
	{
		throw_error(1);
	}

	return (db_pos);
}

bool mdb_single_db::get ( unsigned long k, int &outData )
{
	return get(0, k, outData);
}

bool mdb_single_db::get ( db_handle hdb, unsigned long k, int &outData )
{
	bool found = false;

	MDB_txn *txn; 
	auto rc = mdb_txn_begin(mdbEnv, nullptr, 0, &txn);
	if (rc != 0)
	{
		throw_error(rc);
	}

	MDB_val key;
	MDB_val data;

	key.mv_size = sizeof(unsigned long);
	key.mv_data = &k;

	rc = mdb_get(txn, mdbDb[hdb], &key, &data);
	if (rc == 0)
	{
		if (data.mv_size == sizeof(int))
		{
			outData = *(reinterpret_cast<int *>(data.mv_data)); 

			mdb_txn_commit(txn);
			found = true;
		}
		else
		{
			mdb_txn_abort(txn);
			throw_error(EINVAL);
		}
	}
	else if (rc != MDB_NOTFOUND)
	{
		mdb_txn_abort(txn);
		throw_error(rc);
	}
	else
	{
		// Not found, we still have to release the transaction
		mdb_txn_commit(txn);
	}

	return (found);
}

bool mdb_single_db::get ( unsigned long k, std::vector<unsigned char> &outData )
{
	return get(0, k, outData);
}

bool mdb_single_db::get ( db_handle hdb, unsigned long k, std::vector<unsigned char> &outData )
{
	bool found = false;

	MDB_txn *txn; 
	auto rc = mdb_txn_begin(mdbEnv, nullptr, 0, &txn);
	if (rc != 0)
	{
		throw_error(rc);
	}

	MDB_val key;
	MDB_val data;
	key.mv_size = sizeof(unsigned long );

	key.mv_data = &k;

	rc = mdb_get(txn, mdbDb[hdb], &key, &data);
	if (rc == 0) 
	{
		std::vector<unsigned char> v(reinterpret_cast<unsigned char*>(data.mv_data), reinterpret_cast<unsigned char*>(data.mv_data)+data.mv_size);
		outData = std::move(v);

		mdb_txn_commit(txn);
		found = true;
	}
	else if (rc != MDB_NOTFOUND)
	{
		mdb_txn_abort(txn);
		throw_error(rc);
	}
	else
	{
		// Not found, we still have to release the transaction
		mdb_txn_commit(txn);
	}

	return (found);
}

bool mdb_single_db::get (std::list<unsigned long> k ,std::list<std::unique_ptr<unsigned char>> &outdata)
{
	bool found = false;
	db_handle hdb = 0;
	MDB_txn *txn; 
	auto rc = mdb_txn_begin(mdbEnv, nullptr, 0, &txn);
	if (rc != 0)
	{
		throw_error(rc);
	}

	MDB_cursor *cursor;


	mdb_cursor_open(txn, mdbDb[hdb], &cursor);
	std::list<unsigned long>::const_iterator iterator;	

	for (iterator = k.begin(); iterator != k.end(); ++iterator) {
	MDB_val key;
	MDB_val data;
	key.mv_size = sizeof(unsigned long );
	clock_t time = std::clock();

    unsigned long key1 = *iterator;
	key.mv_data = &key1;
	int rc= mdb_cursor_get(cursor, &key, &data, MDB_SET);
	if (rc == 0) 
	{
//		std::vector<unsigned char> v(reinterpret_cast<unsigned char*>(data.mv_data), reinterpret_cast<unsigned char*>(data.mv_data)+data.mv_size);
		unsigned char* ptr = reinterpret_cast<unsigned char*>(data.mv_data );
		if(ptr==NULL)
		{

			std::cout<<"Null pointer for record "<<*iterator;
		}
		outdata.push_back(std::unique_ptr<unsigned char>(ptr));

		found = true;
	}
	else if (rc == MDB_NOTFOUND)
	{
		std::cout<<"Not found";
		//mdb_txn_abort(txn);
		//throw_error(rc);
	}
	else
	{
				std::cout<<"unknown error occured";

		// Not found, we still have to release the transaction
	}
	time = std::clock() - time;
	std::cout<<"Total Time for read "<<*iterator<<" in ms:"<<((double)time/CLOCKS_PER_SEC)*1000<<std::endl;
	if(!found)
	{

		std::cout<<"Didnt find key"<<*iterator<<"\n";
	}
	}

		mdb_txn_commit(txn);

	
	return (found);
}



bool mdb_single_db::take_first ( unsigned long k, int &outData )
{
	return take_first(0, k, outData);
}

bool mdb_single_db::take_first (db_handle hdb, unsigned k, int &outData )
{
	// Search for the first matching record with key 'k.'
	// Return and delete the record.
	// Note: it's not guaranteed that this will return unique
	// values (two different processes may get the same record).
	// This is due to the MVCC used by lmdb. For what we use it for
	// it should be okay since the returned segment file will just
	// be shared by the two processes.
	bool found = false;

	MDB_txn *txn; 
	auto rc = mdb_txn_begin(mdbEnv, nullptr, 0, &txn);
	if (rc != 0)
	{
		throw_error(rc);
	}

	MDB_val key;
	MDB_val data;

	key.mv_size = sizeof(unsigned long);
	key.mv_data = &k;

	rc = mdb_get(txn, mdbDb[hdb], &key, &data);
	if (rc == 0)
	{
		if (data.mv_size == sizeof(int))
		{
			outData = *(reinterpret_cast<int *>(data.mv_data)); 

			// We got a match, delete the matching key/record combo
			mdb_del(txn, mdbDb[hdb], &key, &data);

			mdb_txn_commit(txn);
			found = true;
		}
		else
		{
			mdb_txn_abort(txn);
			throw_error(EINVAL);
		}
	}
	else if (rc != MDB_NOTFOUND)
	{
		mdb_txn_abort(txn);
		throw_error(rc);
	}
	else
	{
		// Not found, we still have to release the transaction
		mdb_txn_commit(txn);
	}

	return (found);
}

bool mdb_single_db::put_record ( unsigned long k, unsigned v )
{
	return put_record(0, k, v);
}

bool mdb_single_db::put_record ( db_handle hdb, unsigned long k, unsigned v )
{
	unsigned char *rv = (unsigned char*)&v;

	std::vector<unsigned char> bits(rv, rv+sizeof(unsigned));

	return put_record(hdb, k, bits,1);
}

bool mdb_single_db::put_record ( unsigned long k, std::vector<unsigned char> &bits,int iteration )
{
	return put_record(0, k, bits,iteration);
}


bool mdb_single_db::put_record ( db_handle hdb, unsigned long k, std::vector<unsigned char> &bits,int iteration  )
{
	bool storageFullRetry = false;

	MDB_txn *txn; 
	auto rc = mdb_txn_begin(mdbEnv, nullptr, 0, &txn);
	if (rc != 0)
	{
		throw_error(rc);
	}

	MDB_val key;
	MDB_val data;


	for(int i =0;i<iteration;i++)
	{
		if(i!=0)
			k++;
			key.mv_size = sizeof(unsigned long);
			key.mv_data = &k;
			data.mv_size = bits.size();
			data.mv_data = bits.data();
			clock_t indtime = std::clock();
			rc = mdb_put(txn, mdbDb[hdb], &key, &data,MDB_APPEND);
			indtime = std::clock()- indtime;
			std::cout<<"Total Time for key " <<k<<" in ms:"<<((double)indtime/CLOCKS_PER_SEC)*1000<<std::endl;

	}
	if (rc == MDB_MAP_FULL)
	{
		// Special condition, ran out of space, open a new store
		// Abort the current transaction
		mdb_txn_abort(txn);
		std::cout<<"Storage full.....";
		storageFullRetry = true;
	}
	else if (rc != 0)
	{
		mdb_txn_abort(txn);
		throw_error(rc);
	}
	else
	{
		rc = mdb_txn_commit(txn);
		
		if (rc != 0)
		{
			throw_error(rc);
		}
	}

	return (storageFullRetry);
}

void mdb_single_db::delete_record ( unsigned long k )
{
	delete_record(0, k);
}

void mdb_single_db::delete_record ( db_handle hdb, unsigned long k )
{
	MDB_txn *txn; 
	auto rc = mdb_txn_begin(mdbEnv, nullptr, 0, &txn);
	if (rc != 0)
	{
		throw_error(rc);
	}

	MDB_val key;

	key.mv_size = sizeof(unsigned long );
	key.mv_data = &k;

	rc = mdb_del(txn, mdbDb[hdb], &key, nullptr);
	if (rc != 0)
	{
		mdb_txn_abort(txn);
		throw_error(rc);
	}

	rc = mdb_txn_commit(txn);
	if (rc != 0)
	{
		throw_error(rc);
	}
}

void mdb_single_db::throw_error ( int rc )
{
	std::string message("Something went wrong.");
	char sss[128];
	snprintf (sss,128,"Something went wrong %d", rc);
	std::cerr << (sss);

	switch (rc)
	{
	case MDB_VERSION_MISMATCH:
		break;
	case MDB_INVALID:
		message = "Invalid or corrupt data file";
		break;
	case MDB_PANIC:
		break;
	case MDB_MAP_RESIZED:
		break;
	case MDB_READERS_FULL:
		break;
	case MDB_TXN_FULL:
		break;
	case ENOMEM:
		break;
	case ENOENT:
		break;
	case EACCES:
		break;
	case EAGAIN:
		break;
	case EINVAL:
		break;
	case ENOSPC:
		break;
	case EIO:
		break;
		//VM
//	case ERROR_DISK_FULL:
	//	break;
	default:
		break;
	}
	//VM
	//throw runtime_error(message.c_str());
}
