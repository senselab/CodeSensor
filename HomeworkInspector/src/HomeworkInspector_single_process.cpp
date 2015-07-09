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
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <uuid/uuid.h>
#include "pcre.h"

using namespace std;

#define DELETE_WORKSPACE

const char homework_job_queue[] = "/var/homeworks/queue";

const char workspace_root[] = "/tmp";
const char init_script[] = "/usr/bin/HomeworkInspector/init_script";
const char build_script[] =	"/usr/bin/HomeworkInspector/build_script";
#define VALGRIND_MEMCHECK_REPORT "valgrind.memcheck.report"
#define VALGRIND_CALLGRIND_REPORT "valgrind.callgrind.report"
#define VALGRIND_MASSIF_REPORT    "valgrind.massif_out.report"
#define GCC_EXIT_CODE_REPORT "gcc_exit_code.report"
#define ANALYSIS_RESULT "analysis_result"
#define LOG_FILE "/var/log/HomeworkInspector.log"

const uid_t job_uid = 99;
const uid_t job_gid = 99;

#define CHILD_PROCESS_TIMEOUT_SECONDS  (60*5)

pthread_cond_t cond_process;
pthread_mutex_t mutex_queue;

FILE	*fpLog = 0;

string sz_src_dir, sz_src_file, sz_workspace, sz_job_full_path;

void InitLog();
void CloseLog();
int log(const char* format, ...);


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

bool ExtractNumber(int& retV, const char* buf, const char* prefix, const char* suffix = "")
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

		retV = atoi(remove_comma(num).c_str());

		pcre_free(re);
		return true;
	}


	pcre_free(re);
	return false;
}


void analyze_gcc_exit_code_report_file(const char* reportfile, vector<string>& result )
{
	FILE *fp;
	char buf[4096];
	int v;




	fp = fopen(reportfile, "rt");


	if ( !fp)
		return;

	while(fgets(buf, sizeof(buf),fp)) {


		if ( ExtractNumber(v,buf,"") )
			result.push_back(string("gcc_exit_code=")+ int2str(v));

	}

quit:;
	fclose(fp);
}

void analyze_valgrind_callgrind_report_file(const char* reportfile, vector<string>& result )
{
	FILE *fp;
	char buf[4096];
	int v;




	fp = fopen(reportfile, "rt");


	if ( !fp)
		return;

	while(fgets(buf, sizeof(buf),fp)) {


		if ( ExtractNumber(v,buf,"I[\\s]+refs:") )
			result.push_back(string("I_refs=")+ int2str(v));

	}

quit:;
	fclose(fp);
}


void analyze_valgrind_memcheck_report_file(const char* reportfile, vector<string>& result )
{
	FILE *fp;
	char buf[4096];
	int v;




	fp = fopen(reportfile, "rt");


	if ( !fp)
		return;

	while(fgets(buf, sizeof(buf),fp)) {


		if ( ExtractNumber(v,buf,"ERROR SUMMARY:", "errors") )
			result.push_back(string("mem_error=")+ int2str(v));
/*
		if ( strstr(buf, "total heap usage:") ) {
			if ( ExtractNumber(v,buf,"frees,","bytes allocated" )) {
				result.push_back(string("heap_usage="+ int2str(v)));
			}
		}
		*/
		if ( strstr(buf, "in use at exit:") ) {
			if ( ExtractNumber(v,buf,"in use at exit:","bytes" )) {
				result.push_back(string("heap_lost="+ int2str(v)));
			}
		}
	}

quit:;
	fclose(fp);
}

struct tMassifRecord
{
	int nSnapshot;
	int mem_heap, mem_heap_extra, mem_stacks;

	void Reset()
	{
		mem_heap = 0;
		mem_heap_extra = 0;
		mem_stacks = 0;
		nSnapshot = -1;
	}

	tMassifRecord()
	{
		Reset();
	}

	int GetTotal()
	{
		return mem_heap + mem_heap_extra + mem_stacks;
	}
};

void analyze_valgrind_massif_report_file(const char* reportfile, vector<string>& result )
{
	FILE *fp;
	char buf[4096];
	int v;
	
	tMassifRecord curR, maxR;


	fp = fopen(reportfile, "rt");


	if ( !fp) {
		log("unable to open %s\n", reportfile);
		return;
	}

			

	while(fgets(buf, sizeof(buf),fp)) {

//		printf("%s\n", buf);

		if ( ExtractNumber(v,buf,"snapshot=") ) {

			if ( maxR.nSnapshot == -1 || curR.GetTotal() >  maxR.GetTotal() )
				maxR = curR;

	///		printf("snapshot=%d\n", v);

			curR.Reset();
			curR.nSnapshot = v;
		}

		if ( ExtractNumber(v,buf,"mem_heap_B=", "") )
			curR.mem_heap = v;

		if ( ExtractNumber(v,buf,"mem_heap_extra_B=", "") )
			curR.mem_heap_extra = v;

		if ( ExtractNumber(v,buf,"mem_stacks_B=", "") )
			curR.mem_stacks = v;

	}

	if ( curR.nSnapshot != -1 && curR.GetTotal() > maxR.GetTotal())
		maxR = curR;

	if ( maxR.nSnapshot != -1) {
		result.push_back(string("mem_heap="+ int2str(maxR.mem_heap)));
		result.push_back(string("mem_heap_extra="+ int2str(maxR.mem_heap_extra)));
		result.push_back(string("mem_stacks="+ int2str(maxR.mem_stacks)));
		result.push_back(string("mem_total=" + int2str(maxR.GetTotal())));
	}

quit:;
	fclose(fp);
}


void analyze_report(const char* szSrcDir)
{
	char buf[2048];
	vector<string> results;
	FILE *fpOut;


	sprintf(buf, "%s/%s", szSrcDir, VALGRIND_MEMCHECK_REPORT);
	analyze_valgrind_memcheck_report_file (buf, results);

	sprintf(buf, "%s/%s", szSrcDir, VALGRIND_CALLGRIND_REPORT);
	analyze_valgrind_callgrind_report_file(buf, results);

	sprintf(buf, "%s/%s", szSrcDir, VALGRIND_MASSIF_REPORT);
	analyze_valgrind_massif_report_file(buf, results);


	sprintf(buf, "%s/%s", szSrcDir, GCC_EXIT_CODE_REPORT);
	analyze_gcc_exit_code_report_file(buf, results);

	sprintf(buf, "%s/%s", szSrcDir, ANALYSIS_RESULT);

	if ( fpOut = fopen(buf, "wt")) {
		int k;
		for ( k = 0; k < results.size(); k++)	 fprintf(fpOut,"%s\n", results[k].c_str());

		fclose(fpOut);
	}

}

void move_reports() {
	DIR *dir;
	dirent *dirp;
	char buf[4096];

	log("move reports from [%s]...\n", sz_workspace.c_str());

	if (!(dir = opendir(sz_workspace.c_str())))
		goto next;

	while ((dirp = readdir(dir)) != NULL) {


		if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")
				|| !strstr(dirp->d_name, ".report"))
			continue;

		sprintf(buf, "cp -f %s/%s %s", sz_workspace.c_str(), dirp->d_name,	sz_src_dir.c_str());
		//log("cmd [%s]\n", buf);
		system(buf);
	}

	next: ;
	closedir(dir);

	///// fix mode
	sprintf(buf, "chmod 644 %s/*", sz_src_dir.c_str());
	log("execute [%s]\n",buf);
	system(buf);
}

void build_core(const char* job)
{
	char buf[2048];

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
	chdir(sz_workspace.c_str());
	sprintf(buf, "sh ./build_script %s/ %s/ %s > build_script.report 2>&1",  sz_workspace.c_str(), sz_src_dir.c_str(),  sz_src_file.c_str());
	system( buf);

}

void build(const char* job) {
	sigset_t mask;
	sigset_t orig_mask;
	struct timespec timeout;
	pid_t pid;
	char buf[4096];
	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);

	//////////////////////////////////////////////////

	char file_lock[512];
	int fd_lock;

	sz_workspace = "/tmp/placeholder";

	sprintf(file_lock, "%s/.lock", homework_job_queue);

	//	log("build %s\n", file_lock);

retry: ;

	fd_lock = open(file_lock, O_CREAT, S_IRWXU );

	if (fd_lock < 0) {
		if (errno == EINTR)
			goto retry;

		log("failed to open lock errno = %d \n", errno);
		goto quit;
	}

	if (flock(fd_lock, LOCK_EX) < 0) {

		/*log("EWOULDBLOCK=%d\n", EWOULDBLOCK);
		 log("EBADF=%d\n",EBADF);*/

		log("flock error code = %d\n", errno);
		goto quit;
	}

	{
		char *p;
		int n;



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

		uuid_t uu;
		uuid_generate(uu);
		unsigned long long *p_uu = (unsigned long long *)uu;


		sprintf(buf, "%s/job_%s_%llX_%llX", workspace_root, job, p_uu[0] , p_uu[1]);

		sz_workspace = buf;

		sprintf(buf, "\\rm -rf %s", sz_workspace.c_str());

		//		log("cmd [%s]\n", buf);

		system(buf);

		if (mkdir(sz_workspace.c_str(), 0770) < 0)
			log("mkdir errno = %d\n", errno);

		chown(sz_workspace.c_str(), job_uid, job_gid);

		log("Processing job [%s] src_dir [%s]  src_file [%s]\n", job,
				sz_src_dir.c_str(), sz_src_file.c_str());

		///gen gpg sig
		{
			sprintf(buf, "gpg --yes -ab -o %s/gpg.sig.asc %s/%s", sz_src_dir.c_str(), sz_src_dir.c_str(), sz_src_file.c_str());
			system(buf);
		}


		sprintf(buf, "cp -f %s/%s %s/%s",sz_src_dir.c_str(), sz_src_file.c_str(), sz_workspace.c_str(), sz_src_file.c_str());
		system(buf);

		chown((sz_workspace + "/" + sz_src_file).c_str(), job_uid, job_gid);

//		symlink((sz_src_dir + "/" + sz_src_file).c_str(), buf);

		sprintf(buf, "cp -f %s %s/build_script", build_script, sz_workspace.c_str());
		system(buf);

		sprintf(buf, "%s/build_script", sz_workspace.c_str());
		chown(buf, job_uid, job_gid);

		//// run init_script
		sprintf(buf, "sh %s %s/ %s/ %s", init_script, sz_workspace.c_str(), sz_src_dir.c_str(), sz_src_file.c_str());
		system(buf);

	}


	/////////////////////////////////////////////////

	if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0) {
		log("sigprocmask");
		return;
	}

	pid = fork();

	if (pid == -1) {
		log("failed to fork child process\n");
		return;
	}

	if (pid == 0) {
		build_core(job);
		exit(0);
	}

	timeout.tv_sec = CHILD_PROCESS_TIMEOUT_SECONDS;
	timeout.tv_nsec = 0;

	do {
		if (sigtimedwait(&mask, NULL, &timeout) < 0) {
			if (errno == EINTR) {
				/* Interrupted by a signal other than SIGCHLD. */
				continue;
			} else if (errno == EAGAIN) {
				log("Timeout, killing child\n");
				kill(pid, SIGKILL);

				int k;

				for ( k = 0; k < 100; k++ ) {
					system("killall -9 -Z homework_t");
					system("killall -9 -u nobody");
				}
			} else {
				log("sigtimedwait");
				break;
			}
		}

		break;
	} while (1);

	if (waitpid(pid, NULL, 0) < 0) {
		log("waitpid");
	}

/////////////////////////////////////////////////
	move_reports();
	analyze_report(sz_src_dir.c_str());


	unlink(sz_job_full_path.c_str());

error_quit: ;
	//////////////////////
	flock(fd_lock, LOCK_UN);

quit: ;

	close(fd_lock);

#ifdef DELETE_WORKSPACE
	sprintf(buf, "\\rm -rf %s", sz_workspace.c_str());
	system(buf);
#endif


}

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

void monitor_change() {
	//	  GMainLoop *loop;
	//	  GIOChannel *channel;
	int ino_fd = 0;
	int ino_wd = 0;

	/* inotify */
	ino_fd = inotify_init();
	ino_wd = inotify_add_watch(ino_fd, homework_job_queue, IN_CREATE );

	if (ino_wd < 0)
		log("inotify_add_watch");

	/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof (struct inotify_event))

	/* reasonable guess as to size of 1024 events */
#define BUF_LEN        (1024 * (EVENT_SIZE + 16))

	char buf[BUF_LEN];

	while (1) {
		int len;//, i = 0;

		len = read(ino_fd, buf, BUF_LEN);
		/*

		 while (i < len) {
		 struct inotify_event *event;

		 event = (struct inotify_event *) &buf[i];

		 log ("wd=%d mask=%u cookie=%u len=%u\n",
		 event->wd, event->mask,
		 event->cookie, event->len);

		 if (event->len)
		 log ("name=%s\n", event->name);

		 i += EVENT_SIZE + event->len;
		 }*/

		pthread_cond_signal(&cond_process);

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

	vector<string> results;

	analyze_valgrind_memcheck_report_file ("/var/homeworks/Hank/valgrind.memcheck.report", results);

	analyze_valgrind_callgrind_report_file("/var/homeworks/Hank/valgrind.callgrind.report", results);

	analyze_gcc_exit_code_report_file("/var/homeworks/Hank/"GCC_EXIT_CODE_REPORT, results);

	for ( k = 0; k < results.size(); k++)	 log("[%s]\n", results[k].c_str());
}

int main() {
	pthread_t tid;

	//	pthread_attr_t attr;
//	test();

	InitLog();

	pthread_mutex_init(&mutex_queue, NULL);
	pthread_cond_init(&cond_process, NULL);

	pthread_create(&tid, NULL, parse_queue, 0);
	monitor_change();

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


	if ( fpLog) {
		va_start( ap_list, format );
		vfprintf(fpLog,format,ap_list);
		va_end(ap_list);
		fflush(fpLog);
	}



	//UNLOCK_MUTEX(&mutexIRSLog);
 
	return ret;
}
