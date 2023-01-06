//
// Created by Anton on 05.01.2023.
//

#ifndef P5_INTERPRETER_INTERPRETER_H
#define P5_INTERPRETER_INTERPRETER_H


#include "p5_common.h"

class Interpreter {
public:
	Interpreter(P5::store_t &store, P5::addr_t pc_top, P5::addr_t cp):
		store(store), pc_top(pc_top), mp(pc_top), sp(pc_top), ep(5), np(cp){};

	[[noreturn]] void run();


private:
	//program top counter
	P5::addr_t pc_top = 0;

	//program counter
	P5::addr_t pc = 0;

	//stack pointer
	P5::addr_t sp = 0;

	//mark stack pointer
	P5::addr_t mp = 0;

	//new pointer
	P5::addr_t np = 0;

	//extreme stack pointer
	P5::addr_t ep = 0;

	//mark stack part offsets
	static int ms_fret_off;
	static int ms_st_link_off;
	static int ms_dyn_link_off;
	static int ms_ep_off;
	static int ms_stack_bottom_off;
	static int ms_ep_cur_off;
	static int ms_ret_addr_off;

	//storage
	P5::store_t &store;

	P5::addr_t get_base_addr(P5::ins_t p);

	template<typename T>
	void push_stack(T val);

	template<typename T>
	void pop_stack();

	template<typename T>
	void lod_instr();

	template<typename T>
	void ldo_instr();

	template<typename T>
	void str_instr();

	template<typename T>
	T get_pc();

};


#endif //P5_INTERPRETER_INTERPRETER_H
