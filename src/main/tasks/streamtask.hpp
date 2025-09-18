
#pragma once

#include <stdint.h>
#include "src/main/drivers/input.hpp"
#include "src/main/util/rtos.hpp"
#include "src/main/sst.hpp"

namespace tasks {

/* Main file streaming task */

enum StreamCommandType : uint8_t {
	STREAM_CMD_OPEN         = 0,
	STREAM_CMD_CLOSE        = 1,
	STREAM_CMD_PREV_VARIANT = 2,
	STREAM_CMD_NEXT_VARIANT = 3
};

struct StreamCommand {
public:
	uint8_t           deck;
	StreamCommandType cmd;
};

class StreamTask : public util::Task {
private:
	sst::Reader readers_[drivers::NUM_DECKS];

	util::Queue<StreamCommand> commandQueue_;

	inline StreamTask(void) :
		Task("StreamTask", 0x1000)
	{}

	[[noreturn]] void taskMain_(void) override;
	void handleCommand_(const StreamCommand &command);

public:
	inline void issueCommand(int deck, StreamCommandType cmd) {
		const StreamCommand command{
			.deck = uint8_t(deck),
			.cmd  = cmd
		};

		commandQueue_.push(command, true);
	}

	static StreamTask &instance(void);
};

}
