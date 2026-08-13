#pragma once

// fwd
class MessageCommandDispatcher;
class RegistryMessageModelI;

void registerStatusCommands(
	MessageCommandDispatcher& mcd,
	RegistryMessageModelI& rmm
);
