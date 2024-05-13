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
./jobCommander stop job_5
# ./jobCommander poll running
# ./jobCommander poll queued
# ./jobCommander issueJob ps
# ./jobCommander issueJob ls
# ./jobCommander issueJob echo megatron on the table