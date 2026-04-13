	.text
	.globl add
	.type add, @function
add:
Label_1:
	addi t0, sp, -4
	addi sp, sp, -4
	sw a0, 0(t0)
	addi t1, sp, -4
	addi sp, sp, -4
	sw a1, 0(t1)
	lw t0, 0(t0)
	lw t2, 0(t1)
	add t0, t0, t2
	mv a0, t0
	addi sp, sp, 8
	ret
	.size add, .-add
	.globl main
	.type main, @function
main:
	addi sp, sp, -4
	sw ra, 0(sp)
Label_2:
	li t0, 3
	mv a0, t0
	li t1, 4
	mv a1, t1
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
	mv a0, t0
	lw ra, 0(sp)
	addi sp, sp, 4
	ret
	.size main, .-main
