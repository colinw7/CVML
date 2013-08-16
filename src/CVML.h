#ifndef CVML_H
#define CVML_H

#include <CStrUtil.h>
#include <CStrParse.h>
#include <CFile.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <sys/types.h>

typedef unsigned char uchar;

#define CVML_MAGIC   "CVML"
#define CVML_VERSION 1

#define CVML_MEMORY_MAX 0177777

#define OP_ARG_2  1
#define OP_ARG_1H 2
#define OP_ARG_1a 3
#define OP_ARG_1b 4
#define OP_ARG_0H 5
#define OP_ARG_0  6

class CVML;
class CVMLOpCodeData;

typedef uint                               CVMLStringId;
typedef std::map<std::string,CVMLStringId> CVMLStringIdMap;
typedef CVMLStringIdMap::const_iterator    CVMLStringIdMapCI;
typedef std::map<CVMLStringId,std::string> CVMLIdStringMap;
typedef CVMLIdStringMap::const_iterator    CVMLIdStringMapCI;

struct CVMLError {
  std::string msg;

  CVMLError(const std::string &msg1) : msg(msg1) { }
};

struct CVMLOpCode {
  const char     *name;
  uint            num_args;
  uint            code;
  CVMLOpCodeData *data;
};

enum CVMLDebugCommandType {
  CVML_DEBUG_COMMAND_NONE,
  CVML_DEBUG_COMMAND_CONTINUE,
  CVML_DEBUG_COMMAND_EXIT,
  CVML_DEBUG_COMMAND_LIST,
  CVML_DEBUG_COMMAND_NEXT,
  CVML_DEBUG_COMMAND_PRINT,
  CVML_DEBUG_COMMAND_RUN,
  CVML_DEBUG_COMMAND_STEP,
  CVML_DEBUG_COMMAND_UNKNOWN
};

#include <CVMLArgument.h>
#include <CVMLLabel.h>
#include <CVMLOp.h>
#include <CVMLData.h>
#include <CVMLLine.h>
#include <CVMLDataSrc.h>

class CVML {
 private:
  typedef std::list<CVMLLabel *>   LabelList;
  typedef std::vector<CVMLLabel *> LabelArray;

  typedef std::list<CVMLLine *> LineList;

  typedef bool (*CVMLExecuteProc2)(CVML &vml, ushort data1, ushort data2);
  typedef bool (*CVMLExecuteProc1)(CVML &vml, ushort data);
  typedef bool (*CVMLExecuteProc0)(CVML &vml);

  std::string      fileName_;
  CFile           *file_;
  LabelList        labels_;
  LineList         lines_;
  std::string      line_;
  uint             line_num_;
  CStrParse       *parse_;
  LabelArray       parseLabels_;
  uint             pc_;
  uint             begin_pc_;
  uchar           *memory_;
  CVMLStringIdMap  string_id_map_;
  CVMLIdStringMap  id_string_map_;
  CVMLStringId     string_id_;
  bool             execute_trace_flag_;
  bool             execute_stop_flag_;

 public:
  CVML();
 ~CVML();

  void setExecuteStop(bool flag) { execute_stop_flag_ = flag; }
  bool getExecuteStop() { return execute_stop_flag_; }

  bool parse(const std::string &fileName);
  bool parseBin(const std::string &ifileName, const std::string &ofilename);

  friend std::ostream &operator<<(std::ostream &os, CVML &vml) {
    vml.print(os);

    return os;
  }

  void print(std::ostream &os);

  bool encode(CFile *file);

  bool outputSections(CFile *file);

  bool decode(CFile *file);

  void setExecuteTrace(bool flag);

  bool execute(CFile *file);

  bool debug(CFile *file);

  uint getPC() { return pc_; }

  void setPC(uint pc) { pc_ = pc; }

  void addPC(int offset) { pc_ += offset; }

  uint getBeginPC() { return begin_pc_; }

  void setBeginPC(uint pc) { begin_pc_ = pc; }

  void outputString(CFile *file, const std::string &str);
  void outputString(CFile *file, const char *str, uint len);

  void outputValue(CFile *file, ushort value);

  ushort addOpValue(ushort value, ushort value1, ushort shift1, ushort mask1);

  CVMLOp *instructionToOp(CVMLDataSource *src);

  ushort nextMemoryInstruction();

  CVMLLabel *lookupLabelById(CVMLStringId id);
  CVMLLabel *lookupLabelByName(const std::string &name);
  CVMLLabel *lookupLabelByPC(uint pc);

  CVMLOp *lookupOpByPC(uint pc);

  CVMLData *lookupDataByPC(uint pc);

  CVMLStringId lookupStringId(const std::string &str);

  const std::string &lookupIdString(CVMLStringId id) const;

  bool writeString(CFile *file, const std::string &str);
  bool writeString(CFile *file, const char *str, uint len);

  uint readString(CFile *file, std::string &str);

  uint getWriteStringSize(const std::string &str);
  uint getWriteStringSize(uint len);
  uint getWriteStringLen(uint len);

  bool isRegister(const std::string &name, int *reg_num);

  ushort getRegisterAddress(ushort reg_num);
  ushort getAddressRegister(ushort address);

  const char *getRegisterName(ushort reg_num);

  bool isEvenRegisterAddress(ushort);

  bool getVariableValue(const std::string &name, uint *value);
  bool getVariableAddress(const std::string &name, uint *addr);

  ushort getRegisterWord(ushort reg_num);

  void setRegisterWord(ushort reg_num, ushort value);
  void incRegisterWord(ushort reg_num, ushort value);

  uchar  getMemoryByte(ushort addr);
  ushort getMemoryWord(ushort addr);

  void setMemoryByte(ushort addr, uchar value);
  void setMemoryWord(ushort addr, ushort value);
  void setMemoryString(ushort addr, const std::string &str);

  void divideByZero();

  int    unsignedToSigned(uint u);
  uint   signedToUnsigned(int s);
  short  unsignedToSigned(ushort u);
  ushort signedToUnsigned(short s);
  char   unsignedToSigned(uchar u);
  uchar  signedToUnsigned(char s);

 private:
  void reset();

  void deleteLabels();
  void deleteLines();

  bool parseFile(const std::string &filename);
  bool parseNextLine(bool *eof);
  bool parseLine();
  bool parsePreProLine();

  bool parseLabel(std::string &label, int *dim);
  bool parseArgument(CVMLArgument &argument);
  bool parseValue(int *value, bool *is_char);
  bool parseString(std::string &str);
  bool parseInstruction(std::string &identifier);

  bool parseBinFile(const std::string &filename, std::vector<ushort> &codes);
  bool parseBinLine(std::vector<ushort> &codes);

  bool writeHeader(CFile *file);
  bool writeStringTable(CFile *file);
  bool writeDebug(CFile *file);
  bool writeData(CFile *file);
  bool writeInstructions(CFile *file);

  bool outputHeader(CFile *file);
  bool outputStringTable(CFile *file);
  bool outputDebug(CFile *file);
  bool outputData(CFile *file);
  bool outputInstructions(CFile *file);
  bool outputMemory(uchar *data, uint size);

  bool readHeader(CFile *file);
  bool readStringTable(CFile *file);
  bool skipStringTable(CFile *file);
  bool readDebug(CFile *file);
  bool skipDebug(CFile *file);
  bool readData(CFile *file);
  bool skipData(CFile *file);
  bool readInstructions(CFile *file);
  bool skipInstructions(CFile *file);

  bool loadData(CFile *file);
  bool loadInstructions(CFile *file);

  bool executeMemory();
  bool executeInstructions();
  bool executeNextInstruction();
  bool executeInstruction(ushort i);
  bool executeOpCode2(ushort i);
  bool executeOpCode1H(ushort i);
  bool executeOpCode1a(ushort i);
  bool executeOpCode1b(ushort i);
  bool executeOpCode0H(ushort i);
  bool executeOpCode0(ushort i);

  bool getExecuteProc2 (ushort i, CVMLExecuteProc2 *proc, bool *is_byte);
  bool getExecuteProc1H(ushort i, CVMLExecuteProc2 *proc, bool *is_byte);
  bool getExecuteProc1a(ushort i, CVMLExecuteProc1 *proc, bool *is_byte);
  bool getExecuteProc1b(ushort i, CVMLExecuteProc1 *proc, bool *is_byte);
  bool getExecuteProc0H(ushort i, CVMLExecuteProc1 *proc, bool *is_byte);
  bool getExecuteProc0 (ushort i, CVMLExecuteProc0 *proc, bool *is_byte);

  bool getExecuteData(ushort mode, ushort data, ushort *extraData);

  bool processExecuteData(ushort mode, ushort data, ushort extraData,
                          bool is_byte, ushort *processData);

  static bool executeMov (CVML &vml, ushort data1, ushort data2);
  static bool executeCmp (CVML &vml, ushort data1, ushort data2);
  static bool executeAnd (CVML &vml, ushort data1, ushort data2);
  static bool executeXor (CVML &vml, ushort data1, ushort data2);
  static bool executeOr  (CVML &vml, ushort data1, ushort data2);
  static bool executeAdd (CVML &vml, ushort data1, ushort data2);
  static bool executeMovb(CVML &vml, ushort data1, ushort data2);
  static bool executeCmpb(CVML &vml, ushort data1, ushort data2);
  static bool executeAndb(CVML &vml, ushort data1, ushort data2);
  static bool executeXorb(CVML &vml, ushort data1, ushort data2);
  static bool executeOrb (CVML &vml, ushort data1, ushort data2);
  static bool executeSub (CVML &vml, ushort data1, ushort data2);
  static bool executeCall(CVML &vml, ushort data1, ushort data2);
  static bool executeMul (CVML &vml, ushort data1, ushort data2);
  static bool executeDiv (CVML &vml, ushort data1, ushort data2);
  static bool executeSll (CVML &vml, ushort data1, ushort data2);
  static bool executeSlc (CVML &vml, ushort data1, ushort data2);
  static bool executeSrl (CVML &vml, ushort data1, ushort data2);
  static bool executeSrc (CVML &vml, ushort data1, ushort data2);
  static bool executeBrn (CVML &vml, ushort data1);
  static bool executeBne (CVML &vml, ushort data1);
  static bool executeBeq (CVML &vml, ushort data1);
  static bool executeBge (CVML &vml, ushort data1);
  static bool executeBlt (CVML &vml, ushort data1);
  static bool executeBgt (CVML &vml, ushort data1);
  static bool executeBle (CVML &vml, ushort data1);
  static bool executeBvc (CVML &vml, ushort data1);
  static bool executeBvs (CVML &vml, ushort data1);
  static bool executeBcc (CVML &vml, ushort data1);
  static bool executeBcs (CVML &vml, ushort data1);
  static bool executeSwab(CVML &vml, ushort data1);
  static bool executeNot (CVML &vml, ushort data1);
  static bool executeNeg (CVML &vml, ushort data1);
  static bool executeExit(CVML &vml, ushort data1);
  static bool executeNop (CVML &vml);
  static bool executeStop(CVML &vml);

  bool debugLoop();
  bool parseDebugLine(const std::string &line, CVMLDebugCommandType *commandType,
                      std::vector<std::string> &args);
  bool debugContinue(std::vector<std::string> &args);
  bool debugList(std::vector<std::string> &args);
  bool debugNext(std::vector<std::string> &args);
  bool debugPrint(std::vector<std::string> &args);
  bool debugRun(std::vector<std::string> &args);
  bool debugStep(std::vector<std::string> &args);
  bool debugPrintCurrentLine();
  bool debugPrintOffsetLine(ushort offset);
  bool debugPrintValue(const std::string &arg);

  void setArgumentModeData(CVMLArgument *argument, ushort mode, ushort data, ushort extraData);

  bool decodeInstruction(CVMLDataSource *src, ushort i, CVMLOpCode **op_code,
                         uint *mode1, uint *data1, uint *extraData1,
                         uint *mode2, uint *data2, uint *extraData2);

  bool decodeOpCode  (CVMLDataSource *src, ushort i, CVMLOpCode **op_code,
                      uint *mode1, uint *data1, uint *extraData1,
                      uint *mode2, uint *data2, uint *extraData2);
  bool decodeOpCode2 (CVMLDataSource *src, ushort i, CVMLOpCode **op_code,
                      uint *mode1, uint *data1, uint *extraData1,
                      uint *mode2, uint *data2, uint *extraData2);
  bool decodeOpCode1H(CVMLDataSource *src, ushort i, CVMLOpCode **op_code,
                      uint *mode1, uint *data1, uint *extraData1,
                      uint *mode2, uint *data2, uint *extraData2);
  bool decodeOpCode1a(CVMLDataSource *src, ushort i, CVMLOpCode **op_code,
                      uint *mode1, uint *data1, uint *extraData1,
                      uint *mode2, uint *data2, uint *extraData2);
  bool decodeOpCode1b(CVMLDataSource *src, ushort i, CVMLOpCode **op_code,
                      uint *mode1, uint *data1, uint *extraData1,
                      uint *mode2, uint *data2, uint *extraData2);
  bool decodeOpCode0H(CVMLDataSource *src, ushort i, CVMLOpCode **op_code,
                      uint *mode1, uint *data1, uint *extraData1,
                      uint *mode2, uint *data2, uint *extraData2);
  bool decodeOpCode0 (CVMLDataSource *src, ushort i, CVMLOpCode **op_code,
                      uint *mode1, uint *data1, uint *extraData1,
                      uint *mode2, uint *data2, uint *extraData2);

  bool getModeData(CVMLDataSource *src, uint mode, uint data, uint *extraData);

  CVMLOpCode *getOpCode(ushort i);
  CVMLOpCode *getOpCode2(ushort i);
  CVMLOpCode *getOpCode1H(ushort i);
  CVMLOpCode *getOpCode1a(ushort i);
  CVMLOpCode *getOpCode1b(ushort i);
  CVMLOpCode *getOpCode0H(ushort i);
  CVMLOpCode *getOpCode0(ushort i);

  void initMemory();

  void traceMessage(const char *format, ...);

  void   stackPush(ushort data);
  ushort stackPop();

  void setNegativeFlag(bool flag);
  void setZeroFlag(bool flag);
  void setCarryFlag(bool flag);
  void setOverflowFlag(bool flag);

  bool getNegativeFlag();
  bool getZeroFlag();
  bool getCarryFlag();
  bool getOverflowFlag();

  bool stringToChar(const std::string &str, char *c);

  CVMLLabel *addLabel(CVMLStringId id);
  CVMLLabel *addLabel(CVMLStringId id, uint pc);
  CVMLLabel *addLabel(const std::string &str);
  CVMLLabel *addLabel(const std::string &str, uint pc);

  CVMLOpCode *lookupOpCode(const std::string &identifier);

  CVMLLine *addLine(CVMLLine *line);

  void syntaxError();
};

#endif
