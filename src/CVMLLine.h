#ifndef CVML_LINE_H
#define CVML_LINE_H

class CVMLLine {
 private:
  CVMLLabel *label_;
  CVMLData  *data_;
  CVMLOp    *op_;

 public:
  CVMLLine(CVMLLabel *label, CVMLData *data) :
    label_(label), data_(data), op_(NULL) {
  }

  CVMLLine(CVMLLabel *label, CVMLOp *op) :
    label_(label), data_(NULL), op_(op) {
  }

  CVMLLabel *getLabel() const { return label_; }
  CVMLData  *getData () const { return data_ ; }
  CVMLOp    *getOp   () const { return op_   ; }

  bool isData() {
    return (data_ != NULL);
  }

  bool isOp() {
    return (op_ != NULL);
  }

  uint getPC();

  uint getAddressLen();

  friend std::ostream &operator<<(std::ostream &os, CVMLLine &line) {
    line.print(os);

    return os;
  }

  void print(std::ostream &os);

  void encodeData(CVML &vml, CFile *file);

  void writeInstruction(CVML &vml, CFile *file);
};

#endif
