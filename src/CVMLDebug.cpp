#include "CVMLI.h"

bool
CVML::
debug(CFile *file)
{
  reset();

  initMemory();

  if (! readHeader(file)) {
    std::cerr << "Invalid VML file" << std::endl;
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

  ulong pos = file->getPos();

  if (! readInstructions(file)) {
    std::cerr << "Bad Code Section" << std::endl;
    return false;
  }

  file->setPos(pos);

  if (! loadInstructions(file)) {
    std::cerr << "Bad Code Section" << std::endl;
    return false;
  }

  debugLoop();

  return true;
}

bool
CVML::
debugLoop()
{
  std::vector<std::string> args;
  CReadLine                readline;
  CVMLDebugCommandType     commandType;

  setExecuteStop(false);

  bool done = false;

  debugPrintCurrentLine();

  std::string line      = "";
  std::string last_line = "";

  while (! done) {
    readline.setPrompt("> ");

    last_line = line;
    line      = readline.readLine();

    if (line == "")
      line = last_line;

    if (! parseDebugLine(line, &commandType, args))
      continue;

    switch (commandType) {
      case CVML_DEBUG_COMMAND_EXIT:
        done = true;
        break;

      case CVML_DEBUG_COMMAND_CONTINUE:
        debugContinue(args);
        break;
      case CVML_DEBUG_COMMAND_LIST:
        debugList(args);
        break;
      case CVML_DEBUG_COMMAND_NEXT:
        debugNext(args);
        break;
      case CVML_DEBUG_COMMAND_PRINT:
        debugPrint(args);
        break;
      case CVML_DEBUG_COMMAND_RUN:
        debugRun(args);
        break;
      case CVML_DEBUG_COMMAND_STEP:
        debugStep(args);
        break;
      default:
        break;
    }
  }

  return true;
}

bool
CVML::
debugContinue(std::vector<std::string> &)
{
  if (getExecuteStop())
    return false;

  return executeInstructions();
}

bool
CVML::
debugList(std::vector<std::string> &args)
{
  int num_args = args.size();

  int start, end;

  if      (num_args == 0) {
    start = 0;
    end   = 0;
  }
  else if (num_args == 1) {
    start = 0;
    end   = CStrUtil::toInteger(args[0]) - 1;
  }
  else if (num_args == 2) {
    start = CStrUtil::toInteger(args[0]);
    end   = CStrUtil::toInteger(args[1]);
  }
  else {
    std::cerr << "Too many arguments" << std::endl;
    return false;
  }

  for ( ; start <= end; ++start)
    debugPrintOffsetLine(start);

  return true;
}

bool
CVML::
debugNext(std::vector<std::string> &)
{
  if (getExecuteStop())
    return false;

  if (! executeNextInstruction())
    return false;

  debugPrintCurrentLine();

  return true;
}

bool
CVML::
debugPrint(std::vector<std::string> &args)
{
  uint num_args = args.size();

  for (uint i = 0; i < num_args; ++i)
    debugPrintValue(args[i]);

  return true;
}

bool
CVML::
debugRun(std::vector<std::string> &)
{
  setRegisterWord(PC_NUM, 0);

  if (! executeInstructions())
    return false;

  return true;
}

bool
CVML::
debugStep(std::vector<std::string> &)
{
  if (getExecuteStop())
    return false;

  if (! executeNextInstruction())
    return false;

  debugPrintCurrentLine();

  return true;
}

bool
CVML::
debugPrintCurrentLine()
{
  if (getExecuteStop()) {
    std::cout << ">>END<<" << std::endl;
    return false;
  }

  ushort pc = getRegisterWord(PC_NUM);

  CVMLMemoryDataSource src(this);

  CVMLOp *op1 = lookupOpByPC(pc);

  if (op1 != NULL)
    op1->printCode(std::cout);

  CVMLOp *op2 = instructionToOp(&src);

  if (op2 != NULL && (op1 == NULL || ! op1->equal(*op2))) {
    std::cout << " (";

    op2->printCode(std::cout);

    std::cout << ")";
  }

  setRegisterWord(PC_NUM, pc);

  std::cout << std::endl;

  return true;
}

bool
CVML::
debugPrintOffsetLine(ushort offset)
{
  CVMLOp *op;

  ushort pc = getRegisterWord(PC_NUM);

  ushort save_pc = pc;

  while (offset > 0) {
    op = lookupOpByPC(pc);

    if (op == NULL)
      goto done;

    --offset;

    pc += op->getAddressLen();
  }

  op = lookupOpByPC(pc);

  if (op == NULL)
    goto done;

  op->printCode(std::cout);

  std::cout << std::endl;

 done:
  setRegisterWord(PC_NUM, save_pc);

  return true;
}

bool
CVML::
debugPrintValue(const std::string &arg)
{
  if      (CStrUtil::casecmp(arg, "r0"    ) == 0)
    std::cout << getRegisterWord(R0_NUM) << std::endl;
  else if (CStrUtil::casecmp(arg, "r1"    ) == 0)
    std::cout << getRegisterWord(R1_NUM) << std::endl;
  else if (CStrUtil::casecmp(arg, "r2"    ) == 0)
    std::cout << getRegisterWord(R2_NUM) << std::endl;
  else if (CStrUtil::casecmp(arg, "r3"    ) == 0)
    std::cout << getRegisterWord(R3_NUM) << std::endl;
  else if (CStrUtil::casecmp(arg, "r4"    ) == 0)
    std::cout << getRegisterWord(R4_NUM) << std::endl;
  else if (CStrUtil::casecmp(arg, "r5"    ) == 0)
    std::cout << getRegisterWord(R5_NUM) << std::endl;
  else if (CStrUtil::casecmp(arg, "r6"    ) == 0 ||
           CStrUtil::casecmp(arg, "sp"    ) == 0)
    std::cout << getRegisterWord(R6_NUM) << std::endl;
  else if (CStrUtil::casecmp(arg, "r7"    ) == 0 ||
           CStrUtil::casecmp(arg, "pc"    ) == 0)
    std::cout << getRegisterWord(R7_NUM) << std::endl;
  else if (CStrUtil::casecmp(arg, "input" ) == 0)
    std::cout << getMemoryWord(OUTPUT_ADDR) << std::endl;
  else if (CStrUtil::casecmp(arg, "output") == 0)
    std::cout << getMemoryWord(INPUT_ADDR) << std::endl;
  else if (CStrUtil::casecmp(arg, "flags") == 0) {
    std::cout << "N:" << int(getNegativeFlag()) << " " <<
                 "Z:" << int(getZeroFlag    ()) << " " <<
                 "C:" << int(getCarryFlag   ()) << " " <<
                 "V:" << int(getOverflowFlag()) << std::endl;
  }
  else {
    uint value;

    if (getVariableValue(arg, &value))
      std::cout << value << std::endl;
    else
      std::cerr << "Unknown variable " << arg << std::endl;
  }

  return true;
}

bool
CVML::
parseDebugLine(const std::string &line, CVMLDebugCommandType *commandType,
               std::vector<std::string> &args)
{
  args.clear();

  std::vector<std::string> words;

  CStrUtil::addWords(line, words);

  std::vector<std::string>::const_iterator pword1 = words.begin();
  std::vector<std::string>::const_iterator pword2 = words.end  ();

  if (pword1 == pword2) {
    *commandType = CVML_DEBUG_COMMAND_NONE;
    return true;
  }

  std::string command_name = *pword1++;

  copy(pword1, pword2, back_inserter(args));

  if      (CStrUtil::casecmp(command_name, "c"       ) == 0 ||
           CStrUtil::casecmp(command_name, "continue") == 0) {
    *commandType = CVML_DEBUG_COMMAND_CONTINUE;
    return true;
  }
  else if (CStrUtil::casecmp(command_name, "exit") == 0) {
    *commandType = CVML_DEBUG_COMMAND_EXIT;
    return true;
  }
  else if (CStrUtil::casecmp(command_name, "l"   ) == 0 ||
           CStrUtil::casecmp(command_name, "list") == 0) {
    *commandType = CVML_DEBUG_COMMAND_LIST;
    return true;
  }
  else if (CStrUtil::casecmp(command_name, "n"   ) == 0 ||
           CStrUtil::casecmp(command_name, "next") == 0) {
    *commandType = CVML_DEBUG_COMMAND_NEXT;
    return true;
  }
  else if (CStrUtil::casecmp(command_name, "p"    ) == 0 ||
           CStrUtil::casecmp(command_name, "print") == 0) {
    *commandType = CVML_DEBUG_COMMAND_PRINT;
    return true;
  }
  else if (CStrUtil::casecmp(command_name, "r"  ) == 0 ||
           CStrUtil::casecmp(command_name, "run") == 0) {
    *commandType = CVML_DEBUG_COMMAND_RUN;
    return true;
  }
  else if (CStrUtil::casecmp(command_name, "s"   ) == 0 ||
           CStrUtil::casecmp(command_name, "step") == 0) {
    *commandType = CVML_DEBUG_COMMAND_STEP;
    return true;
  }
  else {
    *commandType = CVML_DEBUG_COMMAND_UNKNOWN;
    std::cerr << "Invalid Command: " << command_name << std::endl;
    return false;
  }

  return true;
}
