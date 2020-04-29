CFLAG = -DDEBUG -Wall -std=c99

main: scheduler.o process.o
	gcc $(CFLAG) scheduler.o process.o -o sched.out
##main.o: main.c Makefile
##	gcc $(CFLAG) main.c -c
scheduler.o: scheduler.c Makefile
	gcc $(CFLAG) scheduler.c -c
process.o: process.c process.h Makefile
	gcc $(CFLAG) process.c -c
clean:
	rm -rf *o
run:
	sudo ./sched.out
