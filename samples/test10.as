		lw	0	1	one
		lw	0	2	times
		lw	0	3	szMat
L1		add	0	0	5		j = 0;
L2		lw	5	6	matA11
		lw	5	7	mat11
		add	6	7	7
		sw	5	7	mat11
		add	1	5	5		++j;
		beq	3	5	done2
		beq	0	0	L2
done2	add	1	4	4
		beq	2	4	done1
		beq	0	0	L1
done1	noop
		halt
one		.fill	1
times	.fill	10000
szMat	.fill	4
matA11	.fill	1
matA12	.fill	2
matA21	.fill	3
matA22	.fill	4
mat11	.fill	0
mat12	.fill	0
mat21	.fill	0
mat22	.fill	0
