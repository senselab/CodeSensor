#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "data.hpp"


void solve(tTestData*);


int main()
{
    int fd;
    long int size = sizeof(tTestData);


  //  fd = shm_open(SHM_NAME, O_RDWR, 0);
    fd = shm_open(SHM_NAME, O_RDONLY, 0);

    if (fd == -1) {
        std::cout << "Failed to open shared memory\n";
        return -1;
    }

    tTestData* test_data = (tTestData*)mmap(NULL, size, PROT_READ , MAP_SHARED, fd, 0);

    if (!test_data) {
        std::cout << "mmap failed!\n";
        return -2;
    }

    solve(test_data);

    return 0;
}

