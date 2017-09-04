#include <CVMLDecode.h>
#include <cstring>

bool
CVML::
outputSections(CFile *file)
{
  reset();

  if (! outputHeader(file)) {
    std::cerr << "Not a valid VML file" << std::endl;
    return false;
  }

  if (! outputStringTable(file)) {
    std::cerr << "Bad String Table" << std::endl;
    return false;
  }

  if (! outputDebug(file)) {
    std::cerr << "Bad Debug Section" << std::endl;
    return false;
  }

  if (! outputData(file)) {
    std::cerr << "Bad Data Section" << std::endl;
    return false;
  }

  if (! outputInstructions(file)) {
    std::cerr << "Bad Code Section" << std::endl;
    return false;
  }

  return true;
}

bool
CVML::
outputHeader(CFile *file)
{
  uint version;
  char magic[4];

  if (! file->read((uchar *) magic, 4))
    return false;

  if (strncmp(magic, CVML_MAGIC, 4) != 0)
    return false;

  if (! file->read((uchar *) &version, sizeof(version)))
    return false;

  std::cout << "Version: " << version << std::endl;

  if (! file->read((uchar *) &begin_pc_, sizeof(begin_pc_)))
    return false;

  std::cout << "Begin PC: " << CStrUtil::strprintf("%06o", begin_pc_) << std::endl;

  return true;
}

bool
CVML::
outputStringTable(CFile *file)
{
  uint size;

  if (! file->read((uchar *) &size, sizeof(size)))
    return false;

  uint size1 = 0;

  uint num_strs;

  if (! file->read((uchar *) &num_strs, sizeof(num_strs)))
    return false;

  std::cout << "String Table: Num=" << num_strs << " Size=" << size << std::endl;

  size1 += sizeof(num_strs);

  for (uint i = 0; i < num_strs; ++i) {
    CVMLStringId id;

    if (! file->read((uchar *) &id, sizeof(id)))
      return false;

    size1 += sizeof(id);

    std::string str;

    size1 += readString(file, str);

    //----

    std::cout << id << ") " << str << std::endl;
  }

  if (size != size1)
    return false;

  return true;
}

bool
CVML::
outputDebug(CFile *file)
{
  uint size;

  if (! file->read((uchar *) &size, sizeof(size)))
    return false;

  uint size1 = 0;

  uint num_labels;

  if (! file->read((uchar *) &num_labels, sizeof(num_labels)))
    return false;

  std::cout << "Debug Section: Num=" << num_labels << " Size=" << size << std::endl;

  size1 += sizeof(num_labels);

  for (uint i = 0; i < num_labels; ++i) {
    CVMLStringId id;

    if (! file->read((uchar *) &id, sizeof(id)))
      return false;

    size1 += sizeof(id);

    uint line_num;

    if (! file->read((uchar *) &line_num, sizeof(line_num)))
      return false;

    size1 += sizeof(line_num);

    uint pc;

    if (! file->read((uchar *) &pc, sizeof(pc)))
      return false;

    size1 += sizeof(pc);

    std::cout << "Label: Id=" << id << " LineNum=" << line_num << " PC=" << pc << std::endl;
  }

  if (size != size1)
    return false;

  return true;
}

bool
CVML::
outputData(CFile *file)
{
  uint size;

  if (! file->read((uchar *) &size, sizeof(size)))
    return false;

  uint size1 = 0;

  uint num_data;

  if (! file->read((uchar *) &num_data, sizeof(num_data)))
    return false;

  std::cout << "Data Section: Num=" << num_data << " Size=" << size << std::endl;

  size1 += sizeof(num_data);

  for (uint i = 0; i < num_data; ++i) {
    uint id;

    if (! file->read((uchar *) &id, sizeof(id)))
      return false;

    size1 += sizeof(id);

    std::cout << "Data: Id=" << id << std::endl;

    uint size2;

    CVMLData *data = CVMLData::read(*this, file, &size2);

    if (data == NULL)
      return false;

    std::cout << "        " << *data << std::endl;

    delete data;

    size1 += size2;
  }

  if (size1 != size)
    return false;

  return true;
}

bool
CVML::
outputInstructions(CFile *file)
{
  uint size;

  if (! file->read((uchar *) &size, sizeof(size)))
    return false;

  uchar *data = new uchar [size];

  if (! file->read((uchar *) data, size))
    return false;

  std::cout << "Code Section: " << size << " Bytes" << std::endl;

  outputMemory(data, size);

  delete [] data;

  return true;
}

bool
CVML::
outputMemory(uchar *data, uint size)
{
  ushort s;

  ushort pc = 0;

  for (uint i = 0; i < size; i += 2, pc += 2) {
    memcpy(&s, &data[i], 2);

    std::cout << CStrUtil::strprintf("%06o: %06o", pc, s) << std::endl;
  }

  return true;
}

bool
CVML::
decode(CFile *file)
{
  reset();

  if (! readHeader(file)) {
    std::cerr << "Not a valid VML file" << std::endl;
    return false;
  }

  if (! readStringTable(file)) {
    std::cerr << "Bad String Table" << std::endl;
    return false;
  }

  if (! readDebug(file)) {
    std::cerr << "Bad Debug Section" << std::endl;
    return false;
  }

  if (! readData(file)) {
    std::cerr << "Bad Data Section" << std::endl;
    return false;
  }

  if (! readInstructions(file)) {
    std::cerr << "Bad Code Section" << std::endl;
    return false;
  }

  return true;
}

bool
CVML::
readHeader(CFile *file)
{
  char magic[4];

  if (! file->read((uchar *) magic, 4))
    return false;

  if (strncmp(magic, CVML_MAGIC, 4) != 0)
    return false;

  uint version;

  if (! file->read((uchar *) &version, sizeof(version)))
    return false;

  if (! file->read((uchar *) &begin_pc_, sizeof(begin_pc_)))
    return false;

  return true;
}

bool
CVML::
readStringTable(CFile *file)
{
  uint size;

  if (! file->read((uchar *) &size, sizeof(size)))
    return false;

  uint size1 = 0;

  uint num_strs;

  if (! file->read((uchar *) &num_strs, sizeof(num_strs)))
    return false;

  size1 += sizeof(num_strs);

  for (uint i = 0; i < num_strs; ++i) {
    CVMLStringId id;

    if (! file->read((uchar *) &id, sizeof(id)))
      return false;

    size1 += sizeof(id);

    std::string str;

    size1 += readString(file, str);

    //----

    string_id_map_[str] = id;
    id_string_map_[id ] = str;
  }

  if (size != size1)
    return false;

  return true;
}

bool
CVML::
skipStringTable(CFile *file)
{
  uint size;

  if (! file->read((uchar *) &size, sizeof(size)))
    return false;

  if (! file->setRPos(size))
    return false;

  return true;
}

bool
CVML::
readDebug(CFile *file)
{
  uint size;

  if (! file->read((uchar *) &size, sizeof(size)))
    return false;

  uint size1 = 0;

  uint num_labels;

  if (! file->read((uchar *) &num_labels, sizeof(num_labels)))
    return false;

  size1 += sizeof(num_labels);

  for (uint i = 0; i < num_labels; ++i) {
    CVMLStringId id;

    if (! file->read((uchar *) &id, sizeof(id)))
      return false;

    size1 += sizeof(id);

    uint line_num;

    if (! file->read((uchar *) &line_num, sizeof(line_num)))
      return false;

    size1 += sizeof(line_num);

    uint pc;

    if (! file->read((uchar *) &pc, sizeof(pc)))
      return false;

    size1 += sizeof(pc);

    CVMLLabel *label = addLabel(id, pc);

    label->setLineNum(line_num);
  }

  if (size != size1)
    return false;

  return true;
}

bool
CVML::
skipDebug(CFile *file)
{
  uint size;

  if (! file->read((uchar *) &size, sizeof(size)))
    return false;

  if (! file->setRPos(size))
    return false;

  return true;
}

bool
CVML::
readData(CFile *file)
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

    CVMLStringId id1 = id;

    uint size2;

    CVMLData *data = CVMLData::read(*this, file, &size2);

    if (data == NULL)
      return false;

    size1 += size2;

    CVMLLabel *label = lookupLabelById(id1);

    if (label == NULL)
      label = addLabel(id1, data->getPC());

    CVMLLine *line = new CVMLLine(label, data);

    addLine(line);
  }

  if (size1 != size)
    return false;

  return true;
}

bool
CVML::
skipData(CFile *file)
{
  uint size;

  if (! file->read((uchar *) &size, sizeof(size)))
    return false;

  if (! file->setRPos(size))
    return false;

  return true;
}

bool
CVML::
readInstructions(CFile *file)
{
  uint size;

  if (! file->read((uchar *) &size, sizeof(size)))
    return false;

  while (pc_ < size) {
    ++line_num_;

    CVMLLabel *label = lookupLabelByPC(pc_);
    CVMLData  *data  = lookupDataByPC(pc_);

    CVMLLine *line = NULL;

    if (data == NULL) {
      CVMLFileDataSource src(this, file);

      CVMLOp *op = instructionToOp(&src);

      line = new CVMLLine(label, op);
    }
    else {
      line = new CVMLLine(label, data);

      uint len = data->getAddressLen()/2;

      ushort s;

      for (uint i = 0; i < len; ++i) {
        if (! file->read((uchar *) &s, sizeof(s))) {
          std::cerr << "Failed to read data" << std::endl;
          return false;
        }

        pc_ += 2;
      }
    }

    addLine(line);
  }

  return true;
}

bool
CVML::
skipInstructions(CFile *file)
{
  uint size;

  if (! file->read((uchar *) &size, sizeof(size)))
    return false;

  if (! file->setRPos(size))
    return false;

  return true;
}

uint
CVML::
readString(CFile *file, std::string &str)
{
  uint size = 0;

  uint len;

  if (! file->read((uchar *) &len, sizeof(len)))
    return false;

  size += sizeof(len);

  uint len1 = len;

  if (len1 & 1)
    ++len1;

  char *cstr = new char [len1 + 1];

  if (! file->read((uchar *) cstr, len1)) {
    delete [] cstr;
    return false;
  }

  size += len1;

  cstr[len] = '\0';

  str = std::string(cstr, len);

  delete [] cstr;

  return size;
}
