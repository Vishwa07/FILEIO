./FileWrite.exe /storage/LMDBTPT/Data/ 2048 512
sync
echo 3 > /proc/sys/vm/drop_caches
./FileRead.exe /storage/LMDBTPT/Data/ 2048 512
rm /storage/LMDBTPT/Data/master.mdb
sync
echo 3 > /proc/sys/vm/drop_caches
./FileWrite.exe /storage/LMDBTPT/Data/ 1024 1024
sync
echo 3 > /proc/sys/vm/drop_caches
./FileRead.exe /storage/LMDBTPT/Data/ 1024 1024
rm /storage/LMDBTPT/Data/master.mdb
sync
echo 3 > /proc/sys/vm/drop_caches
./FileWrite.exe /storage/LMDBTPT/Data/ 512 2048
sync
echo 3 > /proc/sys/vm/drop_caches
./FileRead.exe /storage/LMDBTPT/Data/ 512 2048
rm /storage/LMDBTPT/Data/master.mdb
sync
echo 3 > /proc/sys/vm/drop_caches
./FileWrite.exe /storage/LMDBTPT/Data/ 256 4096
sync
echo 3 > /proc/sys/vm/drop_caches
./FileRead.exe /storage/LMDBTPT/Data/ 256 4096
rm /storage/LMDBTPT/Data/master.mdb
sync
echo 3 > /proc/sys/vm/drop_caches
./FileWrite.exe /storage/LMDBTPT/Data/ 128 8192
sync
echo 3 > /proc/sys/vm/drop_caches
./FileRead.exe /storage/LMDBTPT/Data/ 128 8192
rm /storage/LMDBTPT/Data/master.mdb
sync
echo 3 > /proc/sys/vm/drop_caches
./FileWrite.exe /storage/LMDBTPT/Data/ 64 16384
sync
echo 3 > /proc/sys/vm/drop_caches
./FileRead.exe /storage/LMDBTPT/Data/ 64 16384
rm /storage/LMDBTPT/Data/master.mdb
sync
echo 3 > /proc/sys/vm/drop_caches
./FileWrite.exe /storage/LMDBTPT/Data/ 32 32768
sync
echo 3 > /proc/sys/vm/drop_caches
./FileRead.exe /storage/LMDBTPT/Data/ 32 32768
rm /storage/LMDBTPT/Data/master.mdb
sync
echo 3 > /proc/sys/vm/drop_caches
./FileWrite.exe /storage/LMDBTPT/Data/ 16 65536
sync
echo 3 > /proc/sys/vm/drop_caches
./FileRead.exe /storage/LMDBTPT/Data/ 16 65536
rm /storage/LMDBTPT/Data/master.mdb
sync
echo 3 > /proc/sys/vm/drop_caches
