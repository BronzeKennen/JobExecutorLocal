jobexec: jobexecserver.o 
	rm -f jobExecutorServer.txt
	gcc -Wall -Werror jobExecutorServer.o -o jobExecutorServer
	gcc  -Wall -Werror src/jobCommander.c -o jobCommander

jobexecserver.o:
	gcc -c src/jobExecutorServer.c


run: jobexec
	rm -f jobExecutorServer.txt
	./bin/jobexecutor

clean:
	rm -f jobExecutorServer
	rm -f jobExecutorServer.o
	rm -f jobCommander
	rm -f jobExecutorServer.txt