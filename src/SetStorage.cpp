//
// Created by Anton on 06.01.2023.
//

#include "SetStorage.h"
#include "p5_errors.h"

int SetStorage::create_set(std::unordered_set<P5::set_el_t> &&set) {
	struct set_info_t info;
	info.count = 0;
	info.set = std::make_shared<std::unordered_set<P5::set_el_t>>(set);
	storage[next_id++] = info;
	return next_id-1;
}

std::shared_ptr<std::unordered_set<P5::set_el_t>> SetStorage::get_set(int id) {
	auto it = storage.find(id);
	if (it == storage.end()) {
		P5_ERR("can't find set with id %d", id);
	}
	return it->second.set;
}

void SetStorage::notify_push(int id) {
	auto it = storage.find(id);
	if (it == storage.end()) {
		P5_ERR("can't find set with id %d", id);
	}
	it->second.count++;
}

void SetStorage::notify_pop(int id) {
	auto it = storage.find(id);
	if (it == storage.end()) {
		P5_ERR("can't find set with id %d", id);
	}
	it->second.count--;
	if (it->second.count == 0) {
		storage.erase(it);
	}
}
