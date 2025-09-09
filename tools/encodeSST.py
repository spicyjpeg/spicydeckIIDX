#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging, os, time
from argparse        import ArgumentParser, Namespace
from collections.abc import Iterable, Mapping, Sequence
from enum            import IntEnum
from io              import SEEK_SET
from multiprocessing import Pool
from pathlib         import Path
from struct          import Struct

import av
from audio        import NUM_CHANNELS, EncodingPipeline
from av.container import InputContainer
from util         import \
	StringBlobBuilder, findFilesWithExtensions, roundUpToMultiple, setupLogger

## .sst file header generation

class SSTKeyScale(IntEnum):
	SCALE_UNKNOWN = 0
	SCALE_MAJOR   = 1
	SCALE_MINOR   = 2

SST_HEADER_STRUCT:     Struct = Struct("< 4s 3I 12B 16h 452s")
SST_MAX_VARIANTS:      int    = 16
SST_MAX_BLOB_LENGTH:   int    = 452
SST_PITCH_OFFSET_UNIT: int    = 1 << 4

def normalizeMetadata(metadata: Mapping[str, str], defaultTitle: str = ""):
	# Depending on the input file's tag format, FFmpeg may return uppercase keys
	# for metadata fields.
	normalized: dict[str, str] = {
		key.lower(): value for key, value in metadata.items()
	}

	if "title" not in normalized:
		if " - " in defaultTitle:
			artist, title        = defaultTitle.split(" - ", 1)
			normalized["title"]  = title.strip()
			normalized["artist"] = artist.strip()
		else:
			normalized["title"]  = defaultTitle
			normalized["artist"] = ""

	if "/" in normalized.get("track", ""):
		trackNumber, trackCount = normalized["track"].split("/", 1)
		normalized["track"]       = trackNumber.strip()
		normalized["totaltracks"] = trackCount.strip()
	elif "tracktotal" in normalized:
		normalized["totaltracks"] = normalized["tracktotal"]
		del normalized["tracktotal"]

	if "/" in normalized.get("disc", ""):
		discNumber, discCount    = normalized["disc"].split("/", 1)
		normalized["disc"]       = discNumber.strip()
		normalized["totaldiscs"] = discCount.strip()
	elif "disctotal" in normalized:
		normalized["totaldiscs"] = normalized["disctotal"]
		del normalized["disctotal"]

	return normalized

def generateSSTHeader(
	metadata:       Mapping[str, str],
	sampleRate:     int,
	numChunks:      int,
	waveformLength: int,
	pitchOffsets:   Sequence[float],
	key:            tuple[str | None, int] = ( None, 0 )
) -> bytes:
	blob: StringBlobBuilder = StringBlobBuilder(4)

	titleOffset:  int = blob.addString(metadata.get("title",  ""))
	artistOffset: int = blob.addString(metadata.get("artist", ""))
	albumOffset:  int = blob.addString(metadata.get("album",  ""))
	genreOffset:  int = blob.addString(metadata.get("genre",  ""))

	if len(blob.data) > SST_MAX_BLOB_LENGTH:
		raise RuntimeError("string blob too large for header")

	keyScale, keyNote     = key
	keyScale: SSTKeyScale = {
		None:    SSTKeyScale.SCALE_UNKNOWN,
		"major": SSTKeyScale.SCALE_MAJOR,
		"minor": SSTKeyScale.SCALE_MINOR
	}[keyScale]

	pitchOffsetValues: list[int] = [ 0 ] * SST_MAX_VARIANTS

	for i, pitch in enumerate(pitchOffsets):
		pitchOffsetValues[i] = round(pitch * SST_PITCH_OFFSET_UNIT)

	return SST_HEADER_STRUCT.pack(
		b"SST1",
		sampleRate,
		numChunks,
		waveformLength,
		len(pitchOffsets),
		NUM_CHANNELS,
		titleOffset  // 2,
		artistOffset // 2,
		albumOffset  // 2,
		genreOffset  // 2,
		int(metadata.get("track",       "1")),
		int(metadata.get("totaltracks", "1")),
		int(metadata.get("disc",        "1")),
		int(metadata.get("totaldiscs",  "1")),
		keyScale,
		keyNote,
		*pitchOffsetValues,
		blob.data
	)

## .sst file encoding

def encodeFile(
	inputPath:    Path,
	outputPath:   Path,
	sampleRate:   int,
	pitchOffsets: Sequence[float]
):
	try:
		inputFile: InputContainer = av.open(inputPath, "r")
	except:
		logging.error(
			f"skipping {inputPath.name} (cannot open or decode as audio file)"
		)
		return

	if len(inputFile.streams.audio) != 1:
		logging.error(
			f"skipping {inputPath.name} (file has no audio or multiple audio "
			f"streams)"
		)
		return

	metadata: dict[str, str] = normalizeMetadata(
		inputFile.metadata,
		inputPath.stem
	)

	pipeline:  EncodingPipeline = EncodingPipeline(sampleRate, pitchOffsets)
	startTime: float            = time.time()

	with inputFile, open(outputPath, "wb") as outputFile:
		# Use a placeholder for the header, then overwrite it with the actual
		# header once the file has been encoded.
		outputFile.write(bytes(SST_HEADER_STRUCT.size))

		for frame in inputFile.decode(audio = 0):
			pipeline.feed(frame)
			pipeline.flush(outputFile)

		pipeline.feed(None)
		pipeline.flush(outputFile)

		waveformLength: int = len(pipeline.waveformData)
		paddedLength:   int = roundUpToMultiple(waveformLength, 512)

		outputFile.write(pipeline.waveformData.ljust(paddedLength, b"\0"))

		outputFile.seek(0, SEEK_SET)
		outputFile.write(generateSSTHeader(
			metadata,
			sampleRate,
			pipeline.chunksEncoded,
			waveformLength * 2,
			pitchOffsets,
			pipeline.estimateKey()
		))

	encodeTime: float = time.time() - startTime

	logging.info(
		f"converted {outputPath.name} ({encodeTime:.1f}s, "
		f"{pipeline.chunksEncoded / encodeTime:.1f} chunks/s)"
	)

## Main

DEFAULT_EXTENSIONS:    list[str]   = [ "wav", "flac", "mp3", "m4a", "ogg" ]
DEFAULT_PITCH_OFFSETS: list[float] = [ -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0 ]
DEFAULT_SAMPLE_RATE:   int         = 44100

def createParser() -> ArgumentParser:
	parser = ArgumentParser(
		description = \
			"Encodes one or more audio files into the .sst format required by "
			"spicydeckIIDX, optionally pre-rendering pitch-shifted variants of "
			"each track.",
		add_help    = False
	)

	group = parser.add_argument_group("Tool options")
	group.add_argument(
		"-h", "--help",
		action = "help",
		help   = "Show this help message and exit"
	)
	group.add_argument(
		"-v", "--verbose",
		action = "count",
		help   = "Enable additional logging levels"
	)
	group.add_argument(
		"-j", "--jobs",
		type    = lambda value: int(value, 0),
		default = 0,
		help    = \
			"Use up to the specified number of concurrent processes to encode "
			"multiple files at once (autodetected by default)",
		metavar = "num"
	)

	group = parser.add_argument_group("Input options")
	group.add_argument(
		"-e", "--extensions",
		type    = lambda value: value.split(","),
		default = DEFAULT_EXTENSIONS,
		help    = \
			f"List of file extensions to process when scanning directories "
			f"(default {",".join(DEFAULT_EXTENSIONS)})",
		metavar = "ext,ext,..."
	)
	group.add_argument(
		"-f", "--force",
		action = "store_true",
		help   = "Do not skip conversion if output file already exists"
	)

	group = parser.add_argument_group("Conversion options")
	group.add_argument(
		"-r", "--resample",
		type    = lambda value: int(value, 0),
		default = DEFAULT_SAMPLE_RATE,
		help    = \
			f"Encode output tracks at the specified sample rate (default "
			f"{DEFAULT_SAMPLE_RATE})",
		metavar = "rate"
	)
	group.add_argument(
		"-p", "--pitch-offsets",
		type    = lambda value: [ float(pitch) for pitch in value.split(",") ],
		default = DEFAULT_PITCH_OFFSETS,
		help    = \
			f"List of pitch offsets (in semitone units) to generate "
			f"pitch-shifted track variants for (default "
			f"{",".join(map(str, DEFAULT_PITCH_OFFSETS))})",
		metavar = "value,value,..."
	)

	group = parser.add_argument_group("File paths")
	group.add_argument(
		"-o", "--output",
		type    = Path,
		default = os.curdir,
		help    = "Path to output directory (current directory by default)",
		metavar = "dir"
	)
	group.add_argument(
		"input",
		type  = Path,
		nargs = "+",
		help  = \
			"Path to input audio files, and/or directories to scan recursively "
			"for audio files"
	)

	return parser

def main():
	parser: ArgumentParser = createParser()
	args:   Namespace      = parser.parse_args()
	setupLogger(args.verbose)

	if not args.extensions:
		parser.error("at least one input file extension must be specified")
	if not args.pitch_offsets:
		parser.error("at least one pitch offset must be specified")
	if len(args.pitch_offsets) > SST_MAX_VARIANTS:
		parser.error("too many pitch offsets specified")

	args.pitch_offsets.sort()

	# Gather all paths before spawning the encoding pool.
	calls: list[tuple[str, str, list[int], int]] = []

	for path in args.input:
		if path.is_dir():
			inputPaths: Iterable[Path] = \
				findFilesWithExtensions(path, args.extensions)
		elif path.is_file():
			inputPaths: Iterable[Path] = path,
		else:
			parser.error(f"{path} is not a file nor directory")

		for inputPath in inputPaths:
			outputPath: Path = Path(args.output, f"{inputPath.stem}.sst")

			if outputPath.is_file() and not args.force:
				logging.warning(
					f"skipping {inputPath.name} (.sst already exists, use -f "
					f"to overwrite)"
				)
			else:
				calls.append((
					inputPath,
					outputPath,
					args.resample,
					args.pitch_offsets
				))

	logging.info(f"converting {len(calls)} files")

	if not args.output.is_dir():
		args.output.mkdir(parents = True)

	with Pool(args.jobs or None) as pool:
		pool.starmap(encodeFile, calls)

if __name__ == "__main__":
	main()
