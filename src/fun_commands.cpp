#include "./managment_commands.hpp"

//#include <solanaceae/util/config_model.hpp>
#include <solanaceae/contact/contact_store_i.hpp>

#include <solanaceae/message3/message_command_dispatcher.hpp>

//#include <solanaceae/contact/components.hpp>
#include <solanaceae/message3/components.hpp>

#include <random>

void registerFunCommands(
	MessageCommandDispatcher& mcd,
	ConfigModelI& conf,
	ContactStore4I& cs,
	RegistryMessageModelI& rmm
) {
	mcd.registerCommand(
		"fun", "fun",
		"dance",
		[&](std::string_view params, Message3Handle m) -> bool {
			static std::default_random_engine rng{std::random_device{}()};

			const auto contact_from = m.get<Message::Components::ContactFrom>().c;

			static const std::array<std::string_view, 12> dances {
				"DANCE",
				"(~^.^)~",
				"♪┌|∵|┘♪",
				"~( °٢° )~",
				"m/...(>.<)…m/",
				"(~‾⌣‾)~",
				"└[∵┌]└[ ∵ ]┘[┐∵]┘",
				"♨(⋆‿⋆)♨",
				"ヾ(*´∀｀*）ノ",
				"ლ(o◡oლ)",
				"┌（★ｏ☆）┘",
				"ヘ(^_^ヘ)",
			};
			rmm.sendText(
				contact_from,
				dances.at(rng()%dances.size())
			);
			return true;
		},
		"Make totato dance.",
		MessageCommandDispatcher::Perms::EVERYONE
	);
}

