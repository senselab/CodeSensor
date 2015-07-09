#include <cstdio>
#include <vector>
#include <algorithm>
#include <utility>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <msgpack.hpp>
#include <msgpack/fbuffer.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

using namespace std;



int main()
{
	
	int k,   num_tests;
	vector<int> NUMBERS;
	char* input_buf;
	FILE* fpOut;	
	msgpack::fbuffer output_buf(fpOut = fopen("output.txt","wb"));
	msgpack::unpacked result;
	struct stat st;	
	size_t off = 0;

	int fdIn = open("input.txt", O_RDONLY);

	fstat(fdIn, &st);
	printf("input.txt size: %lu\n", (uint64_t)st.st_size);

	input_buf = (char*)mmap(NULL, st.st_size, PROT_READ, MAP_SHARED|MAP_NORESERVE, fdIn, 0);
		
	msgpack::unpack(result, input_buf, st.st_size, off);
	result.get().convert(&num_tests);

	printf("num_tests = %d\n", num_tests);
	

	for ( k = 0; k < num_tests; k++) {
		msgpack::unpack(result, input_buf, st.st_size, off);
		result.get().convert(&NUMBERS);
		sort(NUMBERS.begin(), NUMBERS.end());

		msgpack::pack(&output_buf, NUMBERS);

	}

	munmap(input_buf, st.st_size);
	close(fdIn);
	fclose(fpOut);

	return 0;
}
