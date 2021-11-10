#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>

int main(int argc, char** argv) {
    system("./run.sh 2> test_target.report.final");

	if ( argc==2 && !strcmp(argv[1], "-p") ) {
	    struct rusage ru;
		FILE *fp;
    	getrusage(RUSAGE_CHILDREN, &ru);

    	if ( ru.ru_utime.tv_sec==0 && ru.ru_utime.tv_usec < 1000)
    		ru.ru_utime.tv_usec = 1;

		fp = fopen("proc_stat.report.final", "wt");

	    fprintf(fp, "%lld %lld %lld %lld",   (long long) ru.ru_utime.tv_sec, (long long)ru.ru_utime.tv_usec, (long long)ru.ru_maxrss, ((long long)ru.ru_majflt)+ru.ru_minflt );
		fclose(fp);
	}
}
