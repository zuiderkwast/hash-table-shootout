#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <random>


static const size_t MIN_STRING_SIZE = 50;


double get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (tv.tv_usec / 1000000.0);
}

std::string get_string_for_key(int key) 
{
    return std::string(MIN_STRING_SIZE, 'a') + std::to_string(key);
}

std::vector<int64_t> get_random_ints(size_t nb_ints, int64_t seed) 
{
    std::mt19937_64 rand(seed);
    
    std::vector<int64_t> random_ints(nb_ints);
    std::iota(random_ints.begin(), random_ints.end(), 0);
    std::shuffle(random_ints.begin(), random_ints.end(), rand);
    
    return random_ints;
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

    else if(!strcmp(argv[2], "sequentialread"))
    {
        for(i = 0; i < num_keys; i++)
            INSERT_INT_INTO_HASH(i, value);
        
        before = get_time();
        for(i = 0; i < num_keys; i++) {
            if(FIND_FROM_INT_HASH(i) != value) {
                printf("error");
                exit(1);
            }
        }
    }

    else if(!strcmp(argv[2], "random"))
    {
        const std::vector<int64_t> keys = get_random_ints(num_keys, 1);
        before = get_time();
        for(i = 0; i < num_keys; i++)
            INSERT_INT_INTO_HASH(keys[i], value);
    }

    else if(!strcmp(argv[2], "iteration"))
    {
        for(i = 0; i < num_keys; i++)
            INSERT_INT_INTO_HASH(i, value);
        
        before = get_time();
        for(const auto& key_value : hash) {
            if(GET_VALUE_INT_FROM_KEY_VALUE(key_value) != value) {
                printf("error");
                exit(1);
            }
        }
    }

    else if(!strcmp(argv[2], "randomread"))
    {
        const std::vector<int64_t> keys = get_random_ints(num_keys, 1);
        for(i = 0; i < num_keys; i++)
            INSERT_INT_INTO_HASH(i, value);
        
        before = get_time();
        for(i = 0; i < num_keys; i++) {
            if(FIND_FROM_INT_HASH(keys[i]) != value) {
                printf("error");
                exit(1);
            }
        }
    }

    else if(!strcmp(argv[2], "delete"))
    {
        for(i = 0; i < num_keys; i++)
            INSERT_INT_INTO_HASH(i, value);
        
        before = get_time();
        for(i = 0; i < num_keys; i++)
            DELETE_INT_FROM_HASH(i);
    }

    else if(!strcmp(argv[2], "insertstring"))
    {
        for(i = 0; i < num_keys; i++)
            INSERT_STR_INTO_HASH(get_string_for_key(i), value);
    }

    else if(!strcmp(argv[2], "readstring"))
    {
        for(i = 0; i < num_keys; i++)
            INSERT_STR_INTO_HASH(get_string_for_key(i), value);
        
        before = get_time();
        for(i = 0; i < num_keys; i++) {
            if(FIND_FROM_STR_HASH(get_string_for_key(i)) != value) {
                printf("error");
                exit(1);
            }
        }
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
