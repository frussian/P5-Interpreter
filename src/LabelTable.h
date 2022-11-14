//
// Created by Anton on 13.11.2022.
//

#ifndef P5_INTERPRETER_LABELTABLE_H
#define P5_INTERPRETER_LABELTABLE_H


#include <map>
#include "p5_common.h"

class LabelTable {
public:
	explicit LabelTable(P5::store_t &store): store(store){};
	P5::addr_t lookup(int lbl, int pc);
	int update(int lbl, P5::addr_t lbl_val);

private:
	typedef struct LabelInfo {
		bool defined;
		P5::addr_t ptr;
	} LabelInfo;
	std::map<int, LabelInfo> lb_storage;
	P5::store_t &store;

	LabelInfo *find_or_insert_lbl_info(int lbl);
};


#endif //P5_INTERPRETER_LABELTABLE_H
