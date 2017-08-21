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
#include <iostream>
#include <chrono>
#include <fstream>
#include <unistd.h>


static const std::int64_t SEED = 0;
static std::mt19937_64 generator(SEED);


std::size_t get_memory_usage_bytes() {
    std::ifstream file("/proc/self/statm");
    
    std::size_t memory;
    file >> memory; // Ignore first
    file >> memory;
    
    return memory * getpagesize();
}


/*
 * Generate a vector [0, range_end) and shuffle it
 */
std::vector<std::int64_t> get_random_shuffle_range_ints(std::size_t range_end) {
    std::vector<std::int64_t> random_ints(range_end);
    std::iota(random_ints.begin(), random_ints.end(), 0);
    std::shuffle(random_ints.begin(), random_ints.end(), generator);
    
    return random_ints;
}

std::vector<std::int64_t> get_random_full_ints(std::size_t nb_ints, 
                                          std::int64_t min = 0, 
                                          std::int64_t max = std::numeric_limits<std::int64_t>::max()) 
{
    std::uniform_int_distribution<std::int64_t> rd_uniform(min, max);
    
    std::vector<std::int64_t> random_ints(nb_ints);
    for(std::size_t i = 0; i < random_ints.size(); i++) {
        random_ints[i] = rd_uniform(generator);
    }
    
    return random_ints;
}

std::vector<std::string> get_random_small_strings(std::size_t nb_strings, 
                                                  std::int64_t min = 0, 
                                                  std::int64_t max = std::numeric_limits<std::int64_t>::max()) 
{
    std::uniform_int_distribution<std::int64_t> rd_uniform(min, max);
    
    std::vector<std::string> random_small_strings(nb_strings);
    for(std::size_t i = 0; i < random_small_strings.size(); i++) {
        random_small_strings[i] = std::to_string(rd_uniform(generator));
    }
    
    return random_small_strings;
}

std::vector<std::string> get_random_strings(std::size_t nb_strings, 
                                            std::int64_t min = 0, 
                                            std::int64_t max = std::numeric_limits<std::int64_t>::max()) 
{
    std::uniform_int_distribution<std::int64_t> rd_uniform(min, max);
    
    std::vector<std::string> random_strings(nb_strings);
    for(std::size_t i = 0; i < random_strings.size(); i++) {
        std::string str(50, 'a');
        std::string istr = std::to_string(rd_uniform(generator));
        str.replace(str.size() - istr.size(), istr.size(), istr);
        
        random_strings[i] = std::move(str);
    }
    
    return random_strings;
}



class measurements {
public:    
    measurements(): m_memory_usage_bytes_start(get_memory_usage_bytes()),
                    m_chrono_start(std::chrono::high_resolution_clock::now())
                    
    {
    }
    
    ~measurements() {
        auto chrono_end = std::chrono::high_resolution_clock::now();
        std::size_t memory_usage_bytes_end = get_memory_usage_bytes();
        
        const double nb_seconds = std::chrono::duration<double>(chrono_end - m_chrono_start).count();
        const std::size_t used_memory_bytes = (memory_usage_bytes_end > m_memory_usage_bytes_start)?
                                                    memory_usage_bytes_end - m_memory_usage_bytes_start:0;

        std::cout << nb_seconds << " " << used_memory_bytes << " ";
    }
    
private:    
    std::size_t m_memory_usage_bytes_start;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_chrono_start;
};


int main(int argc, char ** argv) {
    if(argc != 3) {
        std::cout << argv[0] << " num_keys test_type" << std::endl;
        return 1;
    }
    
    const std::int64_t num_keys = std::stoll(argv[1]);
    const std::string test_type = argv[2];
    const std::int64_t value = 1;


    SETUP


    /**
     * Integers
     */
    if(test_type == "insert_random_shuffle_range") {
        const std::vector<std::int64_t> keys = get_random_shuffle_range_ints(num_keys);
        
        
        measurements m;
        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
    }

    else if(test_type == "read_random_shuffle_range") {
        std::vector<std::int64_t> keys = get_random_shuffle_range_ints(num_keys);
        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
        
        std::shuffle(keys.begin(), keys.end(), generator);
        
        
        measurements m;
        for(std::int64_t i = 0; i < num_keys; i++) {
            FIND_INT_EXISTING_FROM_HASH(keys[i]);
        }
    }

    else if(test_type == "insert_random_full") {
        const std::vector<std::int64_t> keys = get_random_full_ints(num_keys);
        
        
        measurements m;
        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
    }

    else if(test_type == "insert_random_full_reserve") {
        const std::vector<std::int64_t> keys = get_random_full_ints(num_keys);
        
        
        measurements m;
        RESERVE_INT(num_keys);
        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
    }

    else if(test_type == "read_random_full") {
        std::vector<std::int64_t> keys = get_random_full_ints(num_keys);
        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
        
        std::shuffle(keys.begin(), keys.end(), generator);
        
        
        measurements m;
        for(std::int64_t i = 0; i < num_keys; i++) {
            FIND_INT_EXISTING_FROM_HASH(keys[i]);
        }
    }

    else if(test_type == "read_miss_random_full") {
        const std::vector<std::int64_t> keys_insert = get_random_full_ints(num_keys, 0, std::numeric_limits<std::int64_t>::max());
        const std::vector<std::int64_t> keys_read = get_random_full_ints(num_keys, std::numeric_limits<std::int64_t>::min(), -3);
        
        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_INT_INTO_HASH(keys_insert[i], value);
        }
        
        
        measurements m;
        for(std::int64_t i = 0; i < num_keys; i++) {
            FIND_INT_MISSING_FROM_HASH(keys_read[i]);
        }
    }

    else if(test_type == "read_random_full_after_delete") {
        std::vector<std::int64_t> keys = get_random_full_ints(num_keys);
        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
        
        std::shuffle(keys.begin(), keys.end(), generator);
        for(std::int64_t i = 0; i < num_keys / 2; i++) {
            DELETE_INT_FROM_HASH(keys[i]);
        }
        
        std::shuffle(keys.begin(), keys.end(), generator);
        
        
        measurements m;
        std::int64_t nb_found = 0;
        for(std::int64_t i = 0; i < num_keys; i++) {
            FIND_INT_EXISTING_FROM_HASH_COUNT(keys[i], nb_found);
        }
        
        if(nb_found != num_keys / 2) {
            printf("error");
            exit(6);
        }
    }

    else if(test_type == "iteration_random_full") {
        const std::vector<std::int64_t> keys = get_random_full_ints(num_keys);
        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
        
        
        measurements m;
        for(auto it = hash.begin(); it != hash.end(); ++it) {
            CHECK_INT_ITERATOR_VALUE(it, value);
        }
    }
    
    else if(test_type == "delete_random_full") {
        std::vector<std::int64_t> keys = get_random_full_ints(num_keys);
        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_INT_INTO_HASH(keys[i], value);
        }
        
        std::shuffle(keys.begin(), keys.end(), generator);
        
        
        measurements m;
        for(std::int64_t i = 0; i < num_keys; i++) {
            DELETE_INT_FROM_HASH(keys[i]);
        }
    }
    
    
    /**
     * Small strings
     */
    else if(test_type == "insert_small_string") {
        const std::vector<std::string> keys = get_random_small_strings(num_keys);
        
        
        measurements m;
        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_STR_INTO_HASH(keys[i], value);
        }
    }

    else if(test_type == "insert_small_string_reserve") {
        const std::vector<std::string> keys = get_random_small_strings(num_keys);
        
        
        measurements m;
        RESERVE_STR(num_keys);
        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_STR_INTO_HASH(keys[i], value);
        }
    }

    else if(test_type == "read_small_string") {
        std::vector<std::string> keys = get_random_small_strings(num_keys);
        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_STR_INTO_HASH(keys[i], value);
        }
        
        std::shuffle(keys.begin(), keys.end(), generator);
        
        
        measurements m;
        for(std::int64_t i = 0; i < num_keys; i++) {
            FIND_STR_EXISTING_FROM_HASH(keys[i]);
        }
    }

    else if(test_type == "read_miss_small_string") {
        const std::vector<std::string> keys_insert = get_random_small_strings(num_keys, 0, std::numeric_limits<std::int64_t>::max());
        const std::vector<std::string> keys_read = get_random_small_strings(num_keys, std::numeric_limits<std::int64_t>::min(), -3);

        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_STR_INTO_HASH(keys_insert[i], value);
        }
        
        
        measurements m;
        for(std::int64_t i = 0; i < num_keys; i++) {
            FIND_STR_MISSING_FROM_HASH(keys_read[i]);
        }
    }

    else if(test_type == "read_small_string_after_delete") {
        std::vector<std::string> keys = get_random_small_strings(num_keys);
        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_STR_INTO_HASH(keys[i], value);
        }
        
        std::shuffle(keys.begin(), keys.end(), generator); 
        for(std::int64_t i = 0; i < num_keys / 2; i++) {
            DELETE_STR_FROM_HASH(keys[i]);
        }
        
        std::shuffle(keys.begin(), keys.end(), generator);
        
        
        measurements m;
        std::int64_t nb_found = 0;
        for(std::int64_t i = 0; i < num_keys; i++) {
            FIND_STR_EXISTING_FROM_HASH_COUNT(keys[i], nb_found);
        }
        
        if(nb_found != num_keys / 2) {
            printf("error");
            exit(6);
        }
    }

    else if(test_type == "delete_small_string") {
        std::vector<std::string> keys = get_random_small_strings(num_keys);
        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_STR_INTO_HASH(keys[i], value);
        }
        
        std::shuffle(keys.begin(), keys.end(), generator); 
        
        
        measurements m;
        for(std::int64_t i = 0; i < num_keys; i++) {
            DELETE_STR_FROM_HASH(keys[i]);
        }
    }
    
    
    
    /**
     * Strings
     */
    else if(test_type == "insert_string") {
        const std::vector<std::string> keys = get_random_strings(num_keys);
        
        
        measurements m;
        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_STR_INTO_HASH(keys[i], value);
        }
    }
    
    else if(test_type == "insert_string_reserve") {
        const std::vector<std::string> keys = get_random_strings(num_keys);
        
        
        measurements m;
        RESERVE_STR(num_keys);
        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_STR_INTO_HASH(keys[i], value);
        }
    }

    else if(test_type == "read_string") {
        std::vector<std::string> keys = get_random_strings(num_keys); 
        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_STR_INTO_HASH(keys[i], value);
        }
        
        std::shuffle(keys.begin(), keys.end(), generator);   
        
        
        measurements m;
        for(std::int64_t i = 0; i < num_keys; i++) {
            FIND_STR_EXISTING_FROM_HASH(keys[i]);
        }
    }

    else if(test_type == "read_miss_string") {
        const std::vector<std::string> keys_insert = get_random_strings(num_keys, 0, std::numeric_limits<std::int64_t>::max());
        const std::vector<std::string> keys_read = get_random_strings(num_keys, std::numeric_limits<std::int64_t>::min(), -3);

        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_STR_INTO_HASH(keys_insert[i], value);
        }
        
        
        measurements m;
        for(std::int64_t i = 0; i < num_keys; i++) {
            FIND_STR_MISSING_FROM_HASH(keys_read[i]);
        }
    }

    else if(test_type == "read_string_after_delete") {
        std::vector<std::string> keys = get_random_strings(num_keys);
        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_STR_INTO_HASH(keys[i], value);
        }
        
        std::shuffle(keys.begin(), keys.end(), generator); 
        for(std::int64_t i = 0; i < num_keys / 2; i++) {
            DELETE_STR_FROM_HASH(keys[i]);
        }
        
        std::shuffle(keys.begin(), keys.end(), generator);
        
        
        measurements m;
        std::int64_t nb_found = 0;
        for(std::int64_t i = 0; i < num_keys; i++) {
            FIND_STR_EXISTING_FROM_HASH_COUNT(keys[i], nb_found);
        }
        
        if(nb_found != num_keys / 2) {
            printf("error");
            exit(6);
        }
    }

    else if(test_type == "delete_string") {
        std::vector<std::string> keys = get_random_strings(num_keys);
        for(std::int64_t i = 0; i < num_keys; i++) {
            INSERT_STR_INTO_HASH(keys[i], value);
        }
        
        std::shuffle(keys.begin(), keys.end(), generator); 
        
        
        measurements m;
        for(std::int64_t i = 0; i < num_keys; i++) {
            DELETE_STR_FROM_HASH(keys[i]);
        }
    }
    
    else {
        std::cout << "Unknown test type: " << test_type << "." << std::endl;
        exit(1);
    }
    
    
    const float load_factor = std::max(LOAD_FACTOR(hash), LOAD_FACTOR(str_hash));
    std::cout << load_factor << std::endl;
}
