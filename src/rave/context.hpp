#pragma once

#include "rave/common.hpp"

namespace Rave {

struct ContextConfig {
    
};

struct Context;

Ptr<Context>
CreateContext(ContextConfig* config = nullptr);

void
DestroyContext(Ptr<Context> context);


bool
LoopContext(Ptr<Context> context);

bool
ContextParseArgs(
	ContextConfig* conf,
	int			   argc,
	char**		   argv
);

} // namespace Rave