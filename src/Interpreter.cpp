//
// Created by Anton on 05.01.2023.
//

#include <iomanip>
#include <cmath>
#include <limits>
#include <fstream>

#include "Interpreter.h"
#include "Assembler.h"
#include "SetStorage.h"
#include "addr_ops.h"
#include "p5_errors.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

int Interpreter::ms_fret_off = 0;
int Interpreter::ms_st_link_off = 8;
int Interpreter::ms_dyn_link_off = 12;
int Interpreter::ms_ep_off = 16;
int Interpreter::ms_stack_bottom_off = 20;
int Interpreter::ms_ep_cur_off = 24;
int Interpreter::ms_ret_addr_off = 28;
P5::addr_t Interpreter::nil_val = 1;
int Interpreter::mark_stack_size = 32;

template<>
void Interpreter::push_stack<P5::bool_t>(P5::bool_t val);
template<>
void Interpreter::push_stack<P5::set_t>(P5::set_t val);

template<>
P5::char_t Interpreter::pop_stack<P5::char_t>();
template<>
P5::set_t Interpreter::pop_stack<P5::set_t>();

template<>
void Interpreter::write<P5::bool_t>();

template<>
void Interpreter::read<P5::char_t>();

//template<>
//void Interpreter::write<P5::char_t>();

template<>
void Interpreter::lod_instr<P5::set_t>(bool is_set);

template<>
void Interpreter::ldo_instr<P5::set_t>(bool is_set);

template<>
void Interpreter::str_instr<P5::set_t>();

template<>
void Interpreter::sro_instr<P5::set_t>();

template<>
void Interpreter::sto_instr<P5::set_t>();

template<>
void Interpreter::ldc_instr<P5::set_t>(int type);

template<>
void Interpreter::ind_instr<P5::set_t>(bool is_set);

/*#if (with_set_push) == 1                      \
	case offset+2: (func)<P5::set_t>(true); break;       \
	#elif\
	case offset+2: (func)<P5::set_t>(); break;       \
    #endif          */

#define COMMON_CASES(start, offset, func) \
	case start: func<P5::int_t>(); break; \
	case offset: func<P5::addr_t>(); break; \
	case offset+1: func<P5::real_t>(); break; \
	case offset+3: func<P5::bool_t>(); break; \
	case offset+4: func<P5::char_t>(); break;

#define CASES_WITH_PUSH(start, offset, func) \
	case offset+2: func<P5::set_t>(true); break; \
    COMMON_CASES(start, offset, func)

#define CASES(start, offset, func) \
	case offset+2: func<P5::set_t>(); break; \
    COMMON_CASES(start, offset, func)

#define BIN_OP_COMP_CASES(start, offset, sign) \
	\
	case start: {\
	bin_op_instr<P5::addr_t>(sign);\
	break;\
	}\
	case offset+2: {\
	bin_op_instr<P5::bool_t>(sign);\
	break;\
	}\
	case offset+4: {\
	bin_op_instr<P5::char_t>(sign);\
	break;\
	}\
	case offset: {\
	bin_op_instr<P5::int_t>(sign);\
	break;\
	}\
	case offset+1: {\
	bin_op_instr<P5::real_t>(sign);\
	break;\
	}\
	case offset+3: {\
	bin_op_set_instr(sign);\
	break;\
	}\
	case offset+5: {\
	bin_op_m_instr(sign);\
	break;\
	}

#define BIN_OP_CASES(start, sign) \
	case start: bin_op_instr<P5::int_t>(sign); break; \
	case start+1: bin_op_instr<P5::real_t>(sign); break;

#define UNARY_OP_CASES(start, sign) \
	case start: unary_op_instr<P5::int_t>(sign); break; \
	case start+1: unary_op_instr<P5::real_t>(sign); break;

#define EQ_SIGN 1
#define NEQ_SIGN 2
#define GE_SIGN 3
#define GT_SIGN 4
#define LE_SIGN 5
#define LT_SIGN 6

#define ADD_OP 7
#define SUB_OP 8
#define MUL_OP 9
#define DIV_OP 10

#define NEG_OP 12
#define SQR_OP 13
#define ABS_OP 14

#define NOT_OP 15
#define AND_OP 16
#define OR_OP 17

#define IN_FILE_ADDR 32
#define OUT_FILE_ADDR 34
#define PRD_FILE_ADDR 36
#define PRR_FILE_ADDR 38

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"

Interpreter::file_info::file_info(P5::addr_t addr) {
	std::ios_base::openmode flags;
	if (addr == PRD_FILE_ADDR) {
		name = "prd";
		flags = std::fstream::in;
	} else if (addr == PRR_FILE_ADDR) {
		name = "prr";
		flags = std::fstream::out;
	} else {
		name = std::string("p5_temp_") + std::to_string(addr) + ".txt";
		flags = std::fstream::in | std::fstream::out | std::fstream::trunc;
	}
//	printf("open %s\n", name.c_str());
	strm = std::fstream(name, flags);
}

Interpreter::file_info::~file_info() {
//	std::cout << "closing " << name << std::endl;
	strm.close();
}

void Interpreter::file_info::reopen() {
//	printf("reopen %s\n", name.c_str());
	strm.close();
	std::ios_base::openmode flags;
	if (name == "prd") {
		flags = std::fstream::in;
	} else if (name == "prr") {
		flags = std::fstream::out;
	} else {
		flags = std::fstream::in | std::fstream::out | std::fstream::trunc;
	}
	strm.open(name, flags);
}

void Interpreter::run() {
	files.emplace(PRD_FILE_ADDR, PRD_FILE_ADDR);
	files.emplace(PRR_FILE_ADDR, PRR_FILE_ADDR);

//	TODO: DEBUG
//	auto &info = get_out_file_strm(pc_top+40);
//	auto &strm = info.strm;
//	std::cout << info.name << std::endl;
//	auto strm = std::fstream("test.txt", std::fstream::in | std::fstream::out | std::fstream::trunc);
//	strm << "test write";
//	strm.write("writ", 4);
//	std::cout << strm.fail() << strm.is_open() << std::endl;
//	strm.close();
//	return;
//	exit(1);
	//DEBUG

	printf("running program\n");
	pc = 0;
	ep = 5;

	auto interpreting = true;

	while (interpreting) {
		auto op_code = get_pc<P5::ins_t>();
//		printf("running op_code %d %s, pc %d\n", op_code, Assembler::op_codes[op_code].name.c_str(), pc-1);
		switch (op_code) {
			//lod
			CASES_WITH_PUSH(0, 105, lod_instr)

			//ldo
			CASES_WITH_PUSH(1, 65, ldo_instr)

			//str
			CASES(2, 70, str_instr)

			//sro
			CASES(3, 75, sro_instr)

			//lda
			case 4: {
				auto p = get_pc<P5::lvl_t>();
				auto q = get_pc<P5::addr_t>();
				push_stack(get_base_addr(p)+q);
				break;
			}

			//lao
			case 5: {
				auto q = get_pc<P5::addr_t>();
				push_stack(pc_top+q);
				break;
			}

			//sto
			CASES(6, 80, sto_instr)

			//ldc
			//other sequence => can not use CASES
			case 7: ldc_instr<P5::set_t>(7); break;
			case 123: ldc_instr<P5::int_t>(123); break;
			case 124: ldc_instr<P5::real_t>(124); break;
			case 125: ldc_instr<P5::addr_t>(125); break;
			case 126: ldc_instr<P5::bool_t>(126); break;
			case 127: ldc_instr<P5::char_t>(127); break;

			//ind
			CASES_WITH_PUSH(9, 85, ind_instr)

			//inc
			CASES_WITH_PUSH(10, 90, inc_instr)

			//mst
			case 11: {
				auto p = get_pc<P5::lvl_t>();
				auto frame_base_addr = sp; //mp
				sp += mark_stack_size;
				put_val_to_addr(store, frame_base_addr + ms_st_link_off, get_base_addr(p));
				put_val_to_addr(store, frame_base_addr + ms_dyn_link_off, mp);
				put_val_to_addr(store, frame_base_addr + ms_ep_off, ep);
				break;
			}

			//cup
			case 12: {
				auto p = get_pc<P5::lvl_t>();
				auto q = get_pc<P5::addr_t>();
//				printf("go to %d\n", q);
				mp = sp - (p+mark_stack_size);
				put_val_to_addr(store, mp+ms_ret_addr_off, pc);
				pc = q;
//				printf("jmp to pc %d\n", pc);
				break;
			}

			//ents
			case 13: {
				auto q = get_pc<P5::addr_t>();
				auto ad = mp + q;
				//fill locals area
				while (sp < ad) {
					store[sp] = 0;
					sp++;
				}
				put_val_to_addr(store, mp+ms_stack_bottom_off, sp);
				break;
			}
			//ente
			case 173: {
				auto q = get_pc<P5::addr_t>();
				ep = sp + q;
				put_val_to_addr(store, mp + ms_ep_cur_off, ep);
				break;
			}

			//retp
			case 14: ret_instr(0); break;
			//retc
			case 130: {
				//нужно, тк на стеке char хранится как int...
				put_val_to_addr(store, mp, (P5::int_t) get_val_at_addr<P5::char_t>(store, mp));
				ret_instr(sizeof(P5::int_t));
				break;
			}
			//retb
			case 131: {
				//аналогично предыдущему случаю
				put_val_to_addr(store, mp, (P5::int_t) get_val_at_addr<P5::bool_t>(store, mp));
				ret_instr(sizeof(P5::int_t));
				break;
			}
			//reti
			case 128: ret_instr(sizeof(P5::int_t)); break;
			//retr
			case 129: ret_instr(sizeof(P5::real_t)); break;
			//reta
			case 132: ret_instr(sizeof(P5::addr_t)); break;

			//csp
			case 15: {
				auto q = get_pc<P5::addr_t>();
				call_sp(q);
				break;
			}

			//ixa
			case 16: {
				auto q = get_pc<P5::addr_t>();
				auto i = pop_stack<P5::int_t>();
				auto addr = pop_stack<P5::addr_t>();
				push_stack(q*i + addr);
				break;
			}

			BIN_OP_COMP_CASES(17, 137, EQ_SIGN)
			BIN_OP_COMP_CASES(18, 143, NEQ_SIGN)
			BIN_OP_COMP_CASES(19, 149, GE_SIGN)
			BIN_OP_COMP_CASES(20, 155, GT_SIGN)
			BIN_OP_COMP_CASES(21, 161, LE_SIGN)
			BIN_OP_COMP_CASES(22, 167, LT_SIGN)

			//ujp
			case 23: {
				pc = get_pc<P5::addr_t>();
//				printf("jmp to pc %d\n", pc);
				break;
			}
			//fjp
			case 24: {
				auto q = get_pc<P5::addr_t>();
				auto i = pop_stack<P5::int_t>();
				if (i == 0) {
					pc = q;
//					printf("jmp to pc %d\n", pc);
				}
				break;
			}
			//xjp
			case 25: {
				auto q = get_pc<P5::addr_t>();
				auto i = pop_stack<P5::int_t>();
				pc = i * 5 + q;
//				printf("jmp to pc %d\n", pc);
				break;
			}

			//chka
			case 95: {
				auto q = get_pc<P5::addr_t>();
				auto addr = pop_stack<P5::addr_t>();
				if (addr == 0) {
					P5_ERR("uninitialized pointer")
				} else if (q != 0 && addr == nil_val) {
					P5_ERR("deref of nil pointer")
				} else if ((addr < np || addr >= cp) && addr != nil_val) {
					P5_ERR("bad pointer value")
				}
				break;
			}
			//chkr
			case 96:
			//chks
			case 97: {
				P5_ERR("instruction error")
			}
			//chkb
			case 98: chk_instr<P5::bool_t>(); break;
			//chkc
			case 99: chk_instr<P5::char_t>(); break;
			//chki
			case 26: chk_instr<P5::int_t>(); break;
			
			//eof
			case 27: {
				auto file_addr = pop_stack<P5::addr_t>();
//				printf("file addr %d %d\n", file_addr, file_addr-pc_top);
				auto &in_strm = get_in_strm(file_addr);
				push_stack((P5::bool_t) is_eof(in_strm));
				break;
			}

			//add
			BIN_OP_CASES(28, ADD_OP)
			//sub
			BIN_OP_CASES(30, SUB_OP)
			//sgs
			case 32: {
				auto i = pop_stack<P5::int_t>();
				std::unordered_set<P5::set_el_t> set;
				//TODO: check set element type
				set.insert(i);
				auto set_id = set_storage->create_set(std::move(set));
				set_id = set_storage->notify_push(set_id);
				push_stack(set_id);
				break;
			}
			//flt
			case 33: {
				auto i = pop_stack<P5::int_t>();
				push_stack((P5::real_t)i);
				break;
			}
			//flo
			case 34: {
				auto r = pop_stack<P5::real_t>();
				auto i = pop_stack<P5::int_t>();
				push_stack((P5::real_t)i);
				push_stack(r);
				break;
			}
			//trc
			case 35: {
				auto r = pop_stack<P5::real_t>();
				push_stack((P5::int_t)r);
				break;
			}
			//ng
			UNARY_OP_CASES(36, NEG_OP);
			//ng
			UNARY_OP_CASES(38, SQR_OP);
			//ab
			UNARY_OP_CASES(40, ABS_OP);

			case 42: {
				unary_op_instr<P5::bool_t>(NOT_OP);
				break;
			}
			case 43: {
				bin_op_instr<P5::bool_t>(AND_OP);
				break;
			}
			case 44: {
				bin_op_instr<P5::bool_t>(OR_OP);
				break;
			}

			//set diff
			case 45: {
				bin_op_set_instr(SUB_OP);
				break;
			}
			//set intersection
			case 46: {
				bin_op_set_instr(MUL_OP);
				break;
			}
			//set union
			case 47: {
				bin_op_set_instr(ADD_OP);
				break;
			}
			//inn
			case 48: {
				auto set_id = pop_stack<P5::set_t>();
				auto set = set_storage->get_set(set_id);
				auto i = pop_stack<P5::int_t>();
				push_stack(set->find(i) != set->end());
				set_storage->notify_pop(set_id);
				break;
			}
			//mod
			case 49: {
				auto i2 = pop_stack<P5::int_t>();
				auto i1 = pop_stack<P5::int_t>();
				push_stack(i1 % i2);
				break;
			}
			//odd
			case 50: {
				auto i = pop_stack<P5::int_t>();
				push_stack((i % 2) == 1);
				break;
			}
			//mp
			BIN_OP_CASES(51, MUL_OP)
			//div
			BIN_OP_CASES(53, DIV_OP)

			//mov
			case 55: {
				auto q = get_pc<P5::addr_t>();
				auto i2 = pop_stack<P5::int_t>();
				auto i1 = pop_stack<P5::int_t>();
				for (int i = 0; i < q; i++) {
					store[i1+i] = store[i2+i];
				}
				break;
			}

			//lca
			case 56: {
				auto q = get_pc<P5::addr_t>();
				push_stack(q);
				break;
			}
			//dec
			case 103:
			case 104:
			case 57: {
				auto q = get_pc<P5::addr_t>();
				auto i = pop_stack<P5::int_t>();
				push_stack(i-q);
				break;
			}

			case 58: interpreting = false; break;

			//ord ignore
			case 134:
			case 136:
			case 59: break;
			//chr
			case 60: break;
			//rnd
			case 62: {
				auto r = pop_stack<P5::real_t>();
				push_stack((P5::int_t)std::round(r));
				break;
			}

			//pck
			case 63: {
				auto q = get_pc<P5::addr_t>();
				auto q1 = get_pc<P5::addr_t>();
				auto a3 = pop_stack<P5::addr_t>();
				auto a2 = pop_stack<P5::addr_t>();
				auto a1 = pop_stack<P5::addr_t>();
				if (a2 + q > q1) {
					P5_ERR("pack elements out of bounds")
				}
				for (int i = 0; i < q; i++) {
					store[a3+i] = store[a1+a2];
					a2++;
				}
				break;
			}
			//upk
			case 64: {
				auto q = get_pc<P5::addr_t>();
				auto q1 = get_pc<P5::addr_t>();
				auto a3 = pop_stack<P5::addr_t>();
				auto a2 = pop_stack<P5::addr_t>();
				auto a1 = pop_stack<P5::addr_t>();
				if (a3 + q > q1) {
					P5_ERR("pack elements out of bounds")
				}
				for (int i = 0; i < q; i++) {
					store[a2+a3] = store[a1+i];
					a3++;
				}
				break;
			}

			//grs
			case 110: {
				auto i2 = pop_stack<P5::int_t>();
				auto i1 = pop_stack<P5::int_t>();
				std::unordered_set<P5::set_el_t> set;
				for (int i = i1; i <= i2; i++) {
					set.insert(i);
				}
				P5::set_t set_id = set_storage->create_set(std::move(set));
				set_id = set_storage->notify_push(set_id);
				push_stack(set_id);
				break;
			}

			//fbv
			case 111: {
//				printf("fbv\n");
				auto file_addr = pop_stack<P5::addr_t>();
				push_stack(file_addr);
				auto &in_strm = get_in_strm(file_addr);
				auto c = in_strm.peek();
				put_val_to_addr(store, file_addr+1, (char)c);
				break;
			}

			//ipj
			case 112: {
				auto p = get_pc<P5::lvl_t>();
				auto q = get_pc<P5::addr_t>();
				pc = q;
//				printf("jmp to pc %d\n", pc);
				mp = get_base_addr(p);
				sp = get_val_at_addr<P5::addr_t>(store, mp+ms_stack_bottom_off);
				ep = get_val_at_addr<P5::addr_t>(store, mp+ms_ep_cur_off);
				break;
			}
			//cip
			case 113: {
				auto p = get_pc<P5::lvl_t>();
				auto ad = pop_stack<P5::addr_t>();
				mp = sp - (p+mark_stack_size);
				put_val_to_addr(store, mp+ms_st_link_off, get_val_at_addr<P5::addr_t>(store, ad));
				put_val_to_addr(store, mp+ms_ret_addr_off, pc);
				pc = get_val_at_addr<P5::addr_t>(store, ad+4); //size of pointer
//				printf("jmp to pc %d\n", pc);
				break;
			}
			//lpa
			case 114: {
				auto p = get_pc<P5::lvl_t>();
				auto q = get_pc<P5::addr_t>();
				push_stack(get_base_addr(p));
				push_stack(q);
				break;
			}

			//TODO: case 115(efb), case 116(fvb)

			//dmp
			case 117: {
				auto q = get_pc<P5::addr_t>();
				sp -= q;
				break;
			}

			//swp
			case 118: {
				auto q = get_pc<P5::addr_t>();
				unsigned char buf[q];
				auto ptr = get_val_at_addr<P5::addr_t>(store, sp-P5::addr_size);
				get_val_at_addr_by_ptr(store, sp-P5::addr_size-q, buf, q);

				put_val_to_addr(store, sp-P5::addr_size-q, ptr);
				put_val_to_addr_by_ptr(store, sp-q, buf, q);
				break;
			}

			//tjp
			case 119: {
				auto q = get_pc<P5::addr_t>();
				auto i = pop_stack<P5::int_t>();
				if (i != 0) {
					pc = q;
//					printf("jmp to pc %d\n", pc);
				}
				break;
			}
			//lip
			case 120: {
				auto p = get_pc<P5::lvl_t>();
				auto q = get_pc<P5::addr_t>();
				auto ad = get_base_addr(p) + q;
				auto i = get_val_at_addr<P5::addr_t>(store, ad);
				auto a1 = get_val_at_addr<P5::addr_t>(store, ad+4); //ptr size
				push_stack(i);
				push_stack(a1);
				break;
			}

			default: {
				P5_ERR("unexpected op code %d", op_code);
			}
		}
	}

	printf("program complete\n");
}
#pragma clang diagnostic pop

P5::addr_t Interpreter::find_free_adr(P5::addr_t req_len) {
	P5::addr_t blk = np;
	P5::addr_t blk2;
	bool found = false;
	P5::addr_t found_blk_len;

	while (blk < cp) {
		found_blk_len = get_val_at_addr<P5::addr_t>(store, blk);
		if (found_blk_len >= req_len + P5::addr_size) {
			found = true;
			break;
		} else {
			blk += abs(found_blk_len);
		}
	}

	if (!found) return 0;

	put_val_to_addr(store, blk, -(req_len+P5::addr_size));
	blk2 = blk;
	blk += P5::addr_size;
	//create new free block from current found block
	if (found_blk_len > req_len + 2 * P5::addr_size) {
		blk2 += req_len + P5::addr_size;
		put_val_to_addr(store, blk2, found_blk_len-(req_len+P5::addr_size));
	}

	return blk;
}

void Interpreter::call_sp(P5::addr_t sp_code) {
	switch (sp_code) {
		//get
		case 0: {
			auto addr = pop_stack<P5::addr_t>();
			auto &in_strm = get_in_strm(addr);
			if (is_eof(in_strm)) {
				P5_ERR("get on end of file")
			}
			in_strm.seekg(1, std::ios::cur);
			break;
		}
		//put
		case 1: {
//			printf("put\n");
			auto addr = pop_stack<P5::addr_t>();
			auto &out_strm = get_out_strm(addr);
			auto c = get_val_at_addr<P5::char_t>(store, addr+1);
			out_strm << c;
			break;
		}
		//new
		case 4: {
			auto req_len = pop_stack<P5::addr_t>();
			//try finding existing block portion
			P5::addr_t blk_addr = find_free_adr(req_len);

			//allocate new block
			if (blk_addr == 0) {
				blk_addr = np - (req_len + P5::addr_size);
				if (blk_addr <= ep) {
					P5_ERR("heap area overlap with stack")
				}
				np = blk_addr;
				put_val_to_addr(store, blk_addr, -(req_len+P5::addr_size));
				blk_addr += P5::addr_size;
			}

			auto put_addr = pop_stack<P5::addr_t>();
			put_val_to_addr(store, put_addr, blk_addr);
			break;
		}
		//dsp release heap space
		case 26: {
//			TODO: implement
			break;
		}
		//wln
		case 5: {
			auto ad = pop_stack<P5::addr_t>();
			push_stack(ad);
			auto &out_strm = get_out_strm(ad);
			out_strm << std::endl;
			break;
		}
		//wrs
		case 6: {
			auto len = pop_stack<P5::int_t>();
			auto width = pop_stack<P5::int_t>();
			auto str_addr = pop_stack<P5::addr_t>();
			auto file_addr = pop_stack<P5::addr_t>();
			//DEBUG
//			printf("wrs pc %d, l %d, w %d ", pc-5, len, width);
//			for (int i = 0; i < len; i++) {
//				printf("%c(%d)", (char)store[str_addr + i], (char)store[str_addr + i]);
//			}
			//DEBUG
			push_stack(file_addr);
			auto &out_strm = get_out_strm(file_addr);
			if (width > len) {
				for (int i = 0; i < width-len; i++) {
					out_strm << ' ';
				}
			} else {
				len = width;
			}
			for (int i = 0; i < len; i++) {
				out_strm << (char)store[str_addr + i];
			}
			break;
		}
		//wri
		case 8: {
//			printf("int pc %d", pc-5);
			write<P5::int_t>();
			break;
		}
		//wrr
		case 9: {
			write<P5::real_t>();
			break;
		}
		//wrc
		case 10: {
//			printf("char pc %d ", pc-5);
			write<P5::char_t>();
			break;
		}
		//wrb
		case 24: {
//			printf("char ");
			write<P5::bool_t>();
			break;
		}
		//wrf
		case 25: {
			auto precision = pop_stack<P5::int_t>();
			auto width = pop_stack<P5::int_t>();
			auto r = pop_stack<P5::real_t>();
			auto file_addr = pop_stack<P5::addr_t>();
			push_stack(file_addr);
			auto &out_strm = get_out_strm(file_addr);
			out_strm << std::setw(width) << std::setprecision(precision+1) << r;
			break;
		}

		//rln
		case 3: {
			auto file_addr = pop_stack<P5::addr_t>();
			push_stack(file_addr);
			auto &in_strm = get_in_strm(file_addr);
			if (in_strm.eof()) {
				P5_ERR("end of file")
			}
			in_strm.ignore(std::numeric_limits<std::streamsize>::max(), in_strm.widen('\n'));
			break;
		}
		//eln
		case 7: {
			auto file_addr = pop_stack<P5::addr_t>();
			auto &in_strm = get_in_strm(file_addr);
			auto next = in_strm.peek();
			auto is_eoln = false;
			if (next == '\n' || next == '\r') {
				is_eoln = true;
			}
			push_stack(is_eoln);
			break;
		}
		//rdi
		case 11: {
			read<P5::int_t>();
			break;
		}
		//rdr
		case 12: {
			read<P5::real_t>();
			break;
		}
		//rdc
		case 13: {
			read<P5::char_t>();
			break;
		}
		case 14: {
			auto r = pop_stack<P5::real_t>();
			push_stack(sin(r));
			break;
		}
		case 15: {
			auto r = pop_stack<P5::real_t>();
			push_stack(cos(r));
			break;
		}
		case 16: {
			auto r = pop_stack<P5::real_t>();
			push_stack(exp(r));
			break;
		}
		case 17: {
			auto r = pop_stack<P5::real_t>();
			push_stack(logl(r)); //ln
			break;
		}
		case 18: {
			auto r = pop_stack<P5::real_t>();
			push_stack(sqrt(r));
			break;
		}
		case 19: {
			auto r = pop_stack<P5::real_t>();
			push_stack(atan(r));
			break;
		}

		//rsf
		case 22: {
			auto ad = pop_stack<P5::addr_t>();
			if (ad - pc_top == IN_FILE_ADDR) {
				P5_ERR("reset on input file")
			}
			auto &in_strm = get_in_file_strm(ad);
			in_strm.reopen();
			break;
		}
		//rwf
		case 23: {
			auto ad = pop_stack<P5::addr_t>();
			if (ad - pc_top == OUT_FILE_ADDR) {
				P5_ERR("rewrite on output file")
			}
			auto &f_strm = get_out_file_strm(ad);
			f_strm.reopen();
			break;
		}
		default: {
			P5_ERR("unknown sp %d", sp_code)
		}
	}
}

P5::addr_t Interpreter::get_base_addr(P5::ins_t p) {
	auto addr = mp;
	while (p > 0) {
		addr = get_val_at_addr<P5::addr_t>(store, addr + ms_st_link_off);
		p--;
	}
	return addr;
}

template<typename T>
void Interpreter::push_stack(T val) {
	//TODO: check for sp < ep
//	std::cout << "push " << val << std::endl;
	put_val_to_addr(store, sp, val);
	sp += sizeof(T);
}

template<>
void Interpreter::push_stack(P5::bool_t val) {
	push_stack((P5::int_t)val);
}
template<>
void Interpreter::push_stack(P5::char_t val) {
	push_stack((P5::int_t)val);
}
template<>
void Interpreter::push_stack(P5::set_t val) {
//	printf("push set %d\n", val);
	put_val_to_addr(store, sp, val);
	sp += P5::set_size;
}

template<typename T>
T Interpreter::pop_stack() {
	sp -= sizeof(T);
	if (sp < mp + mark_stack_size) {
		P5_ERR("stack underflow sp %d, mp %d\n", sp, mp)
	}
	T val;
	get_val_at_addr_by_ptr(store, sp, (unsigned char *) &val, sizeof(T));
//	std::cout << "pop " << val << std::endl;
	return val;
}
template<>
P5::bool_t Interpreter::pop_stack() {
	return pop_stack<P5::int_t>();
}

template<>
P5::char_t Interpreter::pop_stack() {
	return pop_stack<P5::int_t>();
}

template<>
P5::set_t Interpreter::pop_stack() {
	sp -= P5::set_size;
	if (sp < mp + mark_stack_size) {
		P5_ERR("stack underflow sp %d, mp %d\n", sp, mp)
	}
	P5::set_t val;
	get_val_at_addr_by_ptr(store, sp, (unsigned char *) &val, sizeof(P5::set_t));
//	printf("pop set %d\n", val);
	return val;
}

template<typename T>
T Interpreter::get_pc() {
	auto val = get_val_at_addr<T>(store, pc);
	pc += sizeof(T);
	return val;
}

template<>
void Interpreter::lod_instr<P5::set_t>(bool is_set) {
	auto p = get_pc<P5::lvl_t>();
	auto q = get_pc<P5::addr_t>();
	auto val = get_val_at_addr<P5::set_t>(store, get_base_addr(p) + q);
//	printf("lod set %d, p %d, q %d\n", val, p, q);
//	val = set_storage->notify_push(val);
	push_stack(val);
}

template<typename T>
void Interpreter::lod_instr(bool is_set) {
	auto p = get_pc<P5::lvl_t>();
	auto q = get_pc<P5::addr_t>();
	auto val = get_val_at_addr<T>(store, get_base_addr(p) + q);
//	if (is_set) {
//		val = set_storage->notify_push(val);
//	}
	push_stack(val);
}

template<>
void Interpreter::ldo_instr<P5::set_t>(bool is_set) {
	auto q = get_pc<P5::addr_t>();
	auto val = get_val_at_addr<P5::set_t>(store, pc_top + q);
	val = set_storage->get_id(val);
//	printf("ldo set %d, q %d\n", val, q);
	push_stack(val);
}

template<typename T>
void Interpreter::ldo_instr(bool is_set) {
	auto q = get_pc<P5::addr_t>();
	auto val = get_val_at_addr<T>(store, pc_top + q);
//	if (is_set) {
//		val = set_storage->notify_push(val);
//	}
	push_stack(val);
}

template<>
void Interpreter::str_instr<P5::set_t>() {
	auto p = get_pc<P5::lvl_t>();
	auto q = get_pc<P5::addr_t>();
	auto val = pop_stack<P5::set_t>();
//	printf("str set %d, p %d, q %d\n", val, p, q);
//	val = set_storage->get_id(val);
	put_val_to_addr(store, get_base_addr(p) + q, val);
}

template<typename T>
void Interpreter::str_instr() {
	auto p = get_pc<P5::lvl_t>();
	auto q = get_pc<P5::addr_t>();
	//doesn't need to push set index (-1 +1)
	auto val = pop_stack<T>();
	put_val_to_addr(store, get_base_addr(p) + q, val);
}

template<>
void Interpreter::sro_instr<P5::set_t>() {
	auto q = get_pc<P5::addr_t>();
	//doesn't need to push set index (-1 +1)
	auto val = pop_stack<P5::set_t>();
//	printf("sro set %d, q %d\n", val, q);
//	val = set_storage->get_id(val);
	put_val_to_addr(store, pc_top + q, val);
}

template<typename T>
void Interpreter::sro_instr() {
	auto q = get_pc<P5::addr_t>();
	//doesn't need to push set index (-1 +1)
	auto val = pop_stack<T>();
	put_val_to_addr(store, pc_top + q, val);
}

template<>
void Interpreter::sto_instr<P5::set_t>() {
	auto val = pop_stack<P5::set_t>();
	auto addr = pop_stack<P5::addr_t>();
//	printf("sto set %d, addr %d\n", val, addr);
//	val = set_storage->get_id(val);
	put_val_to_addr(store, addr, val);
}

template<typename T>
void Interpreter::sto_instr() {
	auto val = pop_stack<T>();
	auto addr = pop_stack<P5::addr_t>();
	put_val_to_addr(store, addr, val);
}

template<>
void Interpreter::ldc_instr<P5::set_t>(int type) {
	auto set_id = get_pc<P5::set_t>();
//	printf("ldc %d ", set_id);
//	set_id = set_storage->notify_push(set_id);
//	printf("%d\n", set_id);
	push_stack(set_id);
}

template<typename T>
void Interpreter::ldc_instr(int type) {
	if (type == 125) {
		//addr
		push_stack(nil_val);
	} else if (type == 124) {
		//real
		auto q = get_pc<P5::addr_t>();
		push_stack(get_val_at_addr<P5::real_t>(store, q));
	} else if (type == 7) {
		//set
		auto set_id = get_pc<P5::set_t>();
		set_id = set_storage->notify_push(set_id);
		push_stack(set_id);
	}
//	else if (type == 127) {
//		int c = store[pc];
//		pc++;
//		push_stack(c);
//	}
	else{
		auto val = get_pc<T>();
		push_stack(val);
//		std::cout << val << std::endl;
	}
}

template<>
void Interpreter::ind_instr<P5::set_t>(bool is_set) {
	auto q = get_pc<P5::addr_t>();
	auto addr = pop_stack<P5::addr_t>();
	auto val = get_val_at_addr<P5::set_t>(store, addr + q);
//	printf("ind set %d, q %d, addr %d\n", val, q, addr);
//	val = set_storage->notify_push(val);
	push_stack(val);
}

template<typename T>
void Interpreter::ind_instr(bool is_set) {
	auto q = get_pc<P5::addr_t>();
	auto addr = pop_stack<P5::addr_t>();
	T val = get_val_at_addr<T>(store, addr + q);
//	TODO: make functions like ind_set_instr for this
//	if (is_set) {
//		val = set_storage->notify_push(val);
//	}
	push_stack(val);
}

template<typename T>
void Interpreter::inc_instr(bool is_set) {
	if (is_set) {
		P5_ERR("cannot increment set")
	}
	auto q = get_pc<P5::addr_t>();
	auto val = pop_stack<T>();
	push_stack(val+q);
}

void Interpreter::ret_instr(int mp_off) {
	sp = mp + mp_off;
	pc = get_val_at_addr<P5::addr_t>(store, mp + ms_ret_addr_off);
	ep = get_val_at_addr<P5::addr_t>(store, mp + ms_ep_off);
	mp = get_val_at_addr<P5::addr_t>(store, mp + ms_dyn_link_off);
}

template<typename T>
void Interpreter::chk_instr() {
	auto q = get_pc<P5::addr_t>();
	auto val = pop_stack<T>();
	push_stack(val);
	auto lb = get_val_at_addr<P5::int_t>(store, q);
	auto rb = get_val_at_addr<P5::int_t>(store, q+P5::int_size);
	if (val < lb || val > rb) {
		P5_ERR("value out of range %d (%d-%d)", val, lb, rb)
	}
}

template<typename T>
void Interpreter::write() {
	auto w = pop_stack<P5::int_t>();//width
	auto val = pop_stack<T>();
	auto addr = pop_stack<P5::addr_t>();
	push_stack(addr);
//	printf("out addr %d ", addr-pc_top);
	auto &out_strm = get_out_strm(addr);
	out_strm << std::setw(w) << val;
}

////TODO: DEBUG
//template<>
//void Interpreter::write<P5::char_t>() {
//	auto w = pop_stack<P5::int_t>();//width
//	auto val = pop_stack<P5::char_t>();
//	auto addr = pop_stack<P5::addr_t>();
//	push_stack(addr);
//	auto &out_strm = get_out_strm(addr);
////	printf("%c(%d) pc %d ", val, val, pc-5);
////	out_strm << std::setw(w) << val;
//}

template<>
void Interpreter::write<P5::bool_t>() {
	auto w = pop_stack<P5::int_t>();//width
	auto val = pop_stack<P5::bool_t>();
	auto addr = pop_stack<P5::addr_t>();
	push_stack(addr);
	auto &out_strm = get_out_strm(addr);
	std::string str;
	if (val == 0) {
		str = "false";
	} else {
		str = "true";
	}
	out_strm << std::setw(w) << str;
}

template<typename T>
void Interpreter::read() {
	auto put_addr = pop_stack<P5::addr_t>();
	auto file_addr = pop_stack<P5::addr_t>();
	push_stack(file_addr);
	auto &in_strm = get_in_strm(file_addr);
	T val;
	in_strm >> val;
	put_val_to_addr(store, put_addr, val);
}

template<>
void Interpreter::read<P5::char_t>() {
	auto put_addr = pop_stack<P5::addr_t>();
	auto file_addr = pop_stack<P5::addr_t>();
	push_stack(file_addr);
	auto &in_strm = get_in_strm(file_addr);
	P5::char_t val;
	in_strm >> std::noskipws >> val;
	if (val == '\n') {
		val = ' ';
	}
//	printf(" rdc %c(%d) ", val, val);
	put_val_to_addr(store, put_addr, val);
}

Interpreter::file_info &Interpreter::find_or_insert_file_info(P5::addr_t file_addr) {
	auto info_it = files.find(file_addr);
	if (info_it == files.end()) {
		info_it = files.emplace(file_addr, file_addr).first;
	}
	return info_it->second;
}


std::ostream &Interpreter::get_out_strm(P5::addr_t file_addr) {
	file_addr -= pc_top;
	switch (file_addr) {
		case IN_FILE_ADDR: {
			P5_ERR("write to input file");
		}
		case OUT_FILE_ADDR: {
			return std::cout;
		}
		case PRD_FILE_ADDR: {
			P5_ERR("write to prd file");
		}
		default: {
			return find_or_insert_file_info(file_addr).strm;
		}
	}
}

Interpreter::file_info &Interpreter::get_out_file_strm(P5::addr_t file_addr) {
	file_addr -= pc_top;
	switch (file_addr) {
		case IN_FILE_ADDR: {
			P5_ERR("get out file on input file")
		}
		case OUT_FILE_ADDR: {
			P5_ERR("get out file on output file");
		}
		case PRD_FILE_ADDR: {
			P5_ERR("get out file on prd")
		}
		default: {
			return find_or_insert_file_info(file_addr);
		}
	}

}

std::istream &Interpreter::get_in_strm(P5::addr_t file_addr) {
	file_addr -= pc_top;
	switch (file_addr) {
		case IN_FILE_ADDR: {
			return std::cin;
		}
		case OUT_FILE_ADDR: {
			P5_ERR("input from output stream");
		}
		case PRR_FILE_ADDR:
			P5_ERR("input from prr")
		default: {
			return find_or_insert_file_info(file_addr).strm;
		}
	}
}

Interpreter::file_info &Interpreter::get_in_file_strm(P5::addr_t file_addr) {
	file_addr -= pc_top;
	switch (file_addr) {
		case IN_FILE_ADDR: {
			P5_ERR("get file strm from input stream");
		}
		case OUT_FILE_ADDR: {
			P5_ERR("input from output stream");
		}
		case PRR_FILE_ADDR: {
			P5_ERR("input from prr")
		}
		default: {
			return find_or_insert_file_info(file_addr);
			P5_ERR("get in strm unk write to addr %d, pc top %d\n", file_addr, pc_top);
		}
	}
}

bool Interpreter::is_eof(std::istream& strm) {
//	auto cur_pos = strm.tellg();
//	strm.seekg(0, std::ios::end);
//	auto size = strm.tellg();
////	printf("eof %ld %ld\n", (long)cur_pos, (long)size);
//	strm.seekg(cur_pos, std::ios_base::beg);
	auto next = strm.peek();
//	printf("next %d\n", next);
//	if ((long)cur_pos == -1) {
//		P5_ERR("eof err, read %td\n", strm.gcount());
//	}
	return next == std::istream::traits_type::eof();
}

template<typename T>
void Interpreter::bin_op_instr(int sign) {
	auto v2 = pop_stack<T>();
	auto v1 = pop_stack<T>();
	switch (sign) {
		case EQ_SIGN: {
			push_stack((P5::bool_t) (v1 == v2));
			break;
		}
		case NEQ_SIGN: {
			push_stack((P5::bool_t) (v1 != v2));
			break;
		}
		case GE_SIGN: {
			push_stack((P5::bool_t) (v1 >= v2));
			break;
		}
		case GT_SIGN: {
			push_stack((P5::bool_t) (v1 > v2));
			break;
		}
		case LE_SIGN: {
			push_stack((P5::bool_t) (v1 <= v2));
			break;
		}
		case LT_SIGN: {
			push_stack((P5::bool_t) (v1 < v2));
			break;
		}
		case ADD_OP: {
			push_stack(v1+v2);
			break;
		}
		case SUB_OP: {
			push_stack(v1-v2);
			break;
		}
		case MUL_OP: {
			push_stack(v1*v2);
			break;
		}
		case DIV_OP: {
			if (v2 == 0) {
				P5_ERR("division by zero")
			}
			push_stack(v1/v2);
			break;
		}
		case AND_OP: {
			push_stack(v1 && v2);
			break;
		}
		case OR_OP: {
			push_stack(v1 || v2);
			break;
		}
		default: {
			P5_ERR("unknown sign comp")
		}
	}
}

void Interpreter::bin_op_set_instr(int sign) {
	auto s2_id = pop_stack<P5::set_t>();
	auto s1_id = pop_stack<P5::set_t>();
//	printf("bin op on sets %d %d\n", s1_id, s2_id);
	auto s2 = set_storage->get_set(s2_id);
	auto s1 = set_storage->get_set(s1_id);
	switch (sign) {
		case EQ_SIGN: {
			push_stack((P5::bool_t) (*s1 == *s2));
			break;
		}
		case NEQ_SIGN: {
			push_stack((P5::bool_t) (*s1 != *s2));
			break;
		}
		case GE_SIGN: {
			push_stack(check_is_subset(s1, s2));
			break;
		}
		case LE_SIGN: {
			push_stack(check_is_subset(s2, s1));
			break;
		}
		//difference
		case SUB_OP: {
			std::unordered_set<P5::set_el_t> set;
			for (auto v: *s1) {
				if (s2->find(v) == s2->end()) {
					set.insert(v);
				}
			}
			P5::set_t set_id = set_storage->create_set(std::move(set));
			set_id = set_storage->notify_push(set_id);
			push_stack(set_id);
			break;
		}
		//intersection
		case MUL_OP: {
			std::unordered_set<P5::set_el_t> set;
			for (auto v: *s1) {
				if (s2->find(v) != s2->end()) {
					set.insert(v);
				}
			}
			P5::set_t set_id = set_storage->create_set(std::move(set));
			set_id = set_storage->notify_push(set_id);
			push_stack(set_id);
			break;
		}
		//union
		case ADD_OP: {
			std::unordered_set<P5::set_el_t> set;
			for (auto v: *s1) {
				set.insert(v);
			}
			for (auto v: *s2) {
				set.insert(v);
			}
//			for (auto v: *s1) {
//				printf("%d ", v);
//			}
//			printf("\n");
//			for (auto v: *s2) {
//				printf("%d ", v);
//			}
//			printf("\n");
//			for (auto v: set) {
//				printf("%d ", v);
//			}
//			printf("\n");
			P5::set_t set_id = set_storage->create_set(std::move(set));
			set_id = set_storage->notify_push(set_id);
			push_stack(set_id);
			break;
		}
		default: {
			P5_ERR("unknown sign on set")
		}
	}
	set_storage->notify_pop(s1_id);
	set_storage->notify_pop(s2_id);
}

void Interpreter::bin_op_m_instr(int sign) {
	auto q = get_pc<P5::addr_t>();
	auto ad2 = pop_stack<P5::addr_t>();
	auto ad1 = pop_stack<P5::addr_t>();
	bool eq = true;
	int i;
	for (i = 0; i < q; i++) {
		if (store[ad1+i] != store[ad2+i]) {
			eq = false;
			break;
		}
	}

	switch (sign) {
		case EQ_SIGN: {
			push_stack(eq);
			break;
		}
		case NEQ_SIGN: {
			push_stack(!eq);
			break;
		}
		case GE_SIGN: {
			push_stack(eq || (store[ad1+i] >= store[ad2+i]));
			break;
		}
		case GT_SIGN: {
			push_stack(!eq && (store[ad1+i] > store[ad2+i]));
			break;
		}
		case LE_SIGN: {
			push_stack(eq || (store[ad1+i] <= store[ad2+i]));
			break;
		}
		case LT_SIGN: {
			push_stack(!eq && (store[ad1+i] < store[ad2+i]));
			break;
		}
		default: {
			P5_ERR("unknown sign on compm")
		}
	}
//	push_stack(eq);
}

P5::bool_t Interpreter::check_is_subset(std::shared_ptr<std::unordered_set<P5::set_el_t>> set,
								  std::shared_ptr<std::unordered_set<P5::set_el_t>> subset) {
	for (auto &v: *subset) {
		if (set->find(v) == set->end()) {
			return false;
		}
	}
	return true;
}

template<typename T>
void Interpreter::unary_op_instr(int op) {
	auto v = pop_stack<T>();
	switch (op) {
		case NEG_OP: {
			push_stack(-v);
			break;
		}
		case SQR_OP: {
			push_stack(v*v);
			break;
		}
		case ABS_OP: {
			if (v < 0) {
				v = -v;
			}
			push_stack(v);
			break;
		}
		case NOT_OP: {
			push_stack(!v);
			break;
		}
		default: {
			P5_ERR("invalid unary operation %d", op)
		}
	}
}

#pragma clang diagnostic pop