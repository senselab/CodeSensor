//============================================================================
// Name        : HomeworkInspector.cpp
// Author      : Hank
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <list>
#include <map>
#include <exception>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <pwd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <signal.h>
#include <stdarg.h>
#include <uuid/uuid.h>
#include <pcre.h>
#include <time.h>

#include "config.h"

using namespace std;


#define MAX_CHECK_RECORD_NUM  2048

string HW_NAME;	 // e.g. hw4

char homework_user_root_dir[512]; // "/var/homeworks/hw5/1000001"
char homework_job_queue[512];// = "/var/homeworks/hw5/queue";
char homework_inspector_dir[512];

const char workspace_root[] = "/homeworks_workplace";
//const char init_script[] = "/usr/bin/HomeworkInspector/init_script";
char init_script[512];
//const char build_script[] =	"/usr/bin/HomeworkInspector/build_script";
char build_script_phase0[512];
char build_script_phase1[512];

#define HOMEWORK_EXECUTABLE_NAME "HomeworkInspector"
#define VALGRIND_MEMCHECK_REPORT "valgrind.memcheck.report.final"
#define VALGRIND_CALLGRIND_REPORT "valgrind.callgrind.report.final"
//#define VALGRIND_MASSIF_REPORT    "valgrind.massif_out.report.final"
#define SEGMENT_SIZE_REPORT		"segment_size.report.final"
#define GCC_EXIT_CODE_REPORT "gcc_exit_code.report.final"
//#define SYSCALL_USAGE_EXIT_CODE_REPORT "syscall_usage_exit_code.report"
#define SYSCALL_USAGE					"syscall_usage.report.final"
#define CHECK_PATTERN_REPORT "check_pattern_concise.report.final"
#define GCC_DEPENDENCY_REPORT  "gcc_dependency.report.final"
#define ILLEGAL_HEADER_REPORT "illegal_header.report.final"
#define TIME_REPORT "time.report.final"
#define PROC_STAT_REPORT "proc_stat.report.final"
#define ANALYSIS_RESULT "analysis_result"
#define LOG_FILE "/var/log/HomeworkInspector.log"
#define TIMEOUT_FILE "timeout.final"

const char ID_PREFIX[] = "HOMEWORK_USER_";



//double mem_total_size;
long long mem_peak_size;
long long massif_time;

uid_t job_uid = 99;
gid_t job_gid = 99;

char username[512];	// username on Linux when running build_script

int CHILD_PROCESS_TIMEOUT_SECONDS = (60*5);

pthread_cond_t cond_process;
pthread_mutex_t mutex_queue;


FILE	*fpLog = 0;

string sz_src_dir, sz_src_file, sz_workspace, sz_job_full_path;

class analysis_error :  public exception
{
};

struct tResult
{
	enum TYPE {TYPE_INTEGER, TYPE_FLOAT, TYPE_STRING};
	
	TYPE type;
	
	
	
	union {
		long long nV;
		double dV;
	}v;
	
	string key;

	string sV;
	
	void Add(const tResult& r) 
	{
		assert(type == r.type);
		assert(key == r.key);
		
		if (type==TYPE_INTEGER)
			v.nV += r.v.nV;
		else if ( type == TYPE_FLOAT)
			v.dV += r.v.dV;
		else if ( type == TYPE_STRING)
			sV = r.sV;
		else
			assert(false);
	}
	
	string ToString() const
	{
		char buf[512];
		
		if ( type == TYPE_INTEGER)
			sprintf(buf, "%s=%lld", key.c_str(), v.nV);
		else if ( type == TYPE_FLOAT)
			sprintf(buf, "%s=%f", key.c_str(), v.dV);
		else if ( type == TYPE_STRING)
			sprintf(buf, "%s=%s", key.c_str(), sV.c_str());
		else
			assert(false);
		
		return string(buf);
	}
};

typedef map<string,tResult> tResultCollection;


void InitLog();
void CloseLog();
int log(const char* format, ...);
extern int CheckSyscallUsage(const char* szSrcDir);
extern void TestCheckSyscallUsage();
static void sig_timeout(int signo);

void* monitor_new_reports(void* _pParam) ;


void _7za_file(const char* workdir, const char* reportfile)
{
        char buf[4096];

	sprintf(buf, "7za u %s/%s.7z %s/%s", workdir, reportfile, workdir, reportfile);
	system(buf);

        sprintf(buf, "%s/%s", workdir, reportfile);
        unlink(buf);
}

        
bool GetResult (const tResultCollection& rc, const string& key, tResult& r)
{
	tResultCollection::const_iterator it;
	it = rc.find(key);
	if ( it == rc.end())
		return false;
	r = it->second;
	return true;
}

void AddResult(tResultCollection& rc, const string& key, const tResult& r)
{
	pair<tResultCollection::iterator,bool> retV;
		

	retV = rc.insert(tResultCollection::value_type(key, r));
	
	if ( !retV.second) {
		retV.first->second.Add(r);
	}
	
}

void AddResult(tResultCollection& rc, const string& key, long long v)
{
	pair<tResultCollection::iterator,bool> retV;
	tResult r;
	
	r.type = tResult::TYPE_INTEGER;
	r.key = key;
	r.v.nV = v;
	
	AddResult(rc, key, r);
}

void AddResult(tResultCollection& rc, const string& key, double v)
{
	pair<tResultCollection::iterator,bool> retV;
	tResult r;
	
	r.type = tResult::TYPE_FLOAT;
	r.key = key;
	r.v.dV = v;
	
	AddResult(rc, key, r);
}

void AddResult(tResultCollection& rc, const string& key, const string& v)
{
	pair<tResultCollection::iterator,bool> retV;
	tResult r;
	
	r.type = tResult::TYPE_STRING;
	r.key = key;
	r.sV = v;
	
	AddResult(rc, key, r);
}


string int2str(const int &i)
{
  string s;
  stringstream ss(s);
  ss << i;

  return ss.str();
}

string remove_comma(const char* src)
{
	int k, i;
	int len = strlen(src);
	char *buf = new char[len+1];
	string szRet;

	i = 0;

	for ( k = 0; k < len; k++) {
		if ( src[k] != ',')
			buf[i++] = src[k];
	}

	buf[i] = 0;
	szRet=buf;
	delete []buf;

	return szRet;
}

bool ExtractNumber(long long& retV, const char* buf, const char* prefix, const char* suffix = "")
{
	pcre	*re;
	const char      *error;
	int             erroffset;
	int OVECCOUNT = 30;
	int rc;
	int             ovector[OVECCOUNT];
	const char regNumbers[] = "[-+]?(([0-9]+[,]?)*([0-9]+))";
	char pattern[4096];
	char num[512];

	pattern[0] = 0;

	if ( strlen(prefix)>0) {
		strcat(pattern,prefix);
		strcat(pattern,"[\\s]*");
	}

	strcat(pattern, "(");
	strcat(pattern, regNumbers);
	strcat(pattern, ")");

	if ( strlen(suffix)>0) {
		strcat(pattern, "[\\s]*");
		strcat(pattern, suffix);
	}



	re = pcre_compile(pattern,0, &error, &erroffset, NULL);

	if (re == NULL) {
		log("PCRE compilation failed at offset %d: %s\n", erroffset, error);
	    return false;
    }

	rc = pcre_exec(re, NULL, buf, strlen(buf), 0, 0, ovector, OVECCOUNT);

	if ( rc>0){
		sprintf(num, "%.*s", ovector[3]-ovector[2], buf+ovector[2]);

		retV = atoll(remove_comma(num).c_str());

		pcre_free(re);
		return true;
	}


	pcre_free(re);
	return false;
}

int strrncmp(   const char *string1,   const char *string2,   size_t count )
{
	int len1;
	int len2;
	int i, j, cnt;
	
	len1 = strlen(string1);
	len2 = strlen(string2);
	
	cnt = 0;
	
	
	for (  i = len1-1, j = len2-1;  i>=0 && j>=0 && cnt < count; i--, j--, cnt++) {
		if ( string1[i] < string2[j])
			return -1;
		if ( string1[i] > string2[j])
			return 1;
	}
	
	return 0;
}


int analyze_gcc_dependency_report_file(const char* reportfile, const char* illegal_header_report, tResultCollection& results)
{
	FILE *fp;
	char buf[4096];
	pcre	*re;
	const char      *error;
	int             erroffset;
	int OVECCOUNT = 30;
	int rc;
	int             ovector[OVECCOUNT];
	//const char regFilename[] = "/([a-zA-Z\\+\\-_\\.]+)[\\s$]";
	const char regFilename[] = "([^\\s\\:\\\\]+)[\\s$]";
	list<string>	illegal_headers;
	vector<string> DISALLOWED_HEADER;

	fp = fopen(reportfile, "rt");

	if ( !fp)
		return 0;
	
	if( !GetConfigEntry("disallowed_header", DISALLOWED_HEADER) ) {
		DISALLOWED_HEADER.resize(0);
	}
/*
	{
		int k;
		string s = "analyze_gcc_dependency_report_file: DISALLOWED_HEADERS = ";

		for ( k = 0; k < DISALLOWED_HEADER.size(); k++) {
			char buf[512];
			sprintf(buf, "[%s] ", DISALLOWED_HEADER[k].c_str());
			s += buf;
		}

		log(s.c_str());
	}
*/

	re = pcre_compile(regFilename, PCRE_MULTILINE, &error, &erroffset, NULL);
	if (re == NULL) {
		log("PCRE compilation failed at offset %d: %s\n", erroffset, error);
	    goto quit_close_file;
    }	
	
	
	
	//log("\n\ndependency : ");
	
	while(fgets(buf, sizeof(buf),fp)) {
		
		int nOffset = 0;
		int len = strlen(buf);
		
		while(nOffset < len) {
			rc = pcre_exec(re, NULL, buf, len , nOffset, 0, ovector, OVECCOUNT);
			
			//log("rc=%d ", rc);
							
			if ( rc==2){
				
				char name[2048];
				int m;
					
	
				sprintf(name, "%.*s", ovector[3]-ovector[2], buf+ovector[2]);
					
				//log("[%s] ", name);		
					
				for ( m = 0; m < DISALLOWED_HEADER.size(); m++) {
					//if ( !strcmp(DISALLOWED_HEADER[m].c_str(), name) ) {
//					if ( strstr(name, DISALLOWED_HEADER[m].c_str()) ) {
					if ( !strrncmp(name, DISALLOWED_HEADER[m].c_str(), 100000) ) {
						illegal_headers.push_back(name);
						//break;
					}
				}
				
				nOffset = ovector[1];
			}
			else {
				break;
			}
			
		}
	}
	
	//log("\n");

	{
		
		illegal_headers.sort();
		illegal_headers.unique();
		
//		sprintf(buf, "illegal_header=%d", illegal_headers.size());
		//result.push_back(buf);
		AddResult(results, "illegal_header", (long long) (illegal_headers.size()));
		
		FILE	*fpOut = fopen (illegal_header_report, "wt");
		
		if( fpOut) {
			list<string>::iterator it;
			for ( it = illegal_headers.begin(); it != illegal_headers.end(); ++it) {
				fprintf(fpOut, "%s\n", (*it).c_str());
			}
			fclose(fpOut);
		}
	}
	


	pcre_free(re);
	
quit_close_file:;
	fclose(fp);
	
	
	return illegal_headers.size();

}

void merge_check_pattern_files(const char* szDir)
{


	struct tCheckRecord
	{

		int nPassed;
		int nCheck;
		vector<string> comment;

		tCheckRecord()
		{
			nPassed = 0;
			nCheck = 0;
		}
	};

	int k, nSrcNum;
	char* src_name[] = {"memcheck", "callgrind", "massif", "vanilla", 0};
	tCheckRecord records[MAX_CHECK_RECORD_NUM];

	pcre	*re;
	const char      *error;
	int             erroffset;
	int OVECCOUNT = 30;
	int rc;
	int             ovector[OVECCOUNT];
	const char regPattern[] = "check[\\s]+([0-9]+):.*\\(([0-9]+)/([0-9]+)\\)";


	re = pcre_compile(regPattern,0, &error, &erroffset, NULL);
	if (re == NULL) {
		log("PCRE compilation failed at offset %d: %s\n", erroffset, error);
	    return;
    }

	memset(records,0, sizeof(records));

	for (nSrcNum=0; src_name[nSrcNum]; nSrcNum++);

	for (k=0; src_name[k]; k++) {
		FILE	*fp;
		char    filename[512];
		char 	buf[4096];

		sprintf(filename, "%s/check_pattern_concise_%s.report.final", szDir, src_name[k]);
		fp = fopen(filename, "rt");

		if (!fp) {
			log("unable to open %s", filename);
			continue;
		}

		while(fgets(buf, sizeof(buf),fp)) {
			char num[512];
			int nCheckNum, nPassed, nCheck;

			rc = pcre_exec(re, NULL, buf, strlen(buf), 0, 0, ovector, OVECCOUNT);

			if ( rc==4 ){

				sprintf(num, "%.*s", ovector[3]-ovector[2], buf+ovector[2]);
				//log("check_pattern: passed[%s]",num);
				nCheckNum = atoi(num);
				assert(nCheckNum>=0 && nCheckNum < MAX_CHECK_RECORD_NUM);
				sprintf(num, "%.*s", ovector[5]-ovector[4], buf+ovector[4]);
				nPassed = atoi(num);
				sprintf(num, "%.*s", ovector[7]-ovector[6], buf+ovector[6]);
				nCheck = atoi(num);

				char buf2[512];
				sprintf(buf2, "[%s %d/%d]", src_name[k], nPassed, nCheck*nSrcNum);
				records[nCheckNum].comment.push_back(buf2);
				records[nCheckNum].nCheck = nCheck*nSrcNum;
				records[nCheckNum].nPassed += nPassed;

			}

		}


		fclose(fp);
	}

	///////////
	{
		FILE	*fp;
		char buf[2048];
		int max_comment_width = 0;

		sprintf(buf, "%s/%s", szDir, CHECK_PATTERN_REPORT);

		fp = fopen(buf, "wt");
		if ( !fp) {
			log("unable to open %s", buf);
			goto quit;
		}

		for ( k = 0; k < MAX_CHECK_RECORD_NUM; k++) {
			if (records[k].nCheck>0) {
				for ( int m =0; m < records[k].comment.size(); m++)  {
					if ( records[k].comment[m].length()>max_comment_width)
						max_comment_width = records[k].comment[m].length();
				}
			}
		}

		for ( k = 0; k < MAX_CHECK_RECORD_NUM; k++) {
			if (records[k].nCheck>0) {

				fprintf(fp, "check %d: (%d/%d)  =  ", k, records[k].nPassed, records[k].nCheck);
				int m;
				for ( m =0; m < records[k].comment.size(); m++)  {
					fprintf(fp, " %.*s ", max_comment_width, records[k].comment[m].c_str());
					if ( m < records[k].comment.size()-1)
						fprintf(fp, "  +  ");
				}

				fprintf(fp, "\n");
			}
		}

		fclose(fp);

	}

quit:;

	pcre_free(re);
}

void analyze_check_pattern_report_file(const char* reportfile, tResultCollection& results )
{
	FILE *fp;
	char buf[4096];
	int v;
	pcre	*re;
	const char      *error;
	int             erroffset;
	int nCheckNum;
	int OVECCOUNT = 30;
	int rc;
	int             ovector[OVECCOUNT];
	//const char regPattern[] = "\\(([0-9]+)/([0-9]+)\\)";
	//const char regPattern[] = "check[\\s]+[0-9]+:.*\\(([0-9]+)/([0-9]+)\\)";
	const char regPattern[] = "check[\\s]+([0-9]+):.*\\(([0-9]+)/([0-9]+)\\)";
	int nPassed, nCheck;

	re = pcre_compile(regPattern,0, &error, &erroffset, NULL);
	if (re == NULL) {
		log("PCRE compilation failed at offset %d: %s\n", erroffset, error);
	    return;
    }


	fp = fopen(reportfile, "rt");


	if ( !fp)
		return;
	
	nPassed = 0;
	nCheck = 0;

	while(fgets(buf, sizeof(buf),fp)) {
		char num[512];
		rc = pcre_exec(re, NULL, buf, strlen(buf), 0, 0, ovector, OVECCOUNT);
				/*
		log("check_pattern: %s\n", buf);
		log("check_pattern rc = %d\n", rc);
		*/

		if ( rc==4 ){
			sprintf(num, "%.*s", ovector[3]-ovector[2], buf+ovector[2]);
			//log("check_pattern: passed[%s]",num);
			nCheckNum = atoi(num);
			assert(nCheckNum>=0 && nCheckNum < MAX_CHECK_RECORD_NUM);
			sprintf(num, "%.*s", ovector[5]-ovector[4], buf+ovector[4]);
			nPassed += atoi(num);
			sprintf(num, "%.*s", ovector[7]-ovector[6], buf+ovector[6]);
			nCheck = atoi(num);


		}

	}

	if ( nCheck==0) {
		AddResult(results, "check", (double)0);
	}
	else {
		double d = nPassed;
		d = d/nCheck;
		//sprintf(buf,"check_pattern=%f", d);
		AddResult(results, "check", d);
	}
	

	fclose(fp);

	pcre_free(re);	
}

void analyze_syscall_usage_report_file(const char* reportfile, tResultCollection& results )
{
	FILE *fp;
	char buf[4096];
	long long n;


	fp = fopen(reportfile, "rt");


	if ( !fp)
		return;

	n = 0;
	
	while(fgets(buf, sizeof(buf),fp)) {
		if (strstr(buf,":"))
			n++;
	}

	
	AddResult(results, "illegal_func", n);

	

	fclose(fp);
}

void analyze_proc_stat_report_file(const char* reportfile, tResultCollection& results )
{
	FILE *fp;
	char buf[4096];
	long long v;
	long long maxrss = 0;
	long long flt = 0;

	fp = fopen(reportfile, "rt");

	if ( !fp)
		return;

	timeval cpu_time;
	cpu_time.tv_usec =  0;
	cpu_time.tv_sec = 0;

	fscanf(fp, "%ld %ld %ld %ld", &cpu_time.tv_sec, &cpu_time.tv_usec, &maxrss, &flt);

	if (cpu_time.tv_sec >0 || cpu_time.tv_usec>0)
		AddResult(results, "cpu_time", (long long)( cpu_time.tv_usec + cpu_time.tv_sec*1000000) );

	if ( maxrss >0)
		AddResult(results, "maxrss", maxrss*1024);

	if ( flt >0)
		AddResult(results, "flt", flt);

	fclose(fp);	
}

void analyze_gcc_exit_code_report_file(const char* reportfile, tResultCollection& results )
{
	FILE *fp;
	char buf[4096];
	long long v;




	fp = fopen(reportfile, "rt");


	if ( !fp)
		return;

	while(fgets(buf, sizeof(buf),fp)) {


		if ( ExtractNumber(v,buf,"") )
			AddResult(results, "gcc", v );
			//result.push_back(string("gcc_exit_code=")+ int2str(v));

	}


	fclose(fp);
}

void analyze_valgrind_callgrind_report_file(const char* reportfile, tResultCollection& results )
{
	FILE *fp;
	char buf[4096];
	long long v;
	long long i_refs = 0;




	fp = fopen(reportfile, "rt");


	if ( !fp)
		return;

	while(fgets(buf, sizeof(buf),fp)) {


		if ( ExtractNumber(v,buf,"I[\\s]+refs:") )
			i_refs += v;
			//result.push_back(string("I_refs=")+ int2str(v));

	}

	AddResult(results, "I_refs", i_refs );

	fclose(fp);
}


void analyze_valgrind_memcheck_report_file(const char* reportfile, tResultCollection& results )
{
	FILE *fp;
	char buf[4096];
	long long v;

	long long	in_use_at_exit = 0;
	long long	suppressed = 0;
	long long   m_errors = 0;


	fp = fopen(reportfile, "rt");


	if ( !fp)
		return;

	while(fgets(buf, sizeof(buf),fp)) {


		if ( ExtractNumber(v,buf,"ERROR SUMMARY:", "errors") ) {
			m_errors += v;
			continue;
		}
			//result.push_back(string("mem_error=")+ int2str(v));
		
/*
		if ( strstr(buf, "total heap usage:") ) {
			if ( ExtractNumber(v,buf,"frees,","bytes allocated" )) {
				result.push_back(string("heap_usage="+ int2str(v)));
			}
		}
		*/
//		if ( strstr(buf, "in use at exit:") ) {
		if ( ExtractNumber(v,buf,"in use at exit:","bytes" )) {
			in_use_at_exit += v;
			continue;
		}
//		}

//		if ( /*in_use_at_exit>0 && suppressed ==0 &&*/ strstr(buf, "suppressed:") ) {
        if ( ExtractNumber(v,buf,"suppressed:","bytes" )) {
                suppressed += v;
                continue;
		}
		
//		}
	}

	AddResult(results, "heap_lost", in_use_at_exit - suppressed );
	AddResult(results, "m_error", m_errors);

	fclose(fp);
}

struct tMassifRecord
{
	int nSnapshot;
	long long mem_heap, mem_heap_extra, mem_stacks;
	long long tm;

	void Reset()
	{
		tm = 0;
		mem_heap = 0;
		mem_heap_extra = 0;
		mem_stacks = 0;
		nSnapshot = -1;
	}

	tMassifRecord()
	{
		Reset();
	}

	long long GetTotal()
	{
		return mem_heap + mem_heap_extra + mem_stacks;
	}
};

void detect_language(const char* szWorkspace,  tResultCollection& results)
{
	vector<string> values;

	if ( !GetConfigEntry("language", values) || values.size()==0) {
		assert(false);
		log("detect language => language type not found !");
		exit(-1);
	}	


	if ( values[0] == "customized") {
	//	log("detect language at [%s]", srcWorkspace);
		char buf[512];
		FILE *fp;

		sprintf(buf, "%s/build.sh", szWorkspace);

		if( fp = fopen(buf, "rt") ) {
			if ( fgets(buf,sizeof(buf),fp) ) {
				char* pTok = strstr(buf, "lang=");
				
				if ( pTok && (pTok = strtok(pTok+5, "\n\r")	) ) 
					values[0] = pTok;
				else
					log("detect language unknown in [%s]", szWorkspace);	
			}
			else
				log("detect language failed in [%s]", buf);	
			fclose(fp);
		}
		else
			log("detect language failed [%s]", szWorkspace);
	}


	AddResult(results, "lang", values[0]);
}

#if 0
void analyze_segment_size_report_file(const char* srcDir, const char* reportfile, tResultCollection& results)
{
	FILE *fp;
	char buf[4096];
	long long data, bss, text;

	{	//js script / java 

		vector<string> values;

		if ( !GetConfigEntry("language", values) || values.size()==0) {
			assert(false);
			log("language type not found !");
			exit(-1);
		}


		//if ( values[0] == "js" || values[0]=="java") {
		if ( values[0] != "cpp" && values[0]!="cpp11") {
			struct stat st;
			char filepath[1024];

			sprintf(filepath, "%s/%s", srcDir, "code");
			stat (filepath, &st);

			AddResult(results, "m_text", (long long)st.st_size);
			AddResult(results, "m_data", (long long)0);
			AddResult(results, "m_bss",  (long long)0);
			return;
		}

	}

	fp = fopen(reportfile, "rt");


	if ( !fp) {
		log("unable to open %s\n", reportfile);
		return;
	}

	if (!fgets(buf, sizeof(buf),fp) )
		goto quit;
	if (fscanf(fp, "%lld %lld %lld", &text, &data, &bss)!=3 )
		goto quit;

	

	AddResult(results, "m_text", text);
	AddResult(results, "m_data", data);
	AddResult(results, "m_bss", bss);

	
		
	log("massif_time = %lld\n", massif_time);
	log("xmem_total_size before = %f\n", mem_total_size);
	

	mem_total_size +=  (text + data + bss)*((double)massif_time);

	log( "xmem_total_size after = %f\n", mem_total_size);

#ifndef MASSIF_USE_PAGES_AS_HEAP		
	mem_peak_size +=  text + data + bss;
#endif
	
quit:;
	fclose(fp);
}

#endif


void analyze_valgrind_massif_out_file(const char* reportfile )
{
	FILE *fp;
	char buf[4096];
	long long v;
	
	tMassifRecord curR, maxR, prevR;


	fp = fopen(reportfile, "rt");


	if ( !fp) {
		log("analyze massif failed ...unable to open %s\n", reportfile);
		throw analysis_error();		
		exit(-1);
	}

			

	while(fgets(buf, sizeof(buf),fp)) {

//		printf("%s\n", buf);

		if ( ExtractNumber(v,buf,"snapshot=") ) {

			if ( maxR.nSnapshot == -1 || curR.GetTotal() >  maxR.GetTotal() )
				maxR = curR;

	///		printf("snapshot=%d\n", v);
	

			prevR = curR;

			curR.Reset();
			curR.nSnapshot = v;
		}
		
		if ( ExtractNumber(v,buf,"time=", "") ) {
			curR.tm = v;
			
			if ( prevR.nSnapshot != -1) {
				double t = curR.tm - prevR.tm;
/*				assert(t>0);
 *
 * Hank. 2011.12.4  t can be 0
 *
 * mem_total_size is buggy...not in use anymore
 *				*/
				//mem_total_size += t* ((double)(prevR.GetTotal()));
			}

			massif_time = curR.tm;		
		}

		if ( ExtractNumber(v,buf,"mem_heap_B=", "") ) {
			curR.mem_heap = v;
			//assert( curR.mem_heap >=0);
		}

		if ( ExtractNumber(v,buf,"mem_heap_extra_B=", "") ) {
			curR.mem_heap_extra = v;
//			assert(curR.mem_heap_extra >=0);
		}

		if ( ExtractNumber(v,buf,"mem_stacks_B=", "") ) {
			curR.mem_stacks = v;
//			assert(curR.mem_stacks>=0);
		}

	}
	
	if ( curR.nSnapshot != -1 ) {
		//mem_total_size += curR.GetTotal();
	}

	if ( curR.nSnapshot != -1 && curR.GetTotal() > maxR.GetTotal())
		maxR = curR;

	if ( maxR.nSnapshot != -1) {
	//	AddResult(results, "m_heap", maxR.mem_heap);
	//	AddResult(results, "m_heap_extra" ,maxR.mem_heap_extra);
	//	AddResult(results , "m_stack", maxR.mem_stacks);


//		log("maxR mem_peak_Size (before) = %lld", mem_peak_size);

		mem_peak_size += maxR.GetTotal();
	
//		log("maxR.nSnapshot = %lld", maxR.nSnapshot);
//		log("maxR.mem_heap = %lld", maxR.mem_heap);
//		log("maxR.mem_heap_Extra = %lld", maxR.mem_heap_extra);
//		log("maxR.mem_stacks = %lld", maxR.mem_stacks);
//		log("maxR mem_peak_size = %lld", mem_peak_size);
	
		log("\tpeak mem use = %lld", maxR.GetTotal());
	}


	fclose(fp);
}

void analyze_valgrind_massif_out_report(const char* szSrcDir,  tResultCollection& results )
{
	char buf[512];
	DIR *d;
	struct dirent *dir;
	int	OVECCOUNT = 30;
	pcre	*re;
	const char* error;
	int erroffset, rc;
	int             ovector[OVECCOUNT];
	const char pattern[] = "massif\\.out\\.[0-9]+\\.report\\.final";

	mem_peak_size = 0;

	log("analyze_valgrind_massif_out_report => %s", szSrcDir);

	re = pcre_compile(pattern,0, &error, &erroffset, NULL);

	if (re == NULL) {
		log("PCRE compilation failed at offset %d: %s\n", erroffset, error);
	    return;
    }


	d = opendir(szSrcDir);

	if ( !d) {
		log("analyze_valgrind_massif_out_report unable to open dir %s", szSrcDir);
		return;
	}

	while((dir = readdir(d)) != NULL) {
		if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")
				|| !strstr(dir->d_name, ".report.final"))
			continue;

		rc = pcre_exec(re, NULL, dir->d_name, strlen(dir->d_name), 0, 0, ovector, OVECCOUNT);
		if (rc <=0)
			continue;
		
		log("parsing [%s]", dir->d_name);

		sprintf(buf, "%s/%s", szSrcDir, dir->d_name);
		analyze_valgrind_massif_out_file(buf);
	}

	log("(total) mem_peak_size = %lld", mem_peak_size);

	if (mem_peak_size>0)
		AddResult(results, "m_peak", mem_peak_size);

	closedir(d);
	pcre_free(re);
}


void analyze_report(const char* szSrcDir, const char* szWorkspace)
{
	char buf[2048];
	tResultCollection results;
	FILE *fpOut;

	
//	detect_language(szWorkspace, results);
	//AddResult(results, "timeout", (long long int)bTimeout);
		
//	mem_total_size = 0;

	massif_time = 0;
	

//	sprintf(buf, "%s/%s", szSrcDir, VALGRIND_MASSIF_REPORT);
	//analyze_valgrind_massif_report_file(buf, results);
	analyze_valgrind_massif_out_report(szSrcDir, results);

//	sprintf(buf, "%s/%s", szSrcDir, SEGMENT_SIZE_REPORT);
	//analyze_segment_size_report_file(szSrcDir, buf, results);

//	log ("m_total: %lld", mem_total_size);
	
//	AddResult(results, "m_total", mem_total_size);
	
	
	
	sprintf(buf, "%s/%s", szSrcDir, VALGRIND_MEMCHECK_REPORT);
	analyze_valgrind_memcheck_report_file (buf, results);

	sprintf(buf, "%s/%s", szSrcDir, VALGRIND_CALLGRIND_REPORT);
	analyze_valgrind_callgrind_report_file(buf, results);


	sprintf(buf, "%s/%s", szSrcDir, GCC_EXIT_CODE_REPORT);
	analyze_gcc_exit_code_report_file(buf, results);

	{
		char buf2[2048];
		sprintf(buf, "%s/%s", szSrcDir, GCC_DEPENDENCY_REPORT);
		sprintf(buf2, "%s/%s", szSrcDir, ILLEGAL_HEADER_REPORT);
		analyze_gcc_dependency_report_file(buf, buf2, results);
	}

	
	CheckSyscallUsage(szSrcDir);
	sprintf(buf, "%s/%s", szSrcDir,  SYSCALL_USAGE);
	analyze_syscall_usage_report_file(buf, results);
	

	merge_check_pattern_files(szSrcDir);
	sprintf(buf, "%s/%s", szSrcDir, CHECK_PATTERN_REPORT);
	analyze_check_pattern_report_file(buf, results);
	

	sprintf(buf, "%s/%s", szSrcDir, PROC_STAT_REPORT);
	analyze_proc_stat_report_file(buf, results);	
	
	sprintf(buf, "%s/%s", szSrcDir, ANALYSIS_RESULT);

	if ( fpOut = fopen(buf, "wt")) {
		tResultCollection::iterator it;
		
		for ( it = results.begin(); it != results.end(); ++it)
			fprintf(fpOut,"%s\n", it->second.ToString().c_str());
	
		{	///append run.sh.settings.report.final
			sprintf(buf, "%s/%s", szSrcDir, "run.sh.settings.report.final");			
			FILE *fp = fopen(buf, "rt");
			while(fgets(buf, sizeof(buf),fp))
				fprintf(fpOut, "%s", buf);
			fclose(fp);
		}


		fclose(fpOut);
	}

	///////////// archiving potentially lengthy reports
	_7za_file(szSrcDir, "ltrace.report.final");
	_7za_file(szSrcDir, "build_script.report.final");
}

void move_reports(const char* filename = 0) {
	DIR *dir;
	dirent *dirp;
	char buf[4096];

  //  log("move reports from [%s]...", sz_workspace.c_str());

	if ( filename) {
		sprintf(buf, "\\cp -f %s/%s %s", sz_workspace.c_str(), filename,	sz_src_dir.c_str());
		system(buf);		
		
//		log( "move cmd [%s] ", buf);
		
		sprintf(buf, "chmod 644 %s/%s", sz_src_dir.c_str(), filename);
		system(buf);							
	}
	else {
		if (!(dir = opendir(sz_workspace.c_str())))
			goto next;

		while ((dirp = readdir(dir)) != NULL) {


			if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")
					|| !strstr(dirp->d_name, ".report.final"))
				continue;

			sprintf(buf, "\\cp -f %s/%s %s", sz_workspace.c_str(), dirp->d_name,	sz_src_dir.c_str());
			//log("cmd [%s]\n", buf);
			system(buf);
		}

		next: ;
		closedir(dir);
		
	///// fix mode
		sprintf(buf, "chmod 644 %s/*", sz_src_dir.c_str());
//	log("execute [%s]",buf);
		system(buf);		
	}
	

}

void build_core_phase0(const char* job, bool bDisableOptimization)
{
	char buf[2048];
	string szOptions;
	setresgid(job_gid, job_gid, job_gid);
	setresuid(job_uid, job_uid, job_uid);

/*
	{
		uid_t ruid, euid, suid;
		gid_t rgid, egid, sgid;

		getresuid(&ruid, &euid, &suid);
		getresgid(&rgid, &egid, &sgid);
		log("ruid=%d euid=%d suid=%d\n", ruid, euid, suid);
		log("rgid=%d egid=%d sgid=%d\n", rgid, egid, sgid);

	}

*/
	if ( bDisableOptimization )
		szOptions += "  DISABLE_OPTIMIZATION  ";

	chdir(sz_workspace.c_str());

	sprintf(buf, "sh ./build_script_phase0 %s/ %s/ %s  %s > /dev/null 2>&1",  sz_workspace.c_str(), sz_src_dir.c_str(),  sz_src_file.c_str(), szOptions.c_str());


	
	system( buf);

}

bool phase0_check_static_symbol(const char* szSrcDir) 
{
	FILE 	*fp, *fpS;
	char *pTok;
	char buf[512];
	bool retV = true;
	vector<string> 	bl_func;
	int k;
	
	
	sprintf(buf, "%s/syscall_usage.report.final", szSrcDir);
	
	fpS = fopen(buf, "a+t");
	
	if ( !fpS)  {
		log("phase0_check_static_symbole can't open %s", buf);	
		goto quit;
	}
	
	if( !GetConfigEntry("disallowed_func", bl_func) ) {
		bl_func.resize(0);
	}
	
	sprintf(buf, "%s/symbols.report.final", szSrcDir);
	fp = fopen(buf, "rt");
	
	if ( !fp) {
		log("phase0_check_static_symbole can't open %s", buf);	
		goto quit;
	}

	while(fgets(buf, sizeof(buf), fp)) {
		pTok = strtok(buf, "\t\n\r ");
		if (!pTok)
			continue;
			
		for ( k = 0; k < bl_func.size(); k++) 	 {
			if ( bl_func[k] == pTok) {
				fprintf(fpS, ":%s\n", pTok);
				retV = false;
			}
		}
			
	}
	

	fclose(fp);
	fclose(fpS);	
quit:;
	
	return retV;
}

//szSrcDir looks like   /tmp/job_1000000_xxxxx
bool phase0_check(const char* szSrcDir)
{
	if  (!phase0_check_static_symbol(szSrcDir)) {
		log("!!! phase0_check_static_symbol failed");
		return false;
	}
	
	{
		tResult r;
		tResultCollection results;
		char buf2[2048];
		char buf[1024];
		
		sprintf(buf, "%s/%s", szSrcDir, GCC_DEPENDENCY_REPORT);
		sprintf(buf2, "%s/%s", szSrcDir, ILLEGAL_HEADER_REPORT);
		if ( analyze_gcc_dependency_report_file(buf, buf2, results) >0 ) {
			log("!!! phase0_check_gcc denepdency failed");
			return false;
		}

		//Hank 2015.1.17
		sprintf(buf, "%s/%s", szSrcDir, GCC_EXIT_CODE_REPORT);
		analyze_gcc_exit_code_report_file(buf, results);
		if (GetResult(results, "gcc", r)==false || r.v.nV !=0 ) {
			log("!!! phase0_check_gcc exit code failed");
			return false;
		}
		
	}


	return true;
}

void build_core_phase1(const char* job, bool bDisableOptimization)
{
	char buf[2048];
	string szOptions;
	setresgid(job_gid, job_gid, job_gid);
	setresuid(job_uid, job_uid, job_uid);

/*
	{
		uid_t ruid, euid, suid;
		gid_t rgid, egid, sgid;

		getresuid(&ruid, &euid, &suid);
		getresgid(&rgid, &egid, &sgid);
		log("ruid=%d euid=%d suid=%d\n", ruid, euid, suid);
		log("rgid=%d egid=%d sgid=%d\n", rgid, egid, sgid);

	}

*/
	if ( bDisableOptimization )
		szOptions += "  DISABLE_OPTIMIZATION  ";

	chdir(sz_workspace.c_str());

	sprintf(buf, "sh ./build_script_phase1 %s/ %s/ %s  %s > /dev/null 2>&1",  sz_workspace.c_str(), sz_src_dir.c_str(),  sz_src_file.c_str(), szOptions.c_str());
	log(buf);
	
	system( buf);

}

struct tKillParam
{
	struct timespec timeout;
	pid_t pid;
	bool bChildTimeout;
	string username;
};


void* kill_child_process(void* _pParam)
{
	int k;				
	char buf[4096];
	tKillParam *pParam = (tKillParam*)_pParam;
	
	//log("kill_child_process timeout = %d", pParam->timeout.tv_sec);


	sleep(pParam->timeout.tv_sec);
	log("killing child\n");
		
				
	/// try the graceful way first
	//// this works for old thick skeleton targets
	for ( k = 0; k < 10; k++ ) {
		sprintf(buf, "ps  -fu %s  |grep -v %s|awk '{print $2}'|xargs kill -USR1", pParam->username.c_str(), HOMEWORK_EXECUTABLE_NAME);
		//log(buf);
		system(buf);
		sleep(1);
	}
				
//	sleep(10);				
				
	/// do the brutal way
	//kill(pid, SIGKILL);
	log("kill pParam->pid %d", pParam->pid);
	if ( pParam->pid)
		kill(pParam->pid, SIGUSR1);
				
	for ( k = 0; k < 10; k++ ) {
		sprintf(buf, "ps  -fu %s  |grep -v %s|awk '{print $2}'|xargs kill -9", pParam->username.c_str(), HOMEWORK_EXECUTABLE_NAME);
		//log(buf);
		system(buf);
		sleep(1);
	}	
	
	pParam->bChildTimeout = true;
	
	return 0;
}


void wait_if_busy()
{
	const float load_threshold = 8;
	const int base_wait_time = 6*60; //seconds

	while(1) {
	    FILE *fp = fopen("/proc/loadavg","rt");
	    float load_1, load_5, load_15;
	    int wait_time;

	    fscanf(fp, "%f %f %f", &load_1, &load_5, &load_15);
	//    printf("%f %f %f\n", load_1, load_5, load_15);

	    fclose(fp);

		if ( load_1 < load_threshold && load_5 < load_threshold)
			break;

		wait_time = (3 + rand()%base_wait_time)*load_15/load_threshold;
		
		log("load avg : %f %f %f   sleep for %d seconds", load_1, load_5, load_15, wait_time);

		sleep(wait_time);

	}
}


/// job = user id
void build(const char* job, bool bDisableOptimization) {
//	sigset_t mask;
//	sigset_t orig_mask;
	struct timespec timeout;
	pid_t pid;
	char buf[4096];
 	tKillParam	paramKill;
	time_t tmStart, tmEnd;
	int ino_fd_new_reports;


	wait_if_busy();

	//////////////////////////////////////////////////

//	char file_lock[512];
//	int fd_lock;

	sz_workspace = "/tmp/placeholder";

//	sprintf(file_lock, "%s/.lock", homework_job_queue);

	//	log("build %s\n", file_lock);

	{
		char *p;
		int n;

		time(&tmStart);


		sz_job_full_path = homework_job_queue;
		sz_job_full_path = sz_job_full_path + "/" + job;


		if ( (n=readlink(sz_job_full_path.c_str(), buf, sizeof(buf) - 1)) < 0)
			goto error_quit;

		buf[n] = 0;


		if (!(p = strrchr(buf, '/')))
			goto error_quit;

		*p = 0;

		sz_src_dir = buf;

		sz_src_file = p + 1;

		{

			uuid_t uu;
			char str[256];
/*
			uuid_t *uuid;
            char *str;

            uuid_create(&uuid);
            uuid_make(uuid, UUID_MAKE_V1);
            str = NULL;
            uuid_export(uuid, UUID_FMT_STR, &str, NULL);
			 sprintf(buf, "%s/job_%s_%s", workspace_root, job, str);
			free(str);	
            uuid_destroy(uuid);
*/
			uuid_generate(uu);
			uuid_unparse_upper(uu, str);
			sprintf(buf, "%s/job_%s_%s", workspace_root, job, str);

		
//			uuid_generate(uu);
//			unsigned long long *p_uu = (unsigned long long *)uu;

		}

		sz_workspace = buf;

		sprintf(buf, "\\rm -rf %s", sz_workspace.c_str());

		//		log("cmd [%s]\n", buf);

		system(buf);

		if (mkdir(sz_workspace.c_str(), 0770) < 0)
			log("mkdir errno = %d\n", errno);

		chown(sz_workspace.c_str(), job_uid, job_gid);

		log("Processing job [%s] src_dir [%s]  src_file [%s]", job,
				sz_src_dir.c_str(), sz_src_file.c_str());


		sprintf(buf, "cp -f %s/%s %s/%s",sz_src_dir.c_str(), sz_src_file.c_str(), sz_workspace.c_str(), sz_src_file.c_str());
		system(buf);

		sprintf(buf, "cp -f %s/%s.zip %s/%s.zip",sz_src_dir.c_str(), sz_src_file.c_str(), sz_workspace.c_str(), sz_src_file.c_str());	//for customized upload
		system(buf);

		chown((sz_workspace + "/" + sz_src_file).c_str(), job_uid, job_gid);

//		symlink((sz_src_dir + "/" + sz_src_file).c_str(), buf);

		sprintf(buf, "cp -f %s %s/build_script_phase0", build_script_phase0, sz_workspace.c_str());
		system(buf);
		sprintf(buf, "%s/build_script_phase0", sz_workspace.c_str());
		chown(buf, job_uid, job_gid);


		sprintf(buf, "cp -f %s %s/build_script_phase1", build_script_phase1, sz_workspace.c_str());
		system(buf);
		sprintf(buf, "%s/build_script_phase1", sz_workspace.c_str());
		chown(buf, job_uid, job_gid);



		//// run init_script
		sprintf(buf, "sh %s %s/ %s/ %s %s %d %d", init_script, sz_workspace.c_str(), sz_src_dir.c_str(), sz_src_file.c_str(), HW_NAME.c_str(), job_uid, job_gid);
		system(buf);

	}


	/////////////////////////////////////////////////

	{
		pthread_t tidMoveNewReports;
		pthread_create(&tidMoveNewReports, 0, monitor_new_reports, &ino_fd_new_reports);

	}



	///////// phase 0 /////////////////
	pid = fork();

	if (pid == -1) {
		log("failed to fork child process");
		return;
	}

	if (pid == 0) {
		build_core_phase0(job, bDisableOptimization);
		exit(0);
	}
	///////////////////////////////////


	if (waitpid(pid, NULL, 0) < 0) {
		log("waitpid error after phase 0");
	}
//	close(ino_fd_new_reports);
	move_reports();

	if ( !phase0_check(sz_src_dir.c_str()) )
		goto finish_build;

	/////////// phase 1 //////////////
	

/*	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);
			
	if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0) {
		log("sigprocmask");
		return;
	}	
	*/
	
	pid = fork();

	if (pid == -1) {
		log("failed to fork child process");
		return;
	}

	{	// Hank 2015.1.17
		sprintf(buf, "%s/input.txt", sz_workspace.c_str());
		chmod(buf, 0666);
		sprintf(buf, "%s/warmup.txt", sz_workspace.c_str());
		chmod(buf, 0666);		
	}

	if (pid == 0) {
//		pthread_t tidMoveNewReports;
//		pthread_create(&tidMoveNewReports, 0, monitor_new_reports, &ino_fd_new_reports);
		
		build_core_phase1(job, bDisableOptimization);
		exit(0);
	}
	
	///////////////////////////////////////////////////
	

	timeout.tv_sec = CHILD_PROCESS_TIMEOUT_SECONDS;
	timeout.tv_nsec = 0;
	
	
	log("timeout = %d", timeout.tv_sec);

	{	// write timeout 
		FILE* fp = fopen( (sz_src_dir+"/"+TIMEOUT_FILE).c_str(), "wt");
		time_t t = time(0) + timeout.tv_sec;
		log("Will expire at %s", asctime(localtime(&t)));
		fprintf(fp, "%s", asctime(localtime(&t)));
		fclose(fp);
	}


	pthread_t tidKillChild;

	paramKill.timeout = timeout;
	paramKill.pid = pid;
	paramKill.bChildTimeout = false;
	paramKill.username = username;
	
	pthread_create(&tidKillChild, 0, kill_child_process, &paramKill);



	if (waitpid(pid, NULL, 0) < 0 || paramKill.bChildTimeout == true) {
	//	log("waitpid");
		tKillParam	k2;

		log("deep clean...");
		k2.timeout.tv_sec = 0;
		k2.timeout.tv_nsec = 0;
		k2.pid = 0;
		k2.bChildTimeout = false;
		k2.username = username;
		kill_child_process(&k2);
		sleep(5);		
	}

	{	// Hank 2015.1.17

/*		tKillParam	k2;

		log("clean up...");
		k2.timeout.tv_sec = 0;
		k2.timeout.tv_nsec = 0;
		k2.pid = 0;
		k2.bChildTimeout = false;
		k2.username = username;
		kill_child_process(&k2);
		sleep(5);
*/
		sprintf(buf, "cd %s; ./verify.sh memcheck", sz_workspace.c_str());
		system(buf);
		sprintf(buf, "cd %s; ./verify.sh callgrind", sz_workspace.c_str());
		system(buf);
		sprintf(buf, "cd %s; ./verify.sh massif", sz_workspace.c_str());
		system(buf);
		sprintf(buf, "cd %s; ./verify.sh vanilla", sz_workspace.c_str());
		system(buf);		
	}	

//	close(ino_fd_new_reports);
	move_reports();
/////////////////////////////////////////////////

	
	time(&tmEnd);
	
	{	// write time report
		time_t span = tmEnd - tmStart;
		
		char buf[512];
		FILE *fp;
				
		sprintf(buf, "%s/%s", sz_src_dir.c_str(), TIME_REPORT);
		
		log ("execution takes %d seconds", span);
		
		if( fp = fopen(buf, "wt") ) {
			fprintf(fp, "time=\"%d\"\n", span);
			fprintf(fp, "timeout=\"%d\"\n", paramKill.bChildTimeout);
			fclose(fp);
		}
		
	}
	
	

	if ( paramKill.bChildTimeout ) {
		char buf[512];
		FILE *fp;
		
		sprintf(buf, "%s/%s", sz_src_dir.c_str(), ANALYSIS_RESULT);
		if( fp = fopen(buf, "wt") ) {
			fprintf(fp, "Process aborted...analysis report not available! \n");
			fclose(fp);
		}

		sprintf( buf, "cd %s; python3 %s/progress_log.py analysis_result 1> /dev/null 2>&1", sz_src_dir.c_str(), sz_workspace.c_str());
		system(buf);	
	}
	else{
	
		try {	
			analyze_report(sz_src_dir.c_str(), sz_workspace.c_str());
		}
		catch(analysis_error& e) {
			//goto cleanup;
		}	
		
finish_build:;
		sprintf(buf,  "cd %s; python3 %s/progress_log.py illegal_headers > /dev/null 2>&1", sz_src_dir.c_str(), sz_workspace.c_str());
		system(buf);
		sprintf( buf, "cd %s; python3 %s/progress_log.py analysis_result 1 > /dev/null 2>&1", sz_src_dir.c_str(), sz_workspace.c_str());
		log(buf);
		system(buf);
		sprintf( buf, "cd %s; python3 %s/progress_log.py check_pattern_concise > /dev/null 2>&1", sz_src_dir.c_str(), sz_workspace.c_str());
		system(buf);
		sprintf( buf, "cd %s; python3 %s/progress_log.py callcheck > /dev/null 2>&1", sz_src_dir.c_str(), sz_workspace.c_str());
		system(buf);
		

	}

cleanup:;

	unlink(sz_job_full_path.c_str());

error_quit: ;
	//////////////////////
//	flock(fd_lock, LOCK_UN);



//	close(fd_lock);

	{
		vector<string> v;
		
		if( GetConfigEntry("delete_workspace", v) && v.size()==1 && v[0]=="1") {

			sprintf(buf, "\\rm -rf %s", sz_workspace.c_str());
			system(buf);
		}

	}

}
#if 0
void* parse_queue(void* pParam) {
	DIR *dir;
	dirent *dirp;


	timespec ts;
	timeval tv;

	while (1) {

		if (!(dir = opendir(homework_job_queue)))
			goto next;

		while ((dirp = readdir(dir)) != NULL) {
			if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")
					|| !strcmp(dirp->d_name, ".lock"))
				continue;
			//	sprintf(FULL_PATH,"%s/%s", homework_job_queue, dirp->d_name);
			//			log("Found [%s]\n", FULL_PATH);
			build(dirp->d_name);
		}

		closedir(dir);
		next: ;
		gettimeofday(&tv, 0);
		ts.tv_nsec = 0;
		ts.tv_sec = tv.tv_sec + 30;
		pthread_cond_timedwait(&cond_process, &mutex_queue, &ts);
		//pthread_cond_wait(&cond_process, &mutex_queue);
		pthread_mutex_unlock(&mutex_queue);
	}

	return 0;
}
#endif

void* monitor_new_reports(void* _pParam) 
{
	//	  GMainLoop *loop;
	//	  GIOChannel *channel;
	int ino_fd = 0;
	int ino_wd = 0;

	/* inotify */
	ino_fd = inotify_init();
	ino_wd = inotify_add_watch(ino_fd, sz_workspace.c_str(), IN_CLOSE_WRITE );

	*((int*)_pParam) = ino_fd;

	if (ino_wd < 0)
		log("inotify_add_watch");

	/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof (struct inotify_event))

	/* reasonable guess as to size of 1024 events */
#define BUF_LEN        (1024 * (EVENT_SIZE + 16))

	char buf[BUF_LEN];

	log("monitoring %s", sz_workspace.c_str());
	

	while (1) {
		int len;
		int i = 0;

		len = read(ino_fd, buf, BUF_LEN);
		

		 while (i < len) {
			 struct inotify_event *event;

			 event = (struct inotify_event *) &buf[i];

		//	 log ("wd=%d mask=%u cookie=%u len=%u  name=%s\n", event->wd, event->mask,		 event->cookie, event->len, event->name);

			 if (event->len && strstr(event->name, ".report.final") ) {
 			//	log ("name=%s", event->name);
				move_reports(event->name);
			 }


			 i += EVENT_SIZE + event->len;
			 
		 }

//		pthread_cond_signal(&cond_process);
		

	}
}

void test()
{
	int k;
	/*
	int v;

	if ( ExtractNumber(v, "Error: 423,12,3","Error:") )
		log("v=%d\n",v);

	if ( ExtractNumber(v, "Error: 423,12,3 ","Error:") )
			log("v=%d\n",v);

	if ( ExtractNumber(v, "Error: 423,12,3 dog","Error:", "dog") )
			log("v=%d\n",v);

	if ( ExtractNumber(v, "Error:423,12,3 dog","Error:", "dog") )
			log("v=%d\n",v);

	if ( ExtractNumber(v, "Error: 423,12,3dog","Error:", "do") )
			log("v=%d\n",v);*/

	{

	//	ExtractNumber(k, "I refs: 1,420,900", "I refs:");
	}

//	vector<string> results;

	//analyze_gcc_dependency_report_file("/tmp/a.txt", "/tmp/b.txt", results );
	/*
	analyze_valgrind_memcheck_report_file ("/var/homeworks/Hank/valgrind.memcheck.report", results);

	analyze_valgrind_callgrind_report_file("/var/homeworks/Hank/valgrind.callgrind.report", results);

	analyze_gcc_exit_code_report_file("/var/homeworks/Hank/"GCC_EXIT_CODE_REPORT, results);

	
	
	for ( k = 0; k < results.size(); k++)	 log("[%s]\n", results[k].c_str());*/
}

bool HasArgument(int argc, char* argv[], const char* argument)
{
	int k;

	for ( k = 1; k < argc; k++) {
		if ( !strcmp(argv[k],argument) )
			return true;
	}

	return false;
}

void FinalizeAnalysisResult()
{
	char buf[1024];
	sprintf(buf, "mv -f %s/current/%s  %s/current/%s.final", homework_user_root_dir, ANALYSIS_RESULT, homework_user_root_dir, ANALYSIS_RESULT);
	system(buf);
}


void OverwriteAnalysisResult(const char* msg)
{
	char buf[512];
	FILE *fp;

	sprintf(buf, "%s/current/%s", homework_user_root_dir, ANALYSIS_RESULT);
	if( fp = fopen(buf, "wt") ) {
		fprintf(fp, "%s\n", msg);
		fclose(fp);
	}
	
		
	sprintf(buf,  "cd %s/current; python3 /usr/bin/HomeworkInspector/%s/progress_log.py analysis_result 1 > /dev/null 2>&1",  homework_user_root_dir, HW_NAME.c_str());
	log(buf);
	system(buf);
}


int main(int argc, char* argv[]) {
	pthread_t tid;

	nice(25);

	srand(time(0));

	//	pthread_attr_t attr;

	if ( argc < 3 ) {
		printf("homeworkinspector id hw_name\n");
		return -1;
	}
	
	HW_NAME = argv[2];
	
	sprintf(homework_user_root_dir, "/var/homeworks/%s/%s", argv[2], argv[1]);
	sprintf(homework_job_queue, "/var/homeworks/%s/queue", argv[2]);
	sprintf(CONFIG_FILE,  "/var/homeworks/%s/config/homework_inspector_config", argv[2]);
	sprintf(homework_inspector_dir, "/usr/bin/HomeworkInspector/%s", argv[2]);
	sprintf(init_script, "/usr/bin/HomeworkInspector/%s/init_script", argv[2]);
	sprintf(build_script_phase0, "/usr/bin/HomeworkInspector/%s/build_script_phase0", argv[2]);
	sprintf(build_script_phase1, "/usr/bin/HomeworkInspector/%s/build_script_phase1", argv[2]);
	sprintf(HOMEWORK_SETTING_FILE, "%s/current/setting", homework_user_root_dir);	


	InitLog();

	
	
	{
		//TestCheckSyscallUsage();
		//return 0;
		//test();
//		merge_check_pattern_files("/var/homeworks/1000001");
//		return 0;
	}


	LoadConfigFile(CONFIG_FILE);
	LoadConfigFile(HOMEWORK_SETTING_FILE, false);

	{
		vector<string> v;
		
		if ( GetConfigEntry("child_process_timeout_seconds",v) && v.size()==1 ) {
			CHILD_PROCESS_TIMEOUT_SECONDS = atoi(v[0].c_str());
		}
		else {
			log("Error ! can't find child process timeout value from config file");
			assert(false);
			exit(-1);
		}

		log("child process timeout in %d seconds", CHILD_PROCESS_TIMEOUT_SECONDS);
	}
	
	if( signal(SIGUSR1, sig_timeout)==SIG_ERR) {
		printf("Failed to install signal handler for timeout signal . \n");
	}	
	

	pthread_mutex_init(&mutex_queue, NULL);
	pthread_cond_init(&cond_process, NULL);

	sprintf(username, "nobody"); // safety backup
	
	
	{
		uid_t ruid, euid, suid;
		gid_t rgid, egid, sgid;

		setresuid(0, 0 , 0);
		setresgid(0, 0 , 0);
	/*	
		getresuid(&ruid, &euid, &suid);
		getresgid(&rgid, &egid, &sgid);
		log("ruid=%d euid=%d suid=%d\n", ruid, euid, suid);
		log("rgid=%d egid=%d sgid=%d\n", rgid, egid, sgid);
*/
//		CheckSyscallUsage();
//		exit(0);

	}
	
	if ( argc>=3) {
		char buf[2048];
		passwd *pwd;

		system(buf);
		
		sprintf(username, "%s%s", ID_PREFIX, argv[1] );
		sprintf(buf, "/usr/sbin/useradd -s /sbin/nologin %s", username);
		system(buf);
		
		{
			struct passwd pwd;
	        struct passwd *result;
	        char *buf;
	        size_t bufsize;
	        int s;

           bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
           if (bufsize == -1)          /* Value was indeterminate */
               bufsize = 16384;        /* Should be more than enough */

           buf = (char*)malloc(bufsize);
           if (buf == NULL) {
               perror("malloc");
               exit(EXIT_FAILURE);
           }

           s = getpwnam_r(username, &pwd, buf, bufsize, &result);
	           
           if (result == NULL) {
	           if (s == 0)
	                 log("%s Not found\n", username);
	           else {
	                 errno = s;
	                 perror("getpwnam_r");
	           }
	           exit(EXIT_FAILURE);
	       }

	        log("Name: %s; UID: %ld", pwd.pw_gecos, (long) pwd.pw_uid);
   			job_uid = pwd.pw_uid;
   			job_gid = pwd.pw_gid;
			free(buf);
		}
#if 0			
		if ( argc==4  &&  !strcmp(argv[3],"kill") ) {	//kill existing job
			char buf[1024];

			sprintf(buf, "%s/%s", homework_job_queue, argv[1]);	// argv[1] is like '10000001'
			log(buf);
			unlink(buf);
			
			int k;

			for ( k = 0; k < 10; k++ ) {
//					system("killall -9 -Z homework_t");
				//sprintf(buf, "killall -9 -w -u %s", username);
				sprintf(buf, "ps  -fu %s  |grep -v %s|awk '{print $2}'|xargs kill -9", username, HOMEWORK_EXECUTABLE_NAME);
				//log(buf);
				system(buf);
				sleep(1);
			}
			
			OverwriteAnalysisResult("process killed !");
			/*
			sprintf(buf, "killall -9 -w -u %s", username);
			log(buf);
			system(buf);
*/
		}
		else
#endif
		if  ( argc==4  &&  ( !strcmp(argv[3],"expire") ||  !strcmp(argv[3],"kill") ) ) {	//kill existing job
/*	
			Hank 2005.1.27 : This no longer works for customized template where the target (e.g. java, js programs) do not accept USR1 signal


			int k;

			for ( k = 0; k < 10; k++ ) {
				sprintf(buf, "ps  -fu %s  |grep -v %s|awk '{print $2}'|xargs kill -USR1", username, HOMEWORK_EXECUTABLE_NAME);
				//log(buf);
				system(buf);
				sleep(1);
			}			
*/
			tKillParam	paramKill;

			paramKill.timeout.tv_sec = 0;
			paramKill.timeout.tv_nsec = 0;
			paramKill.pid = 0;
			paramKill.bChildTimeout = false;
			paramKill.username = username;
			log("expiring existing job...");
			kill_child_process(&paramKill);


			OverwriteAnalysisResult( "process expired !");
		}
		else {
			bool bDisableOptimization = false;

			if ( HasArgument(argc, argv, "DISABLE_OPTIMIZATION") )
				bDisableOptimization = true;

			build(argv[1] , bDisableOptimization);
		}
	}
//	pthread_create(&tid, NULL, parse_queue, 0);
//	monitor_change();

	FinalizeAnalysisResult();

	CloseLog();

	return 0;
}


void InitLog()
{
	if (fpLog)
		CloseLog();

	fpLog = fopen(LOG_FILE, "a+t");
}

void CloseLog()
{
	fclose(fpLog);
	fpLog = 0;
}

int log(const char* format, ...)
{
	int ret;
	va_list ap_list;


	//LOCK_MUTEX(&mutexIRSLog,true);

	va_start( ap_list, format );     /* Initialize variable arguments. */
	ret = vfprintf(stdout,format,ap_list);
	va_end(ap_list);
	printf("\n");


	if ( fpLog) {
		char buf[512];
		char *p;
		time_t t;
		time(&t);
		strcpy(buf, ctime(&t));
		if( p = strchr(buf,'\n') )
			*p = 0;
		fprintf(fpLog, "[%s] (pid:%d) \t", buf, getpid());
		
		va_start( ap_list, format );
		vfprintf(fpLog,format,ap_list);
		va_end(ap_list);

		fprintf(fpLog, "\n");

		fflush(fpLog);
	}



	//UNLOCK_MUTEX(&mutexIRSLog);
 
	return ret;
}


static void sig_timeout(int signo)
{
	if ( signo==SIGUSR1) {
		log("HomeworkInspector -USR1 received.... aborting !");
		exit(-5);
	}
}
