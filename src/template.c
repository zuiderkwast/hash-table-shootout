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


static const int64_t SEED = 0;
static std::mt19937_64 generator(SEED);

double get_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (tv.tv_usec / 1000000.0);
}

std::string get_string_for_key(int64_t key) 
{
    return "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" + std::to_string(key);
}

std::string get_small_string_for_key(int64_t key) 
{
    return std::to_string(key);
}

// Range starts at 0
std::vector<int64_t> get_random_shuffle_range_ints(size_t range_end) 
{
    std::vector<int64_t> random_ints(range_end);
    std::iota(random_ints.begin(), random_ints.end(), 0);
    std::shuffle(random_ints.begin(), random_ints.end(), generator);
    
    return random_ints;
}

std::vector<int64_t> get_random_full_ints(size_t nb_ints, 
                                          int64_t min = 0, 
                                          int64_t max = std::numeric_limits<int64_t>::max()) 
{
    std::uniform_int_distribution<int64_t> rd_uniform(min, max);
    
    std::vector<int64_t> random_ints(nb_ints);
    for(size_t i=0; i < random_ints.size(); i++) 
    {
        random_ints[i] = rd_uniform(generator);
    }
    
    return random_ints;
}

int main(int argc, char ** argv)
{
    if(argc != 3) 
    {
        return 1;
    }
    
    const int64_t num_keys = std::stoll(argv[1]);
    const std::string test_type = argv[2];
    const int64_t value = 1;


    SETUP

    double before = get_time();

    if(test_type == "sequential")
    {
        for(int64_t i = 0; i < num_keys; i++) 
        {
            INSERT_INT_INTO_HASH(i, value);
        }
    }

    else if(test_type == "sequentialread")
    {
        for(int64_t i = 0; i < num_keys; i++) 
        {
            INSERT_INT_INTO_HASH(i, value);
        }
        
        before = get_time();
        for(int64_t i = 0; i < num_keys; i++) 
        {
            FIND_INT_EXISTING_FROM_HASH(i);
        }
    }

    else if(test_type == "randomshufflerange")
    {
        const std::vector<int64_t> keys = get_random_shuffle_range_ints(num_keys);
        before = get_time();
        
        for(int64_t i = 0; i < num_keys; i++) 
        {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
    }

    else if(test_type == "randomshufflerangeread")
    {
        std::vector<int64_t> keys = get_random_shuffle_range_ints(num_keys);
        for(int64_t i = 0; i < num_keys; i++) 
        {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
        
        std::shuffle(keys.begin(), keys.end(), generator);
        
        before = get_time();
        for(int64_t i = 0; i < num_keys; i++) 
        {
            FIND_INT_EXISTING_FROM_HASH(keys[i]);
        }
    }

    else if(test_type == "randomfull")
    {
        const std::vector<int64_t> keys = get_random_full_ints(num_keys);
        before = get_time();
        
        for(int64_t i = 0; i < num_keys; i++) 
        {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
    }

    else if(test_type == "randomfullread")
    {
        std::vector<int64_t> keys = get_random_full_ints(num_keys);
        for(int64_t i = 0; i < num_keys; i++) 
        {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
        
        std::shuffle(keys.begin(), keys.end(), generator);
        
        before = get_time();
        for(int64_t i = 0; i < num_keys; i++) 
        {
            FIND_INT_EXISTING_FROM_HASH(keys[i]);
        }
    }

    else if(test_type == "randomfullmissread")
    {
        const std::vector<int64_t> keys_insert = get_random_full_ints(num_keys, 0, std::numeric_limits<int64_t>::max());
        const std::vector<int64_t> keys_read = get_random_full_ints(num_keys, std::numeric_limits<int64_t>::min(), -3);
        
        for(int64_t i = 0; i < num_keys; i++) 
        {
            INSERT_INT_INTO_HASH(keys_insert[i], value);
        }
        
        before = get_time();
        for(int64_t i = 0; i < num_keys; i++) 
        {
            FIND_INT_MISSING_FROM_HASH(keys_read[i]);
        }
    }

    else if(test_type == "iteration")
    {
        const std::vector<int64_t> keys = get_random_full_ints(num_keys);
        for(int64_t i = 0; i < num_keys; i++) 
        {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
        
        before = get_time();
        for(auto it = hash.begin(); it != hash.end(); ++it) 
        {
            CHECK_INT_ITERATOR_VALUE(it, value);
        }
    }
    
    else if(test_type == "delete")
    {
        std::vector<int64_t> keys = get_random_full_ints(num_keys);
        for(int64_t i = 0; i < num_keys; i++) 
        {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
        
        std::shuffle(keys.begin(), keys.end(), generator);
        
        before = get_time();
        for(int64_t i = 0; i < num_keys; i++) 
        {
            DELETE_INT_FROM_HASH(keys[i]);
        }
    }

    else if(test_type == "insertsmallstring")
    {
        for(int64_t i = 0; i < num_keys; i++) 
        {
            INSERT_STR_INTO_HASH(get_small_string_for_key(i), value);
        }
    }

    else if(test_type == "readsmallstring")
    {
        for(int64_t i = 0; i < num_keys; i++) 
        {
            INSERT_STR_INTO_HASH(get_small_string_for_key(i), value);
        }
        
        const std::vector<int64_t> keys = get_random_shuffle_range_ints(num_keys);        
        
        before = get_time();
        for(int64_t i = 0; i < num_keys; i++) 
        {
            FIND_STR_EXISTING_FROM_HASH(get_small_string_for_key(keys[i]));
        }
    }

    else if(test_type == "missreadsmallstring")
    {
        for(int64_t i = 0; i < num_keys; i++) 
        {
            INSERT_STR_INTO_HASH(get_small_string_for_key(i), value);
        }
        
        before = get_time();
        for(int64_t i = num_keys; i < num_keys*2; i++) 
        {
            FIND_STR_MISSING_FROM_HASH(get_small_string_for_key(i));
        }
    }

    else if(test_type == "deletesmallstring")
    {
        for(int64_t i = 0; i < num_keys; i++) 
        {
            INSERT_STR_INTO_HASH(get_small_string_for_key(i), value);
        }
        
        const std::vector<int64_t> keys = get_random_shuffle_range_ints(num_keys);  
        
        before = get_time();
        for(int64_t i = 0; i < num_keys; i++) 
        {
            DELETE_STR_FROM_HASH(get_small_string_for_key(keys[i]));
        }
    }
    
    else if(test_type == "insertstring")
    {
        for(int64_t i = 0; i < num_keys; i++) 
        {
            INSERT_STR_INTO_HASH(get_string_for_key(i), value);
        }
    }

    else if(test_type == "readstring")
    {
        for(int64_t i = 0; i < num_keys; i++) 
        {
            INSERT_STR_INTO_HASH(get_string_for_key(i), value);
        }
        
        const std::vector<int64_t> keys = get_random_shuffle_range_ints(num_keys);        
        
        before = get_time();
        for(int64_t i = 0; i < num_keys; i++) 
        {
            FIND_STR_EXISTING_FROM_HASH(get_string_for_key(keys[i]));
        }
    }

    else if(test_type == "missreadstring")
    {
        for(int64_t i = 0; i < num_keys; i++) 
        {
            INSERT_STR_INTO_HASH(get_string_for_key(i), value);
        }
        
        before = get_time();
        for(int64_t i = num_keys; i < num_keys*2; i++) 
        {
            FIND_STR_MISSING_FROM_HASH(get_string_for_key(i));
        }
    }

    else if(test_type == "deletestring")
    {
        for(int64_t i = 0; i < num_keys; i++) 
        {
            INSERT_STR_INTO_HASH(get_string_for_key(i), value);
        }
        
        const std::vector<int64_t> keys = get_random_shuffle_range_ints(num_keys);  
        
        before = get_time();
        for(int64_t i = 0; i < num_keys; i++) 
        {
            DELETE_STR_FROM_HASH(get_string_for_key(keys[i]));
        }
    }
    
    
    double after = get_time();
    
    sleep(1);
    printf("%f\n", after-before);
    fflush(stdout);
    sleep(20);
}
