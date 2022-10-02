#include <CVMLI.h>

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
  if (! file->write(reinterpret_cast<uchar *>(&id_      ), sizeof(id_      )))
    return false;

  if (! file->write(reinterpret_cast<uchar *>(&line_num_), sizeof(line_num_)))
    return false;

  if (! file->write(reinterpret_cast<uchar *>(&pc_      ), sizeof(pc_      )))
    return false;

  return true;
}
