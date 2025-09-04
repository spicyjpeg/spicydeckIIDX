#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
from argparse        import ArgumentParser, FileType, Namespace
from collections.abc import Mapping
from struct          import Struct
from typing          import Any

from PIL  import Image
from util import \
	HashTableBuilder, StringBlobBuilder, arrayDumpToFile, isPowerOf2

## Glyph conversion

SFT_GLYPH_STRUCT: Struct = Struct("< 2B")
BYTES_PER_COLUMN: int    = 2

LUMA_THRESHOLD:  int = 0x80
ALPHA_THRESHOLD: int = 0x80

def convertGlyph(
	image:           Image.Image,
	ignoreAlpha:     bool = False,
	lightBackground: bool = False
) -> bytes:
	# Glyph bitmaps are stored as a series of 16-bit bitfields, each
	# representing one column of pixels (with the LSB being the top pixel). The
	# bitmap is prefixed with the glyph's dimensions.
	glyph: bytearray = bytearray()
	glyph           += SFT_GLYPH_STRUCT.pack(image.width, image.height)

	for x in range(image.width):
		column: int = 0

		for y in range(image.height):
			luma, alpha = image.getpixel(( x, y ))

			if (not ignoreAlpha)     and (alpha < ALPHA_THRESHOLD):
				continue
			if (not lightBackground) and (luma  < LUMA_THRESHOLD):
				continue
			if lightBackground       and (luma  > LUMA_THRESHOLD):
				continue

			column |= 1 << y

		glyph += column.to_bytes(BYTES_PER_COLUMN, "little")

	return bytes(glyph)

## .sft file generation

SFT_HEADER_STRUCT: Struct = Struct("< 4s 3B b 2H")
SFT_ENTRY_STRUCT:  Struct = Struct("< I 2H")

def generateSFT(
	image:           Image.Image,
	metrics:         Mapping[str, Any],
	numBuckets:      int  = 256,
	ignoreAlpha:     bool = False,
	lightBackground: bool = False
) -> bytes:
	spaceWidth:     int = int(metrics["spaceWidth"])
	tabWidth:       int = int(metrics["tabWidth"])
	lineHeight:     int = int(metrics["lineHeight"])
	baselineOffset: int = int(metrics["baselineOffset"])

	if lineHeight > (BYTES_PER_COLUMN * 8):
		raise ValueError("line height exceeds maximum allowed glyph height")

	hashTable: HashTableBuilder  = HashTableBuilder(numBuckets)
	blob:      StringBlobBuilder = StringBlobBuilder(2)

	for ch, entry in metrics["characterSizes"].items():
		x: int = int(entry["x"])
		y: int = int(entry["y"])
		w: int = int(entry["width"])
		h: int = int(entry["height"])

		if h > lineHeight:
			raise ValueError("character height exceeds line height")

		glyph: bytes = convertGlyph(
			image.crop(( x, y, x + w, y + h )),
			ignoreAlpha,
			lightBackground
		)

		hashTable.addEntry(ord(ch), blob.addByteString(glyph))

	blobOffset: int = (0
		+ SFT_HEADER_STRUCT.size
		+ SFT_ENTRY_STRUCT.size * len(hashTable.entries)
	)

	sft: bytearray = bytearray()
	sft           += SFT_HEADER_STRUCT.pack(
		b"SFT1",
		spaceWidth,
		tabWidth,
		lineHeight,
		baselineOffset,
		numBuckets,
		len(hashTable.entries)
	)

	for entry in hashTable.entries:
		if entry is None:
			sft += bytes(SFT_ENTRY_STRUCT.size)
		else:
			sft += SFT_ENTRY_STRUCT.pack(
				entry.fullHash,
				blobOffset + entry.data,
				entry.chainIndex
			)

	sft += blob.data
	return bytes(sft)

## Main

DEFAULT_NUM_BUCKETS: int = 256

def createParser() -> ArgumentParser:
	parser = ArgumentParser(
		description = \
			"Converts a monochrome font spritesheet and a JSON file containing "
			"its metrics into the .sft format required by spicydeckIIDX.",
		add_help    = False
	)

	group = parser.add_argument_group("Tool options")
	group.add_argument(
		"-h", "--help",
		action = "help",
		help   = "Show this help message and exit"
	)

	group = parser.add_argument_group("Conversion options")
	group.add_argument(
		"-b", "--buckets",
		type    = lambda value: int(value, 0),
		default = DEFAULT_NUM_BUCKETS,
		help    = \
			f"Use specified bucket count for generated hash table (must be a "
			f"power of 2, default {DEFAULT_NUM_BUCKETS})",
		metavar = "count"
	)
	group.add_argument(
		"-i", "--ignore-alpha",
		action = "store_true",
		help   = \
			"Ignore input image's alpha channel and assume all pixels are solid"
	)
	group.add_argument(
		"-l", "--light-background",
		action = "store_true",
		help   = \
			"Assume input image has dark glyphs on a light background (default "
			"is to assume light glyphs on a dark background)"
	)

	group = parser.add_argument_group("File paths")
	group.add_argument(
		"-a", "--array",
		type    = FileType("wt"),
		help    = \
			"Generate a text copy of the font that can be included from C/C++ "
			"code in addition to the binary file",
		metavar = "file"
	)
	group.add_argument(
		"image",
		type = Image.open,
		help = "Path to input image file"
	)
	group.add_argument(
		"metrics",
		type = FileType("rt", encoding = "utf-8"),
		help = "Path to JSON file containing font metrics"
	)
	group.add_argument(
		"output",
		type = FileType("wb"),
		help = "Path to output file"
	)

	return parser

def main():
	parser: ArgumentParser = createParser()
	args:   Namespace      = parser.parse_args()

	if not isPowerOf2(args.buckets):
		parser.error("bucket count must be a power of 2")

	with args.image, args.metrics:
		image:   Image.Image    = args.image.convert("LA")
		metrics: dict[str, Any] = json.load(args.metrics)

	sft: bytes = generateSFT(
		image,
		metrics,
		args.buckets,
		args.ignore_alpha,
		args.light_background
	)

	with args.output:
		args.output.write(sft)

	if args.array is not None:
		with args.array:
			arrayDumpToFile(sft, args.array)

if __name__ == "__main__":
	main()
