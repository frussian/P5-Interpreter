#ifndef P5_INTERPRETER_ASSEMBLER_H
#define P5_INTERPRETER_ASSEMBLER_H

#include <string>
#include <unordered_map>

class Assembler {
public:
	explicit Assembler(std::string filename):
		filename(std::move(filename)) {};
	void load();

	//init tables
	static void init();

	struct OpCodeInfo {
		bool has_p;
		int q_len;
	};

public:
	static std::unordered_map<int, OpCodeInfo> op_codes;

private:
	std::string filename;

	//instructions base (for type based instructions) name map
	static std::unordered_map<std::string, int> instr;

	//standard procedures
	static std::unordered_map<std::string, int> sp_table;
};


#endif //P5_INTERPRETER_ASSEMBLER_H
