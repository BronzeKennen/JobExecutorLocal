killall progDelay
./jobCommander issueJob echo "EXECUTED"
./jobCommander issueJob echo "EXECUTED"
./jobCommander issueJob echo "EXECUTED"
./jobCommander issueJob ./progDelay 10
./jobCommander issueJob ./progDelay 10
./jobCommander issueJob ./progDelay 10
./jobCommander issueJob ./progDelay 10
./jobCommander issueJob ./progDelay 10
./jobCommander poll running
./jobCommander poll queued
./jobCommander setConcurrency 4
./jobCommander stop job_4
./jobCommander stop job_5
# ./jobCommander exit
