//
// Created by Anton on 06.01.2023.
//

#include "SetStorage.h"
#include "p5_errors.h"

P5::set_t SetStorage::create_set(std::unordered_set<P5::set_el_t> &&set) {
	struct set_info_t info;
	info.count = 0;
	info.set = std::make_shared<std::unordered_set<P5::set_el_t>>(set);
	storage[next_id++] = info;
	return next_id-1;
}

std::shared_ptr<std::unordered_set<P5::set_el_t>> SetStorage::get_set(P5::set_t id) {
	auto it = storage.find(id);
	if (it == storage.end()) {
		P5_ERR("can't find set with id %d", id);
	}
	return it->second.set;
}

P5::set_t SetStorage::notify_push(P5::set_t id) {
	if (id == 0) {
		std::unordered_set<P5::set_el_t> set;
		id = create_set(std::move(set));
		notify_push(id);
		return id;
	}
	auto it = storage.find(id);
	if (it == storage.end()) {
		P5_ERR("can't find set with id %d", id);
	}
	it->second.count++;
	return id;
}

void SetStorage::notify_pop(P5::set_t id) {
	auto it = storage.find(id);
	if (it == storage.end()) {
		P5_ERR("can't find set with id %d", id);
	}
	it->second.count--;
	if (it->second.count == 0) {
		//TODO: remove
		storage.erase(it);
	}
}
