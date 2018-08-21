ex4q1: ex4q1.o 
	gcc ex4q1.o -o ex4q1 -lm -lpthread
	
ex4q1.o: ex4q1.c
	gcc -c ex4q1.c -lm -lpthread
	
clean:
	rm *.o
