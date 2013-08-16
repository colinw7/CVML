#ifndef CVML_DATA_H
#define CVML_DATA_H

union CVMLDataData {
  char         c;
  ushort       integer;
  CVMLStringId str;
};

class CVMLData {
 public:
  enum CVMLDataType {
    DATA_TYPE_CHAR,
    DATA_TYPE_INTEGER,
    DATA_TYPE_STRING
  };

 private:
  CVML         *vml_;
  uint          pc_;
  CVMLDataType  type_;
  CVMLDataData  data_;
  int           dim_;

 public:
  CVMLData(CVML *vml, uint pc, char c, int dim = 0) :
   vml_(vml), pc_(pc), type_(DATA_TYPE_CHAR), dim_(dim) {
    data_.c = c;
  }

  CVMLData(CVML *vml, uint pc, int integer, int dim = 0) :
   vml_(vml), pc_(pc), type_(DATA_TYPE_INTEGER), dim_(dim) {
    data_.integer = integer;
  }

  CVMLData(CVML *vml, uint pc, const CVMLStringId &str) :
   vml_(vml), pc_(pc), type_(DATA_TYPE_STRING), dim_(0) {
    data_.str = str;
  }

 ~CVMLData() { }

  uint getPC() const { return pc_; }

  char         getCharValue   () const { return data_.c; }
  ushort       getIntegerValue() const { return data_.integer; }
  CVMLStringId getStringValue () const { return data_.str; }

  bool isCharValue   () const { return type_ == DATA_TYPE_CHAR   ; }
  bool isIntegerValue() const { return type_ == DATA_TYPE_INTEGER; }
  bool isStringValue () const { return type_ == DATA_TYPE_STRING ; }

  int getDim() const { return dim_; }

  uint getAddressLen();

  friend std::ostream &operator<<(std::ostream &os, CVMLData &data) {
    data.print(os);

    return os;
  }

  void print(std::ostream &os);

  uint getWriteSize();

  bool write(CFile *file);

  static CVMLData *read(CVML &vml, CFile *file, uint *size);

  static bool readMemory(CVML &vml, CFile *file, uint *size);

  bool outputValues(CFile *file);

  uint getValuesLen();
};

#endif
