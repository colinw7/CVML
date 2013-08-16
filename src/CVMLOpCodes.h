#ifndef CVML_OP_CODES_H
#define CVML_OP_CODES_H

#define MODE_REGISTER                 0
#define MODE_REGISTER_DEFERRED        1
#define MODE_IMMEDIATE                2
#define MODE_AUTO_INCREMENT           2
#define MODE_DEFERRED_AUTO_INCREMENT  3
#define MODE_AUTO_DECREMENT           4
#define MODE_DEFERRED_AUTO_DECREMENT  5
#define MODE_RELATIVE                 6
#define MODE_REGISTER_OFFSET          6
#define MODE_REGISTER_OFFSET_DEFERRED 7

//-----

#define R0_NUM 0
#define R1_NUM 1
#define R2_NUM 2
#define R3_NUM 3
#define R4_NUM 4
#define R5_NUM 5
#define R6_NUM 6
#define SP_NUM 6
#define R7_NUM 7
#define PC_NUM 7

#define R0_ADDR     0177700 // 65472
#define R1_ADDR     0177702 // 65474
#define R2_ADDR     0177704 // 65476
#define R3_ADDR     0177706 // 65478
#define R4_ADDR     0177710 // 65480
#define R5_ADDR     0177712 // 65482
#define R6_ADDR     0177714 // 65484
#define SP_ADDR     0177714 // 65484
#define R7_ADDR     0177716 // 65486
#define PC_ADDR     0177716 // 65486

#define OUTPUT_ADDR 0177720 // 65488
#define INPUT_ADDR  0177722 // 65490

#define STACK_ADDR  0177774 // 65532
#define PSW_ADDR    0177776 // 65534

//-----

#define PSW_N_BIT 3
#define PSW_Z_BIT 2
#define PSW_C_BIT 1
#define PSW_V_BIT 0

//-----

// Two arguments

#define OP_CODE_MOV  0010000
#define OP_CODE_CMP  0020000
#define OP_CODE_AND  0030000
#define OP_CODE_ER   0040000
#define OP_CODE_XOR  0040000
#define OP_CODE_OR   0050000
#define OP_CODE_ADD  0060000
#define OP_CODE_MOVB 0110000
#define OP_CODE_CMPB 0120000
#define OP_CODE_ANDB 0130000
#define OP_CODE_ERB  0140000
#define OP_CODE_XORB 0140000
#define OP_CODE_ORB  0150000
#define OP_CODE_SUB  0160000

// One and a Half arguments

#define OP_CODE_CALL 0004000
#define OP_CODE_MPY  0070000
#define OP_CODE_MUL  0070000
#define OP_CODE_MULT 0070000
#define OP_CODE_DIV  0071000
#define OP_CODE_SLL  0072000
#define OP_CODE_SLC  0073000
#define OP_CODE_SRL  0172000
#define OP_CODE_SRC  0173000

// One argument (Type 1)

#define OP_CODE_BRN  0000400 // 0100400
#define OP_CODE_BNE  0001000 // 0101000
#define OP_CODE_BEQ  0001400 // 0101400
#define OP_CODE_BGE  0002000 // 0102000
#define OP_CODE_BLT  0002400 // 0102400
#define OP_CODE_BGT  0003000 // 0103000
#define OP_CODE_BLE  0003400 // 0103400
#define OP_CODE_BVC  0102000
#define OP_CODE_BVS  0102400
#define OP_CODE_BCC  0103000
#define OP_CODE_BCS  0103400

// One argument (Type 2)

#define OP_CODE_SWAB 0000300
#define OP_CODE_NOT  0005100
#define OP_CODE_NEG  0005400

// Half argument

#define OP_CODE_EXIT 0000200

// No arguments

#define OP_CODE_NOP  0000240
#define OP_CODE_STOP 0000000
#define OP_CODE_HALT 0000000

struct CVMLOpCodeData {
  uint type;
  uint instn_mask;
  uint arg_mask1;
  uint arg_shift1;
  bool arg_signed1;
  uint arg_bits1;
  uint arg_mask2;
  uint arg_shift2;
  bool arg_signed2;
  uint arg_bits2;
};

extern CVMLOpCodeData op_code_data_2;
extern CVMLOpCodeData op_code_data_1H;
extern CVMLOpCodeData op_code_data_1a;
extern CVMLOpCodeData op_code_data_1b;
extern CVMLOpCodeData op_code_data_0H;
extern CVMLOpCodeData op_code_data_0;

#endif
