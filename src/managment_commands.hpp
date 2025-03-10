#pragma once

#include <solanaceae/contact/fwd.hpp>
#include <solanaceae/message3/registry_message_model.hpp>

// fwd
class MessageCommandDispatcher;
struct ConfigModelI;

void registerManagementCommands(
	MessageCommandDispatcher& mcd,
	ConfigModelI& conf,
	ContactStore4I& cs,
	RegistryMessageModelI& rmm
);

