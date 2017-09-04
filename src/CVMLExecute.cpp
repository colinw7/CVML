#include <CVMLExecute.h>
#include <cstring>
#include <cstdio>

void
CVML::
setExecuteTrace(bool flag)
{
  execute_trace_flag_ = flag;
}

bool
CVML::
execute(CFile *file)
{
  reset();

  initMemory();

  if (! readHeader(file)) {
    std::cerr << "Invalid VML file" << std::endl;
    return false;
  }

  if (! skipStringTable(file)) {
    std::cerr << "Bad String Table" << std::endl;
    return false;
  }

  if (! skipDebug(file)) {
    std::cerr << "Bad Debug Section" << std::endl;
    return false;
  }

  if (! skipData(file)) {
    std::cerr << "Bad Data Section" << std::endl;
    return false;
  }

  if (! loadInstructions(file)) {
    std::cerr << "Bad Code Section" << std::endl;
    return false;
  }

  if (! executeMemory()) {
    std::cerr << "Execute Failed" << std::endl;
    return false;
  }

  return true;
}

void
CVML::
initMemory()
{
  memory_ = new uchar [CVML_MEMORY_MAX + 1];

  memset(memory_, 0, CVML_MEMORY_MAX + 1);
}

bool
CVML::
loadData(CFile *file)
{
  uint size;

  if (! file->read((uchar *) &size, sizeof(size)))
    return false;

  uint size1 = 0;

  uint num_data;

  if (! file->read((uchar *) &num_data, sizeof(num_data)))
    return false;

  size1 += sizeof(num_data);

  for (uint i = 0; i < num_data; ++i) {
    uint id;

    if (! file->read((uchar *) &id, sizeof(id)))
      return false;

    size1 += sizeof(id);

    uint size2;

    if (! CVMLData::readMemory(*this, file, &size2))
      return false;

    size1 += size2;
  }

  if (size1 != size)
    return false;

  return true;
}

bool
CVML::
loadInstructions(CFile *file)
{
  uint size;

  if (! file->read((uchar *) &size, sizeof(size)))
    return false;

  if (! file->read((uchar *) memory_, size))
    return false;

  return true;
}

bool
CVML::
executeMemory()
{
  setRegisterWord(PC_NUM, begin_pc_);
  setRegisterWord(SP_NUM, 0);

  return executeInstructions();
}

bool
CVML::
executeInstructions()
{
  setExecuteStop(false);

  while (! getExecuteStop()) {
    if (! executeNextInstruction())
      return false;
  }

  return true;
}

bool
CVML::
executeNextInstruction()
{
  if (getExecuteStop())
    return true;

  ushort i = nextMemoryInstruction();

  if (! executeInstruction(i))
    return false;

  return true;
}

ushort
CVML::
nextMemoryInstruction()
{
  ushort i = getMemoryWord(getRegisterWord(PC_NUM));

  incRegisterWord(PC_NUM, sizeof(ushort));

  return i;
}

bool
CVML::
executeInstruction(ushort i)
{
  if (executeOpCode2 (i))
    return true;

  if (executeOpCode1H(i))
    return true;

  if (executeOpCode1a(i))
    return true;

  if (executeOpCode1b(i))
    return true;

  if (executeOpCode0H(i))
    return true;

  if (executeOpCode0 (i))
    return true;

  return false;
}

bool
CVML::
executeOpCode2(ushort i)
{
  ushort i1 = i & op_code_data_2.instn_mask;

  CVMLExecuteProc2 proc;
  bool             is_byte;

  if (! getExecuteProc2(i1, &proc, &is_byte))
    return false;

  ushort arg1 = (i & op_code_data_2.arg_mask1) >> op_code_data_2.arg_shift1;
  ushort arg2 =  i & op_code_data_2.arg_mask2;

  ushort mode1 = (arg1 & 070) >> 3;
  ushort data1 =  arg1 & 007;

  ushort mode2 = (arg2 & 070) >> 3;
  ushort data2 =  arg2 & 007;

  ushort extraData1, extraData2;
  ushort processData1, processData2;

  if (! getExecuteData(mode1, data1, &extraData1))
    return false;

  if (! processExecuteData(mode1, data1, extraData1, is_byte, &processData1))
    return false;

  if (! getExecuteData(mode2, data2, &extraData2))
    return false;

  if (! processExecuteData(mode2, data2, extraData2, is_byte, &processData2))
    return false;

  if (! (*proc)(*this, processData1, processData2)) {
    std::cerr << "Command Execute Failed" << std::endl;
    return false;
  }

  return true;
}

bool
CVML::
executeOpCode1H(ushort i)
{
  ushort i1 = i & op_code_data_1H.instn_mask;

  CVMLExecuteProc2 proc;
  bool             is_byte;

  if (! getExecuteProc1H(i1, &proc, &is_byte))
    return false;

  ushort arg1 = (i & op_code_data_1H.arg_mask1) >> op_code_data_1H.arg_shift1;
  ushort arg2 =  i & op_code_data_2.arg_mask2;

  ushort mode1 = 0;
  ushort data1 = arg1;

  ushort mode2 = (arg2 & 070) >> 3;
  ushort data2 =  arg2 & 007;

  ushort extraData1 = 0;

  ushort extraData2;
  ushort processData1, processData2;

  if (! processExecuteData(mode1, data1, extraData1, is_byte, &processData1))
    return false;

  if (! getExecuteData(mode2, data2, &extraData2))
    return false;

  if (! processExecuteData(mode2, data2, extraData2, is_byte, &processData2))
    return false;

  (*proc)(*this, processData1, processData2);

  return true;
}

bool
CVML::
executeOpCode1a(ushort i)
{
  ushort i1 = i & op_code_data_1a.instn_mask;

  CVMLExecuteProc1 proc;
  bool             is_byte;

  if (! getExecuteProc1a(i1, &proc, &is_byte))
    return false;

  ushort arg1 = (i & op_code_data_1a.arg_mask1) >> op_code_data_1a.arg_shift1;

  char arg2 = unsignedToSigned((uchar) arg1);

  ushort processData1 = getRegisterWord(PC_NUM) + 2*arg2;

  (*proc)(*this, processData1);

  return true;
}

bool
CVML::
executeOpCode1b(ushort i)
{
  ushort i1 = i & op_code_data_1b.instn_mask;

  CVMLExecuteProc1 proc;
  bool             is_byte;

  if (! getExecuteProc1b(i1, &proc, &is_byte))
    return false;

  ushort arg1 = (i & op_code_data_1b.arg_mask1) >> op_code_data_1b.arg_shift1;

  ushort mode1 = (arg1 & 070) >> 3;
  ushort data1 =  arg1 & 007;

  ushort extraData1;
  ushort processData1;

  if (! getExecuteData(mode1, data1, &extraData1))
    return false;

  if (! processExecuteData(mode1, data1, extraData1, is_byte, &processData1))
    return false;

  (*proc)(*this, processData1);

  return true;
}

bool
CVML::
executeOpCode0H(ushort i)
{
  ushort i1 = i & op_code_data_0H.instn_mask;

  CVMLExecuteProc1 proc;
  bool             is_byte;

  if (! getExecuteProc0H(i1, &proc, &is_byte))
    return false;

  ushort mode1      = 0;
  ushort data1      = i & op_code_data_0H.arg_mask1;
  ushort extraData1 = 0;

  ushort processData1;

  if (! processExecuteData(mode1, data1, extraData1, is_byte, &processData1))
    return false;

  (*proc)(*this, processData1);

  return true;
}

bool
CVML::
executeOpCode0(ushort i)
{
  CVMLExecuteProc0 proc;
  bool             is_byte;

  if (! getExecuteProc0(i, &proc, &is_byte))
    return false;

  (*proc)(*this);

  return true;
}

bool
CVML::
getExecuteProc2(ushort i, CVMLExecuteProc2 *proc, bool *is_byte)
{
  *is_byte = false;

  switch (i) {
    case OP_CODE_MOV:
      *proc = executeMov;

      return true;
    case OP_CODE_CMP:
      *proc = executeCmp;

      return true;
    case OP_CODE_AND:
      *proc = executeAnd;

      return true;
    case OP_CODE_XOR:
      *proc = executeXor;

      return true;
    case OP_CODE_OR:
      *proc = executeOr;

      return true;
    case OP_CODE_ADD:
      *proc = executeAdd;

      return true;
    case OP_CODE_MOVB:
      *proc    = executeMovb;
      *is_byte = true;

      return true;
    case OP_CODE_CMPB:
      *proc    = executeCmpb;
      *is_byte = true;

      return true;
    case OP_CODE_ANDB:
      *proc    = executeAndb;
      *is_byte = true;

      return true;
    case OP_CODE_XORB:
      *proc    = executeXorb;
      *is_byte = true;

      return true;
    case OP_CODE_ORB:
      *proc    = executeOrb;
      *is_byte = true;

      return true;
    case OP_CODE_SUB:
      *proc = executeSub;

      return true;
    default:
      return false;
  }
}

bool
CVML::
getExecuteProc1H(ushort i, CVMLExecuteProc2 *proc, bool *is_byte)
{
  *is_byte = false;

  switch (i) {
    case OP_CODE_CALL:
      *proc = executeCall;

      return true;
    case OP_CODE_MUL:
      *proc = executeMul;

      return true;
    case OP_CODE_DIV:
      *proc = executeDiv;

      return true;
    case OP_CODE_SLL:
      *proc = executeSll;

      return true;
    case OP_CODE_SLC:
      *proc = executeSlc;

      return true;
    case OP_CODE_SRL:
      *proc = executeSrl;

      return true;
    case OP_CODE_SRC:
      *proc = executeSrc;

      return true;
    default:
      return false;
  }
}

bool
CVML::
getExecuteProc1a(ushort i, CVMLExecuteProc1 *proc, bool *is_byte)
{
  *is_byte = false;

  switch (i) {
    case OP_CODE_BRN:
      *proc = executeBrn;

      return true;
    case OP_CODE_BNE:
      *proc = executeBne;

      return true;
    case OP_CODE_BEQ:
      *proc = executeBeq;

      return true;
    case OP_CODE_BGE:
      *proc = executeBge;

      return true;
    case OP_CODE_BLT:
      *proc = executeBlt;

      return true;
    case OP_CODE_BGT:
      *proc = executeBgt;

      return true;
    case OP_CODE_BLE:
      *proc = executeBle;

      return true;
    case OP_CODE_BVC:
      *proc = executeBvc;

      return true;
    case OP_CODE_BVS:
      *proc = executeBvs;

      return true;
    case OP_CODE_BCC:
      *proc = executeBcc;

      return true;
    case OP_CODE_BCS:
      *proc = executeBcs;

      return true;
    default:
      return false;
  }
}

bool
CVML::
getExecuteProc1b(ushort i, CVMLExecuteProc1 *proc, bool *is_byte)
{
  *is_byte = false;

  switch (i) {
    case OP_CODE_NOT:
      *proc = executeNot;

      return true;
    case OP_CODE_NEG:
      *proc = executeNeg;

      return true;
    case OP_CODE_SWAB:
      *proc    = executeSwab;
      *is_byte = false;

      return true;
    default:
      return false;
  }
}

bool
CVML::
getExecuteProc0H(ushort i, CVMLExecuteProc1 *proc, bool *is_byte)
{
  *is_byte = false;

  switch (i) {
    case OP_CODE_EXIT:
      *proc = executeExit;

      return true;
    default:
      return false;
  }
}

bool
CVML::
getExecuteProc0(ushort i, CVMLExecuteProc0 *proc, bool *is_byte)
{
  *is_byte = false;

  switch (i) {
    case OP_CODE_STOP:
      *proc = executeStop;

      return true;
    case OP_CODE_NOP:
      *proc = executeNop;

      return true;
    default:
      return false;
  }
}

bool
CVML::
getExecuteData(ushort mode, ushort data, ushort *extraData)
{
  switch (mode) {
    case MODE_REGISTER:
    case MODE_REGISTER_DEFERRED:
    case MODE_IMMEDIATE: // MODE_AUTO_INCREMENT
      if (data == PC_NUM) {
        *extraData = getRegisterWord(PC_NUM);

        incRegisterWord(PC_NUM, sizeof(ushort));
      }

      break;
    case MODE_DEFERRED_AUTO_INCREMENT:
    case MODE_AUTO_DECREMENT:
    case MODE_DEFERRED_AUTO_DECREMENT:
      break;
    case MODE_RELATIVE: // MODE_REGISTER_OFFSET
      *extraData = getMemoryWord(getRegisterWord(PC_NUM));

      incRegisterWord(PC_NUM, sizeof(ushort));

      break;
    case MODE_REGISTER_OFFSET_DEFERRED:
      *extraData = getMemoryWord(getRegisterWord(PC_NUM));

      incRegisterWord(PC_NUM, sizeof(ushort));

      break;
    default:
      return false;
  }

  return true;
}

bool
CVML::
processExecuteData(ushort mode, ushort data, ushort extraData, bool is_byte, ushort *processData)
{
  switch (mode) {
    case MODE_REGISTER: {
      *processData = getRegisterAddress(data);

      break;
    }
    case MODE_REGISTER_DEFERRED: {
      ushort addr  = getRegisterAddress(data);
      ushort value = getMemoryWord(addr);

      *processData = value;

      break;
    }
    case MODE_IMMEDIATE: { // MODE_AUTO_INCREMENT
      if (data == PC_NUM)
        *processData = extraData;
      else {
        ushort addr  = getRegisterAddress(data);
        ushort value = getMemoryWord(addr);

        *processData = value;

        if (is_byte)
          ++value;
        else
          value += 2;

        setMemoryWord(addr, value);
      }

      break;
    }
    case MODE_DEFERRED_AUTO_INCREMENT: {
      ushort addr  = getRegisterAddress(data);
      ushort value = getMemoryWord(getMemoryWord(addr));

      *processData = value;

      if (is_byte)
        ++value;
      else
        value += 2;

      setMemoryWord(addr, value);
    }
    case MODE_AUTO_DECREMENT: {
      ushort addr  = getRegisterAddress(data);
      ushort value = getMemoryWord(addr);

      if (is_byte)
        --value;
      else
        value -= 2;

      *processData = value;

      setMemoryWord(addr, value);

      break;
    }
    case MODE_DEFERRED_AUTO_DECREMENT: {
      ushort addr  = getRegisterAddress(data);
      ushort value = getMemoryWord(getMemoryWord(addr));

      if (is_byte)
        --value;
      else
        value -= 2;

      *processData = value;

      setMemoryWord(addr, value);

      break;
    }
    case MODE_RELATIVE: { // MODE_REGISTER_OFFSET
      ushort addr = getRegisterWord(data);

      *processData = addr + extraData;

      break;
    }
    case MODE_REGISTER_OFFSET_DEFERRED: {
      ushort addr = getRegisterWord(data);

      *processData = getMemoryWord(addr + extraData);

      break;
    }
    default:
      return false;
  }

  return true;
}

bool
CVML::
executeMov(CVML &vml, ushort data1, ushort data2)
{
  ushort value = vml.getMemoryWord(data1);

  if (vml.execute_trace_flag_)
    vml.traceMessage("mov @%06o, %hd", data2, value);

  vml.setMemoryWord(data2, value);

  return true;
}

bool
CVML::
executeCmp(CVML &vml, ushort data1, ushort data2)
{
  ushort uvalue1 = vml.getMemoryWord(data1);
  ushort uvalue2 = vml.getMemoryWord(data2);

  short svalue1 = vml.unsignedToSigned(uvalue1);
  short svalue2 = vml.unsignedToSigned(uvalue2);

  int result = svalue1 - svalue2;

  vml.setZeroFlag    (result == 0);
  vml.setNegativeFlag(result <  0);

  if (vml.execute_trace_flag_)
    vml.traceMessage("cmp %hd,%hd = %d", svalue1, svalue2, result);

  return true;
}

bool
CVML::
executeAnd(CVML &vml, ushort data1, ushort data2)
{
  ushort value1 = vml.getMemoryWord(data1);
  ushort value2 = vml.getMemoryWord(data2);

  ushort result = value1 & value2;

  vml.setMemoryWord(data2, result);

  if (vml.execute_trace_flag_)
    vml.traceMessage("and %hd,%hd; mov @%06o, %hd", value1, value2, data2, result);

  return true;
}

bool
CVML::
executeXor(CVML &vml, ushort data1, ushort data2)
{
  ushort value1 = vml.getMemoryWord(data1);
  ushort value2 = vml.getMemoryWord(data2);

  ushort result = value1 ^ value2;

  vml.setMemoryWord(data2, result);

  if (vml.execute_trace_flag_)
    vml.traceMessage("xor %hd,%hd; mov @%06o, %hd", value1, value2, data2, result);

  return true;
}

bool
CVML::
executeOr(CVML &vml, ushort data1, ushort data2)
{
  ushort value1 = vml.getMemoryWord(data1);
  ushort value2 = vml.getMemoryWord(data2);

  ushort result = value1 | value2;

  vml.setMemoryWord(data2, result);

  if (vml.execute_trace_flag_)
    vml.traceMessage("or %hd,%hd; mov @%06o, %hd", value1, value2, data2, result);

  return true;
}

// Sets CZVN

bool
CVML::
executeAdd(CVML &vml, ushort data1, ushort data2)
{
  ushort uvalue1 = vml.getMemoryWord(data1);
  ushort uvalue2 = vml.getMemoryWord(data2);

  uint uresult = uvalue1 + uvalue2;

  short svalue1 = vml.unsignedToSigned(uvalue1);
  short svalue2 = vml.unsignedToSigned(uvalue2);

  int sresult = svalue1 + svalue2;

  vml.setCarryFlag   (uresult > 65535);
  vml.setZeroFlag    (sresult == 0);
  vml.setNegativeFlag(sresult < 0);
  vml.setOverflowFlag(sresult < -32768 || sresult > 32767);

  ushort result = uresult & 0xFFFF;

  vml.setMemoryWord(data2, result);

  if (vml.execute_trace_flag_)
    vml.traceMessage("add %hd,%hd; mov @%06o, %hd", uvalue1, uvalue2, data2, result);

  return true;
}

bool
CVML::
executeMovb(CVML &vml, ushort data1, ushort data2)
{
  uchar value = vml.getMemoryByte(data1);

  vml.setMemoryByte(data2, value);

  if (vml.execute_trace_flag_)
    vml.traceMessage("movb @%06o = %hhd", data2, value);

  return true;
}

bool
CVML::
executeCmpb(CVML &vml, ushort data1, ushort data2)
{
  uchar uvalue1 = vml.getMemoryByte(data1);
  uchar uvalue2 = vml.getMemoryByte(data2);

  char svalue1 = vml.unsignedToSigned(uvalue1);
  char svalue2 = vml.unsignedToSigned(uvalue2);

  int result = svalue1 - svalue2;

  vml.setZeroFlag    (result == 0);
  vml.setNegativeFlag(result <  0);

  if (vml.execute_trace_flag_)
    vml.traceMessage("cmpb %hhd,%hhd = %d", svalue1, svalue2, result);

  return true;
}

bool
CVML::
executeAndb(CVML &vml, ushort data1, ushort data2)
{
  uchar value1 = vml.getMemoryByte(data1);
  uchar value2 = vml.getMemoryByte(data2);

  uchar result = value1 & value2;

  vml.setMemoryByte(data2, result);

  if (vml.execute_trace_flag_)
    vml.traceMessage("andb %hhd,%hhd; mov @%06o, %hhd", value1, value2, data2, result);

  return true;
}

bool
CVML::
executeXorb(CVML &vml, ushort data1, ushort data2)
{
  uchar value1 = vml.getMemoryByte(data1);
  uchar value2 = vml.getMemoryByte(data2);

  uchar result = value1 ^ value2;

  vml.setMemoryByte(data2, result);

  if (vml.execute_trace_flag_)
    vml.traceMessage("xorb %hhd,%hhd; mov @%06o, %hhd", value1, value2, data2, result);

  return true;
}

bool
CVML::
executeOrb(CVML &vml, ushort data1, ushort data2)
{
  uchar value1 = vml.getMemoryByte(data1);
  uchar value2 = vml.getMemoryByte(data2);

  uchar result = value1 | value2;

  vml.setMemoryByte(data2, result);

  if (vml.execute_trace_flag_)
    vml.traceMessage("orb %hhd,%hhd; mov @%06o, %hhd", value1, value2, data2, result);

  return true;
}

bool
CVML::
executeSub(CVML &vml, ushort data1, ushort data2)
{
  ushort uvalue1 = vml.getMemoryWord(data1);
  ushort uvalue2 = vml.getMemoryWord(data2);

  uint uresult = uvalue2 - uvalue1;

  short svalue1 = vml.unsignedToSigned(uvalue1);
  short svalue2 = vml.unsignedToSigned(uvalue2);

  int sresult = svalue2 - svalue1;

  vml.setCarryFlag   (uresult > 65535);
  vml.setZeroFlag    (sresult == 0);
  vml.setNegativeFlag(sresult < 0);
  vml.setOverflowFlag(sresult < -32768 || sresult > 32767);

  ushort result = uresult & 0xFFFF;

  vml.setMemoryWord(data2, result);

  if (vml.execute_trace_flag_)
    vml.traceMessage("sub %hd,%hd; mov @%06o, %hd", uvalue1, uvalue2, data2, result);

  return true;
}

bool
CVML::
executeCall(CVML &vml, ushort data1, ushort data2)
{
  vml.stackPush(vml.getMemoryWord(data1));

  vml.setMemoryWord(data1, vml.getRegisterWord(PC_NUM));

  vml.setRegisterWord(PC_NUM, data2);

  if (vml.execute_trace_flag_)
    vml.traceMessage("call %hd", data2);

  return true;
}

bool
CVML::
executeMul(CVML &vml, ushort data1, ushort data2)
{
  ushort uvalue1 = vml.getMemoryWord(data1);
  ushort uvalue2 = vml.getMemoryWord(data2);

  uint uresult = uvalue1 * uvalue2;

  short svalue1 = vml.unsignedToSigned(uvalue1);
  short svalue2 = vml.unsignedToSigned(uvalue2);

  int sresult = svalue1 * svalue2;

  vml.setCarryFlag   (uresult > 65535);
  vml.setZeroFlag    (sresult == 0);
  vml.setNegativeFlag(sresult < 0);
  vml.setOverflowFlag(sresult < -32768 || sresult > 32767);

  uint uresult1 =  uresult        & 0xFFFF;
  uint uresult2 = (uresult >> 16) & 0xFFFF;

  vml.setMemoryWord(data1, uresult1);

  if (vml.execute_trace_flag_)
    vml.traceMessage("mul %hd,%hd; mov @%06o, %hd",
                     uvalue1, uvalue2, data1, uresult1);

  if (vml.isEvenRegisterAddress(data1)) {
    vml.setMemoryWord(data1 + 2, uresult2);

    if (vml.execute_trace_flag_)
      vml.traceMessage("mov @%06o, %hd", data1 + 2, uresult2);
  }

  return true;
}

bool
CVML::
executeDiv(CVML &vml, ushort data1, ushort data2)
{
  ushort value1 = vml.getMemoryWord(data1);
  ushort value2 = vml.getMemoryWord(data2);

  if (value2 == 0) {
    vml.divideByZero();

    return false;
  }

  ushort result1 = value1 / value2;
  ushort result2 = value1 % value2;

  vml.setMemoryWord(data1, result1);

  if (vml.execute_trace_flag_)
    vml.traceMessage("mul %hd,%hd; mov @%06o, %hd", value1, value2, data1, result1);

  if (vml.isEvenRegisterAddress(data1)) {
    vml.setMemoryWord(data1 + 2, result2);

    if (vml.execute_trace_flag_)
      vml.traceMessage("mul %hd,%hd; mov @%06o, %hd", value1, value2, data1 + 2, result2);
  }

  return true;
}

bool
CVML::
executeSll(CVML &vml, ushort data1, ushort data2)
{
  ushort value1 = vml.getMemoryWord(data1);
  ushort value2 = vml.getMemoryWord(data2);

  ushort result = value1 << value2;

  vml.setMemoryWord(data1, result);

  if (vml.execute_trace_flag_)
    vml.traceMessage("sll %hd,%hd; mov @%06o, %hd", value1, value2, data1, result);

  return true;
}

bool
CVML::
executeSlc(CVML &vml, ushort data1, ushort data2)
{
  ushort value1 = vml.getMemoryWord(data1);
  ushort value2 = vml.getMemoryWord(data2);

  ushort carry = 0;

  for (ushort i = 0; i < value2; ++i) {
    carry <<= 1;

    if (value1 & 0x8000)
      carry |= 1;

    value1 <<= 1;
  }

  ushort result = value1 | carry;

  vml.setMemoryWord(data1, result);

  if (vml.execute_trace_flag_)
    vml.traceMessage("slc %hd,%hd; mov @%06o, %hd", value1, value2, data1, result);

  return true;
}

bool
CVML::
executeSrl(CVML &vml, ushort data1, ushort data2)
{
  ushort value1 = vml.getMemoryWord(data1);
  ushort value2 = vml.getMemoryWord(data2);

  ushort result = value1 >> value2;

  vml.setMemoryWord(data1, result);

  if (vml.execute_trace_flag_)
    vml.traceMessage("srl %hd,%hd; mov @%06o, %hd", value1, value2, data1, result);

  return true;
}

bool
CVML::
executeSrc(CVML &vml, ushort data1, ushort data2)
{
  ushort value1 = vml.getMemoryWord(data1);
  ushort value2 = vml.getMemoryWord(data2);

  ushort carry = 0;

  for (ushort i = 0; i < value2; ++i) {
    carry >>= 1;

    if (value1 & 0x0001)
      carry |= 0x8000;

    value1 >>= 1;
  }

  ushort result = value1 | carry;

  vml.setMemoryWord(data1, result);

  if (vml.execute_trace_flag_)
    vml.traceMessage("src %hd,%hd; mov @%06o, %hd", value1, value2, data1, result);

  return true;
}

bool
CVML::
executeBrn(CVML &vml, ushort data1)
{
  vml.setRegisterWord(PC_NUM, data1);

  if (vml.execute_trace_flag_)
    vml.traceMessage("brn %hd", data1);

  return true;
}

bool
CVML::
executeBne(CVML &vml, ushort data1)
{
  bool flag = ! vml.getZeroFlag();

  if (flag)
    vml.setRegisterWord(PC_NUM, data1);

  if (vml.execute_trace_flag_)
    vml.traceMessage("bne %hd (%d)", data1, int(flag));

  return true;
}

bool
CVML::
executeBeq(CVML &vml, ushort data1)
{
  bool flag = vml.getZeroFlag();

  if (flag)
    vml.setRegisterWord(PC_NUM, data1);

  if (vml.execute_trace_flag_)
    vml.traceMessage("beq %hd (%d)", data1, int(flag));

  return true;
}

bool
CVML::
executeBge(CVML &vml, ushort data1)
{
  bool flag = ! vml.getNegativeFlag();

  if (flag)
    vml.setRegisterWord(PC_NUM, data1);

  if (vml.execute_trace_flag_)
    vml.traceMessage("bge %hd (%d)", data1, int(flag));

  return true;
}

bool
CVML::
executeBlt(CVML &vml, ushort data1)
{
  bool flag = vml.getNegativeFlag();

  if (flag)
    vml.setRegisterWord(PC_NUM, data1);

  if (vml.execute_trace_flag_)
    vml.traceMessage("blt %hd (%d)", data1, int(flag));

  return true;
}

bool
CVML::
executeBgt(CVML &vml, ushort data1)
{
  bool flag = ! vml.getNegativeFlag() && ! vml.getZeroFlag();

  if (flag)
    vml.setRegisterWord(PC_NUM, data1);

  if (vml.execute_trace_flag_)
    vml.traceMessage("bgt %hd (%d)", data1, int(flag));

  return true;
}

bool
CVML::
executeBle(CVML &vml, ushort data1)
{
  bool flag = vml.getNegativeFlag() || vml.getZeroFlag();

  if (flag)
    vml.setRegisterWord(PC_NUM, data1);

  if (vml.execute_trace_flag_)
    vml.traceMessage("ble %hd (%d)", data1, int(flag));

  return true;
}

bool
CVML::
executeBvc(CVML &vml, ushort data1)
{
  bool flag = ! vml.getOverflowFlag();

  if (flag)
    vml.setRegisterWord(PC_NUM, data1);

  if (vml.execute_trace_flag_)
    vml.traceMessage("bvc %hd (%d)", data1, int(flag));

  return true;
}

bool
CVML::
executeBvs(CVML &vml, ushort data1)
{
  bool flag = vml.getOverflowFlag();

  if (flag)
    vml.setRegisterWord(PC_NUM, data1);

  if (vml.execute_trace_flag_)
    vml.traceMessage("bvs %hd (%d)", data1, int(flag));

  return true;
}

bool
CVML::
executeBcc(CVML &vml, ushort data1)
{
  bool flag = ! vml.getCarryFlag();

  if (flag)
    vml.setRegisterWord(PC_NUM, data1);

  if (vml.execute_trace_flag_)
    vml.traceMessage("bcc %hd (%d)", data1, int(flag));

  return true;
}

bool
CVML::
executeBcs(CVML &vml, ushort data1)
{
  bool flag = vml.getCarryFlag();

  if (flag)
    vml.setRegisterWord(PC_NUM, data1);

  if (vml.execute_trace_flag_)
    vml.traceMessage("bcs %hd (%d)", data1, int(flag));

  return true;
}

bool
CVML::
executeSwab(CVML &vml, ushort data1)
{
  ushort value = vml.getMemoryWord(data1);

  ushort value1 =  value & 0x00FF;
  ushort value2 = (value & 0xFF00) >> 8;

  ushort result = (value1 << 8) | value2;

  vml.setMemoryWord(data1, result);

  if (vml.execute_trace_flag_)
    vml.traceMessage("swab %hd,%hd; mov @%06o,%hd", value1, value2, data1, result);

  return true;
}

bool
CVML::
executeNot(CVML &vml, ushort data1)
{
  ushort value = vml.getMemoryWord(data1);

  ushort result = ~value;

  vml.setMemoryWord(data1, result);

  if (vml.execute_trace_flag_)
    vml.traceMessage("not %hd; mov @%06o,%hd", value, data1, result);

  return true;
}

bool
CVML::
executeNeg(CVML &vml, ushort data1)
{
  ushort value = vml.getMemoryWord(data1);

  short value1 = -vml.unsignedToSigned(value);

  ushort result = vml.signedToUnsigned(value1);

  vml.setMemoryWord(data1, result);

  if (vml.execute_trace_flag_)
    vml.traceMessage("not %hd; mov @%06o,%hd", value, data1, result);

  return true;
}

bool
CVML::
executeExit(CVML &vml, ushort data1)
{
  ushort value = vml.getMemoryWord(data1);

  vml.setRegisterWord(PC_NUM, value);

  ushort value1 = vml.stackPop();

  vml.setMemoryWord(data1, value1);

  if (vml.execute_trace_flag_)
    vml.traceMessage("exit @%06o", value);

  return true;
}

bool
CVML::
executeNop(CVML &vml)
{
  if (vml.execute_trace_flag_)
    vml.traceMessage("nop");

  return true;
}

bool
CVML::
executeStop(CVML &vml)
{
  vml.setExecuteStop(true);

  if (vml.execute_trace_flag_)
    vml.traceMessage("stop");

  return true;
}

ushort
CVML::
getRegisterWord(ushort reg_num)
{
  return getMemoryWord(getRegisterAddress(reg_num));
}

void
CVML::
setRegisterWord(ushort reg_num, ushort value)
{
  setMemoryWord(getRegisterAddress(reg_num), value);
}

void
CVML::
incRegisterWord(ushort reg_num, ushort value)
{
  ushort addr = getRegisterAddress(reg_num);

  setMemoryWord(addr, getMemoryWord(addr) + value);
}

uchar
CVML::
getMemoryByte(ushort addr)
{
  uchar value;

  if (addr == INPUT_ADDR) {
    CReadLine readline;

    readline.setPrompt("?");

    std::string line = readline.readLine();

    memory_[addr] = line[0];
  }

  memcpy(&value, &memory_[addr], sizeof(value));

  return value;
}

ushort
CVML::
getMemoryWord(ushort addr)
{
  ushort value;

  if (addr == INPUT_ADDR) {
    CReadLine readline;

    readline.setPrompt("?> ");

    std::string line = readline.readLine();

    short value;

    if (CStrUtil::toInteger(line, &value))
      memcpy(&memory_[addr], &value, sizeof(value));
    else {
      memory_[addr + 0] = line[0];
      memory_[addr + 1] = line[1];
    }
  }

  memcpy(&value, &memory_[addr], sizeof(value));

  return value;
}

void
CVML::
setMemoryByte(ushort addr, uchar value)
{
  memcpy(&memory_[addr], &value, sizeof(value));

  if (addr == OUTPUT_ADDR)
    fputc(memory_[addr], stdout);

  if (addr == PC_ADDR)
    pc_ = getMemoryWord(PC_ADDR);
}

void
CVML::
setMemoryWord(ushort addr, ushort value)
{
  memcpy(&memory_[addr], &value, sizeof(value));

  if (addr == OUTPUT_ADDR)
    printf("\t%hd", value);

  if (addr == PC_ADDR)
    pc_ = value;
}

void
CVML::
setMemoryString(ushort addr, const std::string &str)
{
  const char *c_str = str.c_str();
  int         len   = str.size();

  memcpy(&memory_[addr], c_str, len);
}

void
CVML::
traceMessage(const char *format, ...)
{
  std::string str;
  va_list     vargs;

  va_start(vargs, format);

  CStrUtil::vsprintf(str, format, &vargs);

  va_end(vargs);

  std::cerr << CStrUtil::strprintf("%06o", getRegisterWord(PC_NUM)) << ":" << str << std::endl;
}

void
CVML::
stackPush(ushort data)
{
  ushort sp_addr = getRegisterWord(SP_NUM);

  sp_addr -= 2;

  setMemoryWord(sp_addr, data);

  setRegisterWord(SP_NUM, sp_addr);
}

ushort
CVML::
stackPop()
{
  ushort sp_addr = getRegisterWord(SP_NUM);

  ushort data = getMemoryWord(sp_addr);

  sp_addr += 2;

  setRegisterWord(SP_NUM, sp_addr);

  return data;
}
