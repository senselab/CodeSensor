#include <stdio.h>
#include <stdlib.h>
#include <pcre.h>
#include <string.h>
#include <vector>
#include <string>

int nDEBUG_LEVEL = 0;

#define SYSCALL_USAGE_REPORT_FILE  "syscall_usage.report"

using namespace std;

struct tAddrRange
{
	unsigned long long start_addr, end_addr;
};

struct tViolation
{
	unsigned long long addr;
	string	func_name;
};

const char* func_black_list[] = {"mmap", "mprotect", "munmap", "syscall", 0};
bool bAllowDirectSyscall = false;

vector<tAddrRange*> alert_ranges;
vector<tViolation*> violations;

int ReadProcMaps(char* filename)
{
	char buf[2048];
	FILE *fp;

	pcre	*re;
	const char      *error;
	int             erroffset;
	int OVECCOUNT = 30;
	int rc;
	int             ovector[OVECCOUNT];
	const char pattern[] = "([0-9a-fA-F]+)-([0-9a-fA-F]+)[\\s]+([^\\s]+)[\\s]+([^\\s]+)[\\s]+([^\\s]+)[\\s]+([^\\s]+)[\\s]+([^\\s\\n\\r]+)";

	alert_ranges.resize(0);
	
	re = pcre_compile(pattern,0, &error, &erroffset, NULL);

	if (re == NULL) {
		printf("PCRE compilation failed at offset %d: %s\n", erroffset, error);
		return -1;
    }

	
	fp = fopen(filename, "rt");
	
	if ( !fp)
		return -2;

	while(fgets(buf, sizeof(buf), fp) ) {
		
		rc = pcre_exec(re, NULL, buf, strlen(buf), 0, 0, ovector, OVECCOUNT);

			if ( rc==1+7){
				char saddr[512];
				char eaddr[512];
				char priv[512];
				char offset[512];
				char dev[512];
				char length[512];
				char path[512];
				char *p_error;
				tAddrRange	*pR = new tAddrRange;

				//printf("rc=%d\n", rc);
				
				sprintf(saddr, "%.*s", ovector[3]-ovector[2], buf+ovector[2]);
				sprintf(eaddr, "%.*s", ovector[5]-ovector[4], buf+ovector[4]);
				sprintf(priv, "%.*s", ovector[7]-ovector[6], buf+ovector[6]);
				sprintf(offset, "%.*s", ovector[9]-ovector[8], buf+ovector[8]);
				sprintf(dev, "%.*s", ovector[11]-ovector[10], buf+ovector[10]);
				sprintf(length, "%.*s", ovector[13]-ovector[12], buf+ovector[12]);
				sprintf(path, "%.*s", ovector[15]-ovector[14], buf+ovector[14]);
				
				pR->start_addr = strtoull(saddr, &p_error, 16);
				pR->end_addr = strtoull(eaddr, &p_error, 16);

				if ( nDEBUG_LEVEL > 0)
					printf("[%lx] [%lx] %s [%s]\n", pR->start_addr, pR->end_addr, priv, path);
				
				
				if ( strstr(path, "/tmp/") && strstr(priv, "x")) {
					alert_ranges.push_back(pR);
					
					if ( nDEBUG_LEVEL > 0)
						printf("alert range : [%lx]-[%lx]  \n", pR->start_addr, pR->end_addr);
					
				}
				else {
					delete pR;
				}
				
			
				
			}
		
		
	}
	
	pcre_free(re);
	
	return 0;
}

int ReadLTraceLog(char* filename)
{
	char buf[2048];
	FILE *fp;

	pcre	*re;
	const char      *error;
	int             erroffset;
	int OVECCOUNT = 30;
	int rc;
	int             ovector[OVECCOUNT];
	const char pattern[] = "\\[0x([0-9a-fA-F]+)\\][\\s]+([a-zA-Z_0-9]+)";
	char entry_func[512];
	unsigned long long entry_addr, effective_addr;

	violations.resize(0);
	
	re = pcre_compile(pattern,0, &error, &erroffset, NULL);

	if (re == NULL) {
		printf("PCRE compilation failed at offset %d: %s\n", erroffset, error);
		return -1;
    }

	
	fp = fopen(filename, "rt");
	
	if ( !fp)
		return -2;

	entry_func[0] = 0;
	
	while(fgets(buf, sizeof(buf), fp) ) {
		
		
		rc = pcre_exec(re, NULL, buf, strlen(buf), 0, 0, ovector, OVECCOUNT);

		if ( rc==1+2){
			char sz_addr[512];
			char syscall_name[512];
			unsigned long long addr;


			//printf("rc=%d\n", rc);
				
				
			sprintf(sz_addr, "%.*s", ovector[3]-ovector[2], buf+ovector[2]);
			sprintf(syscall_name, "%.*s", ovector[5]-ovector[4], buf+ovector[4]);

			effective_addr = addr = strtoull(sz_addr, 0, 16);			
			
			if ( entry_func[0] )
				effective_addr = entry_addr;
			
			
			if ( nDEBUG_LEVEL > 0) {
				if ( entry_func[0] == 0)
					printf("[%lx] @ [%lx] [%s] \n", effective_addr, addr, syscall_name);
				else
					printf("[%lx] @ [%lx][%s] -> [%lx] [%s] \n", effective_addr, entry_addr, entry_func, addr, syscall_name);
			}
			
			if ( strstr(buf,"<unfinished")) {
				entry_addr = addr;
				strcpy(entry_func, syscall_name);
			}
			
			//////// check

			int m;
			
			for ( m = 0; m < alert_ranges.size(); m++) {
				// check range;				
				if ( effective_addr >= alert_ranges[m]->start_addr && effective_addr <= alert_ranges[m]->end_addr) {

					int k = 0;
			
					//// check direct systecall
					if ( bAllowDirectSyscall == false) {
						if ( effective_addr == addr && !strncmp(syscall_name, "SYS_",4) )
							goto add_violation;
					}
					
					//// check blacklist
					while(func_black_list[k]) {
						if ( !strcmp(syscall_name, func_black_list[k])) {
add_violation:;							
							tViolation	*pV = new tViolation;
							pV->addr = effective_addr;
							pV->func_name = syscall_name;
							violations.push_back(pV);

							goto nextone;											
							
						}
						k++;
					}
		
				}
			}

			
nextone:;
			
		}
		else if ( strstr(buf, "resumed>") && strstr(buf,entry_func)) {
			entry_func[0] = 0;
			entry_addr = 0;
		}
			
		
	}
	
	pcre_free(re);
	
	
	return 0;
} 


void PrintViolations()
{
	int k;
	
	FILE* fp;
	
	fp = fopen(SYSCALL_USAGE_REPORT_FILE, "wt");
	
		
	for ( k =0; k < violations.size(); k++) {
		char buf[4096];
		sprintf(buf,"%d: [%lx] %s\n", k, violations[k]->addr, violations[k]->func_name.c_str());
		fputs(buf, stdout);
		if (fp)
			fputs(buf, fp);
	}
	
	fclose(fp);
}


int main(int argc, char* argv[])
{
	if ( argc != 3) {
		printf("CheckSyscallUsage proc_map ltrace.log\n");
		exit(-1);	
	}

	if ( ReadProcMaps(argv[1]) != 0) {
		printf("Error open proc map file %s\n", argv[1]);
		exit(-2);
	}

	if ( ReadLTraceLog(argv[2]) != 0) {
		printf("Error open ltrace file %s\n", argv[2]);
		exit(-3);
	}	

	PrintViolations();

	return violations.size();
}
