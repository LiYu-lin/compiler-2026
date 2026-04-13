	.text
	.globl main
	.type main, @function
main:
Label_1:
	addi a4, sp, -4
	addi sp, sp, -4
	li t0, 5
	sw t0, 0(a4)
	lw t0, 0(a4)
	li t1, 10
	slt t0, t0, t1
	xori t0, t0, 1
	bne t0, zero, Label_5
	j Label_3
Label_2:
	lw t0, 0(a4)
	li t1, 1
	add t0, t0, t1
	sw t0, 0(a4)
	j Label_4
Label_3:
	lw t0, 0(a4)
	li t1, 1
	sub t0, t0, t1
	sw t0, 0(a4)
	j Label_4
Label_4:
	mv a0, zero
	addi sp, sp, 4
	ret
Label_5:
	lw t0, 0(a4)
	li t1, 20
	slt t0, t0, t1
	bne t0, zero, Label_2
	j Label_3
	.size main, .-main
