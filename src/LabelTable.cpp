//
// Created by Anton on 13.11.2022.
//

#include "LabelTable.h"
#include "addr_ops.h"
#include "Assembler.h"

P5::addr_t LabelTable::lookup(int lbl, int pc) {
	auto info_it = lb_storage.find(lbl);
	if (info_it == lb_storage.end()) {
		info_it = lb_storage.insert({lbl, LabelInfo{
				.defined = false,
				.ptr = -1}}).first;
	}

	LabelInfo *info = &info_it->second;
	int q = info->ptr;
	if (!info->defined) {
		info->ptr = pc;
	}

	return q;
}

int LabelTable::update(int lbl, P5::addr_t lbl_val) {
	auto info_it = lb_storage.find(lbl);
	LabelInfo *info = &info_it->second;
	if (info->defined) {
		//TODO: handle error properly
		printf("duplicated label %d\n", lbl);
		exit(0);
	}

	if (info->ptr != -1) {
		P5::addr_t curr = info->ptr;
		while (curr != -1) {
			unsigned char opcode = store[curr];
			bool has_p = Assembler::op_codes[opcode].has_p;
			auto q = get_addr<P5::addr_t>(store, curr+1+has_p);
			put_addr(store, curr+1+has_p, lbl_val);
			curr = q;
		}
	}

	info->defined = true;
	info->ptr = lbl_val;

	return 0;
}