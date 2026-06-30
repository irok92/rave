#include "context.hpp"

namespace Rave {


struct Worker {

};

struct Context {
    ContextConfig config;
    Worker* workers;
};



bool ContextParseArgs(ContextConfig *conf, int argc, char **argv)
{
    return true;
}


Ptr<Context>
CreateContext(ContextConfig* conf)
{
    Ptr<Context> context = New<Context>();
    
    if(conf != nullptr) {
        context->config = *conf;
    }

    return context;
}



void
DestroyContext(Ptr<Context> context)
{
    
}


bool
LoopContext(Ptr<Context> context)
{
    return true;
}


}