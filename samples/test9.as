		nor	0	0	1
		add	1	1	1
		nor	1	0	1		make 1 without using memory
		lw	0	2	max
L1		beq	3	2	done1
		add	0	0	4
		add	0	0	5
L2		beq	4	3	done2
		add	3	5	5
		add	1	4	4	
		beq	0	0	L2
done2	sw	3	5	arr0
		add	1	3	3
		beq	0	0	L1
done1	halt
max		.fill	10
arr0	.fill	0
arr1	.fill	0
arr2	.fill	0
arr3	.fill	0
arr4	.fill	0
arr5	.fill	0
arr6	.fill	0
arr7	.fill	0
arr8	.fill	0
arr9	.fill	0
