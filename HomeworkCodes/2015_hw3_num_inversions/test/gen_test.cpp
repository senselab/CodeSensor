#include <cstdio>
#include <vector>
#include <algorithm>
#include <utility>
#include <string.h>
#include <msgpack.hpp>
#include <msgpack/fbuffer.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

using namespace std;


msgpack::sbuffer sbuf;

typedef int (*tSourceFunc)(int);

int identical_v;

int Random(int index)
{
	return rand();
}

int Inverse(int index)
{
	return -index;
}

int Identical(int index)
{
	return identical_v;
}

int AlmostIdentical(int index)
{
	if (rand() < RAND_MAX/5)
		return rand();
	else
		return identical_v;
}


void GenTestData(int L, tSourceFunc f)
{

	vector<int> numbers;

	int k, i;

	
	numbers.resize(0);

	int length = L;

	if ( length < 2)	length = 2;

	printf("len = %d\n", length);

	for ( i = 0; i < length; i++) {
		int v = (*f)(i);
		numbers.push_back(v);
	}

	msgpack::pack(&sbuf, numbers);

}


void RealTestData()
{
	int N = 11;

	srand(765761223);	

	msgpack::pack(&sbuf, N);	

	identical_v = rand();
	GenTestData(2, Random);
	GenTestData(2, Inverse);	
	GenTestData(2, Identical);	

	identical_v = rand();
	GenTestData(100, Random);
	GenTestData(100, Inverse);
	GenTestData(100, Identical);	

	identical_v = rand();
	GenTestData(10000, Random);
	GenTestData(10000, Inverse);
	GenTestData(10000, Identical);	
	GenTestData(10000, AlmostIdentical);	

	identical_v = rand();
	GenTestData(500000, Random);
/*	GenTestData(1000000, Inverse);
	GenTestData(1000000, Identical);	
	GenTestData(1000000, AlmostIdentical);	

	GenTestData(50000000, Random);*/
}

int main()
{
	FILE *fp;
	fp = fopen("input.txt", "wb");

	RealTestData();

	fwrite(sbuf.data(), sbuf.size(),1, fp);

	fclose(fp);

	return 0;

}
