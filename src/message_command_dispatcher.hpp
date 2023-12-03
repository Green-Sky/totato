#pragma once

#include <solanaceae/message3/registry_message_model.hpp>

// fwd
struct ConfigModelI;

class MessageCommandDispatcher : public RegistryMessageModelEventI {
	Contact3Registry& _cr;
	RegistryMessageModel& _rmm;
	ConfigModelI& _conf;

	struct Command {
		std::string m; // module
		std::string m_prefix; // module prefix (if any)
		std::string command; // command
		std::function<bool(std::string_view params, Message3Handle m)> fn;
		std::string help_text;

		//Command(const Command&) = delete;
	};
	std::unordered_map<std::string, Command> _command_map;

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
			std::string_view m, // module
			std::string_view m_prefix, // module prefix (if any)
			std::string_view command, // command
			std::function<bool(std::string_view params, Message3Handle m)>&& fn,
			std::string_view help_text
		);

		// generates a help
		bool helpCommand(std::string_view params, Message3Handle m);

	protected: // mm
		bool onEvent(const Message::Events::MessageConstruct& e) override;
		bool onEvent(const Message::Events::MessageUpdated& e) override;
};

