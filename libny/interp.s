	# the startup routine (and the exit function)
	.text
	.code32
	.lcomm stack,8192
	.global _start
_start:
	# set up stack
	movl	$stack,%ebp
	movl	$stack,%esp
	addl	$0x2000,%esp		# stacks have 8KiB combined length
	movl	$0xDEADDEAD,%ebx	# %ebx is TOS

	# fake a bfunc so that main returns to something sane
	movl	$exit_,%eax
	movl	%eax,(%ebp)
	addl	$4,%ebp

	# get the address of main
	movl	$_main,%esi

	# clear the direction flag, we don't know it's state,
	# no afunc should alter it.
	cld

	# standard "next func" routine
	lodsl
	push	%eax
	ret

exit:
	# exit program
	movl	$0x0001,%eax
	int	$0x80

	# put exit in bfunc exit_
	.data
exit_:
	.long	exit


	# here follow the few truly builtin functions
	.text

	.global callb

	.global return
return:
	subl	$4,%ebp
	movl	(%ebp),%esi
	lodsl
	push	%eax
	ret

	.global push
push:
	lodsl
	pushl	%ebx
	movl	%eax,%ebx
	lodsl
	push	%eax
	ret
