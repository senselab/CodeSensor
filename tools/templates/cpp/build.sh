#@lang=cpp
rm -f code.cpp
cp code code.cpp

if [ "$1" = "opt" ]; then
	g++ -g3 -std=gnu++11 -O3 code.cpp
else
	g++ -g3 -std=gnu++11 -O0 code.cpp
fi
