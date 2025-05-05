build:
	gcc -Wall -std=c99 ./src/*.c -o keycapper

run:
	./keycapper

clean:
	rm keycapper
