#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>


static const size_t MIN_STRING_SIZE = 100;


double get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (tv.tv_usec / 1000000.0);
}

std::string get_string_for_key(int key) {
    return std::string(MIN_STRING_SIZE, 'a') + std::to_string(key);
}

int main(int argc, char ** argv)
{
    int num_keys = atoi(argv[1]);
    int i, value = 0;

    if(argc <= 2)
        return 1;

    SETUP

    double before = get_time();

    if(!strcmp(argv[2], "sequential"))
    {
        for(i = 0; i < num_keys; i++)
            INSERT_INT_INTO_HASH(i, value);
    }

    else if(!strcmp(argv[2], "random"))
    {
        srandom(1); // for a fair/deterministic comparison
        for(i = 0; i < num_keys; i++)
            INSERT_INT_INTO_HASH((int)random(), value);
    }

    else if(!strcmp(argv[2], "delete"))
    {
        for(i = 0; i < num_keys; i++)
            INSERT_INT_INTO_HASH(i, value);
        before = get_time();
        for(i = 0; i < num_keys; i++)
            DELETE_INT_FROM_HASH(i);
    }

    else if(!strcmp(argv[2], "sequentialstring"))
    {
        for(i = 0; i < num_keys; i++)
            INSERT_STR_INTO_HASH(get_string_for_key(i), value);
    }

    else if(!strcmp(argv[2], "randomstring"))
    {
        srandom(1); // for a fair/deterministic comparison
        for(i = 0; i < num_keys; i++)
            INSERT_STR_INTO_HASH(get_string_for_key((int)random()), value);
    }

    else if(!strcmp(argv[2], "deletestring"))
    {
        for(i = 0; i < num_keys; i++)
            INSERT_STR_INTO_HASH(get_string_for_key(i), value);
        before = get_time();
        for(i = 0; i < num_keys; i++)
            DELETE_STR_FROM_HASH(get_string_for_key(i));
    }

    double after = get_time();
    printf("%f\n", after-before);
    fflush(stdout);
    sleep(1000000);
}
