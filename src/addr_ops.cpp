//
// Created by Anton on 13.11.2022.
//
#include "addr_ops.h"

void put_addr_by_ptr(P5::store_t &store, P5::addr_t addr, unsigned char *bytes, int size) {
	for (int i = 0; i < size; i++) {
		store[addr+i] = bytes[i];
	}
}

void get_addr_by_ptr(P5::store_t &store, P5::addr_t addr, unsigned char *bytes, int size) {
	for (int i = 0; i < size; i++) {
		bytes[i] = store[addr+i];
	}
}
