#include "./message_command_dispatcher.hpp"

#include <solanaceae/util/config_model.hpp>
#include <solanaceae/message3/components.hpp>

#include <utility>
#include <iostream>

//MessageCommandDispatcher::Command::Command(Command&& other) :
	//m(std::move(other.m)),
	//m_prefix(std::move(other.m_prefix)),
	//command(std::move(other.command)),
	//fn(std::move(other.fn)),
	//help_text(std::move(other.help_text))
//{
	//// is this really needed?
//}

MessageCommandDispatcher::MessageCommandDispatcher(
	Contact3Registry& cr,
	RegistryMessageModel& rmm,
	ConfigModelI& conf
) :
	_cr(cr), _rmm(rmm), _conf(conf)
{
	_rmm.subscribe(this, RegistryMessageModel_Event::message_construct);

	{ // setup basic commands for bot
		registerCommand(
			"host", "",
			"help",
			[this](std::string_view params) -> bool {
				return helpCommand(params);
			},
			"Get help"
		);
	}
}

MessageCommandDispatcher::~MessageCommandDispatcher(void) {
}

void MessageCommandDispatcher::iterate(float time_delta) {
}

static std::string_view get_first_word(std::string_view text) {
	if (text.empty()) {
		return text;
	}

	// trim
	const auto pos_first_non_space = text.find_first_not_of(' ');
	if (pos_first_non_space == std::string_view::npos) {
		// only contains spaces o.o
		return ""; // should return text as is?
	}

	text = text.substr(pos_first_non_space);

	const auto pos_first_space = text.find_first_of(' ');
	if (pos_first_space == 0 || pos_first_space == std::string_view::npos) {
		// does not contain spaces
		// command is whole message
		return text;
	} else {
		return text.substr(0, pos_first_space);
	}
}

void MessageCommandDispatcher::registerCommand(
	std::string_view m, // module
	std::string_view m_prefix, // module prefix (if any)
	std::string_view command, // command
	std::function<bool(std::string_view params)>&& fn,
	std::string_view help_text
) {
	std::string full_command_string = std::string{m_prefix} + std::string{command};

	if (_command_map.count(full_command_string)) {
		std::cout << "MCD warning: overwriting existing '" << full_command_string << "'\n";
	}

	_command_map[full_command_string] = Command{
		std::string{m},
		std::string{m_prefix},
		std::string{command},
		std::move(fn),
		std::string{help_text}
	};
}

bool MessageCommandDispatcher::helpCommand(std::string_view params) {
	std::cout << "MCD: help got called '" << params << "'\n";
	return true;
}

bool MessageCommandDispatcher::onEvent(const Message::Events::MessageConstruct& e) {
	if (!e.e.all_of<Message::Components::MessageText, Message::Components::TagUnread>()) {
		std::cout << "MCD: got message that is not";

		if (!e.e.all_of<Message::Components::MessageText>()) {
			std::cout << " text";
		}
		if (!e.e.all_of<Message::Components::TagUnread>()) {
			std::cout << " unread";
		}

		std::cout << "\n";
		return false;
	}

	if (e.e.any_of<Message::Components::TagMessageIsAction>()) {
		std::cout << "MCD: got message that is";
		if (e.e.all_of<Message::Components::TagMessageIsAction>()) {
			std::cout << " action";
		}
		std::cout << "\n";
		return false;
	}

	std::string_view message_text = e.e.get<Message::Components::MessageText>().text;

	if (message_text.empty()) {
		// empty message?
		return false;
	}

	// TODO: skip unrelyable synced

	// TODO: is private

	// TODO: has the permissions


	if (false) { // is private
	} else {
		// check for command prefix
		if (
			message_text.at(0) != '!' &&
			message_text.at(0) != '/'
		) {
			// does not start with command prefix, not for us
			return false;
		}

		// remove c prefix
		message_text = message_text.substr(1);
	}

	if (message_text.empty()) {
		// empty message?
		std::cout << "MCD: got empty command\n";
		return false;
	}

	std::cout << "MCD: got command '" << message_text << "'\n";


	std::string_view first_word;
	std::string_view second_word;

	first_word = get_first_word(message_text);
	std::cout << "------- first_word:'" << first_word << "'\n";
	if (first_word.size() != message_text.size()) {
		second_word = get_first_word(
			message_text.substr(
				// TODO: optimize this
				message_text.find(first_word)
				+ first_word.size()
			)
		);
	}

	std::cout << "------- second_word:'" << second_word << "' empty:" << second_word.empty() << "\n";

	// first search first + space + second word
	if (!second_word.empty()) {
		std::string query {first_word};
		query += " ";
		query += second_word;

		const auto command_it = _command_map.find(query);
		if (command_it != _command_map.cend()) {
			command_it->second.fn(message_text);
			return true;
		}
	}

	// then seach first word only
	const auto command_it = _command_map.find(std::string{first_word});
	if (command_it != _command_map.cend()) {
		command_it->second.fn(message_text);
		return true;
	}

	return false;
}

bool MessageCommandDispatcher::onEvent(const Message::Events::MessageUpdated&) {
	// do i need this?
	return false;
}

