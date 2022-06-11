# slab-memory-managment
This repo is a simple implementation of Slab memory managment. Slab memory managment is a kernel space Memory management to handle Memory requests of Operating Systems' objects. 
Operating Systems' objects are locks, semaphor, files, processes' information and etc. 

## Run
```bash
cmake -S . -B ./build
cd ./build
make 
./slab
```
