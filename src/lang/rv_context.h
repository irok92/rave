#pragma once

#include "rv_common.h"

typedef struct rv_context_config {

} rv_context_config;



typedef struct rv_context {
    rv_context_config config;
} rv_context;


rv_context* rv_context_create(rv_context_config* config);
void rv_context_destroy(rv_context* context);
bool rv_context_update();
