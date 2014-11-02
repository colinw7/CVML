#include <CVMLI.h>

bool
CVML::
encode(CFile *file)
{
  try {
    if (! writeHeader(file)) {
      std::cerr << "Invalid VML file" << std::endl;
      return false;
    }

    if (! writeStringTable(file)) {
      std::cerr << "Bad String Table" << std::endl;
      return false;
    }

    if (! writeDebug(file)) {
      std::cerr << "Bad Debug Section" << std::endl;
      return false;
    }

    if (! writeData(file)) {
      std::cerr << "Bad Data Section" << std::endl;
      return false;
    }

    if (! writeInstructions(file)) {
      std::cerr << "Bad Code Section" << std::endl;
      return false;
    }
  }
  catch (CVMLError error) {
    std::cerr << error.msg << std::endl;
    return false;
  }
  catch (...) {
    std::cerr << "Uncaught Error" << std::endl;
    return false;
  }

  return true;
}

bool
CVML::
writeHeader(CFile *file)
{
  file->write(CVML_MAGIC, 4);

  uint version = CVML_VERSION;

  file->write((uchar *) &version, sizeof(version));

  file->write((uchar *) &begin_pc_, sizeof(begin_pc_));

  return true;
}

bool
CVML::
writeStringTable(CFile *file)
{
  uint num_strs = string_id_map_.size();

  CVMLStringIdMap::const_iterator p1 = string_id_map_.begin();
  CVMLStringIdMap::const_iterator p2 = string_id_map_.end  ();

  //---

  uint size = sizeof(num_strs);

  for (CVMLStringIdMap::const_iterator p = p1; p != p2; ++p) {
    CVMLStringId id = (*p).second;

    size += sizeof(id);

    std::string str = (*p).first;

    size += getWriteStringSize(str);
  }

  file->write((uchar *) &size, sizeof(size));

  //---

  file->write((uchar *) &num_strs, sizeof(num_strs));

  for (CVMLStringIdMap::const_iterator p = p1; p != p2; ++p) {
    CVMLStringId id = (*p).second;

    file->write((uchar *) &id, sizeof(id));

    std::string str = (*p).first;

    writeString(file, str);
  }

  return true;
}

bool
CVML::
writeDebug(CFile *file)
{
  uint num_labels = labels_.size();

  LabelList::const_iterator l1 = labels_.begin();
  LabelList::const_iterator l2 = labels_.end  ();

  //---

  uint size = sizeof(num_labels);

  for (LabelList::const_iterator l = l1; l != l2; ++l)
    size += (*l)->getWriteSize();

  file->write((uchar *) &size, sizeof(size));

  //---

  file->write((uchar *) &num_labels, sizeof(num_labels));

  for (LabelList::const_iterator l = l1; l != l2; ++l)
    (*l)->write(file);

  return true;
}

bool
CVML::
writeData(CFile *file)
{
  LineList::const_iterator line1 = lines_.begin();
  LineList::const_iterator line2 = lines_.end  ();

  //----

  uint num_data = 0;

  for (LineList::const_iterator line = line1; line != line2; ++line)
    if ((*line)->isData())
      ++num_data;

  //----

  uint size = 0;

  size += sizeof(num_data);

  for (LineList::const_iterator line = line1; line != line2; ++line) {
    if (! (*line)->isData())
      continue;

    size += sizeof(uint); // label id

    CVMLData *data = (*line)->getData();

    size += data->getWriteSize();
  }

  //----

  file->write((uchar *) &size, sizeof(size));

  file->write((uchar *) &num_data, sizeof(num_data));

  for (LineList::const_iterator line = line1; line != line2; ++line) {
    if (! (*line)->isData())
      continue;

    CVMLLabel *label = (*line)->getLabel();

    uint id = 0;

    if (label != NULL)
      id = label->getId();

    file->write((uchar *) &id, sizeof(id));

    CVMLData *data = (*line)->getData();

    data->write(file);
  }

  return true;
}

bool
CVML::
writeInstructions(CFile *file)
{
  LineList::const_iterator line1 = lines_.begin();
  LineList::const_iterator line2 = lines_.end  ();

  // Find max pc

  uint max_pc = 0;

  for (LineList::const_iterator line = line1; line != line2; ++line) {
    uint pc = (*line)->getPC();

    max_pc = std::max(max_pc, pc);
  }

  pc_ = 0;

  uint size = 0;

  for (LineList::const_iterator line = line1; line != line2; ++line) {
    if (! (*line)->isOp())
      continue;

    CVMLOp *op = (*line)->getOp();

    size += op->getEncodeSize();
  }

  uint len = 0;

  while (pc_ <= max_pc) {
    CVMLData *data = lookupDataByPC(pc_);

    if (data != NULL)
      len = data->getValuesLen();
    else
      len = 2;

    size += len;
    pc_  += len;
  }

  pc_ = 0;

  file->write((uchar *) &size, sizeof(size));

  for (LineList::const_iterator line = line1; line != line2; ++line) {
    if (! (*line)->isOp())
      continue;

    CVMLOp *op = (*line)->getOp();

    op->encode(file);
  }

  while (pc_ <= max_pc) {
    CVMLData *data = lookupDataByPC(pc_);

    if (data != NULL)
      data->outputValues(file);
    else
      outputValue(file, OP_CODE_NOP);
  }

  return true;
}

bool
CVML::
writeString(CFile *file, const std::string &str)
{
  uint len = str.size();

  if (! file->write((uchar *) &len, sizeof(len)))
    return false;

  len = getWriteStringLen(len);

  const char *cstr = str.c_str();

  if (! file->write((uchar *) cstr, len))
    return false;

  return true;
}

bool
CVML::
writeString(CFile *file, const char *str, uint len)
{
  if (! file->write((uchar *) &len, sizeof(len)))
    return false;

  len = getWriteStringLen(len);

  if (! file->write((uchar *) str, len))
    return false;

  return true;
}

uint
CVML::
getWriteStringSize(const std::string &str)
{
  uint len = str.size();

  return getWriteStringSize(len);
}

uint
CVML::
getWriteStringSize(uint len)
{
  return (sizeof(len) + getWriteStringLen(len));
}

uint
CVML::
getWriteStringLen(uint len)
{
  if (len & 1)
    ++len;

  return len;
}
