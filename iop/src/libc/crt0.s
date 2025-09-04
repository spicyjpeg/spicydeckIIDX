# ps1-bare-metal - (C) 2023-2025 spicyjpeg
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

.option norelax
.option rvc
.option arch, +zicsr

.set STACK_END, 0x20000800

.set CSR_MSTATUS_MPP_MACHINE, 3 << 11

.section .text._start, "ax", @progbits
.global _start
.type _start, @function

_start:
	# Initialize the stack pointer and set $gp to point to the middle of the
	# .sdata/.sbss sections, ensuring variables placed in those sections can be
	# quickly accessed. See the linker script for more details.
	la    sp, STACK_END - 8
	la    gp, __global_pointer$

	# Defer all other initialization to _startInner(). Instead of invoking it
	# directly, perform an exception return in order to ensure the privilege
	# level is set to "machine" (the only one supported).
	li    a0, CSR_MSTATUS_MPP_MACHINE
	csrw  mstatus, a0
	la    a0, _startInner
	csrw  mepc, a0

	li    a0, 0
	li    a1, 0
	mret
