#ifndef _DATA_H_
#define _DATA_H_

#define SHM_NAME "hw0"

struct tTestData {
    int data[3][5];
};


#ifdef WRITER
#include <fstream>

// return true if success, false otherwise
bool load_data(tTestData* test_data) {
    std::ifstream ifs("input.txt");

    if (!ifs.is_open()) return false;

    for (int cases = 0; cases < 3; ++cases) {
        for (int i = 0; i < 5; ++i) {
            ifs >> test_data->data[cases][i];
        }
    }

    ifs.close();
    return true;
}

#endif // WRITER

#endif // _DATA_H_
