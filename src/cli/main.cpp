#include "rv_cli.h"


int main(int argc, char* argv[]) {

    if(!rv_cli_init(argc, argv)) {
        return 1;
    };

    while(rv_cli_update());

    rv_cli_destroy();
    return 0;

}
