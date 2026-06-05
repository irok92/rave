#include "rv_context.h"


rv_context*
rv_context_create(rv_context_config* config) {

    rv_context* ctx = rv_alloc_t(rv_context);
    ctx->config = *config;


    return ctx;
}


bool
rv_context_update() {
    return true;
}


void
rv_context_destroy(rv_context* ctx) {
    rv_free(ctx);
}
