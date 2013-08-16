#include "CVMLI.h"

std::string
CVMLLabel::
getName()
{
  return vml_->lookupIdString(id_);
}

void
CVMLLabel::
print(std::ostream &os)
{
  os << getName() << ":";
}

uint
CVMLLabel::
getWriteSize()
{
  return sizeof(id_) + sizeof(line_num_) + sizeof(pc_);
}

bool
CVMLLabel::
write(CFile *file)
{
  if (! file->write((uchar *) &id_      , sizeof(id_      )))
    return false;

  if (! file->write((uchar *) &line_num_, sizeof(line_num_)))
    return false;

  if (! file->write((uchar *) &pc_      , sizeof(pc_      )))
    return false;

  return true;
}
