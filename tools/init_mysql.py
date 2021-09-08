#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
import os

if len(sys.argv) != 4:
    print('usage: python3 init_mysql.py [MYSQL_DB_NAME] [MYSQL_USER_ID] [MYSQL_USER_PWD]')
    sys.exit(-1)


script_dir = os.path.dirname(os.path.realpath(__file__)) + '/'

with open(script_dir + 'templates/config_php', 'r') as f, open(script_dir + '../HomeworkInspector_php/config.php', 'w') as f2:
    for line in f:
        if '$' in line:
            f2.write(line)
            f2.write(f'$MYSQL_DB_NAME = "{sys.argv[1]}";\n')
            f2.write(f'$MYSQL_USER_ID = "{sys.argv[2]}";\n')
            f2.write(f'$MYSQL_USER_PWD =  "{sys.argv[3]}";\n')
            break
        else:
            f2.write(line)

    f2.write(f.read())

with open(script_dir + 'templates/dbloader_js', 'r') as f, open(script_dir + 'DBLoader_nodejs/dbloader.js', 'w') as f2:
    f2.write(f"var dbname = '{sys.argv[1]}';\n")
    f2.write(f"var mysql_account = '{sys.argv[2]}';\n")
    f2.write(f"var mysql_pwd = '{sys.argv[3]}';\n")
    f2.write(f.read())

