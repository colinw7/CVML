#include <CVMLI.h>
#include <cstring>
#include <cassert>

CVMLArgument::
CVMLArgument(CVML *vml) :
 vml_(vml), type_(CVML_ARGUMENT_NONE)
{
  memset(&data_, 0, sizeof(CVMLArgumentData));
}

CVMLArgument::
CVMLArgument(const CVMLArgument &arg) :
 vml_(arg.vml_)
{
  set(arg);
}

CVMLArgument::
~CVMLArgument()
{
  reset();
}

CVMLArgument &
CVMLArgument::
operator=(const CVMLArgument &arg)
{
  reset();

  set(arg);

  return *this;
}

void
CVMLArgument::
reset()
{
  if      (type_ == CVML_ARGUMENT_VARIABLE_ADDR ||
           type_ == CVML_ARGUMENT_VARIABLE_VALUE)
    delete [] data_.name;
  else if (type_ == CVML_ARGUMENT_REGISTER_VAR_OFFSET ||
           type_ == CVML_ARGUMENT_REGISTER_VAR_OFFSET_DEFERRED)
    delete [] data_.offset_var_reg.name;

  type_ = CVML_ARGUMENT_NONE;
}

void
CVMLArgument::
set(const CVMLArgument &arg)
{
  assert(vml_ == arg.vml_);

  type_ = arg.type_;

  memcpy(&data_, &arg.data_, sizeof(CVMLArgumentData));

  if      (arg.type_ == CVML_ARGUMENT_VARIABLE_ADDR ||
           arg.type_ == CVML_ARGUMENT_VARIABLE_VALUE)
    data_.name = CStrUtil::strdup(arg.data_.name);
  else if (arg.type_ == CVML_ARGUMENT_REGISTER_VAR_OFFSET ||
           arg.type_ == CVML_ARGUMENT_REGISTER_VAR_OFFSET_DEFERRED)
    data_.offset_var_reg.name = CStrUtil::strdup(arg.data_.offset_var_reg.name);
}

bool
CVMLArgument::
equal(const CVMLArgument &arg)
{
  if (type_ != arg.type_)
    return false;

  switch (type_) {
    case CVML_ARGUMENT_CHAR:
      if (data_.c != arg.data_.c)
        return false;

      break;
    case CVML_ARGUMENT_INTEGER:
      if (data_.integer != arg.data_.integer)
        return false;

      break;
    case CVML_ARGUMENT_REGISTER_ADDR:
    case CVML_ARGUMENT_REGISTER_VALUE:
    case CVML_ARGUMENT_REGISTER_VALUE_INCR:
    case CVML_ARGUMENT_REGISTER_VALUE_DECR:
      if (data_.reg_num != arg.data_.reg_num)
        return false;

      break;
    case CVML_ARGUMENT_VARIABLE_ADDR:
    case CVML_ARGUMENT_VARIABLE_VALUE:
      if (strcmp(data_.name, arg.data_.name) != 0)
        return false;

      break;
    case CVML_ARGUMENT_REGISTER_OFFSET:
    case CVML_ARGUMENT_REGISTER_OFFSET_DEFERRED:
      if (data_.offset_reg.reg_num != arg.data_.offset_reg.reg_num ||
          data_.offset_reg.offset  != arg.data_.offset_reg.offset)
        return false;

      break;
    case CVML_ARGUMENT_REGISTER_VAR_OFFSET:
    case CVML_ARGUMENT_REGISTER_VAR_OFFSET_DEFERRED:
      if (data_.offset_var_reg.reg_num != arg.data_.offset_var_reg.reg_num ||
          strcmp(data_.offset_var_reg.name, arg.data_.offset_var_reg.name) != 0)
        return false;

      break;
    default:
      break;
  }

  return true;
}

void
CVMLArgument::
setArgumentChar(uchar c)
{
  reset();

  type_   = CVML_ARGUMENT_CHAR;
  data_.c = c;
}

void
CVMLArgument::
setArgumentInteger(ushort integer)
{
  reset();

  type_         = CVML_ARGUMENT_INTEGER;
  data_.integer = integer;
}

void
CVMLArgument::
setArgumentRegisterAddr(ushort reg_num)
{
  reset();

  type_         = CVML_ARGUMENT_REGISTER_ADDR;
  data_.reg_num = reg_num;
}

void
CVMLArgument::
setArgumentRegisterValue(ushort reg_num)
{
  reset();

  type_         = CVML_ARGUMENT_REGISTER_VALUE;
  data_.reg_num = reg_num;
}

void
CVMLArgument::
setArgumentRegisterOffset(ushort reg_num, short offset)
{
  reset();

  type_                    = CVML_ARGUMENT_REGISTER_OFFSET;
  data_.offset_reg.reg_num = reg_num;
  data_.offset_reg.offset  = offset;
}

void
CVMLArgument::
setArgumentRegisterOffsetDeferred(ushort reg_num, short offset)
{
  reset();

  type_                    = CVML_ARGUMENT_REGISTER_OFFSET_DEFERRED;
  data_.offset_reg.reg_num = reg_num;
  data_.offset_reg.offset  = offset;
}

void
CVMLArgument::
setArgumentRegisterVarOffset(ushort reg_num, char *name)
{
  reset();

  type_                        = CVML_ARGUMENT_REGISTER_VAR_OFFSET;
  data_.offset_var_reg.reg_num = reg_num;
  data_.offset_var_reg.name    = CStrUtil::strdup(name);
}

void
CVMLArgument::
setArgumentRegisterVarOffsetDeferred(ushort reg_num, char *name)
{
  reset();

  type_                        = CVML_ARGUMENT_REGISTER_VAR_OFFSET_DEFERRED;
  data_.offset_var_reg.reg_num = reg_num;
  data_.offset_var_reg.name    = CStrUtil::strdup(name);
}

void
CVMLArgument::
setArgumentRegisterValueIncr(ushort reg_num)
{
  reset();

  type_         = CVML_ARGUMENT_REGISTER_VALUE_INCR;
  data_.reg_num = reg_num;
}

void
CVMLArgument::
setArgumentRegisterValueDeferredIncr(ushort reg_num)
{
  reset();

  type_         = CVML_ARGUMENT_REGISTER_VALUE_DEFERRED_INCR;
  data_.reg_num = reg_num;
}

void
CVMLArgument::
setArgumentRegisterValueDecr(ushort reg_num)
{
  reset();

  type_         = CVML_ARGUMENT_REGISTER_VALUE_DECR;
  data_.reg_num = reg_num;
}

void
CVMLArgument::
setArgumentRegisterValueDeferredDecr(ushort reg_num)
{
  reset();

  type_         = CVML_ARGUMENT_REGISTER_VALUE_DEFERRED_DECR;
  data_.reg_num = reg_num;
}

void
CVMLArgument::
setArgumentVariableAddr(const std::string &name)
{
  reset();

  type_      = CVML_ARGUMENT_VARIABLE_ADDR;
  data_.name = CStrUtil::strdup(name);
}

void
CVMLArgument::
setArgumentVariableValue(const std::string &name)
{
  reset();

  type_      = CVML_ARGUMENT_VARIABLE_VALUE;
  data_.name = CStrUtil::strdup(name);
}

uint
CVMLArgument::
getAddressLen()
{
  switch (type_) {
    case CVML_ARGUMENT_CHAR:
      return 2;
    case CVML_ARGUMENT_INTEGER:
      return 2;
    case CVML_ARGUMENT_REGISTER_ADDR:
      return 0;
    case CVML_ARGUMENT_REGISTER_VALUE:
      return 0;
    case CVML_ARGUMENT_REGISTER_OFFSET:
    case CVML_ARGUMENT_REGISTER_OFFSET_DEFERRED:
      return 2;
    case CVML_ARGUMENT_REGISTER_VAR_OFFSET:
    case CVML_ARGUMENT_REGISTER_VAR_OFFSET_DEFERRED:
      return 2;
    case CVML_ARGUMENT_REGISTER_VALUE_INCR:
      return 0;
    case CVML_ARGUMENT_REGISTER_VALUE_DECR:
      return 0;
    case CVML_ARGUMENT_VARIABLE_ADDR:
      return 2;
    case CVML_ARGUMENT_VARIABLE_VALUE:
      return 2;
    default:
      return 0;
  }
}

void
CVMLArgument::
print(std::ostream &os)
{
  switch (type_) {
    case CVML_ARGUMENT_NONE:
      break;
    case CVML_ARGUMENT_CHAR:
      os << ".\'" << CStrUtil::encodeCharString(data_.c) << "\'";

      break;
    case CVML_ARGUMENT_INTEGER:
      os << "." << data_.integer;

      break;
    case CVML_ARGUMENT_REGISTER_ADDR:
      os << vml_->getRegisterName(data_.reg_num);

      break;
    case CVML_ARGUMENT_REGISTER_VALUE:
      os << vml_->getRegisterName(data_.reg_num) << "^";

      break;
    case CVML_ARGUMENT_REGISTER_OFFSET:
      os << data_.offset_reg.offset << "[" <<
            vml_->getRegisterName(ushort(data_.offset_reg.reg_num)) << "]";

      break;
    case CVML_ARGUMENT_REGISTER_OFFSET_DEFERRED:
      os << data_.offset_reg.offset << "[" <<
            vml_->getRegisterName(ushort(data_.offset_reg.reg_num)) << "]^";

      break;
    case CVML_ARGUMENT_REGISTER_VAR_OFFSET:
      os << data_.offset_var_reg.name << "[" <<
            vml_->getRegisterName(ushort(data_.offset_var_reg.reg_num)) << "]";

      break;
    case CVML_ARGUMENT_REGISTER_VAR_OFFSET_DEFERRED:
      os << data_.offset_var_reg.name << "[" <<
            vml_->getRegisterName(ushort(data_.offset_var_reg.reg_num)) << "]^";

      break;
    case CVML_ARGUMENT_REGISTER_VALUE_INCR:
      os << vml_->getRegisterName(data_.reg_num) << "^+";

      break;
    case CVML_ARGUMENT_REGISTER_VALUE_DEFERRED_INCR:
      os << vml_->getRegisterName(data_.reg_num) << "^^+";

      break;
    case CVML_ARGUMENT_REGISTER_VALUE_DECR:
      os << "-" << vml_->getRegisterName(data_.reg_num) << "^";

      break;
    case CVML_ARGUMENT_REGISTER_VALUE_DEFERRED_DECR:
      os << "-" << vml_->getRegisterName(data_.reg_num) << "^^";

      break;
    case CVML_ARGUMENT_VARIABLE_ADDR:
      os << data_.name;

      break;
    case CVML_ARGUMENT_VARIABLE_VALUE:
      os << "." << data_.name;

      break;
    default:
      break;
   }
}

void
CVMLArgument::
print(std::ostream &os, bool arg_signed, uint arg_bits)
{
  switch (type_) {
    case CVML_ARGUMENT_NONE:
      break;
    case CVML_ARGUMENT_CHAR:
      os << ".\'" << CStrUtil::encodeCharString(data_.c) << "\'";

      break;
    case CVML_ARGUMENT_INTEGER: {
      os << ".";

      if (arg_signed) {
        if (arg_bits == 8)
          os << int(vml_->unsignedToSigned(uchar(data_.integer)));
        else
          os << int(vml_->unsignedToSigned(ushort(data_.integer)));
      }
      else
        os << data_.integer;

      break;
    }
    case CVML_ARGUMENT_REGISTER_ADDR:
      os << vml_->getRegisterName(data_.reg_num);

      break;
    case CVML_ARGUMENT_REGISTER_VALUE:
      os << vml_->getRegisterName(data_.reg_num) << "^";

      break;
    case CVML_ARGUMENT_REGISTER_OFFSET:
      os << data_.offset_reg.offset << "[" <<
            vml_->getRegisterName(ushort(data_.offset_reg.reg_num)) << "]";

      break;
    case CVML_ARGUMENT_REGISTER_OFFSET_DEFERRED:
      os << data_.offset_reg.offset << "[" <<
            vml_->getRegisterName(ushort(data_.offset_reg.reg_num)) << "]^";

      break;
    case CVML_ARGUMENT_REGISTER_VAR_OFFSET:
      os << data_.offset_var_reg.name << "[" <<
            vml_->getRegisterName(ushort(data_.offset_var_reg.reg_num)) << "]";

      break;
    case CVML_ARGUMENT_REGISTER_VAR_OFFSET_DEFERRED:
      os << data_.offset_var_reg.name << "[" <<
            vml_->getRegisterName(ushort(data_.offset_var_reg.reg_num)) << "]^";

      break;
    case CVML_ARGUMENT_REGISTER_VALUE_INCR:
      os << vml_->getRegisterName(data_.reg_num) << "^+";

      break;
    case CVML_ARGUMENT_REGISTER_VALUE_DEFERRED_INCR:
      os << vml_->getRegisterName(data_.reg_num) << "^^+";

      break;
    case CVML_ARGUMENT_REGISTER_VALUE_DECR:
      os << "-" << vml_->getRegisterName(data_.reg_num) << "^";

      break;
    case CVML_ARGUMENT_REGISTER_VALUE_DEFERRED_DECR:
      os << "-" << vml_->getRegisterName(data_.reg_num) << "^^";

      break;
    case CVML_ARGUMENT_VARIABLE_ADDR:
      os << data_.name;

      break;
    case CVML_ARGUMENT_VARIABLE_VALUE:
      os << "." << data_.name;

      break;
    default:
      break;
   }
}

bool
CVMLArgument::
getArgExtraValue()
{
  switch (type_) {
    case CVML_ARGUMENT_CHAR:
    case CVML_ARGUMENT_INTEGER:
    case CVML_ARGUMENT_VARIABLE_ADDR:
    case CVML_ARGUMENT_VARIABLE_VALUE:
    case CVML_ARGUMENT_REGISTER_OFFSET:
    case CVML_ARGUMENT_REGISTER_OFFSET_DEFERRED:
    case CVML_ARGUMENT_REGISTER_VAR_OFFSET:
    case CVML_ARGUMENT_REGISTER_VAR_OFFSET_DEFERRED:
      return true;
    case CVML_ARGUMENT_REGISTER_ADDR:
    case CVML_ARGUMENT_REGISTER_VALUE:
    case CVML_ARGUMENT_REGISTER_VALUE_INCR:
    case CVML_ARGUMENT_REGISTER_VALUE_DECR:
    default:
      return false;
  }
}

uint
CVMLArgument::
getArgValue(ushort *extraValue, bool *extraValueFlag, int offset)
{
  uint value = 0;

  *extraValueFlag = false;

  switch (type_) {
    case CVML_ARGUMENT_REGISTER_ADDR:
      value           = 000 + data_.reg_num;

      break;
    case CVML_ARGUMENT_REGISTER_VALUE:
      value           = 010 + data_.reg_num;

      break;
    case CVML_ARGUMENT_CHAR:
      value           = 027;
      *extraValueFlag = true;
      *extraValue     = data_.c;

      break;
    case CVML_ARGUMENT_INTEGER:
      value           = 027;
      *extraValueFlag = true;
      *extraValue     = data_.integer;

      break;
    case CVML_ARGUMENT_VARIABLE_VALUE: {
      uint addr;

      if (! vml_->getVariableAddress(data_.name, &addr))
        throw CVMLError("Unknown variable " + std::string(data_.name));

      value           = 027;
      *extraValueFlag = true;
      *extraValue     = ushort(addr);

      break;
    }
    case CVML_ARGUMENT_REGISTER_VALUE_INCR:
      value           = 020 + data_.reg_num;

      break;
    case CVML_ARGUMENT_REGISTER_VALUE_DEFERRED_INCR:
      value           = 030 + data_.reg_num;

      break;
    case CVML_ARGUMENT_REGISTER_VALUE_DECR:
      value           = 040 + data_.reg_num;

      break;
    case CVML_ARGUMENT_REGISTER_VALUE_DEFERRED_DECR:
      value           = 050 + data_.reg_num;

      break;
    case CVML_ARGUMENT_VARIABLE_ADDR: {
      uint addr;

      if (! vml_->getVariableAddress(data_.name, &addr))
        throw CVMLError("Unknown variable " + std::string(data_.name));

      value           = 067;
      *extraValueFlag = true;
      *extraValue     = ushort(addr - vml_->getPC() - offset);

      break;
    }
    case CVML_ARGUMENT_REGISTER_OFFSET:
      value           = 060 + data_.offset_reg.reg_num;
      *extraValueFlag = true;
      *extraValue     = ushort(data_.offset_reg.offset);

      break;
    case CVML_ARGUMENT_REGISTER_VAR_OFFSET: {
      uint addr;

      if (! vml_->getVariableAddress(data_.offset_var_reg.name, &addr))
        throw CVMLError("Unknown variable " + std::string(data_.name));

      value           = 060 + data_.offset_var_reg.reg_num;
      *extraValueFlag = true;
      *extraValue     = ushort(addr);

      break;
    }
    case CVML_ARGUMENT_REGISTER_OFFSET_DEFERRED: {
      value           = 070 + data_.offset_reg.reg_num;
      *extraValueFlag = true;
      *extraValue     = ushort(data_.offset_reg.offset);

      break;
    }
    case CVML_ARGUMENT_REGISTER_VAR_OFFSET_DEFERRED: {
      uint addr;

      if (! vml_->getVariableAddress(data_.offset_var_reg.name, &addr))
        throw CVMLError("Unknown variable " + std::string(data_.name));

      value           = 070 + data_.offset_var_reg.reg_num;
      *extraValueFlag = true;
      *extraValue     = ushort(addr);

      break;
    }
    default:
      break;
  }

  return value;
}

uint
CVMLArgument::
getArgRegNum()
{
  if      (type_ == CVML_ARGUMENT_REGISTER_ADDR ||
           type_ == CVML_ARGUMENT_REGISTER_VALUE)
    return data_.reg_num;
  else if (type_ == CVML_ARGUMENT_REGISTER_OFFSET ||
           type_ == CVML_ARGUMENT_REGISTER_OFFSET_DEFERRED)
    return data_.offset_reg.reg_num;
  else if (type_ == CVML_ARGUMENT_REGISTER_VAR_OFFSET ||
           type_ == CVML_ARGUMENT_REGISTER_VAR_OFFSET_DEFERRED)
    return data_.offset_var_reg.reg_num;
  else
    throw CVMLError("Bad Type for getArgRegNum()");
}

uint
CVMLArgument::
getInteger()
{
  if (type_ == CVML_ARGUMENT_INTEGER)
    return data_.integer;
  else
    throw CVMLError("Bad Type for getInteger()");
}

char *
CVMLArgument::
getName()
{
  if      (type_ == CVML_ARGUMENT_VARIABLE_ADDR ||
           type_ == CVML_ARGUMENT_VARIABLE_VALUE)
    return data_.name;
  else if (type_ == CVML_ARGUMENT_REGISTER_VAR_OFFSET ||
           type_ == CVML_ARGUMENT_REGISTER_VAR_OFFSET_DEFERRED)
    return data_.offset_var_reg.name;
  else
    throw CVMLError("Bad Type for getName()");
}
