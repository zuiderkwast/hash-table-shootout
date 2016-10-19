import sys, os, subprocess, signal

programs = [
    'stl_unordered_map',
    'boost_unordered_map',
    'google_sparse_hash_map',
    'google_dense_hash_map',
    'qt_qhash',
    'spp_sparse_hash_map',
    'hopscotch_map',
    'sherwood_map',
    'emilib_hash_map',
    'rabbit_unordered_map',
    'rabbit_sparse_unordered_map',
]

minkeys  =  18*1000*1000
maxkeys  =  40*1000*1000
interval =   2*1000*1000
best_out_of = 1

# for the final run, use this:
#minkeys  =  2*1000*1000
#maxkeys  = 40*1000*1000
#interval =  2*1000*1000
#best_out_of = 3
# and use nice/ionice
# and shut down to the console
# and swapoff any swap files/partitions

outfile = open('output', 'w')

if len(sys.argv) > 1:
    benchtypes = sys.argv[1:]
else:
    benchtypes = ('sequential', 'sequentialread', 
                  'randomshufflerange', 'randomshufflerangeread',
                  'randomfull', 'randomfullread', 'randomfullreadmiss',
                  'iteration', 'delete', 
                  'insertsmallstring', 'readsmallstring', 'readsmallstringmiss', 'deletesmallstring', 
                  'insertstring', 'readstring', 'readstringmiss', 'deletestring',
                  )

for benchtype in benchtypes:
    nkeys = minkeys
    
    if benchtype.endswith("string") == True and maxkeys > 24*1000*1000:
        maxkeys = 24*1000*1000
        
    while nkeys <= maxkeys:
        for program in programs:
            fastest_attempt = 1000000
            fastest_attempt_data = ''
            
            if program == "sherwood_map" and nkeys >= 6000000:
                continue
            
            for attempt in range(best_out_of):
                proc = subprocess.Popen(['./build/'+program, str(nkeys), benchtype], stdout=subprocess.PIPE)

                # wait for the program to fill up memory and spit out its "ready" message
                try:
                    runtime = float(proc.stdout.readline().strip())
                except:
                    print("Error with %s" % str(['./build/'+program, str(nkeys), benchtype]))
                    break

                ps_proc = subprocess.Popen(['ps up %d | tail -n1' % proc.pid], shell=True, stdout=subprocess.PIPE)
                ps_line = ps_proc.stdout.read().split()
                nbytes_vsz = int(ps_line[4]) * 1024
                nbytes_rss = int(ps_line[5]) * 1024
                ps_proc.wait()

                os.kill(proc.pid, signal.SIGKILL)
                proc.wait()

                if nbytes_vsz and nbytes_rss and runtime: # otherwise it crashed
                    line = ','.join(map(str, [benchtype, nkeys, program, nbytes_vsz, nbytes_rss, "%0.6f" % runtime]))

                    if runtime < fastest_attempt:
                        fastest_attempt = runtime
                        fastest_attempt_data = line
                
                #if (benchtype == "randomshufflerange" or benchtype == "randomshufflerangeread") and (program == "google_sparse_hash_map"):
                    #break
                
                
            if fastest_attempt != 1000000:
                print >> outfile, fastest_attempt_data
                print fastest_attempt_data

        nkeys += interval
