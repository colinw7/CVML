#ifndef CVML_DATA_SOURCE_H
#define CVML_DATA_SOURCE_H

class CVMLDataSource {
 protected:
  CVML *vml_;

 public:
  CVMLDataSource(CVML *vml) :
   vml_(vml) {
  }

  virtual ~CVMLDataSource() { }

  virtual bool getNextWord(ushort *s) = 0;
};

class CVMLFileDataSource : public CVMLDataSource {
 private:
  CFile *file_;

 public:
  CVMLFileDataSource(CVML *vml, CFile *file) :
   CVMLDataSource(vml), file_(file) {
  }

  bool getNextWord(ushort *s);
};

class CVMLMemoryDataSource : public CVMLDataSource {
 public:
  CVMLMemoryDataSource(CVML *vml) :
   CVMLDataSource(vml) {
  }

  bool getNextWord(ushort *s);
};

#endif
