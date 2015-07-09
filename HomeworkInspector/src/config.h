/*
 * config.h
 *
 *  Created on: Sep 30, 2010
 *      Author: Hank
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <map>
#include <vector>

using namespace std;

typedef map<string, vector<string> > tConfigSet;

extern tConfigSet config_set;

extern char CONFIG_FILE[512];
extern char HOMEWORK_SETTING_FILE[512];
//#define CONFIG_FILE "/var/homeworks/config/homework_inspector_config"

void LoadConfigFile (const char* szConfigFilename, bool bClearConfigSet =true);


vector<string>& SetConfigEntry(const string& szProperty, const vector<string>& values);
bool GetConfigEntry(const string& szProperty, vector<string>& values);


#endif /* CONFIG_H_ */
