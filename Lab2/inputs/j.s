.text
j foo
addiu $2, $0, 10
syscall

foo:
j bar
addiu $2, $0, 5
syscall

bar:
addiu $2, $10, 6
syscall 
