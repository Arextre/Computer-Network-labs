# clean old logs and build
rm -r ./logs
rm -r ./build

# cmake & build
mkdir -p ./build
cd ./build
cmake ..
make
cd ..

# run
mkdir -p ./logs
./bin/stop_wait > ./logs/terminal_log.txt