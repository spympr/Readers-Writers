main: build/semaphores.o build/shared_memory.o build/main.o 
	gcc -Wall build/semaphores.o build/shared_memory.o build/main.o -o main -lm
build/main.o: source/main.c 
	gcc -c source/main.c -o build/main.o
build/shared_memory.o: source/shared_memory.c headers/shared_memory.h 
	gcc -c source/shared_memory.c -o build/shared_memory.o
build/semaphores.o: source/semaphores.c headers/semaphores.h 
	gcc -c source/semaphores.c -o build/semaphores.o

clean: 
	rm main build/main.o build/shared_memory.o build/semaphores.o
