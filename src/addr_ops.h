//
// Created by Anton on 13.11.2022.
//

#ifndef P5_INTERPRETER_ADDR_OPS_H
#define P5_INTERPRETER_ADDR_OPS_H
#include "p5_common.h"

template<typename T>
void put_addr(P5::store_t &store, P5::addr_t addr, T v) {
	auto *bytes = (unsigned char*)&v;
	int size = sizeof(T);
	for (int i = 0; i < size; i++) {
		store[addr+i] = bytes[i];
	}
}

template<typename T>
T get_addr(P5::store_t &store, P5::addr_t addr) {
	int size = sizeof(T);
	unsigned char bytes[size];
	for (int i = 0; i < size; i++) {
		bytes[i] = store[addr+i];
	}
	return *(T*)bytes;
}

//void put_addr(P5::store_t store, P5::addr_t addr, P5::addr_t val);
//void put_addr(P5::store_t store, P5::addr_t addr, bool val);
//
//P5::addr_t get_addr(P5::store_t store, P5::addr_t addr);

#endif //P5_INTERPRETER_ADDR_OPS_H