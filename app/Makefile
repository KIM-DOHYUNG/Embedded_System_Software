stopwatch_test : stopwatch_test.o
	arm-none-linux-gnueabi-gcc -static -o stopwatch_test stopwatch_test.o

stopwatch_test.o : stopwatch_test.c
	arm-none-linux-gnueabi-gcc -static -c -o stopwatch_test.o stopwatch_test.c

clean :
	rm -rf *.o stopwatch_test
