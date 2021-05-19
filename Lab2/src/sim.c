#include <stdio.h>
#include "shell.h"
#define OP_SPECIAL    0x00
#define SUBOP_ADD     0x20
#define SUBOP_ADDU    0x21
#define OP_ADDI       0x08
#define OP_ADDIU      0x09
#define SUBOP_SUB     0x22
#define SUBOP_SUBU    0x23
#define OP_ORI        0xd
#define SUBOP_SLL     0x00
#define SUBOP_SRL     0x2
#define SUBOP_SRA     0x3
#define SUBOP_MULTU   0x19
#define SUBOP_DIVU    0x1b
#define SUBOP_SLTU    0x2b
#define SUBOP_MFHI    0x10
#define SUBOP_SYSCALL 0xc

#define SUBOP_MTHI    0x11
#define SUBOP_MTLO    0x13
#define SUBOP_MULT    0x18
#define SUBOP_DIV     0x1a
#define SUBOP_AND     0x24
#define SUBOP_OR      0x25
#define SUBOP_XOR     0x26
#define SUBOP_NOR     0x27

#define OP_ANDI       0xc
#define OP_XORI       0xe

#define OP_J          0x02
#define OP_BEQ        0x4
#define OP_BNE        0x5
#define OP_BGTZ       0x7
#define OP_LUI        0xf
#define OP_LB         0x20
#define OP_LBU        0x24
#define OP_LW         0x23
#define OP_SB         0x28
#define OP_SW         0x2b
uint32_t dcd_op;     /* decoded opcode */
uint32_t dcd_rs;     /* decoded rs operand */
uint32_t dcd_rt;     /* decoded rt operand */
uint32_t dcd_rd;     /* decoded rd operand */
uint32_t dcd_shamt;  /* decoded shift amount */
uint32_t dcd_funct;  /* decoded function */
uint32_t dcd_imm;    /* decoded immediate value */
uint32_t dcd_target; /* decoded target address */
int      dcd_se_imm; /* decoded sign-extended immediate value */
uint32_t inst;       /* machine instruction */
uint64_t temp;       /* 64 bit result of multu*/
uint32_t sign_extend_h2w(uint16_t c)
{
    return (c & 0x8000) ? (c | 0xffff8000) : c;
}
uint32_t sign_extend_b2w(uint8_t c)
{
    return (c & 0x80) ? (c | 0xffffff80) : c;
}
void fetch()
{
    /* fetch the 4 bytes of the current instruction */
    inst = mem_read_32(CURRENT_STATE.PC);
}

void decode()
{
    /* decoding an instruction */
    dcd_op     = (inst >> 26) & 0x3F;
    dcd_rs     = (inst >> 21) & 0x1F;
    dcd_rt     = (inst >> 16) & 0x1F;
    dcd_rd     = (inst >> 11) & 0x1F;
    dcd_shamt  = (inst >> 6) & 0x1F;
    dcd_funct  = (inst >> 0) & 0x3F;
    dcd_imm    = (inst >> 0) & 0xFFFF;
    dcd_se_imm = sign_extend_h2w(dcd_imm);
    dcd_target = (inst >> 0) & ((1UL << 26) - 1);
}

void execute()
{
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE.REGS[0] = 0;

    switch (dcd_op)
    {
        case OP_SPECIAL: 
            switch (dcd_funct)
            {
                case SUBOP_ADD:
                case SUBOP_ADDU: 
                	NEXT_STATE.REGS[dcd_rd] = CURRENT_STATE.REGS[dcd_rs] + CURRENT_STATE.REGS[dcd_rt];
                	break;
                	
                case SUBOP_SUB:
                	NEXT_STATE.REGS[dcd_rd] = CURRENT_STATE.REGS[dcd_rs] - CURRENT_STATE.REGS[dcd_rt];
                	break;
                	
                case SUBOP_SUBU:
                	NEXT_STATE.REGS[dcd_rd] = CURRENT_STATE.REGS[dcd_rs] - CURRENT_STATE.REGS[dcd_rt];
                	break;
                	
                case SUBOP_SLL:
                	NEXT_STATE.REGS[dcd_rd] = CURRENT_STATE.REGS[dcd_rt] << dcd_shamt;
                	break;
                	
                case SUBOP_SRL:
                	NEXT_STATE.REGS[dcd_rd] = CURRENT_STATE.REGS[dcd_rt] >> dcd_shamt;
                	break;
                	
                case SUBOP_SRA: ;
                	//int32_t srt = (int32_t) CURRENT_STATE.REGS[dcd_rt];
                	NEXT_STATE.REGS[dcd_rd] = (int32_t) CURRENT_STATE.REGS[dcd_rt] >> dcd_shamt;
                	break;
                	
                case SUBOP_MULT:
                	temp = (int64_t)(int32_t)CURRENT_STATE.REGS[dcd_rs] * (int64_t)(int32_t)CURRENT_STATE.REGS[dcd_rt];
                	NEXT_STATE.HI = (uint32_t)((temp >> 32) & 0xffffffff);
                	NEXT_STATE.LO = (uint32_t)(temp & 0xffffffff);
                	break;
                	
                case SUBOP_MULTU:
                	NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                	temp = (uint64_t)CURRENT_STATE.REGS[dcd_rs] * (uint64_t)CURRENT_STATE.REGS[dcd_rt];
                	NEXT_STATE.HI = (uint32_t)((temp >> 32) & 0xffffffff);
                	NEXT_STATE.LO = (uint32_t)(temp & 0xffffffff);
                	break;
                	
                case SUBOP_DIV:
                NEXT_STATE.LO = (int) CURRENT_STATE.REGS[dcd_rs] / (int) CURRENT_STATE.REGS[dcd_rt];
                	NEXT_STATE.HI = (int) CURRENT_STATE.REGS[dcd_rs] % (int) CURRENT_STATE.REGS[dcd_rt];
                	break;
                	
                case SUBOP_DIVU:
                	NEXT_STATE.LO = (uint32_t) CURRENT_STATE.REGS[dcd_rs] / (uint32_t) CURRENT_STATE.REGS[dcd_rt];
                	NEXT_STATE.HI = (uint32_t) CURRENT_STATE.REGS[dcd_rs] % (uint32_t) CURRENT_STATE.REGS[dcd_rt];
                	break;
                	
                case SUBOP_SLTU:
                	if (CURRENT_STATE.REGS[dcd_rs] < CURRENT_STATE.REGS[dcd_rt])
                		NEXT_STATE.REGS[dcd_rd] = 1;
                	else
                		NEXT_STATE.REGS[dcd_rd] = 0;
                	break;
                	
                case SUBOP_MFHI:
                	NEXT_STATE.REGS[dcd_rd] = CURRENT_STATE.HI;
                	break;
                	
                case SUBOP_SYSCALL:
                    if (CURRENT_STATE.REGS[2] == 10)
                        RUN_BIT = 0;
                    break;
                    
                case SUBOP_AND:
                	NEXT_STATE.REGS[dcd_rd] = CURRENT_STATE.REGS[dcd_rs] & CURRENT_STATE.REGS[dcd_rt];
                	break;
                	
                case SUBOP_OR:
                	NEXT_STATE.REGS[dcd_rd] = CURRENT_STATE.REGS[dcd_rs] | CURRENT_STATE.REGS[dcd_rt];
                	break;
                	
                case SUBOP_XOR:
                	NEXT_STATE.REGS[dcd_rd] = ~(CURRENT_STATE.REGS[dcd_rs] & CURRENT_STATE.REGS[dcd_rt]) & (CURRENT_STATE.REGS[dcd_rs] | CURRENT_STATE.REGS[dcd_rt]);
                	break;
                case SUBOP_NOR:
                	NEXT_STATE.REGS[dcd_rd] = ~(CURRENT_STATE.REGS[dcd_rs] | CURRENT_STATE.REGS[dcd_rt]);
                	break;
                	
                case SUBOP_MTLO:
                	NEXT_STATE.LO = CURRENT_STATE.REGS[dcd_rs];
                	break;
                	
                case SUBOP_MTHI:
                	NEXT_STATE.HI = CURRENT_STATE.REGS[dcd_rs];
                	break;
            }
            break;

        case OP_ADDI:
        case OP_ADDIU:
        	NEXT_STATE.REGS[dcd_rt] = CURRENT_STATE.REGS[dcd_rs] + dcd_se_imm;
        	break;
        	
        case OP_ORI:
        	NEXT_STATE.REGS[dcd_rt] = CURRENT_STATE.REGS[dcd_rs] | dcd_imm;
        	NEXT_STATE.PC = CURRENT_STATE.PC + 4;
        	break;
        	
        case OP_J:
        	NEXT_STATE.PC = (CURRENT_STATE.PC & 0xF0000000) | (dcd_target << 2);
        	break;
        	
        case OP_BEQ:
        	if ((int)CURRENT_STATE.REGS[dcd_rs] == (int)CURRENT_STATE.REGS[dcd_rt])
            		NEXT_STATE.PC = CURRENT_STATE.PC +  (dcd_se_imm << 2);
        	break;
        	
        case OP_BNE:
        	if (CURRENT_STATE.REGS[dcd_rs] != CURRENT_STATE.REGS[dcd_rt])
            		NEXT_STATE.PC = CURRENT_STATE.PC +  (dcd_se_imm << 2);
        	break;
        	
        case OP_BGTZ:
        	if ((int)(CURRENT_STATE.REGS[dcd_rs]) > 0)
            		NEXT_STATE.PC = CURRENT_STATE.PC +  (dcd_se_imm << 2);
            	//else
            		//NEXT_STATE.PC = CURRENT_STATE.PC +  4;
        	break;
        	
        case OP_LUI:
        if (dcd_rs == 0)
            	NEXT_STATE.REGS[dcd_rt] = (dcd_imm << 16);
        	break;
        	
        case OP_LB:
        	NEXT_STATE.REGS[dcd_rt] = sign_extend_b2w((mem_read_32(CURRENT_STATE.REGS[dcd_rs] + dcd_se_imm)) & 0xFF);
        	NEXT_STATE.PC = CURRENT_STATE.PC + 4;
        	break;
        	
        case OP_LBU:
        	NEXT_STATE.REGS[dcd_rt] = (uint32_t)(mem_read_32(CURRENT_STATE.REGS[dcd_rs] + dcd_se_imm)) & 0xFF;
        	break;
        	
        case OP_LW:
        	NEXT_STATE.REGS[dcd_rt] = mem_read_32((CURRENT_STATE.REGS[dcd_rs] + dcd_se_imm));
        	break;
        	
        case OP_SB:
        	mem_write_32((CURRENT_STATE.REGS[dcd_rs] + dcd_se_imm), (int)((CURRENT_STATE.REGS[dcd_rt]) & 0xFF));
        	break;
        	
        case OP_SW:
        	mem_write_32((CURRENT_STATE.REGS[dcd_rs] + dcd_se_imm), (int)((CURRENT_STATE.REGS[dcd_rt])));
        	break;
        	
        case OP_ANDI:
        	NEXT_STATE.REGS[dcd_rt] = CURRENT_STATE.REGS[dcd_rs] & dcd_imm;
        	break;
        	
        case OP_XORI:
        	NEXT_STATE.REGS[dcd_rt] = ~(CURRENT_STATE.REGS[dcd_rs] & dcd_imm) & (CURRENT_STATE.REGS[dcd_rs] | dcd_imm);
        	break;
    }
    
    CURRENT_STATE.REGS[0] = 0;
}


void process_instruction()
{
   fetch();
   decode();
   execute();
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. */
}

















