#include "./status_commands.hpp"

#include <solanaceae/message3/components.hpp>
#include <solanaceae/message3/message_command_dispatcher.hpp>

#include <chrono>
#include <cstdint>
#include <string>
#include <ratio>

void registerStatusCommands(
	MessageCommandDispatcher& mcd,
	RegistryMessageModelI& rmm
) {
	const auto started_at = std::chrono::steady_clock::now();

	mcd.registerCommand(
		"status", "bot",
		"uptime",
		[&, started_at](std::string_view, Message3Handle m) -> bool {
			const auto contact_from = m.get<Message::Components::ContactFrom>().c;

			const auto uptime = std::chrono::steady_clock::now() - started_at;

			const auto days = std::chrono::duration_cast<std::chrono::duration<int64_t, std::ratio<86400>>>(uptime);
			const auto hours = std::chrono::duration_cast<std::chrono::hours>(uptime) - std::chrono::duration_cast<std::chrono::hours>(days);
			const auto minutes = (std::chrono::duration_cast<std::chrono::minutes>(uptime) - std::chrono::duration_cast<std::chrono::minutes>(days)) - std::chrono::duration_cast<std::chrono::minutes>(hours);
			const auto seconds = ((std::chrono::duration_cast<std::chrono::seconds>(uptime) - std::chrono::duration_cast<std::chrono::seconds>(days)) - std::chrono::duration_cast<std::chrono::seconds>(hours)) - std::chrono::duration_cast<std::chrono::seconds>(minutes);

			std::string reply_text;
			reply_text += "totato uptime: ";
			reply_text += std::to_string(days.count());
			reply_text += "d ";
			reply_text += std::to_string(hours.count());
			reply_text += "h ";
			reply_text += std::to_string(minutes.count());
			reply_text += "min ";
			reply_text += std::to_string(seconds.count());
			reply_text += "s (";
			reply_text += std::to_string(std::chrono::duration_cast<std::chrono::seconds>(uptime).count());
			reply_text += "s)";

			rmm.sendText(
				contact_from,
				reply_text
			);
			return true;
		},
		"get current uptime.",
		MessageCommandDispatcher::Perms::EVERYONE
	);
}
