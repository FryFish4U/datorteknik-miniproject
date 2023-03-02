  # labwork.S
  # Written 2015-2017 by F Lundevall
  # Skeleton file for IS1200/IS1500 lab 1.
  # The macros PUSH and POP are in the public domain.
  # Please add your own code at the end of the file.

  #
  # Please keep the two macros PUSH and POP unchanged
  #
.macro	PUSH reg
	addi	$sp,$sp,-4
	sw	\reg,0($sp)
.endm

.macro	POP reg
	lw	\reg,0($sp)
	addi	$sp,$sp,4
.endm
  #
  # Please add your own code below this line
  #

.global time2string
.global delay
.global enable_interrupt

.text
  delay:
# ms = $a0
	li	$t5, 0			# resets $t5 value to 0 so delayFor works
	bgt	$a0, $0, delayWhile	# checks if code has waited all ms
	nop				
	jr	$ra			# jumps back
	nop
	
	delayWhile:
		blt	$t5, 4429, delayFor	
		nop
		addi	$a0, $a0, -1
		j	delay
		nop
		
		delayFor:
			# current ms length = $t5
			addi	$t5, $t5, 1
			j	delayWhile
			nop

enable_interrupt:

	ei		# executes the "enable interrupts" instruction
	jr $ra	# return to return address
	nop