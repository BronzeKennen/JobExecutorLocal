jobexec: jobcomm.o jobexecserver.o jobexec.o
	gcc jobCommander.o jobExecutorServer.o jobExecutor.o -o bin/jobexecutor

jobcomm.o:
	gcc -c src/jobCommander.c

jobexecserver.o:
	gcc -c src/jobExecutorServer.c

jobexec.o:
	gcc -c src/jobExecutor.c

run: jobexec
	./bin/jobexecutor