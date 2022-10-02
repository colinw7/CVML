#include <CVMLI.h>

bool
CVMLFileDataSource::
getNextWord(ushort *s)
{
  if (! file_->read(reinterpret_cast<uchar *>(s), sizeof(*s))) {
    throw CVMLError("Failed to read instruction");
    return false;
  }

  vml_->addPC(2);

  return true;
}

bool
CVMLMemoryDataSource::
getNextWord(ushort *s)
{
  *s = vml_->nextMemoryInstruction();

  return true;
}
