#include "CVMLI.h"
#include "CVMLOpCodesData.h"
#include <CFuncs.h>
#include <CAux.h>
#include <cstring>
#include <cassert>

CVML::
CVML() :
 line_num_(0), parse_(NULL), pc_(0), begin_pc_(0), memory_(NULL), execute_trace_flag_(false)
{
}

CVML::
~CVML()
{
  reset();

  deleteLabels();
}

void
CVML::
reset()
{
  fileName_ = "";

  deleteLabels();
  deleteLines ();

  line_ = "";

  line_num_ = 0;

  delete parse_;

  parse_ = NULL;

  pc_ = 0;

  delete [] memory_;

  memory_ = NULL;

  string_id_map_.clear();
  id_string_map_.clear();

  string_id_ = 0;

  //----

  addLabel("output", OUTPUT_ADDR);
  addLabel("input" , INPUT_ADDR );
}

void
CVML::
deleteLabels()
{
  CAux::for_each(labels_, CDeletePointer());

  labels_.clear();
}

void
CVML::
deleteLines()
{
  CAux::for_each(lines_, CDeletePointer());

  lines_.clear();
}

void
CVML::
print(std::ostream &os)
{
  try {
    LineList::const_iterator line1 = lines_.begin();
    LineList::const_iterator line2 = lines_.end  ();

    for ( ; line1 != line2; ++line1)
      (*line1)->print(os);
  }
  catch (CVMLError error) {
    std::cerr << error.msg << std::endl;
  }
}

bool
CVML::
stringToChar(const std::string &str, char *c)
{
  uint len = str.size();

  if (len == 1)
    *c = str[0];
  else {
    if (str[0] == '\\') {
      if      (str[1] == 'b')
        *c = '\b';
      else if (str[1] == 'n')
        *c = '\n';
      else if (str[1] == 't')
        *c = '\t';
      else if (str[1] == '0')
        *c = '\0';
      else if (str[1] == '\'')
        *c = '\'';
      else if (str[1] == '\"')
        *c = '\"';
      else {
        std::cerr << "Invalid escape code" << std::endl;
        return false;
      }
    }
    else
      return false;
  }

  return true;
}

bool
CVML::
isRegister(const std::string &name, int *reg_num)
{
  if      (CStrUtil::casecmp(name, "r0") == 0)
    *reg_num = R0_NUM;
  else if (CStrUtil::casecmp(name, "r1") == 0)
    *reg_num = R1_NUM;
  else if (CStrUtil::casecmp(name, "r2") == 0)
    *reg_num = R2_NUM;
  else if (CStrUtil::casecmp(name, "r3") == 0)
    *reg_num = R3_NUM;
  else if (CStrUtil::casecmp(name, "r4") == 0)
    *reg_num = R4_NUM;
  else if (CStrUtil::casecmp(name, "r5") == 0)
    *reg_num = R5_NUM;
  else if (CStrUtil::casecmp(name, "r6") == 0 ||
           CStrUtil::casecmp(name, "sp") == 0)
    *reg_num = R6_NUM;
  else if (CStrUtil::casecmp(name, "r7") == 0 ||
           CStrUtil::casecmp(name, "pc") == 0)
    *reg_num = R7_NUM;
  else
    return false;

  return true;
}

ushort
CVML::
getRegisterAddress(ushort reg_num)
{
  switch (reg_num) {
    case R0_NUM: return R0_ADDR;
    case R1_NUM: return R1_ADDR;
    case R2_NUM: return R2_ADDR;
    case R3_NUM: return R3_ADDR;
    case R4_NUM: return R4_ADDR;
    case R5_NUM: return R5_ADDR;
    case R6_NUM: return R6_ADDR;
    case R7_NUM: return R7_ADDR;
  }

  return 0;
}

ushort
CVML::
getAddressRegister(ushort address)
{
  switch (address) {
    case R0_ADDR: return R0_NUM;
    case R1_ADDR: return R1_NUM;
    case R2_ADDR: return R2_NUM;
    case R3_ADDR: return R3_NUM;
    case R4_ADDR: return R4_NUM;
    case R5_ADDR: return R5_NUM;
    case R6_ADDR: return R6_NUM;
    case R7_ADDR: return R7_NUM;
  }

  return 9999;
}

const char *
CVML::
getRegisterName(ushort reg_num)
{
  switch (reg_num) {
    case R0_NUM: return "r0";
    case R1_NUM: return "r1";
    case R2_NUM: return "r2";
    case R3_NUM: return "r3";
    case R4_NUM: return "r4";
    case R5_NUM: return "r5";
    case R6_NUM: return "sp";
    case R7_NUM: return "pc";
  }

  return 0;
}

bool
CVML::
isEvenRegisterAddress(ushort address)
{
  switch (address) {
    case R0_ADDR: case R2_ADDR: case R4_ADDR: case R6_ADDR:
      return true;
    default:
      return false;
  }
}

bool
CVML::
getVariableValue(const std::string &name, uint *value)
{
  uint addr;

  if (! getVariableAddress(name, &addr))
    return false;

  *value = getMemoryWord(addr);

  return true;
}

bool
CVML::
getVariableAddress(const std::string &name, uint *addr)
{
  CVMLLabel *label = lookupLabelByName(name);

  if (label == NULL)
    return false;

  *addr = label->getPC();

  return true;
}

CVMLLabel *
CVML::
addLabel(const std::string &name)
{
  return addLabel(name, pc_);
}

CVMLLabel *
CVML::
addLabel(const std::string &name, uint pc)
{
  CVMLStringId id = lookupStringId(name);

  return addLabel(id, pc);
}

CVMLLabel *
CVML::
addLabel(CVMLStringId id)
{
  return addLabel(id, pc_);
}

CVMLLabel *
CVML::
addLabel(CVMLStringId id, uint pc)
{
  CVMLLabel *label = lookupLabelById(id);

  if (label != NULL)
    return label;

  label = new CVMLLabel(this, id, line_num_);

  labels_.push_back(label);

  label->setPC(pc);

  const std::string &str = lookupIdString(id);

  if (str == "begin")
    setBeginPC(pc);

  return label;
}

CVMLLabel *
CVML::
lookupLabelById(CVMLStringId id)
{
  LabelList::const_iterator l1 = labels_.begin();
  LabelList::const_iterator l2 = labels_.end  ();

  for ( ; l1 != l2; ++l1)
    if ((*l1)->getId() == id)
      return *l1;

  return NULL;
}

CVMLLabel *
CVML::
lookupLabelByName(const std::string &name)
{
  CVMLStringId id = lookupStringId(name);

  LabelList::const_iterator l1 = labels_.begin();
  LabelList::const_iterator l2 = labels_.end  ();

  for ( ; l1 != l2; ++l1)
    if ((*l1)->getId() == id)
      return *l1;

  return NULL;
}

CVMLLabel *
CVML::
lookupLabelByPC(uint pc)
{
  LabelList::const_iterator l1 = labels_.begin();
  LabelList::const_iterator l2 = labels_.end  ();

  for ( ; l1 != l2; ++l1)
    if ((*l1)->getPC() == pc)
      return *l1;

  return NULL;
}

CVMLOp *
CVML::
lookupOpByPC(uint pc)
{
  LineList::const_iterator line1 = lines_.begin();
  LineList::const_iterator line2 = lines_.end  ();

  for ( ; line1 != line2; ++line1) {
    if (! (*line1)->isOp())
      continue;

    CVMLOp *op = (*line1)->getOp();

    if (op->getPC() == pc)
      return op;
  }

  return NULL;
}

CVMLData *
CVML::
lookupDataByPC(uint pc)
{
  LineList::const_iterator line1 = lines_.begin();
  LineList::const_iterator line2 = lines_.end  ();

  for ( ; line1 != line2; ++line1) {
    if (! (*line1)->isData())
      continue;

    CVMLData *data = (*line1)->getData();

    if (data->getPC() == pc)
      return data;
  }

  return NULL;
}

CVMLOpCode *
CVML::
lookupOpCode(const std::string &identifier)
{
  for (uint i = 0; i < num_op_codes; ++i)
    if (strcmp(op_codes[i]->name, identifier.c_str()) == 0)
      return op_codes[i];

  return NULL;
}

CVMLLine *
CVML::
addLine(CVMLLine *line)
{
  lines_.push_back(line);

  return line;
}

CVMLStringId
CVML::
lookupStringId(const std::string &str)
{
  CVMLStringIdMap::const_iterator p1 = string_id_map_.find(str);
  CVMLStringIdMap::const_iterator p2 = string_id_map_.end();

  if (p1 != p2)
    return (*p1).second;

  ++string_id_;

  string_id_map_[str       ] = string_id_;
  id_string_map_[string_id_] = str;

  return string_id_;
}

const std::string &
CVML::
lookupIdString(CVMLStringId id) const
{
  CVMLIdStringMap::const_iterator p1 = id_string_map_.find(id);
  CVMLIdStringMap::const_iterator p2 = id_string_map_.end();

  assert(p1 != p2);

  return (*p1).second;
}

void
CVML::
syntaxError()
{
  std::cerr << "Syntax Error at line " << line_num_ << std::endl;

  uint pos = parse_->getPos();

  uint len = line_.size();

  std::cerr << line_ << std::endl;

  for (uint i = 0; i < pos; ++i)
    std::cerr << " ";

  std::cerr << "^";

  for (uint i = pos + 1; i < len; ++i)
    std::cerr << " ";

  std::cerr << std::endl;
}

void
CVML::
setArgumentModeData(CVMLArgument *argument, ushort mode, ushort data, ushort extraData)
{
  switch (mode) {
    case MODE_REGISTER:
      argument->setArgumentRegisterAddr(data);

      break;
    case MODE_REGISTER_DEFERRED:
      argument->setArgumentRegisterValue(data);

      break;
    case MODE_IMMEDIATE: // MODE_AUTO_INCREMENT
      if (data == PC_NUM)
        argument->setArgumentInteger(extraData);
      else
        argument->setArgumentRegisterValueIncr(data);

      break;
    case MODE_DEFERRED_AUTO_INCREMENT:
      argument->setArgumentRegisterValueDeferredIncr(data);

      break;
    case MODE_AUTO_DECREMENT:
      argument->setArgumentRegisterValueDecr(data);

      break;
    case MODE_DEFERRED_AUTO_DECREMENT:
      argument->setArgumentRegisterValueDeferredDecr(data);

      break;
    case MODE_RELATIVE: { // MODE_REGISTER_OFFSET
      ushort offset = unsignedToSigned(extraData);

      if (data == PC_NUM) {
        if      ((extraData + pc_) == INPUT_ADDR)
          argument->setArgumentVariableAddr("input");
        else if ((extraData + pc_) == OUTPUT_ADDR)
          argument->setArgumentVariableAddr("output");
        else
          argument->setArgumentRegisterOffset(data, offset);
      }
      else
        argument->setArgumentRegisterOffset(data, offset);

      break;
    }
    case MODE_REGISTER_OFFSET_DEFERRED: {
      ushort offset = unsignedToSigned(extraData);

      argument->setArgumentRegisterOffsetDeferred(data, offset);

      break;
    }
    default:
      throw CVMLError("Invalid Mode");

      break;
  }
}

CVMLOp *
CVML::
instructionToOp(CVMLDataSource *src)
{
  ushort save_pc = pc_;

  ushort s;

  if (! src->getNextWord(&s))
    return NULL;

  CVMLOpCode *op_code;
  uint        mode1, mode2;
  uint        data1, data2;
  uint        extraData1, extraData2;

  if (! decodeInstruction(src, s, &op_code, &mode1, &data1, &extraData1,
                          &mode2, &data2, &extraData2)) {
    std::cerr << "Illegal Instruction " << CStrUtil::strprintf("%06o", s) << std::endl;
    return NULL;
  }

  CVMLOp *op = new CVMLOp(*this, op_code, save_pc, line_num_);

  if (op->isArgument(0))
    setArgumentModeData(op->getArgument(0), mode1, data1, extraData1);

  if (op->isArgument(1))
    setArgumentModeData(op->getArgument(1), mode2, data2, extraData2);

  return op;
}

bool
CVML::
decodeInstruction(CVMLDataSource *src, ushort i, CVMLOpCode **op_code,
                  uint *mode1, uint *data1, uint *extraData1,
                  uint *mode2, uint *data2, uint *extraData2)
{
  if (! decodeOpCode(src, i, op_code, mode1, data1, extraData1,
                     mode2, data2, extraData2))
    return false;

  return true;
}

bool
CVML::
decodeOpCode(CVMLDataSource *src, ushort i, CVMLOpCode **op_code,
             uint *mode1, uint *data1, uint *extraData1,
             uint *mode2, uint *data2, uint *extraData2)
{
  *mode1 = 0; *data1 = 0; *extraData1 = 0;
  *mode2 = 0; *data2 = 0; *extraData2 = 0;

  if (decodeOpCode2 (src, i, op_code, mode1, data1, extraData1,
                     mode2, data2, extraData2))
    return true;

  if (decodeOpCode1H(src, i, op_code, mode1, data1, extraData1,
                     mode2, data2, extraData2))
    return true;

  if (decodeOpCode1a(src, i, op_code, mode1, data1, extraData1,
                     mode2, data2, extraData2))
    return true;

  if (decodeOpCode1b(src, i, op_code, mode1, data1, extraData1,
                     mode2, data2, extraData2))
    return true;

  if (decodeOpCode0H(src, i, op_code, mode1, data1, extraData1,
                     mode2, data2, extraData2))
    return true;

  if (decodeOpCode0 (src, i, op_code, mode1, data1, extraData1,
                     mode2, data2, extraData2))
    return true;

  return false;
}

bool
CVML::
decodeOpCode2(CVMLDataSource *src, ushort i, CVMLOpCode **op_code,
              uint *mode1, uint *data1, uint *extraData1,
              uint *mode2, uint *data2, uint *extraData2)
{
  *op_code = getOpCode2(i);

  if (*op_code == NULL)
    return false;

  uint arg1 = (i & (*op_code)->data->arg_mask1) >> (*op_code)->data->arg_shift1;

  *mode1 = (arg1 & 070) >> 3;
  *data1 =  arg1 & 007;

  if (! getModeData(src, *mode1, *data1, extraData1))
    return false;

  uint arg2 =  i & (*op_code)->data->arg_mask2;

  *mode2 = (arg2 & 070) >> 3;
  *data2 =  arg2 & 007;

  if (! getModeData(src, *mode2, *data2, extraData2))
    return false;

  return true;
}

bool
CVML::
decodeOpCode1H(CVMLDataSource *src, ushort i, CVMLOpCode **op_code,
               uint *mode1, uint *data1, uint *extraData1,
               uint *mode2, uint *data2, uint *extraData2)
{
  *op_code = getOpCode1H(i);

  if (*op_code == NULL)
    return false;

  uint arg1 = (i & (*op_code)->data->arg_mask1) >> (*op_code)->data->arg_shift1;

  *mode1      = 0;
  *data1      = arg1;
  *extraData1 = 0;

  uint arg2 =  i & (*op_code)->data->arg_mask2;

  *mode2 = (arg2 & 070) >> 3;
  *data2 =  arg2 & 007;

  if (! getModeData(src, *mode2, *data2, extraData2))
    return false;

  return true;
}

bool
CVML::
decodeOpCode1a(CVMLDataSource *, ushort i, CVMLOpCode **op_code,
               uint *mode1, uint *data1, uint *extraData1,
               uint *, uint *, uint *)
{
  *op_code = getOpCode1a(i);

  if (*op_code == NULL)
    return false;

  uint arg1 = i & (*op_code)->data->arg_mask1;

  *mode1      = MODE_RELATIVE;
  *data1      = 7;
  *extraData1 = arg1;

  return true;
}

bool
CVML::
decodeOpCode1b(CVMLDataSource *src, ushort i, CVMLOpCode **op_code,
               uint *mode1, uint *data1, uint *extraData1,
               uint *, uint *, uint *)
{
  *op_code = getOpCode1b(i);

  if (*op_code == NULL)
    return false;

  uint arg1 = i & (*op_code)->data->arg_mask1;

  *mode1 = (arg1 & 070) >> 3;
  *data1 =  arg1 & 007;

  if (! getModeData(src, *mode1, *data1, extraData1))
    return false;

  return true;
}

bool
CVML::
decodeOpCode0H(CVMLDataSource *, ushort i, CVMLOpCode **op_code,
               uint *mode1, uint *data1, uint *extraData1,
               uint *, uint *, uint *)
{
  *op_code = getOpCode0H(i);

  if (*op_code == NULL)
    return false;

  *mode1      = 0;
  *data1      = i & (*op_code)->data->arg_mask1;
  *extraData1 = 0;

  return true;
}

bool
CVML::
decodeOpCode0(CVMLDataSource *, ushort i, CVMLOpCode **op_code,
              uint *, uint *, uint *, uint *, uint *, uint *)
{
  *op_code = getOpCode0(i);

  if (*op_code == NULL)
    return false;

  return true;
}

bool
CVML::
getModeData(CVMLDataSource *src, uint mode, uint data, uint *extraData)
{
  switch (mode) {
    case MODE_REGISTER:
      break;
    case MODE_REGISTER_DEFERRED:
      break;
    case MODE_IMMEDIATE: { // MODE_AUTO_INCREMENT
      if (data == 7) {
        ushort s;

        if (! src->getNextWord(&s))
          return false;

        *extraData = s;
      }

      break;
    }
    case MODE_DEFERRED_AUTO_INCREMENT:
      break;
    case MODE_AUTO_DECREMENT:
      break;
    case MODE_DEFERRED_AUTO_DECREMENT:
      break;
    case MODE_RELATIVE: { // MODE_REGISTER_OFFSET
      ushort s;

      if (! src->getNextWord(&s))
        return false;

      *extraData = s;

      break;
    }
    case MODE_REGISTER_OFFSET_DEFERRED: {
      ushort s;

      if (! src->getNextWord(&s))
        return false;

      *extraData = s;

      break;
    }
    default:
      break;
  }

  return true;
}

CVMLOpCode *
CVML::
getOpCode2(ushort i)
{
  ushort i1 = i & op_code_data_2.instn_mask;

  switch (i1) {
    case OP_CODE_MOV:
      return &op_code_mov;
    case OP_CODE_CMP:
      return &op_code_cmp;
    case OP_CODE_AND:
      return &op_code_and;
    case OP_CODE_XOR:
      return &op_code_xor;
    case OP_CODE_OR:
      return &op_code_or;
    case OP_CODE_ADD:
      return &op_code_add;
    case OP_CODE_MOVB:
      return &op_code_movb;
    case OP_CODE_CMPB:
      return &op_code_cmpb;
    case OP_CODE_ANDB:
      return &op_code_andb;
    case OP_CODE_XORB:
      return &op_code_xorb;
    case OP_CODE_ORB:
      return &op_code_orb;
    case OP_CODE_SUB:
      return &op_code_sub;
    default:
      return NULL;
  }
}

CVMLOpCode *
CVML::
getOpCode1H(ushort i)
{
  ushort i1 = i & op_code_data_1H.instn_mask;

  switch (i1) {
    case OP_CODE_CALL:
      return &op_code_call;
    case OP_CODE_MUL:
      return &op_code_mul;
    case OP_CODE_DIV:
      return &op_code_div;
    case OP_CODE_SLL:
      return &op_code_sll;
    case OP_CODE_SLC:
      return &op_code_slc;
    case OP_CODE_SRL:
      return &op_code_srl;
    case OP_CODE_SRC:
      return &op_code_src;
    default:
      return NULL;
  }
}

CVMLOpCode *
CVML::
getOpCode1a(ushort i)
{
  ushort i1 = i & op_code_data_1a.instn_mask;

  switch (i1) {
    case OP_CODE_BRN:
      return &op_code_brn;
    case OP_CODE_BNE:
      return &op_code_bne;
    case OP_CODE_BEQ:
      return &op_code_beq;
    case OP_CODE_BGE:
      return &op_code_bge;
    case OP_CODE_BLT:
      return &op_code_blt;
    case OP_CODE_BGT:
      return &op_code_bgt;
    case OP_CODE_BLE:
      return &op_code_ble;
    case OP_CODE_BVC:
      return &op_code_bvc;
    case OP_CODE_BVS:
      return &op_code_bvs;
    case OP_CODE_BCC:
      return &op_code_bcc;
    case OP_CODE_BCS:
      return &op_code_bcs;
    default:
      return NULL;
  }
}

CVMLOpCode *
CVML::
getOpCode1b(ushort i)
{
  ushort i1 = i & op_code_data_1b.instn_mask;

  switch (i1) {
    case OP_CODE_NOT:
      return &op_code_not;
    case OP_CODE_NEG:
      return &op_code_neg;
    case OP_CODE_SWAB:
      return &op_code_swab;
    default:
      return NULL;
  }
}

CVMLOpCode *
CVML::
getOpCode0H(ushort i)
{
  ushort i1 = i & op_code_data_0H.instn_mask;

  switch (i1) {
    case OP_CODE_EXIT:
      return &op_code_exit;
    default:
      return NULL;
  }
}

CVMLOpCode *
CVML::
getOpCode0(ushort i)
{
  ushort i1 = i & op_code_data_0.instn_mask;

  switch (i1) {
    case OP_CODE_STOP:
      return &op_code_stop;
    case OP_CODE_NOP:
      return &op_code_nop;
    default:
      return NULL;
  }
}

void
CVML::
setNegativeFlag(bool state)
{
  if (state)
    SET_BIT(memory_[PSW_ADDR], PSW_N_BIT);
  else
    RESET_BIT(memory_[PSW_ADDR], PSW_N_BIT);
}

void
CVML::
setZeroFlag(bool state)
{
  if (state)
    SET_BIT(memory_[PSW_ADDR], PSW_Z_BIT);
  else
    RESET_BIT(memory_[PSW_ADDR], PSW_Z_BIT);
}

void
CVML::
setCarryFlag(bool state)
{
  if (state)
    SET_BIT(memory_[PSW_ADDR], PSW_C_BIT);
  else
    RESET_BIT(memory_[PSW_ADDR], PSW_C_BIT);
}

void
CVML::
setOverflowFlag(bool state)
{
  if (state)
    SET_BIT(memory_[PSW_ADDR], PSW_V_BIT);
  else
    RESET_BIT(memory_[PSW_ADDR], PSW_V_BIT);
}

bool
CVML::
getNegativeFlag()
{
  return IS_BIT(memory_[PSW_ADDR], PSW_N_BIT);
}

bool
CVML::
getZeroFlag()
{
  return IS_BIT(memory_[PSW_ADDR], PSW_Z_BIT);
}

bool
CVML::
getCarryFlag()
{
  return IS_BIT(memory_[PSW_ADDR], PSW_C_BIT);
}

bool
CVML::
getOverflowFlag()
{
  return (memory_[PSW_ADDR] & (1<<PSW_V_BIT)) != 0;
}

void
CVML::
outputString(CFile *file, const std::string &str)
{
  uint len = str.size();

  uint len1 = getWriteStringLen(len)/2;

  for (uint i = 0; i < len1; ++i) {
    ushort value;

    memcpy(&value, &str[i*2], sizeof(ushort));

    outputValue(file, value);
  }
}

void
CVML::
outputString(CFile *file, const char *str, uint len)
{
  uint len1 = getWriteStringLen(len)/2;

  for (uint i = 0; i < len1; ++i) {
    ushort value;

    memcpy(&value, &str[i*2], sizeof(ushort));

    outputValue(file, value);
  }
}

void
CVML::
outputValue(CFile *file, ushort value)
{
  file->write((const char *) &value, sizeof(value));

  pc_ += 2;
}

ushort
CVML::
addOpValue(ushort value, ushort value1, ushort shift1, ushort mask1)
{
  ushort rvalue = value | ((value1 & mask1) << shift1);

  return rvalue;
}

void
CVML::
divideByZero()
{
  std::cerr << "Divide By Zero" << std::endl;
  exit(1);
}

int
CVML::
unsignedToSigned(uint u)
{
  int s;

  if (u & 0x8000000)
    s = -((u ^ 0xFFFFFFF) + 1);
  else
    s = u;

  return s;
}

uint
CVML::
signedToUnsigned(int s)
{
  uint u;

  if (s < 0)
    u = ((-s) ^ 0xFFFFFFFF) + 1;
  else
    u = s;

  return u;
}

short
CVML::
unsignedToSigned(ushort u)
{
  short s;

  if (u & 0x8000)
    s = -((u ^ 0xFFFF) + 1);
  else
    s = u;

  return s;
}

ushort
CVML::
signedToUnsigned(short s)
{
  ushort u;

  if (s < 0)
    u = ((-s) ^ 0xFFFF) + 1;
  else
    u = s;

  return u;
}

char
CVML::
unsignedToSigned(uchar u)
{
  char s;

  if (u & 0x80)
    s = -((u ^ 0xFF) + 1);
  else
    s = u;

  return s;
}

uchar
CVML::
signedToUnsigned(char s)
{
  uchar u;

  if (s < 0)
    u = ((-s) ^ 0xFF) + 1;
  else
    u = s;

  return u;
}
