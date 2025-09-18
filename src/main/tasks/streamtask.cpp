
#include "src/main/tasks/audiotask.hpp"
#include "src/main/tasks/streamtask.hpp"

namespace tasks {

/* Main file streaming task */

[[noreturn]] void StreamTask::taskMain_(void) {
	auto &audioTask = AudioTask::instance();

	for (;;) {
		StreamCommand command;

		while (commandQueue_.pop(command))
			handleCommand_(command);

		// TODO: implement
	}
}

void StreamTask::handleCommand_(const StreamCommand &command) {
	// TODO: implement
}

StreamTask &StreamTask::instance(void) {
	static StreamTask task;

	return task;
}

}
