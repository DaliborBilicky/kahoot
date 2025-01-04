rm -rf ./server/build
mkdir ./server/build
cd ./server/build/
cmake ..
make 
cd ../../
./server/build/src/kahoot_server
