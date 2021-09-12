#@lang=cpp_shm
rm -f code.cpp
cp code code.cpp

if [ "$1" = "opt" ]; then
	g++ -g3 -std=gnu++11 -O3 shm_loader.cpp code.cpp -lrt
else
	g++ -g3 -std=gnu++11 -O0 shm_loader.cpp code.cpp -lrt
fi
