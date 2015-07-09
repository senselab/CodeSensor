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
 
long long int NumberOfInversions( vector<int>& array)
{
	int k, m, t;
	long long int cnt = 0;
	int length = array.size();	


	for ( k =1; k < length; k++) {
		t = array[k];		
		for ( m = k-1; m>=0; m--) {
			if ( array[m] > t) {
				array[m+1] = array[m];
				cnt++;
			}
			else
				break;
		}
		array[m+1] = t;
	}

	

	return cnt;
}


int main()
{
	
	
	FILE *fpIn, *fpOut;
	char *pTok, c;
	int k, p=2, num_tests;
	vector<int> NUMBERS;
 
	msgpack::sbuffer sbuf;

 

	msgpack::unpacked result;
	struct stat st;	
	size_t off = 0;

 
	stat("input.txt", &st);
	

	char* buf = new char[st.st_size];

	fpIn = fopen("input.txt", "rb");
	
	fread(buf, st.st_size, 1, fpIn );
	fclose(fpIn);
		
	msgpack::unpack(result, buf, st.st_size, off);
	result.get().convert(&num_tests);

	printf("num_tests = %d\n", num_tests);

	

	for ( k = 0; k < num_tests; k++) {
		msgpack::unpack(result, buf, st.st_size, off);
		result.get().convert(&NUMBERS);
		long long int n_inv = NumberOfInversions(NUMBERS);

		printf("length = %lu,  inversion = %lld\n", NUMBERS.size(), n_inv);

		msgpack::pack(&sbuf, n_inv);

	}

	assert(off == st.st_size);

	fpOut = fopen("output.txt", "wb");

	fwrite(sbuf.data(), sbuf.size(),1, fpOut);

	if(fpOut)
		fclose(fpOut);

	delete[] buf;

	return 0;
}
