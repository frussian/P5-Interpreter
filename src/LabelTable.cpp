//
// Created by Anton on 13.11.2022.
//

#include "LabelTable.h"
#include "addr_ops.h"
#include "Assembler.h"
#include "p5_errors.h"

P5::addr_t LabelTable::lookup(int lbl, int pc) {
	LabelInfo *info = find_or_insert_lbl_info(lbl);

	int q = info->ptr;
	if (!info->defined) {
		info->ptr = pc;
	}

	return q;
}

int LabelTable::update(int lbl, P5::addr_t lbl_val) {
	LabelInfo *info = find_or_insert_lbl_info(lbl);
	if (info->defined) {
		P5_ERR("duplicated label %d\n", lbl);
	}

	if (info->ptr != -1) {
		P5::addr_t curr = info->ptr;
		while (curr != -1) {
			unsigned char opcode = store[curr];
			bool has_p = Assembler::op_codes[opcode].has_p;
			auto q = get_val_at_addr<P5::addr_t>(store, curr + 1 + (int) has_p);
			put_val_to_addr(store, curr + 1 + has_p, lbl_val);
			curr = q;
		}
	}

	info->defined = true;
	info->ptr = lbl_val;

	return 0;
}

LabelTable::LabelInfo *LabelTable::find_or_insert_lbl_info(int lbl) {
	auto info_it = lb_storage.find(lbl);
	if (info_it == lb_storage.end()) {
		info_it = lb_storage.insert({lbl, LabelInfo{
				.defined = false,
				.ptr = -1}}).first;
	}
	return &info_it->second;
}
