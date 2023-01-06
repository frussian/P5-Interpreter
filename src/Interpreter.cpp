//
// Created by Anton on 05.01.2023.
//

#include "Interpreter.h"
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

/*#if (with_set_push) == 1                      \
	case offset+2: (func)<P5::set_t>(true); break;       \
	#elif\
	case offset+2: (func)<P5::set_t>(); break;       \
    #endif          */

#define COMMON_CASES(start, offset, func) \
	case start: func<P5::int_t>(); break; \
	case offset: func<P5::addr_t>(); break; \
	case offset+1: func<P5::real_t>(); break;      \
	case offset+3: func<P5::bool_t>(); break; \
	case offset+4: func<P5::char_t>(); break;

#define CASES_WITH_PUSH(start, offset, func) \
	case offset+2: func<P5::set_t>(true); break; \
    COMMON_CASES(start, offset, func)

#define CASES(start, offset, func) \
	case offset+2: func<P5::set_t>(); break; \
    COMMON_CASES(start, offset, func)

[[noreturn]] void Interpreter::run() {
	pc = 0;
	ep = 5;

	auto interpreting = true;

	while (interpreting) {
		auto op_code = get_pc<P5::ins_t>();
		switch (op_code) {
			//lod
			CASES_WITH_PUSH(0, 105, lod_instr)

			//ldo
			CASES_WITH_PUSH(1, 65, ldo_instr)

			//str
			CASES(2, 70, str_instr)

			//sroi
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
			case 7: ldc_instr<P5::set_t>(7); break;
			case 123: ldc_instr<P5::int_t>(); break;
			case 124: ldc_instr<P5::real_t>(124); break;
			case 125: ldc_instr<P5::addr_t>(125); break;
			case 126: ldc_instr<P5::bool_t>(); break;
			case 127: ldc_instr<P5::char_t>(); break;

			//ind
			CASES_WITH_PUSH(9, 85, ind_instr)

			case 58: interpreting = false; break;
			default: {
				P5_ERR("unexpected op code %d", op_code);
			}
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

template<typename T>
T Interpreter::pop_stack() {
	//TODO: check for sp > ?
	sp -= sizeof(T);
	T val;
	get_val_at_addr_by_ptr(store, sp, (unsigned char *) &val, sizeof(T));
	return val;
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
	} else {
		auto val = get_pc<T>();
		push_stack(val);
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
