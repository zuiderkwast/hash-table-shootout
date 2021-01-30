#!/usr/bin/env python3

import sys, json
from collections import OrderedDict

######################################################################
### Fill free to change the following defaults
proper_names = OrderedDict([
    ('std_unordered_map', 'std::unordered_map'),
    ('google_dense_hash_map', 'google::dense_hash_map'),
    ('qt_qhash', 'QHash'),
    ('tsl_sparse_map', 'tsl::sparse_map'),
    ('tsl_hopscotch_map', 'tsl::hopscotch_map'),
    ('tsl_robin_map', 'tsl::robin_map'),
    ('tsl_hopscotch_map_store_hash', 'tsl::hopscotch_map (with StoreHash)'),
    ('tsl_robin_map_store_hash', 'tsl::robin_map (with StoreHash)'),
    ('tsl_hopscotch_map_mlf_0_5', 'tsl::hopscotch_map (0.5 mlf)'),
    ('tsl_robin_map_mlf_0_9', 'tsl::robin_map (0.9 mlf)'),
    ('tsl_ordered_map', 'tsl::ordered_map'),
    ('tsl_robin_pg_map', 'tsl::robin_pg_map'),
    ('ska_flat_hash_map', 'ska::flat_hash_map'),
    ('ska_flat_hash_map_power_of_two', 'ska::flat_hash_map (power of two)'),
    ('ska_bytell_hash_map', 'ska::bytell_hash_map'),
    ('google_dense_hash_map_mlf_0_9', 'google::dense_hash_map (0.9 mlf)'),
    ('google_sparse_hash_map', 'google::sparse_hash_map'),
    ('boost_unordered_map', 'boost::unordered_map'),
    ('spp_sparse_hash_map', 'spp::sparse_hash_map'),
    ('emilib_hash_map', 'emilib::HashMap'),
    ('tsl_array_map', 'tsl::array_map'),
    ('tsl_array_map_mlf_1_0', 'tsl::array_map (1.0 mlf)'),
    ('cuckoohash_map', 'libcuckoo::cuckoohash_map')
])

# do them in the desired order to make the legend not overlap
# the chart data too much
program_slugs = [
    'std_unordered_map',
    'google_dense_hash_map',
    'qt_qhash',
    'tsl_sparse_map',
    'tsl_hopscotch_map',
    'tsl_robin_map',
    'tsl_hopscotch_map_store_hash',
    'tsl_robin_map_store_hash',
    'tsl_hopscotch_map_mlf_0_5',
    'tsl_robin_map_mlf_0_9',
    'tsl_ordered_map',
    'tsl_robin_pg_map',
    'ska_flat_hash_map',
    'ska_bytell_hash_map',
    'google_dense_hash_map_mlf_0_9',
    'ska_flat_hash_map_power_of_two',
    'google_sparse_hash_map',
    'boost_unordered_map',
    'spp_sparse_hash_map',
    'emilib_hash_map',
    'tsl_array_map',
    'tsl_array_map_mlf_1_0',
    'judyL',
    'judyHS',
    'nata88',
    'nataF8',
    'glib_tree',
    'glib_hash_table',
    'cuckoohash_map'
]

# hashmap which will be shown (checkbox checked),
# by default all hashmaps are enabled.
#default_programs_show = [
#    'std_unordered_map',
#    'google_dense_hash_map',
#    'qt_qhash',
#    'tsl_sparse_map',
#    'tsl_hopscotch_map',
#    'tsl_robin_map',
#    'tsl_hopscotch_map_store_hash',
#    'tsl_robin_map_store_hash']

######################################################################
lines = [ line.strip() for line in sys.stdin if line.strip() ]

by_benchtype = {}

for line in lines:
    if len(line) == 0:
        next
    benchtype, nkeys, program, load_factor, nbytes, runtime = line.split(',')
    nkeys = int(nkeys)
    nbytes = float(nbytes) / nkeys # bytes per (key,value) pair
    runtime = float(runtime) * 1000000000 / nkeys # microseconds per operation
    load_factor = float(load_factor)

    if runtime > 0 and nbytes > 0:
        by_benchtype.setdefault("%s_runtime" % benchtype, {}).setdefault(program, []).append([nkeys, runtime, load_factor])
        by_benchtype.setdefault("%s_bopsnsec" % benchtype, {}).setdefault(program, []).append([nkeys, runtime * nbytes, load_factor])
        if benchtype in ('insert_random_shuffle_range', 'insert_random_full', 'insert_small_string', 'insert_string',
                         'insert_random_full_reserve', 'insert_small_string_reserve', 'insert_string_reserve'):
            by_benchtype.setdefault("%s_memory"  % benchtype, {}).setdefault(program, []).append([nkeys, nbytes, load_factor])

for program in program_slugs:
    proper_names.setdefault(program, program)

chart_data = {}
existing_proper_names = {}
real_default_programs_show = set()

for i, (benchtype, programs) in enumerate(by_benchtype.items()):
    chart_data[benchtype] = []
    for j, program in enumerate(program_slugs):
        if program not in programs:
            continue

        existing_proper_names[program] = proper_names[program]
        if "default_programs_show" not in dir() or (program in default_programs_show):
            real_default_programs_show.add(program);
        data = programs[program]
        chart_data[benchtype].append({
            'program': program,
            'label': proper_names[program],
            'data': [],
        })

        for k, (nkeys, value, load_factor) in enumerate(data):
            chart_data[benchtype][-1]['data'].append([nkeys, value, load_factor])

json_text = json.dumps(chart_data)
json_text = json_text.replace("}], ", "}], \n")
print('chart_data = ' + json_text + ';')
print('\nprograms = ' + json.dumps(existing_proper_names, indent=1) + ';')
print('\ndefault_programs_show = ' + str(list(real_default_programs_show)) + ';')
