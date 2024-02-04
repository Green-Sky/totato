#pragma once

#include <solanaceae/message3/registry_message_model.hpp>
#include <solanaceae/util/config_model.hpp>

class MessageCleanser {
	Contact3Registry& _cr;
	RegistryMessageModel& _rmm;
	ConfigModelI& _conf;

	static constexpr int64_t _old_age_default{150}; // minutes

	float _interval{3.f*60.f}; // every 3min
	float _timer{0.f};

	public:
		MessageCleanser(
			Contact3Registry& cr,
			RegistryMessageModel& rmm,
			ConfigModelI& conf
		);
		~MessageCleanser(void);

		float iterate(float time_delta);
};
