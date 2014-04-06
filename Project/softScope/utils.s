	.syntax unified
	.cpu cortex-m4
	.eabi_attribute 27, 3
	.eabi_attribute 28, 1
	.fpu fpv4-sp-d16
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 1
	.eabi_attribute 30, 1
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.thumb
	.text
	.thumb
	.thumb_func
	.syntax unified
	.align	2
	.global	memcpy32
	.type	memcpy32, %function
memcpy32:
	@ Function supports interworking.
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	stmfd	sp!, {r3-r7}	    @ store on the stack
	movs	r3, r2, lsr #4	    @ nBytes / 16
	beq	.copylast	    @ if nBytes / 16 = 0 -> copy the last block
	ldmia	r1!, {r4-r7}	    @ load the first 4*2 = 8 samples = 16 bytes
.copyblock:
	stmia	r0!, {r4-r7}
	ldmia	r1!, {r4-r7}
	subs	r3, #1
	bne	.copyblock
.copylast:
	mov	r3, r2		    @ check how many bytes are left should by a multiple of two
	and	r3, #0xf
	movs	r3, r3, lsl #2	    @ divide by 4
	beq	.L1
	sub	r1, #16		    @ to correct for the load/increase of the last time
.copyloop:
	ldr	r4, [r1], 4	    @ load samples[0] with post-increment
	str	r4, [r0], 4	    @ save the sample
	subs	r3, #1		    @ N--
	bne	.copyloop
.L1:
	ldmfd	sp!, {r3-r7}	    @ restore from the stack!
	bx	lr
	.size	memcpy32, .-memcpy32
	.ident	"GCC: (GNU Tools for ARM Embedded Processors) 4.8.3 20131129 (release) [ARM/embedded-4_8-branch revision 205641]"
