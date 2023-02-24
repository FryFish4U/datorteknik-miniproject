.global enable_interrupt

.text

enable_interrupt:

	ei		# executes the "enable interrupts" instruction
	jr $ra	# return to return address
	nop