#pragma once

#include <solanaceae/message3/registry_message_model.hpp>

// fwd
class MessageCommandDispatcher;
struct ConfigModelI;

void registerFunCommands(
	MessageCommandDispatcher& mcd,
	ConfigModelI& conf,
	ContactStore4I& cs,
	RegistryMessageModelI& rmm
);

