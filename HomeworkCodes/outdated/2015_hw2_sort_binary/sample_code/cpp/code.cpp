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


void GenTestData()
{
	FILE *fp;
	msgpack::sbuffer sbuf;
	vector<int> numbers;
	int N = 5, L=20000;
	int k, i;

	srand(time(0));

	msgpack::pack(&sbuf, N);


	for ( k =0; k < N; k++) {
		numbers.resize(0);
		
		for ( i = 0; i < L; i++) {
			int v = rand();
			numbers.push_back(v);
	//		fprintf(fpTxt, "%d ", v);
		}

		msgpack::pack(&sbuf, numbers);
	//	fprintf(fpTxt, "\n");
	}

	fp = fopen("input.txt", "wb");

	fwrite(sbuf.data(), sbuf.size(),1, fp);
	fclose(fp);
	//fclose(fpTxt);
}

int main()
{
	
	
	FILE *fpIn, *fpOut;
	char *pTok, c;
	int k, p=2, num_tests;
	vector<int> NUMBERS;
	msgpack::sbuffer sbuf;

	GenTestData();
	return 0;

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
		sort(NUMBERS.begin(), NUMBERS.end());

		msgpack::pack(&sbuf, NUMBERS);

	}

	assert(off == st.st_size);

	fpOut = fopen("output.txt", "wb");

	fwrite(sbuf.data(), sbuf.size(),1, fpOut);

	if(fpOut)
		fclose(fpOut);

	delete[] buf;

	return 0;
}
