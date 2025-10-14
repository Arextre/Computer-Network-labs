rm ./logs/*.txt
cd ./build
cmake ..
make
cd ..
./bin/stop_wait > ./logs/terminal_log.txt