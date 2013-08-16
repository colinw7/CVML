#ifndef CVML_ARGUMENT_H
#define CVML_ARGUMENT_H

enum CVMLArgumentType {
  CVML_ARGUMENT_NONE,
  CVML_ARGUMENT_CHAR,
  CVML_ARGUMENT_INTEGER,
  CVML_ARGUMENT_REGISTER_ADDR,
  CVML_ARGUMENT_REGISTER_VALUE,
  CVML_ARGUMENT_REGISTER_OFFSET,
  CVML_ARGUMENT_REGISTER_OFFSET_DEFERRED,
  CVML_ARGUMENT_REGISTER_VAR_OFFSET,
  CVML_ARGUMENT_REGISTER_VAR_OFFSET_DEFERRED,
  CVML_ARGUMENT_REGISTER_VALUE_INCR,
  CVML_ARGUMENT_REGISTER_VALUE_DEFERRED_INCR,
  CVML_ARGUMENT_REGISTER_VALUE_DECR,
  CVML_ARGUMENT_REGISTER_VALUE_DEFERRED_DECR,
  CVML_ARGUMENT_VARIABLE_ADDR,
  CVML_ARGUMENT_VARIABLE_VALUE
};

class CVMLArgument;

struct CVMLArgumentOffsetReg {
  uint reg_num;
  int  offset;
};

struct CVMLArgumentOffsetVarReg {
  uint  reg_num;
  char *name;
};

union CVMLArgumentData {
  uchar                     c;
  ushort                    integer;
  ushort                    reg_num;
  char                     *name;
  CVMLArgumentOffsetReg     offset_reg;
  CVMLArgumentOffsetVarReg  offset_var_reg;
};

class CVMLArgument {
 private:
  CVML             *vml_;
  CVMLArgumentType  type_;
  CVMLArgumentData  data_;

 public:
  CVMLArgument(CVML *vml);
  CVMLArgument(const CVMLArgument &arg);
 ~CVMLArgument();

  bool equal(const CVMLArgument &arg);

  CVMLArgumentType getType() const { return type_; }

  void setType(CVMLArgumentType type) { type_ = type; }

  void setArgumentChar(uchar c);
  void setArgumentInteger(ushort integer);
  void setArgumentRegisterAddr(ushort reg_num);
  void setArgumentRegisterValue(ushort reg_num);
  void setArgumentRegisterOffset(ushort reg_num, short offset);
  void setArgumentRegisterOffsetDeferred(ushort reg_num, short offset);
  void setArgumentRegisterVarOffset(ushort reg_num, char *name);
  void setArgumentRegisterVarOffsetDeferred(ushort reg_num, char *name);
  void setArgumentRegisterValueIncr(ushort reg_num);
  void setArgumentRegisterValueDeferredIncr(ushort reg_num);
  void setArgumentRegisterValueDecr(ushort reg_num);
  void setArgumentRegisterValueDeferredDecr(ushort reg_num);
  void setArgumentVariableAddr(const std::string &name);
  void setArgumentVariableValue(const std::string &name);

  CVMLArgument &operator=(const CVMLArgument &arg);

  uint getAddressLen();

  friend std::ostream &operator<<(std::ostream &os, CVMLArgument &arg) {
    arg.print(os);

    return os;
  }

  void print(std::ostream &os);
  void print(std::ostream &os, bool arg_signed, uint arg_bits);

  bool getArgExtraValue();

  uint  getArgRegNum();
  uint  getInteger();
  char *getName();

  uint getArgValue(ushort *extraValue, bool *extraValueFlag, int offset);

 private:
  void reset();
  void set(const CVMLArgument &arg);
};

#endif
