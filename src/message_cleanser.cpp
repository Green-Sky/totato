#include "./message_cleanser.hpp"

#include <solanaceae/contact/components.hpp>
#include <solanaceae/message3/components.hpp>
#include <solanaceae/util/utils.hpp>

#include <iostream>
#include <chrono>
#include <cstdint>

MessageCleanser::MessageCleanser(
	Contact3Registry& cr,
	RegistryMessageModel& rmm,
	ConfigModelI& conf
) : _cr(cr), _rmm(rmm), _conf(conf) {
	if (!_conf.has_int("MessageCleanser", "old_age_minutes")) {
		_conf.set("MessageCleanser", "old_age_minutes", int64_t(_old_age_default));
	}
}

MessageCleanser::~MessageCleanser(void) {
}

float MessageCleanser::iterate(float time_delta) {
	_timer += time_delta;
	if (_timer >= _interval) {
		_timer = 0.f;

		//std::cout << "MC: cleaning up old messages...\n";

		uint64_t now_ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

		// TODO: iterate rmm directly
		//_rmm.get();
		// workaround by iterating contacts
		for (const auto& c : _cr.view<Contact::Components::TagBig>()) {
			if (auto* reg = _rmm.get(c); reg != nullptr) {
				float old_age {0.f};
				{ // old age from conf
					// TODO: find some way to extract this (maybe map into contact reg?)
					if (const auto* id_comp = _cr.try_get<Contact::Components::ID>(c); id_comp != nullptr) {
						const auto id_hex = bin2hex(id_comp->data);
						old_age = _conf.get_int("MessageCleanser", "old_age_minutes", id_hex).value_or(_old_age_default);
					} else {
						old_age = _conf.get_int("MessageCleanser", "old_age_minutes").value_or(_old_age_default);
					}
					old_age *= 60.f; // to sec
				}

				std::vector<Message3> to_remove;
				// assuming all messages have Timestamp comp
				reg->view<Message::Components::Timestamp>().each([this, now_ts, old_age, &to_remove](const Message3 ent, const Message::Components::Timestamp& ts) {
					if (now_ts >= ts.ts + static_cast<uint64_t>(old_age*1000.f)) {
						to_remove.push_back(ent);
					}
				});

				//reg->destroy(to_remove.cbegin(), to_remove.cend());
				// we need to notify for every destruction, and give every listener a last chance
				for (const auto c : to_remove) {
					_rmm.throwEventDestroy(*reg, c);
					reg->destroy(c);
				}

				if (!to_remove.empty()) {
					std::cout << "MC: cleaned up " << to_remove.size() << ", age >= " << old_age << "sec\n";
				}
			}
		}
	}

	return _interval - _timer;
}
