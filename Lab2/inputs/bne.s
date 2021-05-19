.text
bne $8, $11, nottaken
bne $9, $11, taken
bne $zero, $zero, nottaken
addiu $3, $0, 1
addiu $2, $0, 10
syscall

nottaken:
addiu $3, $0, 2
addiu $2, $0, 10
syscall

taken:
addiu $3, $0, 3
addiu $2, $0, 10
syscall
