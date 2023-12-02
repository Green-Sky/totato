#include "./message_cleanser.hpp"

#include <solanaceae/contact/components.hpp>
#include <solanaceae/message3/components.hpp>

#include <iostream>
#include <chrono>

MessageCleanser::MessageCleanser(Contact3Registry& cr, RegistryMessageModel& rmm) : _cr(cr), _rmm(rmm) {
}

MessageCleanser::~MessageCleanser(void) {
}

void MessageCleanser::iterate(float time_delta) {
	_timer += time_delta;
	if (_timer >= _interval) {
		_timer = 0.f;

		//std::cout << "MC: cleaning up old messages...\n";

		uint64_t now_ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

		uint64_t deleted_count{0};

		// TODO: iterate rmm directly
		//_rmm.get();
		// workaround by iterating contacts
		for (const auto& c : _cr.view<Contact::Components::TagBig>()) {
			if (auto* reg = _rmm.get(c); reg != nullptr) {
				std::vector<Message3> to_remove;
				// assuming all messages have Timestamp comp
				reg->view<Message::Components::Timestamp>().each([this, now_ts, &to_remove](const Message3 ent, const Message::Components::Timestamp& ts) {
					if (now_ts >= ts.ts + static_cast<uint64_t>(_old_age*1000.f)) {
						to_remove.push_back(ent);
					}
				});

				reg->destroy(to_remove.cbegin(), to_remove.cend());
				deleted_count += to_remove.size();
			}
		}

		if (deleted_count > 0) {
			std::cout << "MC: cleaned up " << deleted_count << "\n";
		}
	}
}
