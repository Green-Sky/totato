#include "./managment_commands.hpp"

#include <solanaceae/contact/contact_model3.hpp>
#include <solanaceae/util/config_model.hpp>

#include <solanaceae/message3/message_command_dispatcher.hpp>

#include <solanaceae/message3/components.hpp>
#include <solanaceae/contact/components.hpp>

#include <iostream>

void registerConfigCommands(
	MessageCommandDispatcher& mcd,
	ConfigModelI& conf,
	Contact3Registry& cr,
	RegistryMessageModelI& rmm
) {
	mcd.registerCommand(
		"Config", "conf",
		"set-string",
		[&](std::string_view params, Message3Handle m) -> bool {
			// x x x
			// 01234
			if (params.size() < 5) {
				return false;
			}

			const auto contact_from = m.get<Message::Components::ContactFrom>().c;

			// split params into:
			// - module
			// - category
			const auto first_space_pos = params.find_first_of(' ');

			// x x x
			// 01234
			if (first_space_pos >= int64_t(params.size())-2) {
				// TODO: error?
				return false;
			}

			const auto second_space_pos = params.find_first_of(' ', first_space_pos+1);
			if (second_space_pos >= params.size()-2) {
				// TODO: error?
				return false;
			}

			const auto fist_word = params.substr(0, first_space_pos);
			const auto second_word = params.substr(first_space_pos+1, second_space_pos);
			const auto rest_word = params.substr(second_space_pos+1);


			//conf.set("MessageCommandDispatcher", group, params, true);

			rmm.sendText(
				contact_from,
				"he"
			);
			return true;
		},
		"Set the config value for a category.",
		MessageCommandDispatcher::Perms::ADMIN // yes, always
	);
}

