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
	int N = 5, L=50000;
	int k, i;

	srand(time(0));

	msgpack::pack(&sbuf, N);


	for ( k =0; k < N; k++) {
		numbers.resize(0);

		int length = L * (k+1)  / N;

		if ( length < 2)	length = 2;
	
		printf("len = %d\n", length);

		for ( i = 0; i < length; i++) {
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

class CFast
{
public:

    long long int count;


    CFast() {
    	count = 0;
    }

    void InsertionSort(int A[],int T[], int L, int R)
    {
        int i, j, key;
        for( i = L; i <= R; i++ )
        {
            key = A[i];
            for( j = i; (j > L) && (T[j-1] > key); j-- )
                T[j] = T[j-1];
            T[j] = key;
            count += (i - j);
        }
    }

    void Merge(int A[], int T[], int L, int M, int R)
    {
        int Lpos = L;
        int Rpos = M + 1;

        while( Lpos <= M && Rpos <= R )
            if( A[Lpos] <= A[Rpos] )
                T[L++] = A[Lpos++];
            else
            {
                count += (M - Lpos + 1);
                T[L++] = A[Rpos++];
            }

        while( Lpos <= M )
            T[L++] = A[Lpos++];
        while( Rpos <= R )
            T[L++] = A[Rpos++];
    }

    void MergeSort(int A[], int T[], int L, int R)
    {
        if( R - L < 60 )
            InsertionSort(A, T, L, R);
        else
        {
            int M = (L + R) / 2;
            MergeSort(T, A, L, M);
            MergeSort(T, A, M + 1, R);
            Merge(A, T, L, M, R);
        }
    }

    long long int NumberOfInversion(vector<int>& A)
    {
    	int k;
    	int N = A.size();
        count = 0;
        int *B = new int [N];
        int *T = new int [N];

        //memmove(B, A, sizeof(int)*N );
        //memmove(T, A, sizeof(int)*N );

        
        for (k=0; k < N; k++) 
        	B[k] = T[k] = A[k];


        MergeSort(B, T, 0, N-1);

        delete [] B;
        delete [] T;
        return count;
    }

};    

long long int NumberOfInversions_fast( vector<int>& array)
{
	CFast f;

	return f.NumberOfInversion(array);
}

long long int NumberOfInversions_bruteforce( vector<int>& array)
{
	int k, m, t;
	long long int cnt = 0;
	int length = array.size();	


	for ( k =1; k < length; k++) {
		for ( m = k-1; m>=0; m--) {
			if ( array[m] > array[k]) 
				cnt++;
		}
	}

	

	return cnt;
}



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
	//vector<long long int> r;
	msgpack::sbuffer sbuf;

//	GenTestData();
//	return 0;

	msgpack::unpacked result;
	struct stat st;	
	size_t off = 0;

//	r.resize(1);

	stat("input.txt", &st);
	

	char* buf = new char[st.st_size];

	fpIn = fopen("input.txt", "rb");
	
	fread(buf, st.st_size, 1, fpIn );
	fclose(fpIn);
		
	msgpack::unpack(result, buf, st.st_size, off);
	result.get().convert(&num_tests);

	printf("num_tests = %d\n", num_tests);

	int *ptr = (int*)num_tests;
	*ptr = 0;
	

	for ( k = 0; k < num_tests; k++) {
		msgpack::unpack(result, buf, st.st_size, off);
		result.get().convert(&NUMBERS);
		//long long int n_inv = NumberOfInversions(NUMBERS);
		//long long int n_inv = NumberOfInversions_bruteforce(NUMBERS);
		long long int n_inv = NumberOfInversions_fast(NUMBERS);



		printf("length = %lu,  inversion = %lld\n", NUMBERS.size(), n_inv);

//		r[0] = n_inv;
//		msgpack::pack(&sbuf, r);
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
