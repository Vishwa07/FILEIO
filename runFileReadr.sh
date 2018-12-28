sync
echo 3 > /proc/sys/vm/drop_caches
./FileRead.exe /storage/LMDBTPT/Data/ 2048 512
sync
echo 3 > /proc/sys/vm/drop_caches
./FileRead.exe /storage/LMDBTPT/Data/ 1024 1024
sync
echo 3 > /proc/sys/vm/drop_caches
./FileRead.exe /storage/LMDBTPT/Data/ 512 2048
sync
echo 3 > /proc/sys/vm/drop_caches
./FileRead.exe /storage/LMDBTPT/Data/ 256 4096
sync
echo 3 > /proc/sys/vm/drop_caches
./FileRead.exe /storage/LMDBTPT/Data/ 128 8192
sync
echo 3 > /proc/sys/vm/drop_caches
./FileRead.exe /storage/LMDBTPT/Data/ 64 16384
sync
echo 3 > /proc/sys/vm/drop_caches
./FileRead.exe /storage/LMDBTPT/Data/ 32 32768
sync
echo 3 > /proc/sys/vm/drop_caches
./FileRead.exe /storage/LMDBTPT/Data/ 16 32768
sync
echo 3 > /proc/sys/vm/drop_caches
