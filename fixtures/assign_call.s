	.text
	.globl add
	.type add, @function
add:
	addi sp, sp, -16
Label_1:
	sw a0, 0(sp)
	sw a1, 4(sp)
	lw t1, 0(sp)
	lw t2, 4(sp)
	add t1, t1, t2
	mv a0, t1
	addi sp, sp, 16
	ret
	.size add, .-add
	.globl main
	.type main, @function
main:
	addi sp, sp, -16
	sw ra, 4(sp)
Label_2:
	li t0, 5
	sw t0, 0(sp)
	lw t0, 0(sp)
	mv a0, t0
	li t0, 1
	mv a1, t0
	addi sp, sp, -32
	sw t0, 0(sp)
	sw t1, 4(sp)
	sw t2, 8(sp)
	sw t3, 12(sp)
	sw t4, 16(sp)
	sw t5, 20(sp)
	sw t6, 24(sp)
	call add
	lw t0, 0(sp)
	lw t1, 4(sp)
	lw t2, 8(sp)
	lw t3, 12(sp)
	lw t4, 16(sp)
	lw t5, 20(sp)
	lw t6, 24(sp)
	addi sp, sp, 32
	mv t0, a0
	sw t0, 0(sp)
	lw t0, 0(sp)
	mv a0, t0
	lw ra, 4(sp)
	addi sp, sp, 16
	ret
	.size main, .-main
