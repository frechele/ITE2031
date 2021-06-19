		lw	0	1	one	range start
		lw	0	2	ten	range end
		add	0	1	3	variable i
start	beq	3	2	exit
		add	4	3	4	add to result register
		add	1	3	3	increment i
		beq	0	0	start
exit	sw	0	4	result
		halt
one		.fill	1
ten	    .fill   10
result	.fill	0
