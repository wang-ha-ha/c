#ifdef _WIN32
#include "x86/include/type_def.h"
#endif

#if defined(__mips__) || defined(__mips64__)
#include "mips/include/type_def.h"
#endif

namespace maix{
	#define mxbool	bool
	#define mxtrue	true
	#define mxfalse false
}
