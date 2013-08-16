#ifndef CVML_OP_CODES_DATA_H
#define CVML_OP_CODES_DATA_H

#define OP_ARG_2__I_MASK  0170000
#define OP_ARG_2__1_MASK  0007700
#define OP_ARG_2__1_SHIFT 6
#define OP_ARG_2__1_BITS  0
#define OP_ARG_2__2_MASK  0000077
#define OP_ARG_2__2_SHIFT 0
#define OP_ARG_2__2_BITS  0

#define OP_ARG_1H_I_MASK  0177000
#define OP_ARG_1H_1_MASK  0000700
#define OP_ARG_1H_1_SHIFT 6
#define OP_ARG_1H_1_BITS  0
#define OP_ARG_1H_2_MASK  0000077
#define OP_ARG_1H_2_SHIFT 0
#define OP_ARG_1H_2_BITS  0

#define OP_ARG_1a_I_MASK  0177400
#define OP_ARG_1a_1_MASK  0000377
#define OP_ARG_1a_1_SHIFT 0
#define OP_ARG_1a_1_BITS  8

#define OP_ARG_1b_I_MASK  0177700
#define OP_ARG_1b_1_MASK  0000077
#define OP_ARG_1b_1_SHIFT 0
#define OP_ARG_1b_1_BITS  0

#define OP_ARG_0H_I_MASK  0177770
#define OP_ARG_0H_1_MASK  0000007
#define OP_ARG_0H_1_SHIFT 0
#define OP_ARG_0H_1_BITS  0

#define OP_ARG_0__I_MASK  0177777

CVMLOpCodeData
op_code_data_2 = {
  OP_ARG_2,
  OP_ARG_2__I_MASK,
  OP_ARG_2__1_MASK, OP_ARG_2__1_SHIFT, false, OP_ARG_2__1_BITS,
  OP_ARG_2__2_MASK, OP_ARG_2__2_SHIFT, false, OP_ARG_2__2_BITS,
};

CVMLOpCodeData
op_code_data_1H = {
  OP_ARG_1H,
  OP_ARG_1H_I_MASK,
  OP_ARG_1H_1_MASK, OP_ARG_1H_1_SHIFT, false, OP_ARG_2__1_BITS,
  OP_ARG_1H_2_MASK, OP_ARG_1H_2_SHIFT, false, OP_ARG_1H_2_BITS
};

CVMLOpCodeData
op_code_data_1a = {
  OP_ARG_1a,
  OP_ARG_1a_I_MASK,
  OP_ARG_1a_1_MASK, OP_ARG_1a_1_SHIFT, true, OP_ARG_1a_1_BITS,
  0, 0, false, 0
};

CVMLOpCodeData
op_code_data_1b = {
  OP_ARG_1b,
  OP_ARG_1b_I_MASK,
  OP_ARG_1b_1_MASK, OP_ARG_1b_1_SHIFT, false, OP_ARG_1b_1_BITS,
  0, 0, false, 0
};

CVMLOpCodeData
op_code_data_0H = {
  OP_ARG_0H,
  OP_ARG_0H_I_MASK,
  OP_ARG_0H_1_MASK, OP_ARG_0H_1_SHIFT, false, OP_ARG_0H_1_BITS,
  0, 0, false, 0
};

CVMLOpCodeData
op_code_data_0 = {
  OP_ARG_0,
  OP_ARG_0__I_MASK,
  0, 0, false, 0,
  0, 0, false, 0
};

// -------

static CVMLOpCode op_code_add  = { "add" , 2, OP_CODE_ADD , &op_code_data_2  };
static CVMLOpCode op_code_and  = { "and" , 2, OP_CODE_AND , &op_code_data_2  };
static CVMLOpCode op_code_andb = { "andb", 2, OP_CODE_ANDB, &op_code_data_2  };
static CVMLOpCode op_code_cmp  = { "cmp" , 2, OP_CODE_CMP , &op_code_data_2  };
static CVMLOpCode op_code_cmpb = { "cmpb", 2, OP_CODE_CMPB, &op_code_data_2  };
static CVMLOpCode op_code_er   = { "er"  , 2, OP_CODE_ER  , &op_code_data_2  };
static CVMLOpCode op_code_erb  = { "erb" , 2, OP_CODE_ERB , &op_code_data_2  };
static CVMLOpCode op_code_mov  = { "mov" , 2, OP_CODE_MOV , &op_code_data_2  };
static CVMLOpCode op_code_movb = { "movb", 2, OP_CODE_MOVB, &op_code_data_2  };
static CVMLOpCode op_code_or   = { "or"  , 2, OP_CODE_OR  , &op_code_data_2  };
static CVMLOpCode op_code_orb  = { "orb" , 2, OP_CODE_ORB , &op_code_data_2  };
static CVMLOpCode op_code_sub  = { "sub" , 2, OP_CODE_SUB , &op_code_data_2  };
static CVMLOpCode op_code_xor  = { "xor" , 2, OP_CODE_XOR , &op_code_data_2  };
static CVMLOpCode op_code_xorb = { "xorb", 2, OP_CODE_XORB, &op_code_data_2  };

static CVMLOpCode op_code_call = { "call", 2, OP_CODE_CALL, &op_code_data_1H };
static CVMLOpCode op_code_div  = { "div" , 2, OP_CODE_DIV , &op_code_data_1H };
static CVMLOpCode op_code_mpy  = { "mpy" , 2, OP_CODE_MPY , &op_code_data_1H };
static CVMLOpCode op_code_mul  = { "mul" , 2, OP_CODE_MUL , &op_code_data_1H };
static CVMLOpCode op_code_mult = { "mult", 2, OP_CODE_MULT, &op_code_data_1H };
static CVMLOpCode op_code_slc  = { "slc" , 2, OP_CODE_SLC , &op_code_data_1H };
static CVMLOpCode op_code_sll  = { "sll" , 2, OP_CODE_SLL , &op_code_data_1H };
static CVMLOpCode op_code_src  = { "src" , 2, OP_CODE_SRC , &op_code_data_1H };
static CVMLOpCode op_code_srl  = { "srl" , 2, OP_CODE_SRL , &op_code_data_1H };

static CVMLOpCode op_code_beq  = { "beq" , 1, OP_CODE_BEQ , &op_code_data_1a };
static CVMLOpCode op_code_beqa = { "b="  , 1, OP_CODE_BEQ , &op_code_data_1a };
static CVMLOpCode op_code_beqb = { "b=0" , 1, OP_CODE_BEQ , &op_code_data_1a };
static CVMLOpCode op_code_bge  = { "bge" , 1, OP_CODE_BGE , &op_code_data_1a };
static CVMLOpCode op_code_bgea = { "b>=" , 1, OP_CODE_BGE , &op_code_data_1a };
static CVMLOpCode op_code_bgeb = { "b>=0", 1, OP_CODE_BGE , &op_code_data_1a };
static CVMLOpCode op_code_bgt  = { "bgt" , 1, OP_CODE_BGT , &op_code_data_1a };
static CVMLOpCode op_code_bgta = { "b>"  , 1, OP_CODE_BGT , &op_code_data_1a };
static CVMLOpCode op_code_bgtb = { "b>0" , 1, OP_CODE_BGT , &op_code_data_1a };
static CVMLOpCode op_code_ble  = { "ble" , 1, OP_CODE_BLE , &op_code_data_1a };
static CVMLOpCode op_code_blea = { "b<=" , 1, OP_CODE_BLE , &op_code_data_1a };
static CVMLOpCode op_code_bleb = { "b<=0", 1, OP_CODE_BLE , &op_code_data_1a };
static CVMLOpCode op_code_blt  = { "blt" , 1, OP_CODE_BLT , &op_code_data_1a };
static CVMLOpCode op_code_blta = { "b<"  , 1, OP_CODE_BLT , &op_code_data_1a };
static CVMLOpCode op_code_bltb = { "b<0" , 1, OP_CODE_BLT , &op_code_data_1a };
static CVMLOpCode op_code_bne  = { "bne" , 1, OP_CODE_BNE , &op_code_data_1a };
static CVMLOpCode op_code_bnea = { "b<>" , 1, OP_CODE_BNE , &op_code_data_1a };
static CVMLOpCode op_code_bneb = { "b<>0", 1, OP_CODE_BNE , &op_code_data_1a };
static CVMLOpCode op_code_brn  = { "brn" , 1, OP_CODE_BRN , &op_code_data_1a };
static CVMLOpCode op_code_bvc  = { "bvc" , 1, OP_CODE_BVC , &op_code_data_1a };
static CVMLOpCode op_code_bvs  = { "bvs" , 1, OP_CODE_BVS , &op_code_data_1a };
static CVMLOpCode op_code_bcc  = { "bcc" , 1, OP_CODE_BCC , &op_code_data_1a };
static CVMLOpCode op_code_bcs  = { "bcs" , 1, OP_CODE_BCS , &op_code_data_1a };

static CVMLOpCode op_code_neg  = { "neg" , 1, OP_CODE_NEG , &op_code_data_1b };
static CVMLOpCode op_code_not  = { "not" , 1, OP_CODE_NOT , &op_code_data_1b };
static CVMLOpCode op_code_swab = { "swab", 1, OP_CODE_SWAB, &op_code_data_1b };

static CVMLOpCode op_code_exit = { "exit", 1, OP_CODE_EXIT, &op_code_data_0H };

static CVMLOpCode op_code_halt = { "halt", 0, OP_CODE_HALT, &op_code_data_0  };
static CVMLOpCode op_code_nop  = { "nop" , 0, OP_CODE_NOP , &op_code_data_0  };
static CVMLOpCode op_code_stop = { "stop", 0, OP_CODE_STOP, &op_code_data_0  };

static CVMLOpCode *
op_codes[] = {
 // 2 args
 &op_code_add ,
 &op_code_and ,
 &op_code_andb,
 &op_code_cmp ,
 &op_code_cmpb,
 &op_code_er  ,
 &op_code_erb ,
 &op_code_mov ,
 &op_code_movb,
 &op_code_or  ,
 &op_code_orb ,
 &op_code_sub ,
 &op_code_xor ,
 &op_code_xorb,

 // 1 1/2 args
 &op_code_call,
 &op_code_div ,
 &op_code_mpy ,
 &op_code_mul ,
 &op_code_mult,
 &op_code_slc ,
 &op_code_sll ,
 &op_code_src ,
 &op_code_srl ,

 // 1 arg (type 1)
 &op_code_beq ,
 &op_code_beqa,
 &op_code_beqb,
 &op_code_bge ,
 &op_code_bgea,
 &op_code_bgeb,
 &op_code_bgt ,
 &op_code_bgta,
 &op_code_bgtb,
 &op_code_ble ,
 &op_code_blea,
 &op_code_bleb,
 &op_code_blt ,
 &op_code_blta,
 &op_code_bltb,
 &op_code_bne ,
 &op_code_bnea,
 &op_code_bneb,
 &op_code_brn ,
 &op_code_bvc ,
 &op_code_bvs ,
 &op_code_bcc ,
 &op_code_bcs ,

 // 1 arg (type 2)
 &op_code_neg ,
 &op_code_not ,
 &op_code_swab,

 // 1/2 arg
 &op_code_exit,

 // 0 arg
 &op_code_halt,
 &op_code_nop ,
 &op_code_stop,
};

static uint num_op_codes =
  sizeof(op_codes)/sizeof(CVMLOpCode *);

#if 0
static CVMLOpCode *
op_codes_2[] = {
 &op_code_add ,
 &op_code_and ,
 &op_code_andb,
 &op_code_cmp ,
 &op_code_cmpb,
 &op_code_er  ,
 &op_code_erb ,
 &op_code_mov ,
 &op_code_movb,
 &op_code_or  ,
 &op_code_orb ,
 &op_code_sub ,
 &op_code_xor ,
 &op_code_xorb,
};

static uint num_op_codes_2 =
  sizeof(op_codes_2)/sizeof(CVMLOpCode *);

static CVMLOpCode *
op_codes_1H[] = {
 &op_code_call,
 &op_code_div ,
 &op_code_mpy ,
 &op_code_mul ,
 &op_code_mult,
 &op_code_slc ,
 &op_code_sll ,
 &op_code_src ,
 &op_code_srl ,
};

static uint num_op_codes_1H =
  sizeof(op_codes_1H)/sizeof(CVMLOpCode *);

static CVMLOpCode *
op_codes_1a[] = {
 &op_code_beq ,
 &op_code_beqa,
 &op_code_beqb,
 &op_code_bge ,
 &op_code_bgea,
 &op_code_bgeb,
 &op_code_bgt ,
 &op_code_bgta,
 &op_code_bgtb,
 &op_code_ble ,
 &op_code_blea,
 &op_code_bleb,
 &op_code_blt ,
 &op_code_blta,
 &op_code_bltb,
 &op_code_bne ,
 &op_code_bnea,
 &op_code_bneb,
 &op_code_brn ,
 &op_code_bvc ,
 &op_code_bvs ,
 &op_code_bcc ,
 &op_code_bcs ,
};

static uint num_op_codes_1a =
  sizeof(op_codes_1a)/sizeof(CVMLOpCode *);

static CVMLOpCode *
op_codes_1b[] = {
 &op_code_neg ,
 &op_code_not ,
 &op_code_swab,
};

static uint num_op_codes_1b =
  sizeof(op_codes_1b)/sizeof(CVMLOpCode *);

static CVMLOpCode *
op_codes_0H[] = {
 &op_code_exit,
};

static uint num_op_codes_0H =
  sizeof(op_codes_0H)/sizeof(CVMLOpCode *);

static CVMLOpCode *
op_codes_0[] = {
 &op_code_halt,
 &op_code_nop ,
 &op_code_stop,
};

static uint num_op_codes_0 =
  sizeof(op_codes_0)/sizeof(CVMLOpCode *);
#endif

#endif
