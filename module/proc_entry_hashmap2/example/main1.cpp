#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <thread>
#include <memory>
#include <vector>
#include <random>

#include <unistd.h>
#include <sys/file.h>
#include <sys/fcntl.h>

#include "../kmod/proc_entry_hashmap.h"

class Worker
{
public:
    int fd; // file
    int id;
    std::unique_ptr<std::thread> thread;
    volatile bool running;

    Worker(int id, const char *path)
        : id(id)
    {
        fd = open(path, O_RDWR);
        if (fd <= 0)
        {
            perror("open failed");
            // goto open_err;
        }

        running = true;
        thread = std::make_unique<std::thread>([this]() { this->run(); });
    }

    ~Worker()
    {
        running = false;
        thread->join();
    }

    void run()
    {
        printf("Worker::run, id: %d\n", id);

        std::mt19937 rand(time(NULL));

        while (running)
        {
            // printf("rand: %lld\n", rand());

            struct hello_entry_param param = {
                .operation = HELLO_ENTRY_INSERT,
                .key = (long long)rand() & 0xffffff,
                .value = (long long)rand(),
            };

            // for (int i = 0; i < 3; i++)
            if (write(fd, &param, sizeof(param)) < 0)
            {
                perror("insert error");
                running = false;
            }
           //  printf("insert key: %lld, value: %lld\n", param.key, param.value);

            param.operation = HELLO_ENTRY_GET;
            param.value = 0;

            // for (int i = 0; i < 3; i++)
            if (read(fd, &param, sizeof(param)) < 0)
            {
                perror("get error");
                // return -1;
            }
            // printf("read key: %lld, vlaue: %lld\n", param.key, param.value);

            // param.operation = HELLO_ENTRY_REMOVE;
            // if (write(fd, &param, sizeof(param)) < 0)
            // {
            //     perror("remove error");
            //     // return -1;
            // }
            // printf("remove key: %lld\n", param.key);

            // usleep(1000);
        }
    }
};

// void* workder(void* arg)
// {
//     long long id = (long long)arg;
//     printf("worker: 1, id: %lld\n", id);

//     return NULL;
// }

// int main()
// {
//     int fd = open("/proc/example/hello", O_RDWR);

//     struct hello_entry_param param = {
//         .operation = HELLO_ENTRY_INSERT,
//         .key = 1,
//         .value = 1,
//     };

//     // for (int i = 0; i < 3; i++)
//     if  (write(fd, &param, sizeof(param)) < 0)
//     {
//         perror("insert error");
//         return -1;
//     }
//     printf("insert key: %lld, value: %lld\n", param.key, param.value);

//     param.operation = HELLO_ENTRY_GET;
//     param.value = 0;

//     // for (int i = 0; i < 3; i++)
//     if  (read(fd, &param, sizeof(param)) < 0)
//     {
//         perror("get error");
//         return -1;
//     }
//     printf("read key: %lld, vlaue: %lld\n", param.key, param.value);

//     param.operation = HELLO_ENTRY_REMOVE;
//     if  (write(fd, &param, sizeof(param)) < 0)
//     {
//         perror("remove error");
//         return -1;
//     }
//     printf("remove key: %lld\n", param.key);

//     return 0;
// }

const char *path = "/proc/example/hello";

int main(int argc, char **argv)
{
    int thread_num = 1;
    if (argc > 1)
        if (sscanf(argv[1], "%d", &thread_num) < 0)
        {
            perror("bad thread_num");
            return -1;
        }

    int sleep_time = 10;
    if (argc > 2)
        if (sscanf(argv[2], "%d", &sleep_time) < 0)
        {
            perror("bad sleep_time");
            return -1;
        }

    std::vector<std::unique_ptr<Worker>> workers(thread_num);
    for (int i = 0; i < workers.size(); i++)
        workers[i] = std::make_unique<Worker>(i, path);

    sleep(sleep_time);

    return 0;
}