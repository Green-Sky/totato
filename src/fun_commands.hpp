#pragma once

#include <solanaceae/contact/contact_model3.hpp>
#include <solanaceae/message3/registry_message_model.hpp>

// fwd
class MessageCommandDispatcher;
struct ConfigModelI;

void registerFunCommands(
	MessageCommandDispatcher& mcd,
	ConfigModelI& conf,
	Contact3Registry& cr,
	RegistryMessageModel& rmm
);

