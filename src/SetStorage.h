//
// Created by Anton on 06.01.2023.
//

#ifndef P5_INTERPRETER_SETSTORAGE_H
#define P5_INTERPRETER_SETSTORAGE_H

#include <unordered_set>
#include <unordered_map>
#include <memory>
#include "p5_common.h"

class SetStorage {
public:
//	class Set {
//	public:
//		int id;
//	private:
//		std::unordered_set<P5::set_el_t> set;
//		explicit Set(SetStorage *storage, int id) {
//			this->setStorage = storage;
//			this->id = id;
//		}
//		~Set() {
//
//		}
//	};

	int create_set(std::unordered_set<P5::set_el_t> &&set);
	std::shared_ptr<std::unordered_set<P5::set_el_t>> get_set(int id);
	void notify_push(int id);
	void notify_pop(int id);

private:
	int next_id = 0;
	struct set_info_t {
		std::shared_ptr<std::unordered_set<P5::set_el_t>> set;
		int count;
	};
	std::unordered_map<int, struct set_info_t> storage;
};


#endif //P5_INTERPRETER_SETSTORAGE_H
