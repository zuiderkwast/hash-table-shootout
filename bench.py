#!/usr/bin/env python3

import sys, os, subprocess, re

# samples of use:
#  ./bench.py
#  ./bench.py insert_random_full insert_random_full_reserve
#  APPS='judyHS ska_bytell_hash_map' ./bench.py small_string string

######################################################################
### Fill free to change the following defaults
programs = []
for line in open("apps.txt"):
    line = re.sub("#.*$", "", line).strip()
    if line:
        programs.append(line)

minkeys  =  2*100*1000
maxkeys  = 15*1000*1000
#interval =  2*100*1000
step_percent =  30 # you may use this variable instead of "interval" for exponetial step
best_out_of = 5

######################################################################
outfile = open('output', 'w')

apps_env = os.environ.get('APPS', None)
if apps_env:
    programs = apps_env.strip().split()

short_names = {
    'random_shuffle_range': [
        'insert_random_shuffle_range', 'reinsert_random_shuffle_range',
        'read_random_shuffle_range'
    ],
    'random_full': [
        'insert_random_full', 'reinsert_random_full',
#        'insert_random_full_reserve',
        'read_random_full', 'read_miss_random_full',
        'delete_random_full', 'read_random_full_after_delete',
#        'iteration_random_full'
    ],
    
    'small_string': [
        'insert_small_string', 'reinsert_small_string',
#        'insert_small_string_reserve',
        'read_small_string', 'read_miss_small_string',
        'delete_small_string',
        'read_small_string_after_delete'
    ],
    'string': [
        'insert_string', 'reinsert_string',
        'insert_string_reserve',
        'read_string', 'read_miss_string',
        'delete_string',
        'read_string_after_delete'
    ]
    
}

if ("interval" in dir() and "step_percent" in dir()) or \
   ("interval" not in dir() and "step_percent" not in dir()):
    print("Either (exclusively) 'interval' or 'step_percent' variable should be set")
    sys.exit(1)

if len(sys.argv) > 1:
    benchtypes = []
    for x in sys.argv[1:]:
        benchtypes.extend(short_names.get(x, [x]))
else:
    benchtypes =  short_names['random_full'] \
    + short_names['small_string'] #+ short_names['string']

    '''short_names['random_shuffle_range'] + '''
if "interval" in dir():
    points = range(minkeys, maxkeys + 1, interval)
else:
    points = []
    keys = minkeys
    while keys <= maxkeys:
        points.append(keys)
        keys = int(max(keys + 1, keys * (100 + step_percent) / 100))

for nkeys in points:
    for benchtype in benchtypes:
        for program in programs:
            fastest_attempt = 1000000
            fastest_attempt_data = ''

            for attempt in range(best_out_of):
                try:
                    output = subprocess.check_output(['./build/' + program, str(nkeys), benchtype], text=True, stderr=subprocess.STDOUT)
                    words = output.strip().split()
                    
                    runtime_seconds = float(words[0])
                    memory_usage_bytes = int(words[1])
                    load_factor = float(words[2])
                except KeyboardInterrupt as e:
                    sys.exit(130);
                except subprocess.CalledProcessError as e:
                    if e.returncode == 71: # unknown test type for program?
                        continue # silently ignore this case

                    print("Error with %s" % str(['./build/' + program, str(nkeys), benchtype]), file=sys.stderr)
                    print("Exit status is %d" % e.returncode, file=sys.stderr)
                    print(e.stdout, file=sys.stderr)
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
        if fastest_attempt != 1000000:
            print(file=outfile)
            print()
