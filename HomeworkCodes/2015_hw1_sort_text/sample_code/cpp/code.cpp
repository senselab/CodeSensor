#include <cstdio>
#include <vector>
#include <algorithm>
#include <utility>
#include <string.h>

using namespace std;

#define MAX_NUMBERS_CNT 1000000 

int NUMBERS[MAX_NUMBERS_CNT];

int main()
{
	char buf[100];
	FILE *fpIn, *fpOut;
	char *pTok, c;
	int k, p=2, num_tests, len;


	fpIn = fopen("input.txt", "rt");
	fpOut = fopen("output.txt", "wt");

	fscanf(fpIn, "%d\n", &num_tests)	;
	printf("num_tests = %d\n", num_tests);


	for ( k = 0; k < num_tests; k++) {
//		fgets(buf, sizeof(buf), fpIn );
	//	pTok = strtok(buf, " \t\n\r");
		len = 0;
	
		for ( len =0; len < MAX_NUMBERS_CNT; ) {	
			int i = 0;
			while( c= fgetc(fpIn)) {

				if ( c == EOF ) {
					k = num_tests;
					goto quit;
				}


				if (c==' ' || c=='\t' || c=='\n') {
					buf[i] = 0;
					pTok = strtok(buf, " \n\t\r");
					if (pTok) {
						NUMBERS[len++] = atoi(pTok);

						if ( c=='\n') 
    	                	goto quit;
						else
							break;
					}

					if ( c=='\n')
                    	goto quit;

				}
				else {
					buf[i++] = c;
				}

			}
		}
quit:;

/*		for (p = 0; p < len; p++)
			printf("%d ", NUMBERS[p]);
		printf("\n");
*/
		sort(NUMBERS, NUMBERS+len);

		for ( p = 0; p < len; p++) {
			fprintf(fpOut, "%d ", NUMBERS[p]);
		}

		fprintf(fpOut, "\n");
	}

	if (fpIn)
		fclose(fpIn);	

	if(fpOut)
		fclose(fpOut);

	return 0;
}
