#ifndef CVML_OP_H
#define CVML_OP_H

class CVMLOp {
 private:
  enum { MAX_ARGUMENTS = 2 };

  CVML          &vml_;
  CVMLOpCode    *op_code_;
  uint   pc_;
  uint   line_num_;
  uint   num_arguments_;
  CVMLArgument **arguments_;

 public:
  CVMLOp(CVML &vml, CVMLOpCode *op_code,
         uint pc, uint line_num) :
   vml_(vml), op_code_(op_code), pc_(pc), line_num_(line_num) {
    num_arguments_ = op_code->num_args;
    arguments_     = new CVMLArgument * [num_arguments_];

    for (uint i = 0; i < num_arguments_; ++i)
      arguments_[i] = new CVMLArgument(&vml_);
  }

 ~CVMLOp() {
    for (uint i = 0; i < num_arguments_; ++i)
      delete arguments_[i];

    delete [] arguments_;
  }

  bool equal(const CVMLOp &op);

  uint getPC() const { return pc_; }

  void setArgument(uint i, CVMLArgument &argument) {
    if (i < num_arguments_)
      *(arguments_[i]) = argument;
  }

  bool isArgument(uint i) {
    return (i < num_arguments_);
  }

  CVMLArgument *getArgument(uint i) {
    if (i < num_arguments_)
      return arguments_[i];
    else
      return NULL;
  }

  uint getAddressLen();

  friend std::ostream &operator<<(std::ostream &os, CVMLOp &op) {
    op.print(os);

    return os;
  }

  void print(std::ostream &os);

  void printData(std::ostream &os, bool show_code, int indent);

  void printCode(std::ostream &os);

  void encode(CFile *file);

  uint getEncodeSize();

 private:
  ushort getValue(ushort *extraValue1, bool *extraValueFlag1,
                          ushort *extraValue2, bool *extraValueFlag2);
};

#endif
