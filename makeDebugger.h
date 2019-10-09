

//this doesn't work, but it would be nice if it did:

#define MakeDebugger( dbgfn , config )\
\
#ifdef config\
#include "easyconsole.h"\
static EasyConsole<decltype(config)> dbgfn(config, true);\
#else\
static void dbgfn(...) {}\
#endif
