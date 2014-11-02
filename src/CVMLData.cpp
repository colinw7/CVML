#include <CVMLI.h>
#include <cstring>

uint
CVMLData::
getAddressLen()
{
  if      (type_ == DATA_TYPE_CHAR)
    return 1; //TODO:: 2 ?
  else if (type_ == DATA_TYPE_INTEGER)
    return 2;
  else {
    const std::string &str = vml_->lookupIdString(getStringValue());

    return vml_->getWriteStringLen(str.size());
  }
}

void
CVMLData::
print(std::ostream &os)
{
  if      (type_ == DATA_TYPE_CHAR || type_ == DATA_TYPE_INTEGER) {
    uint dim = dim_;

    if (dim_ == 0)
      dim = 1;

    for (uint i = 0; i < dim; ++i) {
      if (i > 0)
        os << std::endl << "        ";

      short value = 0;

      if (type_ == DATA_TYPE_CHAR) {
        char c = getCharValue();

        memcpy(&value, &c, sizeof(c));
      }
      else {
        ushort integer = getIntegerValue();

        memcpy(&value, &integer, sizeof(integer));
      }

      os << CStrUtil::strprintf("%06o:%06o", pc_ + 2*i, value) << "  ; ";

      os << '.';

      if  (type_ == DATA_TYPE_CHAR)
        os << '\'' << CStrUtil::encodeCharString(getCharValue()) << '\'';
      else
        os << getIntegerValue();
    }
  }
  else {
    const std::string &str = vml_->lookupIdString(getStringValue());

    uint hlen = vml_->getWriteStringLen(str.size())/2;

    for (uint i = 0; i < hlen; ++i) {
      if (i > 0)
        os << std::endl << "        ";

      short value = 0;

      memcpy(&value, &str[i*2], sizeof(value));

      os << CStrUtil::strprintf("%06o:%06o", pc_ + 2*i, value);

      if (i == 0) {
        os << "  ; .\"";

        for (uint i = 0; i < str.size(); ++i) {
          if      (str[i] == '\b')
            os << "\\b";
          else if (str[i] == '\n')
            os << "\\n";
          else if (str[i] == '\t')
            os << "\\t";
          else if (str[i] == '\0')
            os << "\\0";
          else
            os << str[i];
        }

        os << '\"';
      }
    }
  }
}

uint
CVMLData::
getWriteSize()
{
  uint size = 0;

  size += 3*sizeof(uint); // pc, type, dim

  if      (type_ == DATA_TYPE_CHAR)
    size += sizeof(uint);
  else if (type_ == DATA_TYPE_INTEGER)
    size += sizeof(uint);
  else {
    const std::string &str = vml_->lookupIdString(getStringValue());

    size += vml_->getWriteStringSize(str.size());
  }

  return size;
}

bool
CVMLData::
write(CFile *file)
{
  uint pc = pc_;

  if (! file->write((char *) &pc_, sizeof(pc)))
    return false;

  uint type = type_;

  if (! file->write((char *) &type, sizeof(type)))
    return false;

  if (! file->write((char *) &dim_, sizeof(dim_)))
    return false;

  if      (type_ == DATA_TYPE_CHAR) {
    uint value = getCharValue();

    if (! file->write((char *) &value, sizeof(value)))
      return false;
  }
  else if (type_ == DATA_TYPE_INTEGER) {
    uint integer = getIntegerValue();

    if (! file->write((char *) &integer, sizeof(integer)))
      return false;
  }
  else {
    const std::string &str = vml_->lookupIdString(getStringValue());

    if (! vml_->writeString(file, str))
      return false;
  }

  return true;
}

CVMLData *
CVMLData::
read(CVML &vml, CFile *file, uint *size)
{
  *size = 0;

  uint pc;

  if (! file->read((uchar *) &pc, sizeof(pc)))
    return NULL;

  *size += sizeof(pc);

  uint type;

  if (! file->read((uchar *) &type, sizeof(type)))
    return NULL;

  *size += sizeof(type);

  int dim;

  if (! file->read((uchar *) &dim, sizeof(dim)))
    return NULL;

  *size += sizeof(dim);

  CVMLData *data;

  if      (type == CVMLData::DATA_TYPE_CHAR) {
    uint c;

    if (! file->read((uchar *) &c, sizeof(c)))
      return NULL;

    *size += sizeof(c);

    data = new CVMLData(&vml, pc, char(c), dim);
  }
  else if (type == CVMLData::DATA_TYPE_INTEGER) {
    int integer;

    if (! file->read((uchar *) &integer, sizeof(integer)))
      return NULL;

    *size += sizeof(integer);

    data = new CVMLData(&vml, pc, integer, dim);
  }
  else if (type == CVMLData::DATA_TYPE_STRING) {
    std::string str;

    *size += vml.readString(file, str);

    CVMLStringId id = vml.lookupStringId(str);

    data = new CVMLData(&vml, pc, id);
  }
  else
    return NULL;

  return data;
}

bool
CVMLData::
readMemory(CVML &vml, CFile *file, uint *size)
{
  *size = 0;

  uint pc;

  if (! file->read((uchar *) &pc, sizeof(pc)))
    return false;

  vml.setRegisterWord(PC_NUM, pc);

  *size += sizeof(pc);

  uint type;

  if (! file->read((uchar *) &type, sizeof(type)))
    return false;

  *size += sizeof(type);

  int dim;

  if (! file->read((uchar *) &dim, sizeof(dim)))
    return false;

  *size += sizeof(dim);

  if      (type == CVMLData::DATA_TYPE_CHAR) {
    uint c;

    if (! file->read((uchar *) &c, sizeof(c)))
      return false;

    *size += sizeof(c);

    char c1 = c;

    if (dim == 0)
      vml.setMemoryByte(vml.getRegisterWord(PC_NUM), c1);
    else {
      for (int i = 0; i < dim; ++i)
        vml.setMemoryByte(vml.getRegisterWord(PC_NUM) + i, c1);
    }
  }
  else if (type == CVMLData::DATA_TYPE_INTEGER) {
    uint integer;

    if (! file->read((uchar *) &integer, sizeof(integer)))
      return false;

    *size += sizeof(integer);

    short s = integer;

    if (dim == 0)
      vml.setMemoryWord(vml.getRegisterWord(PC_NUM), s);
    else {
      for (int i = 0; i < dim; ++i)
        vml.setMemoryWord(vml.getRegisterWord(PC_NUM) + 2*i, s);
    }
  }
  else if (type == CVMLData::DATA_TYPE_STRING) {
    std::string str;

    *size += vml.readString(file, str);

    vml.setMemoryString(vml.getRegisterWord(PC_NUM), str);
  }
  else
    return false;

  return true;
}

bool
CVMLData::
outputValues(CFile *file)
{
  if      (type_ == DATA_TYPE_CHAR) {
    short value = getCharValue();

    if (dim_ == 0)
      vml_->outputValue(file, value);
    else {
      for (int i = 0; i < dim_; ++i)
        vml_->outputValue(file, value);
    }
  }
  else if (type_ == DATA_TYPE_INTEGER) {
    short value = getIntegerValue();

    if (dim_ == 0)
      vml_->outputValue(file, value);
    else {
      for (int i = 0; i < dim_; ++i)
        vml_->outputValue(file, value);
    }
  }
  else {
    const std::string &str = vml_->lookupIdString(getStringValue());

    vml_->outputString(file, str);
  }

  return true;
}

uint
CVMLData::
getValuesLen()
{
  uint len = 0;

  if      (type_ == DATA_TYPE_CHAR) {
    if (dim_ == 0)
      len += sizeof(short);
    else
      len += dim_*sizeof(short);
  }
  else if (type_ == DATA_TYPE_INTEGER) {
    if (dim_ == 0)
      len += sizeof(short);
    else
      len += dim_*sizeof(short);
  }
  else {
    const std::string &str = vml_->lookupIdString(getStringValue());

    len = vml_->getWriteStringLen(str.size())/2;
  }

  return true;
}
