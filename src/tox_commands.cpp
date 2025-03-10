#include "./tox_commands.hpp"

#include <solanaceae/contact/contact_store_i.hpp>
#include <solanaceae/util/config_model.hpp>
#include <solanaceae/util/utils.hpp>
#include <solanaceae/toxcore/tox_interface.hpp>
#include <solanaceae/toxcore/tox_private_interface.hpp>

#include <solanaceae/message3/message_command_dispatcher.hpp>

#include <solanaceae/message3/components.hpp>
#include <solanaceae/contact/components.hpp>
#include <solanaceae/tox_contacts/components.hpp>

#include <iostream>

void registerToxCommands(
	MessageCommandDispatcher& mcd,
	ConfigModelI& conf,
	ContactStore4I& cs,
	RegistryMessageModelI& rmm,
	ToxI& t,
	ToxPrivateI& tp
) {
	mcd.registerCommand(
		"tox", "tox",
		"status",
		[&](std::string_view, Message3Handle m) -> bool {
			const auto tox_self_status = t.toxSelfGetConnectionStatus();

			const auto contact_from = m.get<Message::Components::ContactFrom>().c;

			std::string reply{"dht:"};

			if (tox_self_status == Tox_Connection::TOX_CONNECTION_UDP) {
				reply += "upd-direct";
			} else if (tox_self_status == Tox_Connection::TOX_CONNECTION_TCP) {
				reply += "tcp-relayed";
			}

			reply += "\ndht-closenum:";
			reply += std::to_string(tp.toxDHTGetNumCloselist());
			reply += "\ndht-closenum-announce-capable:";
			reply += std::to_string(tp.toxDHTGetNumCloselistAnnounceCapable());

			const auto& cr = cs.registry();

			if (cr.all_of<Contact::Components::ToxFriendEphemeral>(contact_from)) {
				const auto con_opt = t.toxFriendGetConnectionStatus(cr.get<Contact::Components::ToxFriendEphemeral>(contact_from).friend_number);
				if (!con_opt.has_value() || con_opt.value() == Tox_Connection::TOX_CONNECTION_NONE) {
					reply += "\nfriend:offline";
				} else if (con_opt.value() == Tox_Connection::TOX_CONNECTION_UDP) {
					reply += "\nfriend:udp-direct";
				} else if (con_opt.value() == Tox_Connection::TOX_CONNECTION_TCP) {
					reply += "\nfriend:tcp-relayed";
				}
			} else if (cr.all_of<Contact::Components::ToxGroupPeerEphemeral>(contact_from)) {
				const auto [group_number, peer_number] = cr.get<Contact::Components::ToxGroupPeerEphemeral>(contact_from);

				const auto [con_opt, _] = t.toxGroupPeerGetConnectionStatus(group_number, peer_number);
				if (!con_opt.has_value() || con_opt.value() == Tox_Connection::TOX_CONNECTION_NONE) {
					reply += "\ngroup-peer:offline";
				} else if (con_opt.value() == Tox_Connection::TOX_CONNECTION_UDP) {
					reply += "\ngroup-peer:udp-direct";
				} else if (con_opt.value() == Tox_Connection::TOX_CONNECTION_TCP) {
					reply += "\ngroup-peer:tcp-relayed";
				}
			} else if (cr.any_of<Contact::Components::ToxFriendPersistent, Contact::Components::ToxGroupPeerPersistent>(contact_from)) {
				reply += "\noffline";
			} else {
				reply += "\nunk";
			}

			rmm.sendText(
				contact_from,
				reply
			);

			return true;
		},
		"Query the tox status of dht and to you.",
		MessageCommandDispatcher::Perms::EVERYONE
	);

	mcd.registerCommand(
		"tox", "tox",
		"add",
		[&](std::string_view params, Message3Handle m) -> bool {
			const auto contact_from = m.get<Message::Components::ContactFrom>().c;

			if (params.size() != 38*2) {
				rmm.sendText(
					contact_from,
					"error adding friend, id is not the correct size!"
				);
				return true;
			}

			// TODO: add tcm interface
			const auto [_, err] = t.toxFriendAdd(hex2bin(std::string{params}), "Add me, I am totato");

			if (err == Tox_Err_Friend_Add::TOX_ERR_FRIEND_ADD_OK) {
				rmm.sendText(
					contact_from,
					"freind request sent"
				);
			} else {
				rmm.sendText(
					contact_from,
					"error adding friend, error code " + std::to_string(err)
				);
			}

			return true;
		},
		"add a tox friend by id"
	);

	mcd.registerCommand(
		"tox", "tox",
		"join",
		[&](std::string_view params, Message3Handle m) -> bool {
			const auto contact_from = m.get<Message::Components::ContactFrom>().c;

			if (params.size() != 32*2) {
				rmm.sendText(
					contact_from,
					"error joining group, id is not the correct size!"
				);
				return true;
			}

			auto name_opt = conf.get_string("tox", "name");

			// TODO: add tcm interface
			const auto [_, err] = t.toxGroupJoin(hex2bin(std::string{params}), std::string{name_opt.value_or("no-name-found")}, "");
			if (err == Tox_Err_Group_Join::TOX_ERR_GROUP_JOIN_OK) {
				rmm.sendText(
					contact_from,
					"joining group..."
				);
			} else {
				rmm.sendText(
					contact_from,
					"error joining group, error code " + std::to_string(err)
				);
			}

			return true;
		},
		"join a tox group by id"
	);

	mcd.registerCommand(
		"tox", "tox",
		"invite",
		[&](std::string_view params, Message3Handle m) -> bool {
			const auto contact_from = m.get<Message::Components::ContactFrom>().c;

			// trim friend extra stuff
			if (params.size() < 32*2) {
				rmm.sendText(
					contact_from,
					"error inviting, friend id is not the correct size!"
				);
				return true;
			}
			const auto friend_id = hex2bin(std::string{params.substr(0, 32*2)});

			auto [friend_number_opt, err] = t.toxFriendByPublicKey(friend_id);
			if (!friend_number_opt.has_value()) {
				rmm.sendText(
					contact_from,
					"error inviting, friend not found!"
				);
				return true;
			}

			const auto& cr = cs.registry();

			// get current group
			if (!cr.all_of<Contact::Components::Parent>(contact_from)) {
				rmm.sendText(
					contact_from,
					"error inviting, not sent from group!"
				);
				return true;
			}
			const auto contact_group = cr.get<Contact::Components::Parent>(contact_from).parent;

			if (!cr.all_of<Contact::Components::ToxGroupEphemeral>(contact_from)) {
				rmm.sendText(
					contact_from,
					"error inviting, group not connected (what?!?)!"
				);
				return true;
			}

			const auto inv_err = t.toxGroupInviteFriend(
				cr.get<Contact::Components::ToxGroupEphemeral>(contact_group).group_number,
				friend_number_opt.value()
			);
			if (inv_err == Tox_Err_Group_Invite_Friend::TOX_ERR_GROUP_INVITE_FRIEND_OK) {
				rmm.sendText(
					contact_from,
					"invite sent..."
				);
			} else {
				rmm.sendText(
					contact_from,
					"error inviting to group, error code " + std::to_string(inv_err)
				);
			}

			return true;
		},
		"invite a tox friend to the same tox group"
	);
}

