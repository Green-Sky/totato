#include "./message_command_dispatcher.hpp"
#include "nlohmann/detail/input/position_t.hpp"
#include "solanaceae/message3/registry_message_model.hpp"

#include <cwchar>
#include <solanaceae/util/config_model.hpp>
#include <solanaceae/message3/components.hpp>

#include <string_view>
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
			[this](std::string_view params, Message3Handle m) -> bool {
				return helpCommand(params, m);
			},
			"Get help"
		);
	}
}

MessageCommandDispatcher::~MessageCommandDispatcher(void) {
}

void MessageCommandDispatcher::iterate(float time_delta) {
}

static std::string_view get_first_word(std::string_view text, std::string_view::size_type& out_next) {
	if (text.empty()) {
		out_next = std::string_view::npos;
		return text;
	}

	// trim
	const auto pos_first_non_space = text.find_first_not_of(' ');
	if (pos_first_non_space == std::string_view::npos) {
		// only contains spaces o.o
		out_next = std::string_view::npos;
		return "";
	}

	text = text.substr(pos_first_non_space);
	out_next += pos_first_non_space;

	const auto pos_first_space = text.find_first_of(' ');
	if (pos_first_space == 0 || pos_first_space == std::string_view::npos) {
		// does not contain spaces
		// command is whole message
		out_next = std::string_view::npos;
		return text;
	} else {
		out_next += pos_first_space;
		return text.substr(0, pos_first_space);
	}
}

void MessageCommandDispatcher::registerCommand(
	std::string_view m, // module
	std::string_view m_prefix, // module prefix (if any)
	std::string_view command, // command
	std::function<bool(std::string_view params, Message3Handle m)>&& fn,
	std::string_view help_text
) {
	std::string full_command_string = (m_prefix.empty() ? "" : std::string{m_prefix} + " ") + std::string{command};

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

bool MessageCommandDispatcher::helpCommand(std::string_view params, Message3Handle m) {
	std::cout << "MCD: help got called '" << params << "'\n";

	_rmm.sendText(
		m.get<Message::Components::ContactFrom>().c,
		"I am still missing :), ping green for how it actually works."
	);

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
	std::string_view::size_type pos_next = 0;

	first_word = get_first_word(message_text, pos_next);
	std::cout << "------- first_word:'" << first_word << "' pos_next:" << pos_next << "\n";
	if (first_word.size() != message_text.size()) {
		second_word = get_first_word(
			message_text.substr(pos_next),
			pos_next
		);
	}

	std::cout << "------- second_word:'" << second_word << "' empty:" << second_word.empty() << " pos_next:" << pos_next << "\n";

	std::string params;
	if (pos_next != std::string_view::npos && message_text.size() > pos_next+1) {
		auto tmp_params = message_text.substr(pos_next);

		const auto params_pos_first_non_space = tmp_params.find_first_not_of(' ');
		if (params_pos_first_non_space == std::string_view::npos) {
			tmp_params = {};
		} else if (params_pos_first_non_space != 0) {
			// trim leading whitespace
			tmp_params = tmp_params.substr(params_pos_first_non_space);
		}

		params = tmp_params;

		std::cout << "------- params:'" << params << "'\n";
	}

	// first search first + space + second word
	if (!second_word.empty()) {
		std::string query {first_word};
		query += " ";
		query += second_word;

		const auto command_it = _command_map.find(query);
		if (command_it != _command_map.cend()) {
			command_it->second.fn(params, e.e);
			return true;
		}
	}

	// then seach first word only
	const auto command_it = _command_map.find(std::string{first_word});
	if (command_it != _command_map.cend()) {
		params = std::string{second_word} + " " + params;
		command_it->second.fn(params, e.e);
		return true;
	}

	return false;
}

bool MessageCommandDispatcher::onEvent(const Message::Events::MessageUpdated&) {
	// do i need this?
	return false;
}

