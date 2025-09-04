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

.section .text.setjmp, "ax", @progbits
.global setjmp
.type setjmp, @function

setjmp:
	sw    ra, 0x0(a0)
	sw    sp, 0x4(a0)
	sw    s0, 0x8(a0)
	sw    s1, 0xc(a0)

	# return 0;
	li    a0, 0
	ret

.section .text.longjmp, "ax", @progbits
.global longjmp
.type longjmp, @function

longjmp:
	lw    ra, 0x0(a0)
	lw    sp, 0x4(a0)
	lw    s0, 0x8(a0)
	lw    s1, 0xc(a0)

	# return status;
	move  a0, a1
	ret
