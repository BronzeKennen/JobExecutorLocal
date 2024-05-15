killall progDelay
./jobCommander issueJob ./progDelay 10
./jobCommander issueJob ./progDelay 10
./jobCommander issueJob ./progDelay 10
./jobCommander issueJob ./progDelay 10
./jobCommander issueJob ./progDelay 10
./jobCommander issueJob ./progDelay 10
./jobCommander setConcurrency 4
./jobCommander poll running
./jobCommander poll queued
./jobCommander stop job_4
./jobCommander poll running
./jobCommander poll queued