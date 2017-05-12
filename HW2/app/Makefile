driver_test : driver_test.o
	arm-none-linux-gnueabi-gcc -static -o driver_test driver_test.o 

driver_test.o : driver_test.c
	arm-none-linux-gnueabi-gcc -static -c -o driver_test.o driver_test.c

clean :
	rm -rf *.o driver_test
