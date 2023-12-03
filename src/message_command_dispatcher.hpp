#pragma once

#include <solanaceae/message3/registry_message_model.hpp>

// fwd
struct ConfigModelI;

class MessageCommandDispatcher : public RegistryMessageModelEventI {
	Contact3Registry& _cr;
	RegistryMessageModel& _rmm;
	ConfigModelI& _conf;

	public:
		MessageCommandDispatcher(Contact3Registry& cr, RegistryMessageModel& rmm, ConfigModelI& conf);
		~MessageCommandDispatcher(void);

		void iterate(float time_delta); // do we?

		// module command/command <param type(s)>
		// permissions?
		//  - user(s)
		//  - group(s)
		//  - everyone else?
		// callable
		// help text?
		void registerCommand(
			std::string_view module,
			std::string_view command
		);

	protected: // mm
		bool onEvent(const Message::Events::MessageConstruct& e) override;
		bool onEvent(const Message::Events::MessageUpdated& e) override;
};

