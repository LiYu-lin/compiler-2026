	.text
	.globl main
	.type main, @function
main:
Label_1:
	addi t0, sp, -4
	addi sp, sp, -4
	li t1, 5
	sw t1, 0(t2)
	lw t1, 0(t2)
	li t2, 10
	slt t0, t1, t2
	xori t0, t0, 1
	sub t0, t0, zero
	snez t0, t0
	bne t0, zero, Label_5
	j Label_3
Label_2:
	lw t0, 0(t2)
	li t3, 1
	add t0, t0, t3
	sw t0, 0(t2)
	j Label_4
Label_3:
	lw t1, 0(t2)
	li t4, 1
	sub t0, t1, t4
	sw t0, 0(t2)
	j Label_4
Label_4:
	mv a0, zero
	addi sp, sp, 4
	ret
Label_5:
	lw t0, 0(t2)
	li t5, 20
	slt t0, t0, t5
	sub t0, t0, zero
	snez t0, t0
	bne t0, zero, Label_2
	j Label_3
	.size main, .-main
