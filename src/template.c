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
static const int64_t seed = 1;
static std::mt19937_64 generator(seed);

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

// Range start at 0
std::vector<int64_t> get_random_shuffle_range_ints(size_t range_end) 
{
    std::vector<int64_t> random_ints(range_end);
    std::iota(random_ints.begin(), random_ints.end(), 0);
    std::shuffle(random_ints.begin(), random_ints.end(), generator);
    
    return random_ints;
}

std::vector<int64_t> get_random_full_ints(size_t nb_ints) 
{
    std::uniform_int_distribution<int64_t> rd_uniform(0, std::numeric_limits<int64_t>::max());
    
    std::vector<int64_t> random_ints(nb_ints);
    for(size_t i=0; i < random_ints.size(); i++) {
        random_ints[i] = rd_uniform(generator);
    }
    
    return random_ints;
}

int main(int argc, char ** argv)
{
    if(argc <= 2)
        return 1;
    
    const int64_t num_keys = std::stoll(argv[1]);
    const int64_t value = 1;


    SETUP

    double before = get_time();

    if(!strcmp(argv[2], "sequential"))
    {
        for(int64_t i = 0; i < num_keys; i++) {
            INSERT_INT_INTO_HASH(i, value);
        }
    }

    else if(!strcmp(argv[2], "sequentialread"))
    {
        for(int64_t i = 0; i < num_keys; i++) {
            INSERT_INT_INTO_HASH(i, value);
        }
        
        before = get_time();
        for(int64_t i = 0; i < num_keys; i++) {
            if(FIND_FROM_INT_HASH(i) != value) {
                printf("error");
                exit(1);
            }
        }
    }

    else if(!strcmp(argv[2], "randomshufflerange"))
    {
        const std::vector<int64_t> keys = get_random_shuffle_range_ints(num_keys);
        before = get_time();
        
        for(int64_t i = 0; i < num_keys; i++) {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
    }

    else if(!strcmp(argv[2], "randomshufflerangeread"))
    {
        std::vector<int64_t> keys = get_random_shuffle_range_ints(num_keys);
        for(int64_t i = 0; i < num_keys; i++) {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
        
        std::shuffle(keys.begin(), keys.end(), generator);
        
        before = get_time();
        for(int64_t i = 0; i < num_keys; i++) {
            if(FIND_FROM_INT_HASH(keys[i]) != value) {
                printf("error");
                exit(1);
            }
        }
    }

    else if(!strcmp(argv[2], "randomfull"))
    {
        const std::vector<int64_t> keys = get_random_full_ints(num_keys);
        before = get_time();
        
        for(int64_t i = 0; i < num_keys; i++) {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
    }

    else if(!strcmp(argv[2], "randomfullread"))
    {
        std::vector<int64_t> keys = get_random_full_ints(num_keys);
        for(int64_t i = 0; i < num_keys; i++) {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
        
        std::shuffle(keys.begin(), keys.end(), generator);
        
        before = get_time();
        for(int64_t i = 0; i < num_keys; i++) {
            if(FIND_FROM_INT_HASH(keys[i]) != value) {
                printf("error");
                exit(1);
            }
        }
    }


    else if(!strcmp(argv[2], "iteration"))
    {
        const std::vector<int64_t> keys = get_random_full_ints(num_keys);
        for(int64_t i = 0; i < num_keys; i++) {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
        
        before = get_time();
        for(const auto& key_value : hash) {
            if(GET_VALUE_INT_FROM_KEY_VALUE(key_value) != value) {
                printf("error");
                exit(1);
            }
        }
    }
    else if(!strcmp(argv[2], "delete"))
    {
        std::vector<int64_t> keys = get_random_full_ints(num_keys);
        for(int64_t i = 0; i < num_keys; i++) {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
        
        std::shuffle(keys.begin(), keys.end(), generator);
        
        before = get_time();
        for(int64_t i = 0; i < num_keys; i++) {
            DELETE_INT_FROM_HASH(keys[i]);
        }
    }

    else if(!strcmp(argv[2], "insertstring"))
    {
        for(int64_t i = 0; i < num_keys; i++) {
            INSERT_STR_INTO_HASH(get_string_for_key(i), value);
        }
    }

    else if(!strcmp(argv[2], "readstring"))
    {
        for(int64_t i = 0; i < num_keys; i++) {
            INSERT_STR_INTO_HASH(get_string_for_key(i), value);
        }
        
        const std::vector<int64_t> keys = get_random_shuffle_range_ints(num_keys);        
        
        before = get_time();
        for(int64_t i = 0; i < num_keys; i++) {
            if(FIND_FROM_STR_HASH(get_string_for_key(keys[i])) != value) {
                printf("error");
                exit(1);
            }
        }
    }

    else if(!strcmp(argv[2], "deletestring"))
    {
        for(int64_t i = 0; i < num_keys; i++) {
            INSERT_STR_INTO_HASH(get_string_for_key(i), value);
        }
        
        const std::vector<int64_t> keys = get_random_shuffle_range_ints(num_keys);  
        
        before = get_time();
        for(int64_t i = 0; i < num_keys; i++) {
            DELETE_STR_FROM_HASH(get_string_for_key(keys[i]));
        }
    }
    double after = get_time();
    
    sleep(1);
    printf("%f\n", after-before);
    fflush(stdout);
    sleep(20);
}
