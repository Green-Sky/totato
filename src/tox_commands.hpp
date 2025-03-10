#pragma once

#include <solanaceae/contact/fwd.hpp>
#include <solanaceae/message3/registry_message_model.hpp>

// fwd
class MessageCommandDispatcher;
struct ConfigModelI;
struct ToxI;
struct ToxPrivateI;

void registerToxCommands(
	MessageCommandDispatcher& mcd,
	ConfigModelI& conf,
	ContactStore4I& cs,
	RegistryMessageModelI& rmm,
	ToxI& t,
	ToxPrivateI& tp
);

