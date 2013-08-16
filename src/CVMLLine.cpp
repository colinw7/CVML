#include "CVMLI.h"

uint
CVMLLine::
getPC()
{
  if (data_ != NULL)
    return data_->getPC();
  else
    return op_->getPC();
}

uint
CVMLLine::
getAddressLen()
{
  if (data_ != NULL)
    return data_->getAddressLen();
  else
    return op_->getAddressLen();
}

void
CVMLLine::
print(std::ostream &os)
{
  int len = 8;

  if (label_ != NULL) {
    os << *label_;

    len -= label_->getName().size() + 1;
  }

  if (len <= 0) {
    os << std::endl;

    len = 8;
  }

  for (int i = 0; i < len; ++i)
    os << " ";

  len = 8;

  if      (op_ != NULL)
    os << *op_ << std::endl;
  else if (data_ != NULL)
    os << *data_ << std::endl;
}

void
CVMLLine::
encodeData(CVML &, CFile *file)
{
  if (data_ != NULL)
    data_->write(file);
}

void
CVMLLine::
writeInstruction(CVML &, CFile *file)
{
  if (op_ != NULL)
    op_->encode(file);
}
