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

	P5::set_t create_set(std::unordered_set<P5::set_el_t> &&set);
	std::shared_ptr<std::unordered_set<P5::set_el_t>> get_set(P5::set_t id);
	P5::set_t notify_push(P5::set_t id);
	void notify_pop(P5::set_t id);
	P5::set_t get_id(P5::set_t id);
private:
	P5::set_t next_id = 1;
	struct set_info_t {
		std::shared_ptr<std::unordered_set<P5::set_el_t>> set;
		int count;
	};
	std::unordered_map<P5::set_t, struct set_info_t> storage;
};


#endif //P5_INTERPRETER_SETSTORAGE_H
