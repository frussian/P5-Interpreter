#include <iostream>
#include "Assembler.h"

void print_help(char *prog) {
	printf("Usage: %s <pcode filepath>\n", prog);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		print_help(argv[0]);
		return 1;
	}
	Assembler::init();
	P5::store_t store;
	store.resize(P5::max_store);
	Assembler assembler(store, argv[1]);
	assembler.load();

}
