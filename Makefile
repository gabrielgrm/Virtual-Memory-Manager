all: vm.c
	gcc -Wall -Wextra -std=c99 -O2 -o vm vm.c