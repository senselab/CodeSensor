#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define WRITER
#include "data.hpp"


void open_shm();

int main(int argc, char* argv[])
{
    if (argc > 1) {
        shm_unlink(SHM_NAME);
    } else {
        open_shm();  
    }

    return 0;
}

void open_shm()
{
    int k;
    int fd;
    tTestData* test_data;
    long int size = sizeof(tTestData);
    

    fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRWXU| S_IRGRP| S_IROTH);

    if (fd == -1) {
        std::cout << "shared memory creation falied\n";
        return;
    }

    ftruncate(fd, size);


    test_data = (tTestData*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (!test_data) {
        std::cout << "mmap failed!\n";
        close(fd);
        return;
    }

    if (load_data(test_data) == false) {
        std::cout << "failed to load data\n";
        close(fd);
        return;
    }

    close(fd);
}
