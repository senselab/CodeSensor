#ifndef _DATA_H_
#define _DATA_H_

#define SHM_NAME "hw_test"

struct tTestData {
    int cnt;
    int data[5][20000];
};


#ifdef WRITER
#include <fstream>

// return true if success, false otherwise
bool load_data(tTestData* test_data) {
    std::ifstream ifs("input.txt");

    if (!ifs.is_open()) return false;

    ifs >> test_data->cnt;
    for (int cases = 0; cases < test_data->cnt; ++cases) {
        for (int i = 0; i < 20000; ++i) {
            ifs >> test_data->data[cases][i];
        }
    }

    ifs.close();
    return true;
}

#endif // WRITER

#endif // _DATA_H_
