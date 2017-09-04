#include <CVML.h>
#include <CArgs.h>
#include <CFile.h>

static std::string opts = "\
-c:f (compile text) \
-C:f (compile bin) \
-d:f (decode) \
-e:f (execute) \
-o:s (output file) \
-t:f (trace) \
-v:f (verbose) \
-x:f (debug) \
";

static void compile_txt
             (const std::string &ifilename, const std::string &ofilename, bool verbose);
static void compile_bin
             (const std::string &ifilename, const std::string &ofilename);
static void decode
             (const std::string &ifilename, bool verbose);
static void execute
             (const std::string &ifilename, bool trace_flag);
static void debug
             (const std::string &ifilename);

int
main(int argc, char **argv)
{
  CArgs *cargs = new CArgs(opts);

  cargs->parse(&argc, argv);

  if (argc != 2)
    exit(1);

  std::string ifilename = argv[1];
  std::string ofilename = "codefile";

  bool compile_txt_flag = cargs->getBooleanArg("-c");
  bool compile_bin_flag = cargs->getBooleanArg("-C");
  bool decode_flag      = cargs->getBooleanArg("-d");
  bool execute_flag     = cargs->getBooleanArg("-e");
  bool trace_flag       = cargs->getBooleanArg("-t");
  bool verbose_flag     = cargs->getBooleanArg("-v");
  bool debug_flag       = cargs->getBooleanArg("-x");

  if (cargs->isStringArgSet("-o"))
    ofilename = cargs->getStringArg("-o");

  if      (compile_txt_flag) {
    compile_txt(ifilename, ofilename, verbose_flag);

    ifilename = ofilename;
  }
  else if (compile_bin_flag) {
    compile_bin(ifilename, ofilename);

    ifilename = ofilename;
  }

  if (decode_flag)
    decode(ifilename, verbose_flag);

  if (execute_flag)
    execute(ifilename, trace_flag);

  if (debug_flag)
    debug(ifilename);

  return 0;
}

static void
compile_txt(const std::string &ifilename, const std::string &ofilename, bool verbose)
{
  CVML vml;

  vml.parse(ifilename);

  if (verbose)
    std::cout << vml;

  CFile file(ofilename);

  vml.encode(&file);

  file.close();
}

static void
compile_bin(const std::string &ifilename, const std::string &ofilename)
{
  CVML vml;

  vml.parseBin(ifilename, ofilename);
}

static void
decode(const std::string &ifilename, bool verbose)
{
  CVML vml;

  CFile file(ifilename);

  file.open(CFileBase::Mode::READ);

  if (verbose) {
    vml.outputSections(&file);

    file.rewind();
  }

  vml.decode(&file);

  file.close();

  std::cout << vml;
}

static void
execute(const std::string &ifilename, bool trace_flag)
{
  CVML vml;

  CFile file(ifilename);

  file.open(CFileBase::Mode::READ);

  vml.setExecuteTrace(trace_flag);

  vml.execute(&file);

  file.close();
}

static void
debug(const std::string &ifilename)
{
  CVML vml;

  CFile file(ifilename);

  file.open(CFileBase::Mode::READ);

  vml.debug(&file);

  file.close();
}
