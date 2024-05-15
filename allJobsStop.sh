#!/bin/bash
qJobs=$(./jobCommander poll queued)
if [ qJobs != "No jobs to print" ]; then             
    while IFS= read -r line; do  #for every line read:                    
        jobId=$(echo "$line" | cut -d ',' -f 1) #cut at comma    
        ./jobCommander stop $jobId #stop the jobs 
    done <<< "$qJobs"
fi

#same exact script for running
rJobs=$(./jobCommander poll running)
if [ rJobs != "No jobs to print" ]; then
    while IFS= read -r line; do
        jobId=$(echo "$line" | cut -d ',' -f 1) 
        ./jobCommander stop $jobId 
    done <<< "$rJobs"
fi