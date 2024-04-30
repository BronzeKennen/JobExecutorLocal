jobexec: jobexecserver.o 
	gcc -Wall -Werror jobExecutorServer.o -o jobExecutorServer
	gcc  -Wall -Werror src/jobCommander.c -o jobCommander

jobexecserver.o:
	gcc -c src/jobExecutorServer.c


run: jobexec
	rm -f jobExecutorServer.txt
	./bin/jobexecutor
