//
// Created by Anton on 05.01.2023.
//

#include <iomanip>

#include "Interpreter.h"
#include "Assembler.h"
#include "SetStorage.h"
#include "addr_ops.h"
#include "p5_errors.h"

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
P5::char_t Interpreter::pop_stack();
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

void Interpreter::run() {
	printf("running program\n");
	pc = 0;
	ep = 5;

	auto interpreting = true;

	while (interpreting) {
		auto op_code = get_pc<P5::ins_t>();
//		printf("running op_code %d %s\n", op_code, Assembler::op_codes[op_code].name.c_str());
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

			//dmp
			case 117: {
				auto q = get_pc<P5::addr_t>();
				sp -= q;
				break;
			}

			//swp
			case 118: {
				auto q = get_pc<P5::addr_t>();
				//TODO: change this
//				q = 1;
				unsigned char buf[q];
				auto ptr = get_val_at_addr<P5::addr_t>(store, sp-P5::addr_size);
				get_val_at_addr_by_ptr(store, sp-P5::addr_size-q, buf, q);

				put_val_to_addr(store, sp-P5::addr_size-q, ptr);
				put_val_to_addr_by_ptr(store, sp-q, buf, q);
				break;
			}

			//fbv
			case 111: {
				//TODO: implement
				break;
			}

			case 58: interpreting = false; break;
			default: {
				P5_ERR("unexpected op code %d", op_code);
			}
		}
	}
}

void Interpreter::call_sp(P5::addr_t sp_code) {
	switch (sp_code) {
		//wln
		case 5: {
			auto ad = pop_stack<P5::addr_t>();
			push_stack(ad);
			printf("\n");
			break;
		}
		//wri
		case 8: {
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
			write<P5::char_t>();
			break;
		}
		//wrc
		case 24: {
			write<P5::bool_t>();
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


template<typename T>
T Interpreter::pop_stack() {
	sp -= sizeof(T);
	if (sp < mp + mark_stack_size) {
		P5_ERR("stack underflow sp %d, mp %d\n", sp, mp)
	}
	T val;
	get_val_at_addr_by_ptr(store, sp, (unsigned char *) &val, sizeof(T));
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

template<typename T>
T Interpreter::get_pc() {
	auto val = get_val_at_addr<T>(store, pc);
	pc += sizeof(T);
	return val;
}

template<typename T>
void Interpreter::lod_instr(bool is_set) {
	auto p = get_pc<P5::lvl_t>();
	auto q = get_pc<P5::addr_t>();
	auto val = get_val_at_addr<T>(store, get_base_addr(p) + q);
	if (is_set) {
		setStorage->notify_push(val);
	}
	push_stack(val);
}

template<typename T>
void Interpreter::ldo_instr(bool is_set) {
	auto q = get_pc<P5::addr_t>();
	auto val = get_val_at_addr<T>(store, pc_top + q);
	if (is_set) {
		setStorage->notify_push(val);
	}
	push_stack(val);
}

template<typename T>
void Interpreter::str_instr() {
	auto p = get_pc<P5::lvl_t>();
	auto q = get_pc<P5::addr_t>();
	//doesn't need to push set index (-1 +1)
	auto val = pop_stack<T>();
	put_val_to_addr(store, get_base_addr(p) + q, val);
}

template<typename T>
void Interpreter::sro_instr() {
	auto q = get_pc<P5::addr_t>();
	//doesn't need to push set index (-1 +1)
	auto val = pop_stack<T>();
	put_val_to_addr(store, pc_top + q, val);
}

template<typename T>
void Interpreter::sto_instr() {
	auto val = pop_stack<T>();
	auto addr = pop_stack<P5::addr_t>();
	put_val_to_addr(store, addr, val);
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
		setStorage->notify_push(set_id);
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

template<typename T>
void Interpreter::ind_instr(bool is_set) {
	auto q = get_pc<P5::addr_t>();
	auto addr = pop_stack<P5::addr_t>();
	T val = get_val_at_addr<T>(store, addr + q);
	push_stack(val);
	//TODO: make functions like ind_set_instr for this
	if (is_set) {
		setStorage->notify_push(val);
	}
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
	addr -= pc_top;
	switch (addr) {
		case 32:
			P5_ERR("write to input file");
			break;
		case 34:
//			printf("write to output\n");
			std::cout << std::setw(w) << val;
			break;
		case 36:
			P5_ERR("write to prd file");
			break;
		case 38:
			printf("write to prr\n");
			//TODO: write here
			break;
		default:
			P5_ERR("unk write to addr %d, pc top %d\n", addr, pc_top);
	}
}
