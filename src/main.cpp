#include <iostream>
#include "Assembler.h"

int main() {
	Assembler::init();
	P5::store_t store;
	store.resize(P5::max_store);
	Assembler assembler(store, "../tests/test2.txt");
	assembler.load();
}
