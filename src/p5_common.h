//
// Created by Anton on 13.11.2022.
//

#ifndef P5_INTERPRETER_P5_COMMON_H
#define P5_INTERPRETER_P5_COMMON_H

#include <vector>

namespace P5 {
	typedef int addr_t;  //q
	typedef unsigned char ins_t; //opcode
	typedef unsigned char lvl_t; //p
	typedef unsigned char set_el_t;
	typedef short set_t;
	typedef std::vector<unsigned char> store_t;
	typedef double real_t;
	typedef bool bool_t;
	typedef char char_t;//TODO: maybe unsigned char
	typedef int int_t;

	static int addr_size = sizeof(P5::addr_t);
	static int int_size = sizeof(P5::int_t);
	static int char_size = sizeof(P5::char_t);
	static int bool_size = sizeof(P5::bool_t);
	static int set_size = 32;
	static int max_store = 16777216;
}

#endif //P5_INTERPRETER_P5_COMMON_H
