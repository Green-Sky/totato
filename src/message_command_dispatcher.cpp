#include "./message_command_dispatcher.hpp"

#include <solanaceae/util/config_model.hpp>

MessageCommandDispatcher::MessageCommandDispatcher(
	Contact3Registry& cr,
	RegistryMessageModel& rmm,
	ConfigModelI& conf
) :
	_cr(cr), _rmm(rmm), _conf(conf)
{
}

MessageCommandDispatcher::~MessageCommandDispatcher(void) {
}

void MessageCommandDispatcher::iterate(float time_delta) {
}

bool MessageCommandDispatcher::onEvent(const Message::Events::MessageConstruct& e) {
	return false;
}

bool MessageCommandDispatcher::onEvent(const Message::Events::MessageUpdated& e) {
	return false;
}

