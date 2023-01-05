#ifndef P5_INTERPRETER_ASSEMBLER_H
#define P5_INTERPRETER_ASSEMBLER_H

#include <string>
#include <unordered_map>

#include "p5_common.h"

class LabelTable;
class Lexer;

class Assembler {
public:
	explicit Assembler(P5::store_t &store, std::string filename);
	~Assembler();
	void load();
	void dump();

	//init tables
	static void init();

	struct OpCodeInfo {
		bool has_p;
		int q_len;
	};

public:
	static std::unordered_map<P5::ins_t, OpCodeInfo> op_codes;

private:
	//source file
	std::string filename;

	//instructions base (for type based instructions) name map
	static std::unordered_map<std::string, P5::ins_t> instr;

	//standard procedures
	static std::unordered_map<std::string, P5::ins_t> sp_table;

	//label table
	LabelTable *lb_table = nullptr;
	Lexer *lexer = nullptr;

	//program counter
	P5::addr_t pc = 0;

	//program top counter
	P5::addr_t pc_top = 0;

	//constants pointer
	P5::addr_t cp = 0;

	//storage
	P5::store_t &store;

	void generate();
	void assemble();

	template<typename T>
	void store_pc(T val);

	P5::addr_t label_search();
	static void expect_char(char ec, char ac);
};


#endif //P5_INTERPRETER_ASSEMBLER_H
