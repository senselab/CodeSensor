#include <stdio.h>
#include <stdlib.h>
#include <pcre.h>
#include <string.h>
#include <vector>
#include <string>
#include "config.h"

int nDEBUG_LEVEL = 0;

#define SYSCALL_USAGE_REPORT_FILE  "syscall_usage.report.final"
#define SYSCALL_USAGE_SKIP_RANGE	"syscall_usage_skip_range.report.final"
#define MAP_REPORT_FILE 			"maps.report.final"
#define LTRACE_REPORT_FILE 			"ltrace.report.final"

const long long TOKEN_START_CALL_USAGE_CHECK[] = {7832,98689,278,642,12122,0};
const long long TOKEN_END_CALL_USAGE_CHECK[] = {588, 59, 189, 6855,984598,0};


bool bCALL_USAGE_CHECK_ENABLED = false;

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

//const char* func_black_list[] = {"mmap", "mprotect", "munmap", "syscall", 0};
vector<string> disallowed_func;
bool bAllowDirectSyscall = false;

vector<tAddrRange> alert_ranges;
vector<tAddrRange> skip_ranges;
vector<tViolation*> violations;

extern int log(const char* format, ...);
extern bool ExtractNumber(long long& retV, const char* buf, const char* prefix, const char* suffix = "");

bool IsAddrInSkipRange(unsigned long long addr)
{
	int k;

	for ( k =0; k < skip_ranges.size(); k++) {
		if ( skip_ranges[k].start_addr <= addr && addr <= skip_ranges[k].end_addr)
			return true;
	}

	return false;
}

bool IsAddrInAlertRanges(unsigned long long addr)
{

	int m;
	
	for ( m = 0; m < alert_ranges.size(); m++) {
		// check range;				
		if ( addr >= alert_ranges[m].start_addr && addr <= alert_ranges[m].end_addr) {
			return true;
		}
	}
	
	return false;
}

int ReadSkipRange(char* filename)
{
	 FILE	*fp;
	 char buf[2048];
	 char	*pTok1, *pTok2;
	 tAddrRange	R;
	 
	 skip_ranges.clear();
	 
	 fp = fopen(filename, "rt");
	 
	 if ( !fp) {
		 log("unable to open %s", filename);
		 return -1;
	 }
	 
	 while(fgets(buf, sizeof(buf),fp)) {

		 //log("ReadSkipRange %s", buf);

		 if( !(pTok1 = strtok(buf, "-\t\n\r ")) )
			 continue;
		 if( !(pTok2 = strtok(0, "-\t\n\r ")) )
			 continue;
		 


		 R.start_addr = strtoull(pTok1, 0, 16);
		 R.end_addr = strtoull(pTok2, 0, 16);

		 if ( IsAddrInSkipRange(R.start_addr) && IsAddrInSkipRange(R.end_addr))
			 continue;
		 
		 skip_ranges.push_back(R);
		 log("add skip range %llx-%llx", R.start_addr, R.end_addr);
	 }
	 
	 fclose(fp);
	 
	 return 0;
}



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
		log("PCRE compilation failed at offset %d: %s\n", erroffset, error);
		return -1;
    }

	
	fp = fopen(filename, "rt");
	
	if ( !fp) {
		log("can't open %s", filename);
		return -2;
	}

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
				tAddrRange	R;

				//printf("rc=%d\n", rc);
				
				sprintf(saddr, "%.*s", ovector[3]-ovector[2], buf+ovector[2]);
				sprintf(eaddr, "%.*s", ovector[5]-ovector[4], buf+ovector[4]);
				sprintf(priv, "%.*s", ovector[7]-ovector[6], buf+ovector[6]);
				sprintf(offset, "%.*s", ovector[9]-ovector[8], buf+ovector[8]);
				sprintf(dev, "%.*s", ovector[11]-ovector[10], buf+ovector[10]);
				sprintf(length, "%.*s", ovector[13]-ovector[12], buf+ovector[12]);
				sprintf(path, "%.*s", ovector[15]-ovector[14], buf+ovector[14]);
				
				R.start_addr = strtoull(saddr, &p_error, 16);
				R.end_addr = strtoull(eaddr, &p_error, 16);

				if ( nDEBUG_LEVEL > 2)
					log("[%llx] [%llx] %s [%s]\n", R.start_addr, R.end_addr, priv, path);
				
				
				if ( strstr(path, "/tmp/") && strstr(priv, "x")) {
					alert_ranges.push_back(R);
					
					if ( nDEBUG_LEVEL > 0)
						log("alert range : [%llx]-[%llx]  \n", R.start_addr, R.end_addr);
					
				}
				
			
				
			}
		
		
	}
	
	pcre_free(re);
	
	return 0;
}

void add_violation(unsigned long long addr, const string& func_name)
{
	tViolation	*pV = new tViolation;
	pV->addr = addr;
	pV->func_name = func_name;
	violations.push_back(pV);

	if ( nDEBUG_LEVEL > 0) {
		log("add violation disallowed func [%llx][%s]", pV->addr, pV->func_name.c_str());
	}

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
	const char pattern[] = "[0-9]+[\\s]\\[0x([0-9a-fA-F]+)\\][\\s]+([a-zA-Z_0-9]+)@SYS[\\s]*([^\\n\\r]+)";
	char entry_func[512];
	unsigned long long entry_addr, effective_addr;

	int nTokenStartCallUsageCheck = 0;
	int nTokenEndCallUsageCheck = 0;

	violations.resize(0);
	
	re = pcre_compile(pattern,0, &error, &erroffset, NULL);

	if (re == NULL) {
		log("PCRE compilation failed at offset %d: %s\n", erroffset, error);
		return -1;
    }

	
	fp = fopen(filename, "rt");
	
	if ( !fp) {
		log("failed to open %s", filename);
		return -2;
	}

	entry_func[0] = 0;
	
	while(fgets(buf, sizeof(buf), fp) ) {
		if ( strstr(buf, "__libc_start_main"))
			continue;

		rc = pcre_exec(re, NULL, buf, strlen(buf), 0, 0, ovector, OVECCOUNT);

		if ( rc==1+3){
			char sz_addr[512];
			char syscall_name[512];
			char call_argument[1024];
			unsigned long long addr;


			//printf("rc=%d\n", rc);
				
				
			sprintf(sz_addr, "%.*s", ovector[3]-ovector[2], buf+ovector[2]);
			sprintf(syscall_name, "%.*s", ovector[5]-ovector[4], buf+ovector[4]);
			sprintf(call_argument, "%.*s", ovector[7]-ovector[6], buf+ovector[6]);

			effective_addr = addr = strtoull(sz_addr, 0, 16);			
			
			if ( entry_func[0] )
				effective_addr = entry_addr;
			
			
			if ( nDEBUG_LEVEL > 2) {
				if ( entry_func[0] == 0)
					log("[%llx] @ [%llx] [%s] [%s] bCALL_USAGE_CHECK_ENABLED=%d", effective_addr, addr, syscall_name, call_argument, bCALL_USAGE_CHECK_ENABLED);
				else
					log("[%llx] @ [%llx][%s] -> [%llx] [%s] [%s] bCALL_USAGE_CHECK_ENABLED=%d", effective_addr, entry_addr, entry_func, addr, syscall_name, call_argument,bCALL_USAGE_CHECK_ENABLED);
			}
			
			if ( entry_addr==0 && strstr(buf,"<unfinished")) {
				entry_addr = addr;
				strcpy(entry_func, syscall_name);
			}
			
			//////// check

			if ( !strcmp(syscall_name, "fstat"))	{	/// to detect StartCallUsageCheck / EndCallUsageCheck
				long long nToken;

				if ( !ExtractNumber(nToken, call_argument, "\\(",  ",") )
					goto quit_start_end_tokens;

				if ( nToken ==  TOKEN_START_CALL_USAGE_CHECK[nTokenStartCallUsageCheck]) {
					nTokenStartCallUsageCheck++;

					if ( TOKEN_START_CALL_USAGE_CHECK[nTokenStartCallUsageCheck]==0) { // all checks passed
						bCALL_USAGE_CHECK_ENABLED = true;
						nTokenStartCallUsageCheck = 0;
						nTokenEndCallUsageCheck = 0;
					}

					goto nextone;
				}
				else if (nTokenStartCallUsageCheck)
					goto quit_start_end_tokens;

				if (nToken == TOKEN_END_CALL_USAGE_CHECK[nTokenEndCallUsageCheck]) {
					nTokenEndCallUsageCheck++;

					if ( TOKEN_END_CALL_USAGE_CHECK[nTokenEndCallUsageCheck]==0) { // all checks passed
						bCALL_USAGE_CHECK_ENABLED = false;
						nTokenStartCallUsageCheck = 0;
						nTokenEndCallUsageCheck = 0;
					}

					goto nextone;
				}
				else if ( nTokenEndCallUsageCheck)
					goto quit_start_end_tokens;



			}
			else {
quit_start_end_tokens:;
				nTokenStartCallUsageCheck = 0;
				nTokenEndCallUsageCheck = 0;
			}



			/*if ( !bCALL_USAGE_CHECK_ENABLED)
				goto nextone;
			if ( IsAddrInSkipRange(effective_addr))
				goto nextone;*/
/*				
			 if ( !IsAddrInAlertRanges(effective_addr) )
				 goto nextone;
*/
			/////////////// check blacklist ///////////////
			{
				int k;

				for ( k = 0; k < disallowed_func.size(); k++) {
					char buf[512];

					if ( !strcmp(syscall_name, disallowed_func[k].c_str())) 
						goto match_found;
/* Hank 2011.6.8
					sprintf(buf, "SYS_%s", disallowed_func[k].c_str());
					if (!strcmp(syscall_name, buf))
						goto match_found;
*/
					continue;
match_found:;
					log("[%s] vs [%s]", syscall_name, disallowed_func[k].c_str());
					add_violation( effective_addr, syscall_name);
					goto nextone;

				}

			}

			//////////check direct system call////////////

			//// check direct system call
			if ( bAllowDirectSyscall == false ) {
				if ( effective_addr == addr && IsAddrInAlertRanges(effective_addr) && !strncmp(syscall_name, "SYS_",4) ) {
					add_violation(effective_addr, syscall_name);
					goto nextone;
					//goto add_violation;
				}
			}
#if 0
			int m;
			
			for ( m = 0; m < alert_ranges.size(); m++) {
				// check range;				
				if ( effective_addr >= alert_ranges[m].start_addr && effective_addr <= alert_ranges[m].end_addr) {

					int k = 0;
			
					//// check direct system call
					if ( bAllowDirectSyscall == false) {
						if ( effective_addr == addr && !strncmp(syscall_name, "SYS_",4) ) {
							add_violation(effective_addr, syscall_name);
							goto nextone;
							//goto add_violation;
						}
					}

		
				}
			}
#endif
			
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


void PrintViolations(const char* szSrcDir)
{
	int k;
	char PATH[2048];
	FILE* fp;
	
	sprintf(PATH, "%s/%s", szSrcDir, SYSCALL_USAGE_REPORT_FILE);
	
	fp = fopen(PATH, "a+t");
	
		
	for ( k =0; k < violations.size(); k++) {
		char buf[4096];
		sprintf(buf,"%d: [%llx] %s\n", k, violations[k]->addr, violations[k]->func_name.c_str());
		fputs(buf, stdout);
		if (fp)
			fputs(buf, fp);
	}
	
	fclose(fp);
}

void ClearViolations()
{
	int k;

	for ( k = 0;k < violations.size(); k++) {
		delete violations[k];
	}

	violations.resize(0);
}

int CheckSyscallUsage(const char* szSrcDir)
{
	violations.resize(0);

	{
		vector<string> values;

		if( GetConfigEntry("allow_direct_syscall", values) && values.size()==1) {
			bAllowDirectSyscall = atoi(values[0].c_str());
		}

		disallowed_func.resize(0);
		GetConfigEntry("disallowed_func", disallowed_func);

		if ( nDEBUG_LEVEL > 0 ) {
			char buf[512];
			string s;
			int k;

			sprintf(buf, "CheckSyscallUsage: bAllowDirectSyscall = %d", bAllowDirectSyscall);
			log(buf);

			s = "CheckSyscallUsage: disallowed_func = ";
			for ( k = 0; k < disallowed_func.size(); k++) {
				sprintf(buf, "[%s] ", disallowed_func[k].c_str());
				s += buf;
			}

			log("%s", s.c_str());
		}

	}

	char PATH1[512], PATH2[512], PATH3[512];

	sprintf(PATH1, "%s/%s", szSrcDir, MAP_REPORT_FILE );
	sprintf(PATH2, "%s/%s", szSrcDir, LTRACE_REPORT_FILE);
	sprintf(PATH3, "%s/%s", szSrcDir, SYSCALL_USAGE_SKIP_RANGE);
	
	//ReadSkipRange(PATH3);
	//ReadProcMaps(PATH1);
	if ( ReadLTraceLog(PATH2) == 0 ) {

		PrintViolations(szSrcDir);
		ClearViolations();
	}

	return violations.size();
}


void TestCheckSyscallUsage()
{

	ReadLTraceLog("/tmp/ltrace.report");
}

