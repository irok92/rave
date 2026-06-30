#include "rave/context.hpp"



int main(int argc, char** argv)
{
   using namespace Rave;
    ContextConfig config;
    if(!ContextParseArgs(&config, argc, argv)) {
        return 0;
    }

    Ptr<Context> rave = CreateContext(&config);


    while (LoopContext(rave)) {

    }

    Rave::DestroyContext(rave);

    return 0;
}