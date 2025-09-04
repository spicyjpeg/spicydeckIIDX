# -*- coding: utf-8 -*-

import logging, math, os, re
from collections.abc import Generator, Sequence
from dataclasses     import dataclass
from pathlib         import Path
from typing          import Any, TextIO

## Value manipulation

def roundUpToMultiple(value: int, length: int) -> int:
	diff: int = value % length

	return (value - diff + length) if diff else value

def byteSwap(value: int, byteLength: int) -> int:
	return int.from_bytes(value.to_bytes(byteLength, "big"), "little")

def encodeSigned(value: int, bitLength: int) -> int:
	valueMask: int = (1 << bitLength) - 1

	return value & valueMask

def decodeSigned(value: int, bitLength: int) -> int:
	signMask:  int = 1 << (bitLength - 1)
	valueMask: int = signMask - 1

	return (value & valueMask) - (value & signMask)

def isPowerOf2(value: int) -> bool:
	if value <= 0:
		return False

	return value == (1 << int(math.log2(value)))

## String manipulation

_NAME_INVALID_REGEX: re.Pattern = re.compile(r"[^0-9A-Z+,-._]", re.IGNORECASE)

def toPrintableChar(value: int) -> str:
	if (value < 0x20) or (value > 0x7e):
		return "."

	return chr(value)

def hexdumpToFile(
	data:   Sequence[int],
	output: TextIO,
	width:  int = 16,
	indent: str = ""
):
	for i in range(0, len(data), width):
		hexBytes: map[str] = map(lambda value: f"{value:02x}", data[i:i + width])
		hexLine:  str      = " ".join(hexBytes).ljust(width * 3 - 1)

		asciiBytes: map[str] = map(toPrintableChar, data[i:i + width])
		asciiLine:  str      = "".join(asciiBytes).ljust(width)

		output.write(f"{indent}{i:04x}: {hexLine} |{asciiLine}|\n")

def arrayDumpToFile(
	data:   Sequence[int],
	output: TextIO,
	width:  int = 12,
	indent: str = ""
):
	for i in range(0, len(data), width):
		hexBytes: map[str] = map(lambda value: f"{value:#04x}", data[i:i + width])
		hexLine:  str      = ", ".join(hexBytes)

		if (i + width) < len(data):
			hexLine += ","

		output.write(f"{indent}{hexLine}\n")

def normalizeFileName(value: str) -> str:
	return _NAME_INVALID_REGEX.sub("_", value)

## Recursive directory scanning

def findFilesWithExtensions(
	root:       str | Path,
	extensions: Sequence[str]
) -> Generator[Path, None, None]:
	normalized: list[str] = []

	for ext in extensions:
		if not ext.startswith("."):
			ext = f".{ext}"

		normalized.append(ext.lower())

	for path, _, files in os.walk(root):
		for fileName in files:
			filePath: Path = Path(path, fileName)

			if filePath.suffix.lower() in normalized:
				yield filePath

## Logging

def setupLogger(level: int | None):
	logging.basicConfig(
		format = "[{levelname:8s}] {message}",
		style  = "{",
		level  = (
			logging.WARNING,
			logging.INFO,
			logging.DEBUG
		)[min(level or 0, 2)]
	)

## Hash table generator

@dataclass
class HashTableEntry:
	fullHash:   int
	chainIndex: int
	data:       Any

class HashTableBuilder:
	def __init__(self, numBuckets: int = 256):
		self._numBuckets: int = numBuckets

		self.entries: list[HashTableEntry | None] = [ None ] * numBuckets

	def addEntry(self, fullHash: int, data: Any) -> int:
		index: int = fullHash % self._numBuckets

		entry:  HashTableEntry        = HashTableEntry(fullHash, 0, data)
		bucket: HashTableEntry | None = self.entries[index]

		# If no bucket exists for the entry's index, create one.
		if bucket is None:
			self.entries[index] = entry
			return index
		if bucket.fullHash == fullHash:
			raise KeyError(f"hash collision detected ({fullHash:#010x})")

		# Otherwise, follow the buckets's chain, find the last chained item and
		# link the new entry to it.
		while bucket.chainIndex:
			bucket = self.entries[bucket.chainIndex]

			if bucket.fullHash == fullHash:
				raise KeyError(f"hash collision detected, ({fullHash:#010x})")

		bucket.chainIndex = len(self.entries)
		self.entries.append(entry)

		return bucket.chainIndex

class StringBlobBuilder:
	def __init__(self, alignment: int = 1):
		self._alignment: int              = alignment
		self._offsets:   dict[bytes, int] = {}

		self.data: bytearray = bytearray()

	def addByteString(self, string: bytes) -> int:
		# If the same string is already in the blob, return its offset without
		# adding new data.
		offset: int | None = self._offsets.get(string, None)

		if offset is None:
			offset = len(self.data)

			self._offsets[string] = offset
			self.data            += string

			while len(self.data) % self._alignment:
				self.data.append(0)

		return offset

	def addString(self, string: str, encoding: str = "utf-8") -> int:
		return self.addByteString(string.encode(encoding) + b"\0")
