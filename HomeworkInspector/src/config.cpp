/*
 * config.cpp
 *
 *  Created on: Sep 30, 2010
 *      Author: Hank
 */

#include <string>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"

using namespace std;

char CONFIG_FILE[512];
char HOMEWORK_SETTING_FILE[512];

int DEBUG_LEVEL = 1;
tConfigSet config_set;
extern int log(const char* format, ...);

string RemoveHeadTailSpace(const string& szSrc)
{
	char *szDup, *szStart;
	string szRetV;
	int k;

	szDup = strdup(szSrc.c_str());

	// remove head space
	szStart = szDup + strspn(szDup, " \t");

	for ( k = strlen(szStart)-1; k>=0; k--) {	// remove tailing spaces
		if ( szStart[k] != ' ' && szStart[k] != '\t' && szStart[k] != '\n' && szStart[k] != '\r') {
			szStart[k+1] = 0;
			break;
		}
	}

	szRetV = szStart;

	free(szDup);

	return szRetV;
}

void LoadConfigFile(const char* szConfigFilename, bool bClearConfigSet )
{
	FILE* fp;
	char buf[2048];
	char *pTok, *pTok2;
	string szProperty, szValueSet;
	vector<string> values;

	if ( bClearConfigSet)
		config_set.clear();

	fp = fopen(szConfigFilename, "rt");

	if (!fp) {
		log("can't open config file %s\n", szConfigFilename);
	}

	while(fgets(buf,sizeof(buf),fp)) {
		if( !(pTok = strtok(buf,"=")) )
			continue;

		szProperty = RemoveHeadTailSpace(pTok);

		if ( !(pTok=strtok(0, "=")) )
			continue;

		values.resize(0);


		pTok2 = strtok(pTok, ",");


		while(pTok2) {
			string s = RemoveHeadTailSpace(pTok2);

			//printf("TOKEN [%s]\n", s.c_str());


			if ( s[0]=='\"' && s.length() >=2)
				values.push_back(s.substr(1, s.length()-2));
			else
				values.push_back(s);
	next:;
			pTok2 = strtok(0, ",");
		}

		if (DEBUG_LEVEL>0) {
			int k;
			char buf[512];
			string s;

			s = szProperty + " = ";

			for ( k = 0; k < values.size(); k++) {
				sprintf(buf, "[%s] ", values[k].c_str());
				s += buf;
			}

			log(s.c_str());
		}

		config_set.insert(tConfigSet::value_type(szProperty, values));
	}

	fclose(fp);

}

vector<string>& SetConfigEntry(const string& szProperty, const vector<string>& values)
{
	pair<tConfigSet::iterator,bool> retV;

	retV = config_set.insert(tConfigSet::value_type(szProperty, values));

	if ( retV.second == false)
		retV.first->second = values;

	return retV.first->second;
}

bool GetConfigEntry(const string& szProperty, vector<string>& values)
{
	tConfigSet::iterator it;

	it =  config_set.find(szProperty);

	if ( it == config_set.end())
		return false;

	values = it->second;

	return true;
}
