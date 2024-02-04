#pragma once

#include <solanaceae/message3/registry_message_model.hpp>

class MessageCleanser {
	Contact3Registry& _cr;
	RegistryMessageModel& _rmm;

	float _old_age{150.f*60.f}; // max 150min
	float _interval{3.f*60.f}; // every 3min
	float _timer{0.f};

	public:
		MessageCleanser(Contact3Registry& cr, RegistryMessageModel& rmm);
		~MessageCleanser(void);

		float iterate(float time_delta);
};
