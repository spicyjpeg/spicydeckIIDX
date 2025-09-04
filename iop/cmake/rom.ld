/*
 * ps1-bare-metal - (C) 2023-2025 spicyjpeg
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

ENTRY(_start)

MEMORY {
	/*
	 * While the flash is canonically mapped at 0x08000000, the chip's actual
	 * entry point is in its mirror at 0x00000000.
	 */
	ROM (rx)  : ORIGIN = 0x00000000, LENGTH = 0x4000
	RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x800
}

SECTIONS {
	/* Vector table (must be 1024 byte aligned) */

	.vectors : ALIGN(1024) {
		KEEP(*(.text._vectorJumpArea))
		KEEP(*(.data._vectorTable .rodata._vectorTable))
	} > ROM

	/* Code sections */

	.text : ALIGN(4) {
		_textStart = .;

		*(.text .text.* .gnu.linkonce.t.*)
		*(.plt .MIPS.stubs)
	} > ROM

	.rodata : {
		*(.rodata .rodata.* .gnu.linkonce.r.*)

		_textEnd = .;
	} > ROM

	/* Global constructor/destructor arrays */

	.preinit_array (READONLY) : ALIGN(4) {
		_preinitArrayStart = .;

		KEEP(*(.preinit_array))

		_preinitArrayEnd = .;
	} > ROM

	.init_array (READONLY) : ALIGN(4) {
		_initArrayStart = .;

		KEEP(*(SORT(.init_array.*) SORT(.ctors.*)))
		KEEP(*(.init_array .ctors))

		_initArrayEnd = .;
	} > ROM

	.fini_array (READONLY) : ALIGN(4) {
		_finiArrayStart = .;

		KEEP(*(.fini_array .dtors))
		KEEP(*(SORT(.fini_array.*) SORT(.dtors.*)))

		_finiArrayEnd = .;
	} > ROM

	/* Data sections */

	/*
	 * Ensure the .data + .sdata region is aligned at both ends in order to
	 * allow for fast copying when relocating it from flash to RAM on startup.
	 */
	.data : {
		_romDataStart = LOADADDR(.data);
		_dataStart    = .;

		*(.data .data.* .gnu.linkonce.d.*)
	} > RAM AT > ROM

	/*
	 * Set __global_pointer$ (copied to $gp) to point to the beginning of .sdata
	 * plus 0x800, so anything within .sdata and .sbss can be accessed using the
	 * $gp register as base plus a signed 12-bit immediate.
	 */
	.sdata : {
		__global_pointer$ = ALIGN(16) + 0x7f0;

		*(.srodata .srodata.*)
		*(.sdata .sdata.* .gnu.linkonce.s.*)

		.        = ALIGN(4);
		_dataEnd = .;
	} > RAM AT > ROM

	/*
	 * Ensure the entire BSS region is aligned at both ends in order to allow
	 * for fast clearing.
	 */
	.sbss (NOLOAD) : ALIGN(4) {
		_bssStart = .;

		*(.sbss .sbss.* .gnu.linkonce.sb.*)
		*(.scommon)
	} > RAM

	.bss (NOLOAD) : {
		*(.bss .bss.* .gnu.linkonce.b.*)
		*(COMMON)

		.       = ALIGN(4);
		_bssEnd = .;
	} > RAM

	/* Dummy sections */

	.dummy (NOLOAD) : {
		KEEP(*(.dummy))
	} > RAM

	/DISCARD/ : {
		*(.note.* .gnu_debuglink .gnu.lto_*)
	}
}
