#include "rv_context.h"

#include "rv_os_mem.h"

rv_context*
rv_context_create(rv_context_config* config) {
	rv_os_mem_init();

    rv_context* ctx = rv_os_alloc_t(rv_context);
    if(config != RV_NULL) {
    	ctx->config = *config;
    }

    return ctx;
}


bool
rv_context_update() {
    return true;
}


void
rv_context_destroy(rv_context* ctx) {
    rv_os_free(ctx);
}
