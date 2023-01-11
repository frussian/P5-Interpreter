//
// Created by Anton on 05.01.2023.
//

#ifndef P5_INTERPRETER_INTERPRETER_H
#define P5_INTERPRETER_INTERPRETER_H

#include <unordered_set>
#include <memory>
#include <fstream>
#include <unordered_map>

#include "p5_common.h"

class SetStorage;

class Interpreter {
public:
	Interpreter(P5::store_t &store, SetStorage *setStorage, P5::addr_t pc_top, P5::addr_t cp):
			store(store), set_storage(setStorage), pc_top(pc_top), mp(pc_top), sp(pc_top), ep(5), cp(cp), np(cp){};

	void run();

private:
	SetStorage *set_storage;

	//program top counter
	P5::addr_t pc_top = 0;

	//constant storage pointer
	P5::addr_t cp = 0;

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
	static P5::addr_t nil_val;
	static int mark_stack_size;

	//storage
	P5::store_t &store;

	P5::addr_t find_free_adr(P5::addr_t req_len);
	void call_sp(P5::addr_t sp_code);

	P5::addr_t get_base_addr(P5::ins_t p);

	template<typename T>
	void push_stack(T val);

	template<typename T>
	T pop_stack();

	class file_info {
	public:
		file_info(P5::addr_t addr);
		void reopen();
		std::string name;
		std::fstream strm;
	};
	file_info &find_or_insert_file_info(P5::addr_t file_addr);
	std::unordered_map<P5::addr_t, file_info> files;

	std::ostream &get_out_strm(P5::addr_t file_addr);
	Interpreter::file_info &get_out_file_strm(P5::addr_t file_addr);
	std::istream &get_in_strm(P5::addr_t file_addr);
	Interpreter::file_info &get_in_file_strm(P5::addr_t file_addr);
	static bool is_eof(std::istream& strm);

	template<typename T>
	void lod_instr(bool is_set = false);

	template<typename T>
	void ldo_instr(bool is_set = false);

	template<typename T>
	void str_instr();

	template<typename T>
	void sro_instr();

	template<typename T>
	void sto_instr();

	template<typename T>
	void ldc_instr(int type);

	template<typename T>
	void ind_instr(bool is_set = false);

	template<typename T>
	void inc_instr(bool is_set = false);

	void ret_instr(int mp_off);

	template<typename T>
	void chk_instr();

	template<typename T>
	void bin_op_instr(int sign);
	static P5::bool_t check_is_subset(std::shared_ptr<std::unordered_set<P5::set_el_t>> set, std::shared_ptr<std::unordered_set<P5::set_el_t>> subset);
	void bin_op_set_instr(int sign);
	void bin_op_m_instr(int sign);

	template<typename T>
	void unary_op_instr(int op);

	template<typename T>
	void write();

	template<typename T>
	void read();

	template<typename T>
	T get_pc();

};


#endif //P5_INTERPRETER_INTERPRETER_H
