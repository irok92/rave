#pragma once

#include "rv_common.h"

struct rv_context_config {

};

struct rv_context {
    rv_context_config config;
};


rv_context* rv_context_create(rv_context_config* config RV_OPT(nullptr));
void rv_context_destroy(rv_context* context RV_OPT(nullptr));
bool rv_context_update();
