#include "./managment_commands.hpp"

#include <solanaceae/contact/contact_store_i.hpp>
#include <solanaceae/util/config_model.hpp>

#include <solanaceae/message3/message_command_dispatcher.hpp>

#include <solanaceae/message3/components.hpp>
#include <solanaceae/contact/components.hpp>
#include <solanaceae/util/utils.hpp>

#include <iostream>
#include <algorithm>

static std::optional<Contact4> getContactFromIDStr(
	ContactStore4I& cs,
	std::string_view id_str
) {
	const std::vector<uint8_t> id = hex2bin(id_str);
	const auto found_contact = cs.getOneContactByID(ByteSpan{id});

	if (static_cast<bool>(found_contact)) {
		return found_contact;
	} else {
		return std::nullopt;
	}
}

static std::string getStatusFromContact(
	ContactHandle4 c
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
	ContactStore4I& cs,
	RegistryMessageModelI& rmm,

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

	const auto target_opt = getContactFromIDStr(cs, params);
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
	reply += getStatusFromContact(cs.contactHandle(target_opt.value()));

	rmm.sendText(
		contact_from,
		reply
	);

	return true;
}

void registerManagementCommands(
	MessageCommandDispatcher& mcd,
	ConfigModelI& conf,
	ContactStore4I& cs,
	RegistryMessageModelI& rmm
) {
	mcd.registerCommand(
		"Management", "manage",
		"admin-add",
		[&](std::string_view params, Message3Handle m) -> bool {
			return handleContactAddToGroup(conf, cs, rmm, params, m, "admin");
		},
		"Add an admin by ID.",
		MessageCommandDispatcher::Perms::ADMIN
	);

	mcd.registerCommand(
		"Management", "manage",
		"mod-add",
		[&](std::string_view params, Message3Handle m) -> bool {
			return handleContactAddToGroup(conf, cs, rmm, params, m, "moderator");
		},
		"Add a moderator by ID.",
		MessageCommandDispatcher::Perms::ADMIN
	);
}

