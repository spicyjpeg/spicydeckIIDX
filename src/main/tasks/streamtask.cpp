
#include "src/main/drivers/input.hpp"
#include "src/main/tasks/audiotask.hpp"
#include "src/main/tasks/streamtask.hpp"
#include "src/main/sst.hpp"

namespace tasks {

/* Main file streaming task */

static constexpr int CHUNK_INDEX_UNIT_ =
	sst::SAMPLE_OFFSET_UNIT * sst::SAMPLES_PER_SECTOR;

static int predictNextChunk_(
	const DeckState &state,
	int             numChunks,
	int             lookahead
) {
	int chunk = state.playbackOffset / CHUNK_INDEX_UNIT_;

	if (chunk >= numChunks)
		return -1;

	for (; lookahead > 0; lookahead--) {
		chunk++;
		int newOffset = chunk * CHUNK_INDEX_UNIT_;

		if (state.flags & DECK_FLAG_LOOPING) {
			while (newOffset >= state.loopEnd)
				newOffset -= state.loopEnd - state.loopStart;

			chunk = newOffset / CHUNK_INDEX_UNIT_;
		}

		// If the end of the track has been reached and looping is disabled,
		// stop buffering chunks.
		if (chunk >= numChunks)
			return -1;
	}

	return chunk;
}

[[noreturn]] void StreamTask::taskMain_(void) {
	auto &audioTask = AudioTask::instance();

	for (;;) {
		StreamCommand command;

		while (commandQueue_.pop(command))
			handleCommand_(command);

		for (int i = 0; i < drivers::NUM_DECKS; i++) {
			auto header = readers_[i].getHeader();

			if (!header)
				continue;

			// Predict which chunk is going to be played next by this deck,
			// taking into account the chunks that have been buffered into the
			// queue so far.
			DeckState state;

			audioTask.getDeckState(state, i);

			const int chunk = predictNextChunk_(
				state,
				header->info.numChunks,
				audioTask.getQueueLength(i)
			);

			if (chunk < 0)
				continue;

			auto entry = audioTask.feedSector(i);

			if (entry) {
				entry->chunk = chunk;
				readers_[i].read(entry->sector, chunk);
				audioTask.finalizeFeed(i);
			}
		}
	}
}

void StreamTask::handleCommand_(const StreamCommand &command) {
	auto &reader = readers_[command.deck];

	switch (command.cmd) {
		case STREAM_CMD_OPEN:
			reader.open(command.path);
			break;

		case STREAM_CMD_CLOSE:
			reader.close();
			break;

		case STREAM_CMD_PREV_VARIANT:
			reader.setVariant(reader.getVariant() - 1);
			break;

		case STREAM_CMD_NEXT_VARIANT:
			reader.setVariant(reader.getVariant() + 1);
			break;

		case STREAM_CMD_RESET_VARIANT:
			reader.resetVariant();
			break;
	}
}

StreamTask &StreamTask::instance(void) {
	static StreamTask task;

	return task;
}

}
