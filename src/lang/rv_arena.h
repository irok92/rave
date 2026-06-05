#pragma once

struct rv_arena {

};

rv_arena* rv_arena_create(size_t min_size);
void* rv_arena_push_by(void* data, size_t count);
void* rv_arena_pop_by(size_t count);
size_t rv_arena_pop_to(void* mark);
void* rv_arena_mark();
