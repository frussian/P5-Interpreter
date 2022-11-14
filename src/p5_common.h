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
	typedef std::vector<unsigned char> store_t;

	static int int_size = 4;
	static int max_store = 16777216;
}

#endif //P5_INTERPRETER_P5_COMMON_H
