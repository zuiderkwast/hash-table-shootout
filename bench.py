#!/usr/bin/env python3

import sys, os, subprocess, signal

# samples of use:
#  ./bench.py
#  ./bench.py insert_random_full insert_random_full_reserve
#  ./bench.py small_string string

######################################################################
### Fill free to change the following defaults
programs = [
    'std_unordered_map',
    'boost_unordered_map',
    'google_sparse_hash_map',
    'google_dense_hash_map',
    'google_dense_hash_map_mlf_0_9',
    'qt_qhash',
    'spp_sparse_hash_map',
    'emilib_hash_map',
    'ska_flat_hash_map',
    'ska_flat_hash_map_power_of_two',
    'tsl_sparse_map',
    'tsl_hopscotch_map',
    'tsl_hopscotch_map_mlf_0_5',
    'tsl_hopscotch_map_store_hash',
    'tsl_robin_map',
    'tsl_robin_map_mlf_0_9',
    'tsl_robin_map_store_hash',
    'tsl_robin_pg_map',
    'tsl_ordered_map',
    'tsl_array_map',
    'tsl_array_map_mlf_1_0',
    'judyL',
    'judyHS',
    #'nata88',
    #'nataF8'
]

minkeys  =  2*100*1000
maxkeys  = 30*100*1000
interval =  2*100*1000
best_out_of = 5

######################################################################
outfile = open('output', 'w')

short_names = {
    'random_shuffle_range': [
        'insert_random_shuffle_range', 'read_random_shuffle_range'
    ],
    'random_full': [
        'insert_random_full', 'insert_random_full_reserve',
        'read_random_full', 'read_miss_random_full',
        'delete_random_full', 'read_random_full_after_delete',
        'iteration_random_full'
    ],
    'small_string': [
        'insert_small_string', 'insert_small_string_reserve',
        'read_small_string', 'read_miss_small_string',
        'delete_small_string',
        'read_small_string_after_delete'
    ],
    'string': [
        'insert_string', 'insert_string_reserve',
        'read_string', 'read_miss_string',
        'delete_string',
        'read_string_after_delete'
    ]
}

if len(sys.argv) > 1:
    benchtypes = []
    for x in sys.argv[1:]:
        benchtypes.extend(short_names.get(x, [x]))
else:
    benchtypes = short_names['random_shuffle_range'] + short_names['random_full'] \
        + short_names['small_string'] + short_names['string']

for nkeys in range(minkeys, maxkeys + 1, interval):
    for benchtype in benchtypes:
        for program in programs:
            fastest_attempt = 1000000
            fastest_attempt_data = ''

            for attempt in range(best_out_of):
                try:
                    output = subprocess.check_output(['./build/' + program, str(nkeys), benchtype])
                    words = output.strip().split()
                    
                    runtime_seconds = float(words[0])
                    memory_usage_bytes = int(words[1])
                    load_factor = float(words[2])
                except:
                    print("Error with %s" % str(['./build/' + program, str(nkeys), benchtype]))
                    break

                line = ','.join(map(str, [benchtype, nkeys, program, "%0.2f" % load_factor, 
                                          memory_usage_bytes, "%0.6f" % runtime_seconds]))

                if runtime_seconds < fastest_attempt:
                    fastest_attempt = runtime_seconds
                    fastest_attempt_data = line

            if fastest_attempt != 1000000:
                print(fastest_attempt_data, file=outfile)
                print(fastest_attempt_data)
        
        # Print blank line
        print(file=outfile)
        print()
