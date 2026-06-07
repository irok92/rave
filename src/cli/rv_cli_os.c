#include "rv_cli_os.h"


#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#undef _CRT_SECURE_NO_WARNINGS


char* rv_getenv(const char* var) {
	return getenv(var);
}
