#include <CVMLParse.h>
#include <cstring>

bool
CVML::
parse(const std::string &filename)
{
  try {
    reset();

    pc_ = 0;

    if (! parseFile(filename))
      return false;
  }
  catch (CVMLError error) {
    std::cerr << error.msg << std::endl;
    return false;
  }

  return true;
}

bool
CVML::
parseFile(const std::string &filename)
{
  fileName_ = filename;
  line_num_ = 0;

  if (! CFile::exists(fileName_)) {
    std::cerr << "File \'" << fileName_ << "\' does not exist" << std::endl;
    return false;
  }

  file_ = new CFile(fileName_);

  bool eof;

  for (;;) {
    if (! parseNextLine(&eof)) {
      delete file_;
      return false;
    }

    if (eof)
      break;
  }

  delete file_;

  return true;
}

bool
CVML::
parseNextLine(bool *eof)
{
  *eof = false;

  if (! file_->readLine(line_)) {
    *eof = true;
    return true;
  }

  while (! line_.empty() && line_[line_.size() - 1] == '\\') {
    line_ = line_.substr(0, line_.size() - 1);

    std::string line1;

    if (! file_->readLine(line1))
      break;

    line_ += line1;
  }

  ++line_num_;

  if (! parseLine()) {
    syntaxError();
    return false;
  }

  return true;
}

bool
CVML::
parseLine()
{
  std::string identifier;

  parse_ = new CStrParse(line_);

  //-----

  parse_->skipSpace();

  //-----

  // Process pre-processor line

  if (parse_->isChar('#')) {
    if (! parsePreProLine())
      return false;

    return true;
  }

  //-----

  // Skip Comment

  if (parse_->isChar(';'))
    return true;

  //-----

  // Read optional label

  CVMLLabel *label = NULL;

  int dim = 0;

  if (parseLabel(identifier, &dim)) {
    label = addLabel(identifier);

    if (label == NULL)
      return false;
  }

  parse_->skipSpace();

  //-----

  // If end of line then add label with zero value
  // (don't increment pc so next line will have same address)
  // TODO: handle disconnected labels

  if (parse_->eof() || parse_->isChar(';')) {
    if (label != NULL)
      parseLabels_.push_back(label);

    return true;
  }

  if (label != NULL)
    parseLabels_.push_back(label);

  //-----

  // Check for label followed by value

  if (! parseLabels_.empty() || parse_->isChar(':')) {
    if (parse_->isChar(':')) {
      parse_->skipChar();

      parse_->skipSpace();
    }

    CVMLData *data = NULL;

    if      (parse_->isChar('.')) {
      int  value;
      bool is_char;

      if (! parseValue(&value, &is_char))
        return false;

      if (dim > 0) {
        if (is_char)
          data = new CVMLData(this, pc_, char(value), dim);
        else
          data = new CVMLData(this, pc_, ushort(value), dim);
      }
      else {
        if (is_char)
          data = new CVMLData(this, pc_, char(value));
        else
          data = new CVMLData(this, pc_, ushort(value));
      }
    }
    else if (parse_->isChar('\"')) {
      std::string str;

      if (! parseString(str))
        return false;

      if (dim > 0)
        return false;

      CVMLStringId id = lookupStringId(str);

      data = new CVMLData(this, pc_, id);
    }

    if (data != NULL) {
      CVMLLine *line = NULL;

      auto numParseLabels = parseLabels_.size();

      for (uint i = 0; i < numParseLabels; ++i) {
        CVMLLabel *label1 = parseLabels_[i];

        line = new CVMLLine(label1, data);

        addLine(line);
      }

      parseLabels_.clear();

      parse_->skipSpace();

      if (! parse_->eof() && ! parse_->isChar(';'))
        return false;

      pc_ += line->getAddressLen();

      return true;
    }
  }

  //-----

  if (dim > 0)
    return false;

  //-----

  // Read instruction

  if (parse_->isAlpha()) {
    if (! parseInstruction(identifier))
      return false;

    CVMLOpCode *opCode = lookupOpCode(identifier);

    if (opCode == NULL)
      return false;

    parse_->skipSpace();

    //-----

    // Read instruction arguments

    CVMLArgument **arguments = new CVMLArgument * [opCode->num_args];

    for (uint i = 0; i < opCode->num_args; i++) {
      arguments[i] = new CVMLArgument(this);

      if (i > 0) {
        if (! parse_->isChar(','))
          return false;

        parse_->skipChar();

        parse_->skipSpace();
      }

      if (! parseArgument(*arguments[i]))
        return false;

      parse_->skipSpace();
    }

    //-----

    CVMLOp *op = new CVMLOp(*this, opCode, pc_, line_num_);

    for (uint i = 0; i < opCode->num_args; i++)
      op->setArgument(i, *arguments[i]);

    CVMLLine *line = NULL;

    if (! parseLabels_.empty()) {
      auto numParseLabels = parseLabels_.size();

      for (uint i = 0; i < numParseLabels; ++i) {
        CVMLLabel *label1 = parseLabels_[i];

        line = new CVMLLine(label1, op);

        addLine(line);
      }

      parseLabels_.clear();
    }
    else {
      line = new CVMLLine(NULL, op);

      addLine(line);
    }

    pc_ += line->getAddressLen();

    for (uint i = 0; i < opCode->num_args; i++)
      delete arguments[i];

    delete [] arguments;
  }

  //-----

  parse_->skipSpace();

  //-----

  if (! parse_->eof() && ! parse_->isChar(';'))
    return false;

  return true;
}

bool
CVML::
parsePreProLine()
{
  parse_->skipChar();

  parse_->skipSpace();

  std::string cmd;

  if (! parse_->readIdentifier(cmd))
    return true;

  if (CStrUtil::casecmp(cmd, "include") == 0) {
    parse_->skipSpace();

    std::string filename;

    if (! parse_->readString(filename))
      return false;

    filename = filename.substr(1, filename.size() - 2);

    std::string save_filename = fileName_;
    int         save_line_num = line_num_;

    bool flag = parseFile(filename);

    fileName_ = save_filename;
    line_num_ = save_line_num;

    return flag;
  }
  else {
    std::cerr << "Invalid command " << cmd << std::endl;
    return false;
  }

  return true;
}

bool
CVML::
parseLabel(std::string &label, int *dim)
{
  *dim = 0;

  parse_->skipSpace();

  int pos = parse_->getPos();

  if (! parse_->isIdentifier())
    goto fail;

  if (! parse_->readIdentifier(label))
    goto fail;

  parse_->skipSpace();

  if      (parse_->isChar('@')) {
    parse_->skipChar();

    parse_->skipSpace();

    if (parse_->isDigit()) {
      int i;

      if (! parse_->readInteger(&i))
        goto fail;

      if (i < 0)
        goto fail;

      pc_ = i;

      parse_->skipSpace();
    }
  }
  else if (parse_->isChar('[')) {
    parse_->skipChar();

    parse_->skipSpace();

    if (! parse_->readInteger(dim))
      goto fail;

    if (*dim <= 0)
      goto fail;

    parse_->skipSpace();

    if (! parse_->isChar(']'))
      goto fail;

    parse_->skipChar();

    parse_->skipSpace();
  }

  if (! parse_->isChar(':'))
    goto fail;

  parse_->skipChar();

  return true;

 fail:
  parse_->setPos(pos);

  return false;
}

bool
CVML::
parseArgument(CVMLArgument &argument)
{
  int pos = parse_->getPos();

  if (parse_->isChar('.')) {
    int  value;
    bool is_char;

    if (parseValue(&value, &is_char)) {
      if (is_char)
        argument.setArgumentChar(char(value));
      else
        argument.setArgumentInteger(ushort(value));
    }
    else {
      parse_->skipChar();

      if (parse_->isIdentifier()) {
        std::string identifier;

        if (! parse_->readIdentifier(identifier))
          goto fail;

        argument.setArgumentVariableValue(identifier);
      }
      else
        goto fail;
    }
  }
  else {
    bool decrement = false;

    if (parse_->isChar('-')) {
      parse_->skipChar();

      decrement = true;
    }

    std::string identifier;
    int         reg_num, offset;

    if (parse_->isDigit()) {
      if (! parse_->readInteger(&offset))
        goto fail;

      if (decrement) {
        offset = -offset;

        decrement = false;
      }

      argument.setArgumentInteger(ushort(offset));
    }
    else {
      if (! parse_->readIdentifier(identifier))
        goto fail;

      if (isRegister(identifier, &reg_num))
        argument.setArgumentRegisterAddr(ushort(reg_num));
      else
        argument.setArgumentVariableAddr(identifier);
    }

    if      (parse_->isChar('[')) {
      CVMLArgument argument1(this);

      parse_->skipChar();

      if (! parseArgument(argument1))
        goto fail;

      if (! parse_->isChar(']'))
        goto fail;

      parse_->skipChar();

      if (argument.getType() == CVML_ARGUMENT_VARIABLE_ADDR)
        argument.setType(CVML_ARGUMENT_VARIABLE_VALUE);

      if (argument.getType() != CVML_ARGUMENT_VARIABLE_VALUE &&
          argument.getType() != CVML_ARGUMENT_INTEGER)
        goto fail;

      if (argument1.getType() != CVML_ARGUMENT_REGISTER_ADDR)
        goto fail;

      if (argument.getType() == CVML_ARGUMENT_VARIABLE_VALUE)
        argument.setArgumentRegisterVarOffset(ushort(argument1.getArgRegNum()), argument.getName());
      else {
        short offset1 = unsignedToSigned(ushort(argument.getInteger()));

        argument.setArgumentRegisterOffset(ushort(argument1.getArgRegNum()), offset1);
      }

      if (parse_->isChar('^')) {
        parse_->skipChar();

        if (argument.getType() == CVML_ARGUMENT_REGISTER_OFFSET)
          argument.setType(CVML_ARGUMENT_REGISTER_OFFSET_DEFERRED);
        else
          argument.setType(CVML_ARGUMENT_REGISTER_VAR_OFFSET_DEFERRED);
      }
    }
    else if (parse_->isChar('^')) {
      bool deferred  = false;
      bool increment = false;

      parse_->skipChar();

      if (parse_->isChar('^')) {
        deferred = true;

        parse_->skipChar();
      }

      if (parse_->isChar('+')) {
        increment = true;

        parse_->skipChar();
      }

      if (decrement && increment)
        goto fail;

      if (argument.getType() != CVML_ARGUMENT_REGISTER_ADDR)
        goto fail;

      argument.setType(CVML_ARGUMENT_REGISTER_VALUE);

      if      (decrement) {
        if (deferred)
          argument.setType(CVML_ARGUMENT_REGISTER_VALUE_DEFERRED_DECR);
        else
          argument.setType(CVML_ARGUMENT_REGISTER_VALUE_DECR);
      }
      else if (increment) {
        if (deferred)
          argument.setType(CVML_ARGUMENT_REGISTER_VALUE_DEFERRED_INCR);
        else
          argument.setType(CVML_ARGUMENT_REGISTER_VALUE_INCR);
      }
    }
    else {
      if (decrement)
        goto fail;
    }
  }

  return true;

 fail:
  parse_->setPos(pos);

  return false;
}

bool
CVML::
parseValue(int *value, bool *is_char)
{
  int pos = parse_->getPos();

  if (! parse_->isChar('.'))
    goto fail;

  parse_->skipChar();

  if      (parse_->isChar('\'')) {
    char c;

    parse_->skipChar();

    std::string str;

    while (! parse_->eof()) {
      if      (parse_->isChar('\\')) {
        if (! parse_->readChar(&c))
          break;

        str += c;

        if (! parse_->eof()) {
          if (! parse_->readChar(&c))
            break;

          str += c;
        }
      }
      else if (parse_->isChar('\''))
        break;
      else {
        if (! parse_->readChar(&c))
          goto fail;

        str += c;
      }
    }

    if (parse_->eof())
      goto fail;

    parse_->skipChar();

    if (! stringToChar(str, &c))
      goto fail;

    *value   = c;
    *is_char = true;
  }
  else if (parse_->isChar('-')) {
    int i;

    parse_->skipChar();

    if (parse_->isDigit()) {
      if (! parse_->readInteger(&i))
        goto fail;

      i = -i;

      *value   = i;
      *is_char = false;
    }
    else
      goto fail;
  }
  else if (parse_->isChar('0')) {
    int i;

    if (! parse_->readBaseInteger(8, &i))
      goto fail;

    *value   = i;
    *is_char = false;
  }
  else if (parse_->isDigit()) {
    int i;

    if (! parse_->readInteger(&i))
      goto fail;

    *value   = i;
    *is_char = false;
  }
  else
    goto fail;

  return true;

 fail:
  parse_->setPos(pos);

  return false;
}

bool
CVML::
parseString(std::string &str)
{
  char c;

  int pos = parse_->getPos();

  bool found = false;

  if (! parse_->isChar('\"'))
    goto fail;

  parse_->skipChar();

  while (! parse_->eof()) {
    if (! parse_->readChar(&c))
      goto fail;

    if (c == '\\') {
      if (! parse_->readChar(&c))
        goto fail;

      if      (c == 'b')
        str += '\b';
      else if (c == 'n')
        str += '\n';
      else if (c == 't')
        str += '\t';
      else if (c == '0')
        str += '\0';
      else if (c == '\'')
        str += '\'';
      else if (c == '\"')
        str += '\"';
      else {
        std::cerr << "Invalid escape code" << std::endl;
        goto fail;
      }
    }
    else {
      if (c == '\"') {
        found = true;
        break;
      }

      str += c;
    }
  }

  if (! found)
    goto fail;

  return true;

 fail:
  parse_->setPos(pos);

  return false;
}

bool
CVML::
parseInstruction(std::string &identifier)
{
  int pos = parse_->getPos();

  if (! parse_->isAlpha())
    goto fail;

  char c;

  if (! parse_->readChar(&c))
    goto fail;

  identifier = "";

  identifier += c;

  while (! parse_->eof() && ! parse_->isSpace()) {
    if (! parse_->readChar(&c))
      goto fail;

    identifier += c;
  }

  return true;

 fail:
  parse_->setPos(pos);

  return false;
}

//------------------------

bool
CVML::
parseBin(const std::string &ifilename, const std::string &ofilename)
{
  CFile file(ofilename);

  std::vector<ushort> codes;
  uint                num_codes, size;

  reset();

  if (! writeHeader(&file)) {
    std::cerr << "Invalid VML file" << std::endl;
    goto fail;
  }

  if (! writeStringTable(&file)) {
    std::cerr << "Bad String Table" << std::endl;
    goto fail;
  }

  if (! writeDebug(&file)) {
    std::cerr << "Bad Debug Section" << std::endl;
    goto fail;
  }

  if (! writeData(&file)) {
    std::cerr << "Bad Data Section" << std::endl;
    goto fail;
  }

  pc_ = 0;

  if (! parseBinFile(ifilename, codes))
    goto fail;

  num_codes = uint(codes.size());

  size = num_codes*sizeof(ushort);

  file.write(reinterpret_cast<uchar *>(&size), sizeof(size));

  for (uint i = 0; i < num_codes; ++i) {
    ushort code = codes[i];

    file.write(reinterpret_cast<uchar *>(&code), sizeof(code));
  }

  file.close();

  return true;

 fail:
  file.close();

  return false;
}

bool
CVML::
parseBinFile(const std::string &filename, std::vector<ushort> &codes)
{
  fileName_ = filename;

  if (! CFile::exists(fileName_)) {
    std::cerr << "File \'" << fileName_ << "\' does not exist" << std::endl;
    return false;
  }

  file_ = new CFile(fileName_);

  while (file_->readLine(line_)) {
    ++line_num_;

    if (! parseBinLine(codes)) {
      syntaxError();
      delete file_;
      return false;
    }
  }

  delete file_;

  return true;
}

bool
CVML::
parseBinLine(std::vector<ushort> &codes)
{
  parse_ = new CStrParse(line_);

  parse_->skipSpace();

  std::string address, code;

  char c;

  while (parse_->isDigit()) {
    if (! parse_->readChar(&c))
      goto fail;

    address += c;
  }

  parse_->skipSpace();

  if (! parse_->isChar(':'))
    goto fail;

  if (! parse_->skipChar())
    goto fail;

  parse_->skipSpace();

  while (parse_->isDigit()) {
    if (! parse_->readChar(&c))
      goto fail;

    code += c;
  }

  parse_->skipSpace();

  if (! parse_->eof())
    goto fail;

  int address_val, code_val;

  if (! CStrUtil::toBaseInteger(address, 8, &address_val))
    goto fail;

  if (! CStrUtil::toBaseInteger(code   , 8, &code_val   ))
    goto fail;

  while (pc_ < ushort(address_val)) {
    codes.push_back(0);

    pc_ += 2;
  }

  codes.push_back(ushort(code_val));

  pc_ += 2;

  return true;

 fail:
  return false;
}
