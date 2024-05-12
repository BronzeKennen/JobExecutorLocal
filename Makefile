jobexec: jobexecserver.o  pq.o
	rm -f jobExecutorServer.txt
	gcc -Wall -Werror jobExecutorServer.o pq.o -o jobExecutorServer
	gcc  -Wall -Werror src/jobCommander.c -o jobCommander
	killall jobExecutorServer

jobexecserver.o: pq.o
	gcc -c src/jobExecutorServer.c

pq.o: src/pq.c
	gcc -c src/pq.c

run: jobexec
	rm -f jobExecutorServer.txt
	./bin/jobexecutor

clean:
	rm -f jobExecutorServer
	rm -f jobExecutorServer.o
	rm -f jobCommander
	rm -f jobExecutorServer.txt