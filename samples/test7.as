		lw		0	1	addr
		jalr	1	2
		lw		0	2	err
		noop
succ	halt
		lw		0	2	err
		halt
err		.fill	-1
addr	.fill	succ
