/* SPDX-License-Identifier: GPL-2.0 */
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)
    .section .piggydata, "a"
#else
    .section .piggydata,#alloc
#endif
	.globl	input_data
input_data:
	.incbin	"arch/arm/boot/compressed/piggy_data"
	.globl	input_data_end
input_data_end:
