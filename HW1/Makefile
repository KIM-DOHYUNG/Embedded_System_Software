20121564 : ipc.o input_device.o output_device.o clock.o counter.o text_editor.o draw_board.o solve_problem.o
	arm-none-linux-gnueabi-gcc -static -o 20121564 ipc.o input_device.o output_device.o clock.o counter.o text_editor.o draw_board.o solve_problem.o

ipc.o : ipc.c
	arm-none-linux-gnueabi-gcc -static -c -o ipc.o ipc.c

input_device.o : input_device.c
	arm-none-linux-gnueabi-gcc -static -c -o input_device.o input_device.c

output_device.o : output_device.c
	arm-none-linux-gnueabi-gcc -static -c -o output_device.o output_device.c

clock.o : clock.c
	arm-none-linux-gnueabi-gcc -static -c -o clock.o clock.c

counter.o : counter.c
	arm-none-linux-gnueabi-gcc -static -c -o counter.o counter.c

text_editor.o : text_editor.c
	arm-none-linux-gnueabi-gcc -static -c -o text_editor.o text_editor.c

draw_board.o : draw_board.c
	arm-none-linux-gnueabi-gcc -static -c -o draw_board.o draw_board.c

solve_problem.o : solve_problem.c
	arm-none-linux-gnueabi-gcc -static -c -o solve_problem.o solve_problem.c

clean :
	rm *.o 20121564

