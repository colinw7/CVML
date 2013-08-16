#ifndef CVML_LABEL_H
#define CVML_LABEL_H

class CVMLLabel {
 private:
  CVML         *vml_;
  CVMLStringId  id_;
  uint  line_num_;
  uint  pc_;

 public:
  CVMLLabel(CVML *vml, CVMLStringId id, int line_num) :
   vml_(vml), id_(id), line_num_(line_num), pc_(0) {
  }

 ~CVMLLabel() {
  }

  CVMLStringId getId() const { return id_; }

  std::string getName();

  void setLineNum(uint line_num) {
    line_num_ = line_num;
  }

  uint getLineNum() {
    return line_num_;
  }

  void setPC(uint pc) {
    pc_ = pc;
  }

  uint getPC() const {
    return pc_;
  }

  friend std::ostream &operator<<(std::ostream &os, CVMLLabel &op) {
    op.print(os);

    return os;
  }

  void print(std::ostream &os);

  uint getWriteSize();

  bool write(CFile *file);
};

#endif
