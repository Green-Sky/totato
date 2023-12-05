#include "./managment_commands.hpp"

#include <solanaceae/contact/contact_model3.hpp>
#include <solanaceae/util/config_model.hpp>

#include "./message_command_dispatcher.hpp"

#include <solanaceae/message3/components.hpp>
#include <solanaceae/contact/components.hpp>
#include <solanaceae/toxcore/utils.hpp>

#include <iostream>
#include <algorithm>

static std::optional<Contact3> getContactFromIDStr(
	Contact3Registry& cr,
	std::string_view id_str
) {
	const std::vector<uint8_t> id = hex2bin(std::string{id_str});
	const auto view = cr.view<Contact::Components::ID>();
	const auto found_contact = std::find_if(view.begin(), view.end(), [&id, &view](const Contact3 c) -> bool {
		return view.get<Contact::Components::ID>(c).data == id;
	});

	if (found_contact != view.end()) {
		return *found_contact;
	} else {
		return std::nullopt;
	}
}

static std::string getStatusFromContact(
	Contact3Handle c
) {
	std::string status_str;

	// name
	if (c.all_of<Contact::Components::Name>()) {
		status_str += "  name: " + c.get<Contact::Components::Name>().name;
	} else {
		status_str += "  name: not found";
	}

	// connection state
	if (c.all_of<Contact::Components::ConnectionState>()) {
		status_str += "\n  connection: " + std::to_string(c.get<Contact::Components::ConnectionState>().state);
	} else {
		status_str += "\n  connection: not found";
	}

	return status_str;
}

bool handleContactAddToGroup(
	ConfigModelI& conf,
	Contact3Registry& cr,
	RegistryMessageModel& rmm,

	std::string_view params,
	Message3Handle m,

	const std::string_view group
) {
	const auto contact_from = m.get<Message::Components::ContactFrom>().c;

	// TODO: move parameter parsing to mcd
	if (params.empty()) {
		rmm.sendText(
			contact_from,
			std::string{"error: command requires the ID for the contact to promote to "} + std::string{group}
		);

		return true;
	}

	const auto target_opt = getContactFromIDStr(cr, params);
	if (!target_opt.has_value()) {
		rmm.sendText(
			contact_from,
			"error: unknown contact\n"
			"For practicality reasons the contact has to be observed once, before it can be added."
		);

		return true;
	}

	std::string reply;

	if (conf.get_bool("MessageCommandDispatcher", group, params).value_or(false)) {
		reply += "warning: contact already ";
		reply += group;
		reply += "!\n";
	}

	conf.set("MessageCommandDispatcher", group, params, true);

	reply += "Added '";
	reply += params;
	reply += "' to the ";
	reply += group;
	reply += "s.\n";
	reply += getStatusFromContact(Contact3Handle{cr, target_opt.value()});

	rmm.sendText(
		contact_from,
		reply
	);

	return true;
}

void registerManagementCommands(
	MessageCommandDispatcher& mcd,
	ConfigModelI& conf,
	Contact3Registry& cr,
	RegistryMessageModel& rmm
) {
	mcd.registerCommand(
		"Management", "manage",
		"admin-add",
		[&](std::string_view params, Message3Handle m) -> bool {
			return handleContactAddToGroup(conf, cr, rmm, params, m, "admin");
		},
		"Add an admin by ID.",
		MessageCommandDispatcher::Perms::ADMIN
	);

	mcd.registerCommand(
		"Management", "manage",
		"mod-add",
		[&](std::string_view params, Message3Handle m) -> bool {
			return handleContactAddToGroup(conf, cr, rmm, params, m, "moderator");
		},
		"Add a moderator by ID.",
		MessageCommandDispatcher::Perms::ADMIN
	);
}

