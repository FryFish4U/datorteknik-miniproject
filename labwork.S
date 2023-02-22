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
  hexasc:
  	andi $t0, $a0, 0x0000000F	# masques the 4 LSB in $a1 for copy into t0
    	li $t1, 10			# $t1 gets the decimal value 10
    	slt $t2, $t0, $t1		# if $t0 is less than $t1 then set $t2 to 1
    	beqz $t2, hexascTenPlus		# if $t2 is 1 branch (i.e. run other block) to tenplus 
    	nop
    	
    	addi $v0, $t0, 0x30		# add immediate: $t0 + 0x30 = $v0 
    	jr $ra				# end
    	nop

	hexascTenPlus:
    		addi $v0, $t0, 55	# add immediate: $t0 + 55 = $v0
   		jr $ra			# end
   		nop
  
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
  
time2string:
	# $a0 has the address of an area of in mem. and the 16 least sign. digits for BCD
	# use hexasc to convert each BCD digit to ASCII then use sb to store each byte (4) in each destination

	PUSH $ra	# due to previous jal, we need to save $ra in stack using push so we later can POP it back
	PUSH $s0
	move $s0, $a0
	move $a0, $a1
	
	# !M! M:SS
	 
	jal hexasc
	nop
	sb $v0, 4($s0)	# store the LSB in $v0 to $a0
	
	# M !M! :SS
	srl $a0, $a0, 4	# shift $a1 right 4 bits
	 
	jal hexasc
	nop
	sb $v0, 3($s0)	# store the LSB in $v0 to $a0
		
	# lines for dividing the digits with : 
	li $t4, 0x3A	# loads the hexacode for ASCII :
	sb $t4, 2($s0)	# saves ASCII hexacode to $a0
		
	# MM: !S! S
	srl $a0, $a0, 4	# shift $a1 right 4 bits
	jal hexasc
	nop
	sb $v0, 1($s0)	# store the LSB in $v0 to $a0
	
	# MM:S !S!
	srl $a0, $a0, 4	# shift $a1 right 4 bits
	jal hexasc
	nop
	sb $v0, 0($s0)	# store the LSB in $v0 to $a0
		
	move $a0, $s0
										
	# removes the text from start of code
	li $t4, 0x00	# loads null into $t4
	sb $t4, 5($a0)	# stores null into the 5th byte of $a0
	


	POP $s0
	POP $ra
	jr $ra
	nop

enable_interrupt:

	ei		# executes the "enable interrupts" instruction
	jr $ra	# return to return address
	nop