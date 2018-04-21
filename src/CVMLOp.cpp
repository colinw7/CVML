#include <CVMLI.h>
#include <cstring>

bool
CVMLOp::
equal(const CVMLOp &op)
{
  if (op_code_ != op.op_code_)
    return false;

  for (uint i = 0; i < num_arguments_; ++i)
    if (! arguments_[0]->equal(*op.arguments_[0]))
      return false;

  return true;
}

uint
CVMLOp::
getAddressLen()
{
  uint len = 2;

  if      (op_code_->data->type == OP_ARG_2) {
    len += arguments_[0]->getAddressLen();
    len += arguments_[1]->getAddressLen();
  }
  else if (op_code_->data->type == OP_ARG_1H) {
    len += arguments_[0]->getAddressLen();
    len += arguments_[1]->getAddressLen();
  }
  else if (op_code_->data->type == OP_ARG_0H) {
    len += arguments_[0]->getAddressLen();
  }

  return len;
}

void
CVMLOp::
encode(CFile *file)
{
  if (vml_.getPC() > pc_) return;

  ushort extraValue1, extraValue2;
  bool                extraValueFlag1, extraValueFlag2;

  while (vml_.getPC() < pc_) {
    CVMLData *data = vml_.lookupDataByPC(vml_.getPC());

    if (data != NULL)
      data->outputValues(file);
    else
      vml_.outputValue(file, OP_CODE_NOP);
  }

  ushort value = getValue(&extraValue1, &extraValueFlag1, &extraValue2, &extraValueFlag2);

  vml_.outputValue(file, value);

  if (extraValueFlag1)
    vml_.outputValue(file, extraValue1);

  if (extraValueFlag2)
    vml_.outputValue(file, extraValue2);
}

uint
CVMLOp::
getEncodeSize()
{
  if (vml_.getPC() > pc_) return 0;

  uint size = 0;

  ushort extraValue1, extraValue2;
  bool                extraValueFlag1, extraValueFlag2;

  while (vml_.getPC() < pc_) {
    size += sizeof(short);

    vml_.addPC(2);
  }

  ushort value = getValue(&extraValue1, &extraValueFlag1, &extraValue2, &extraValueFlag2);

  size += sizeof(value);

  vml_.addPC(2);

  if (extraValueFlag1) {
    size += sizeof(short);

    vml_.addPC(2);
  }

  if (extraValueFlag2) {
    size += sizeof(short);

    vml_.addPC(2);
  }

  return size;
}

ushort
CVMLOp::
getValue(ushort *extraValue1, bool *extraValueFlag1, ushort *extraValue2, bool *extraValueFlag2)
{
  ushort value = op_code_->code;

  *extraValueFlag1 = false;
  *extraValueFlag2 = false;

  if      (op_code_->data->type == OP_ARG_2) {
    ushort offset = 2;

    if (arguments_[0]->getArgExtraValue())
      offset += 2;

    ushort value1 =
      arguments_[0]->getArgValue(extraValue1, extraValueFlag1, offset);

    if (arguments_[1]->getArgExtraValue())
      offset += 2;

    ushort value2 =
      arguments_[1]->getArgValue(extraValue2, extraValueFlag2, offset);

    ushort shift1, mask1;
    ushort shift2, mask2;

    CUtil::decodeMask(op_code_->data->arg_mask1, &shift1, &mask1);
    CUtil::decodeMask(op_code_->data->arg_mask2, &shift2, &mask2);

    value = vml_.addOpValue(value, value1, shift1, mask1);
    value = vml_.addOpValue(value, value2, shift2, mask2);
  }
  else if (op_code_->data->type == OP_ARG_1H) {
    ushort offset = 2;

    if (arguments_[0]->getArgExtraValue())
      offset += 2;

    ushort value1 =
      arguments_[0]->getArgValue(extraValue1, extraValueFlag1, offset);

    if (arguments_[1]->getArgExtraValue())
      offset += 2;

    ushort value2 =
      arguments_[1]->getArgValue(extraValue2, extraValueFlag2, offset);

    ushort shift1, mask1;
    ushort shift2, mask2;

    CUtil::decodeMask(op_code_->data->arg_mask1, &shift1, &mask1);
    CUtil::decodeMask(op_code_->data->arg_mask2, &shift2, &mask2);

    value = vml_.addOpValue(value, value1, shift1, mask1);
    value = vml_.addOpValue(value, value2, shift2, mask2);
  }
  else if (op_code_->data->type == OP_ARG_1a) {
    ushort offset = 2;

    (void)
      arguments_[0]->getArgValue(extraValue1, extraValueFlag1, offset);

    *extraValueFlag1 = false;

    ushort shift1, mask1;

    CUtil::decodeMask(op_code_->data->arg_mask1, &shift1, &mask1);

    short s = *(short *) extraValue1;

    s /= 2;

    ushort s1 = CMathGen::twosCompliment(s);

    value = vml_.addOpValue(value, s1, shift1, mask1);
  }
  else if (op_code_->data->type == OP_ARG_1b) {
    ushort offset = 2;

    if (arguments_[0]->getArgExtraValue())
      offset += 2;

    ushort value1 =
      arguments_[0]->getArgValue(extraValue1, extraValueFlag1, offset);

    ushort shift1, mask1;

    CUtil::decodeMask(op_code_->data->arg_mask1, &shift1, &mask1);

    value = vml_.addOpValue(value, value1, shift1, mask1);
  }
  else if (op_code_->data->type == OP_ARG_0H) {
    ushort offset = 2;

    if (arguments_[0]->getArgExtraValue())
      offset += 2;

    ushort value1 =
      arguments_[0]->getArgValue(extraValue1, extraValueFlag1, offset);

    ushort shift1, mask1;

    CUtil::decodeMask(op_code_->data->arg_mask1, &shift1, &mask1);

    value = vml_.addOpValue(value, value1, shift1, mask1);
  }

  return value;
}

void
CVMLOp::
print(std::ostream &os)
{
  printData(os, true, 8);
}

void
CVMLOp::
printData(std::ostream &os, bool show_code, int indent)
{
  ushort extraValue1, extraValue2;
  bool   extraValueFlag1, extraValueFlag2;

  vml_.setPC(pc_);

  ushort value = getValue(&extraValue1, &extraValueFlag1, &extraValue2, &extraValueFlag2);

  os << CStrUtil::strprintf("%06o:%06o", pc_, value);

  if (show_code) {
    os << "  ; ";

    printCode(os);
  }

  uint pc = pc_;

  if (extraValueFlag1) {
    pc += 2;

    os << std::endl;

    for (int i = 0; i < indent; ++i)
      os << ' ';

    os << CStrUtil::strprintf("%06o:%06o", pc, extraValue1);
  }

  if (extraValueFlag2) {
    pc += 2;

    os << std::endl;

    for (int i = 0; i < indent; ++i)
      os << ' ';

    os << CStrUtil::strprintf("%06o:%06o", pc, extraValue2);
  }
}

void
CVMLOp::
printCode(std::ostream &os)
{
  int len = 5;

  os << op_code_->name;

  len -= strlen(op_code_->name);

  for (int i = 0; i < len; ++i)
    os << " ";

  if (num_arguments_ >= 1) {
    arguments_[0]->print(os, op_code_->data->arg_signed1, op_code_->data->arg_bits1);

    if (num_arguments_ >= 2) {
      os << ",";

      arguments_[1]->print(os, op_code_->data->arg_signed2, op_code_->data->arg_bits2);
    }
  }
}
