	.text
	.globl add
	.type add, @function
add:
	addi sp, sp, -32
Label_1:
	sw a0, 0(sp)
	sw a1, 4(sp)
	sw a0, 8(sp)
	sw a1, 16(sp)
	lw t0, 8(sp)
	lw t1, 16(sp)
	addw t0, t0, t1
	mv a0, t0
	addi sp, sp, 32
	ret
	.size add, .-add
	.globl main
	.type main, @function
main:
	addi sp, sp, -32
	sd ra, 24(sp)
Label_2:
	li t0, 2
	sw t0, 4(sp)
	li t0, 40
	sw t0, 12(sp)
	addi sp, sp, -96
	sd t0, 0(sp)
	sd t1, 8(sp)
	sd t2, 16(sp)
	sd t3, 24(sp)
	sd t4, 32(sp)
	sd t5, 40(sp)
	sd t6, 48(sp)
	fsw ft5, 64(sp)
	fsw ft6, 68(sp)
	fsw ft7, 72(sp)
	fsw ft8, 76(sp)
	fsw ft9, 80(sp)
	fsw ft10, 84(sp)
	fsw ft11, 88(sp)
	lw t0, 108(sp)
	mv a0, t0
	lw t0, 100(sp)
	mv a1, t0
	call add
	ld t0, 0(sp)
	ld t1, 8(sp)
	ld t2, 16(sp)
	ld t3, 24(sp)
	ld t4, 32(sp)
	ld t5, 40(sp)
	ld t6, 48(sp)
	flw ft5, 64(sp)
	flw ft6, 68(sp)
	flw ft7, 72(sp)
	flw ft8, 76(sp)
	flw ft9, 80(sp)
	flw ft10, 84(sp)
	flw ft11, 88(sp)
	addi sp, sp, 96
	mv t0, a0
	sw t0, 0(sp)
	mv a0, t0
	ld ra, 24(sp)
	addi sp, sp, 32
	ret
	.size main, .-main
