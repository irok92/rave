#include <catch_amalgamated.hpp>

#include "rv_arena.h"

namespace {
	struct rv_os_mem_initializer {
		rv_os_mem_initializer() { rv_os_mem_init(); }
	} init_os_mem;
}

struct test_struct {
	rv_s32 a;
	rv_f64 b;
	rv_c8 c[16];
};

TEST_CASE("rv_arena_create_release", "[arena]") {
	rv_arena* arena = RAVE_NEW_ARENA(RV_ARENA_NO_FLAGS, 1024 * 1024, 4096, RV_NULL);
	REQUIRE(arena != RV_NULL);
	rv_arena_release(arena);
}

TEST_CASE("rv_arena_push_basic", "[arena]") {
	rv_arena* arena = RAVE_NEW_ARENA(RV_ARENA_NO_FLAGS, 1024 * 1024, 4096, RV_NULL);

	rv_s32* a = (rv_s32*)rv_arena_push(arena, sizeof(rv_s32), alignof(rv_s32), false);
	REQUIRE(a != RV_NULL);
	*a = 42;
	REQUIRE(*a == 42);

	rv_f64* b = (rv_f64*)rv_arena_push(arena, sizeof(rv_f64), alignof(rv_f64), true);
	REQUIRE(b != RV_NULL);
	REQUIRE(*b == 0.0);
	*b = 3.14;
	REQUIRE(*b == 3.14);

	rv_arena_release(arena);
}

TEST_CASE("rv_arena_push_zeroed", "[arena]") {
	rv_arena* arena = RAVE_NEW_ARENA(RV_ARENA_NO_FLAGS, 1024 * 1024, 4096, RV_NULL);

	rv_u8* mem = (rv_u8*)rv_arena_push(arena, 128, 8, true);
	REQUIRE(mem != RV_NULL);
	for (rv_u64 i = 0; i < 128; i++) {
		REQUIRE(mem[i] == 0);
	}

	rv_arena_release(arena);
}

TEST_CASE("rv_arena_alignment", "[arena]") {
	rv_arena* arena = RAVE_NEW_ARENA(RV_ARENA_NO_FLAGS, 1024 * 1024, 4096, RV_NULL);

	rv_u8* a = (rv_u8*)rv_arena_push(arena, 1, 1, false);
	REQUIRE(a != RV_NULL);
	REQUIRE((rv_u64)a % 1 == 0);

	rv_u64* b = (rv_u64*)rv_arena_push(arena, sizeof(rv_u64), alignof(rv_u64), false);
	REQUIRE(b != RV_NULL);
	REQUIRE((rv_u64)b % alignof(rv_u64) == 0);

	void* c = rv_arena_push(arena, 16, 16, false);
	REQUIRE(c != RV_NULL);
	REQUIRE((rv_u64)c % 16 == 0);

	rv_arena_release(arena);
}

TEST_CASE("rv_arena_get_pos_and_pop_to", "[arena]") {
	rv_arena* arena = RAVE_NEW_ARENA(RV_ARENA_NO_FLAGS, 1024 * 1024, 4096, RV_NULL);

	rv_u64 pos0 = rv_arena_get_pos(arena);

	rv_arena_push(arena, 64, 8, false);
	rv_u64 pos1 = rv_arena_get_pos(arena);
	REQUIRE(pos1 > pos0);

	rv_arena_push(arena, 64, 8, false);
	rv_u64 pos2 = rv_arena_get_pos(arena);
	REQUIRE(pos2 > pos1);

	rv_arena_pop_to(arena, pos1);
	REQUIRE(rv_arena_get_pos(arena) == pos1);

	rv_arena_push(arena, 32, 8, false);
	REQUIRE(rv_arena_get_pos(arena) == pos1 + 32);

	rv_arena_pop_to(arena, pos0);
	REQUIRE(rv_arena_get_pos(arena) == pos0);

	rv_arena_release(arena);
}

TEST_CASE("rv_arena_pop", "[arena]") {
	rv_arena* arena = RAVE_NEW_ARENA(RV_ARENA_NO_FLAGS, 1024 * 1024, 4096, RV_NULL);

	rv_u64 pos0 = rv_arena_get_pos(arena);
	rv_arena_push(arena, 100, 8, false);
	rv_u64 pos1 = rv_arena_get_pos(arena);

	rv_arena_pop(arena, 40);
	rv_u64 after_pop = rv_arena_get_pos(arena);
	REQUIRE(after_pop == pos1 - 40);

	rv_arena_pop(arena, after_pop - pos0);
	REQUIRE(rv_arena_get_pos(arena) == pos0);

	rv_arena_release(arena);
}

TEST_CASE("rv_arena_clear", "[arena]") {
	rv_arena* arena = RAVE_NEW_ARENA(RV_ARENA_NO_FLAGS, 1024 * 1024, 4096, RV_NULL);

	rv_u64 initial = rv_arena_get_pos(arena);

	rv_arena_push(arena, 256, 8, false);
	rv_arena_push(arena, 256, 8, false);
	rv_arena_push(arena, 256, 8, false);
	REQUIRE(rv_arena_get_pos(arena) > initial);

	rv_arena_clear(arena);
	REQUIRE(rv_arena_get_pos(arena) == initial);

	rv_arena_push(arena, 128, 8, false);
	REQUIRE(rv_arena_get_pos(arena) > initial);

	rv_arena_release(arena);
}

TEST_CASE("rv_arena_markers", "[arena]") {
	rv_arena* arena = RAVE_NEW_ARENA(RV_ARENA_NO_FLAGS, 1024 * 1024, 4096, RV_NULL);

	rv_arena_marker m1 = rv_arena_start_marker(arena);
	rv_arena_push(arena, 64, 8, false);
	rv_arena_push(arena, 64, 8, false);
	rv_arena_push(arena, 64, 8, false);
	rv_arena_marker m2 = rv_arena_start_marker(arena);
	rv_arena_push(arena, 32, 8, false);

	rv_arena_end_marker(arena, m2);
	rv_u64 pos_after_m2 = rv_arena_get_pos(arena);
	REQUIRE(pos_after_m2 == m2.position);

	rv_arena_end_marker(arena, m1);
	REQUIRE(rv_arena_get_pos(arena) == m1.position);

	rv_arena_release(arena);
}

TEST_CASE("rv_arena_new_template", "[arena]") {
	rv_arena* arena = RAVE_NEW_ARENA(RV_ARENA_NO_FLAGS, 1024 * 1024, 4096, RV_NULL);

	test_struct* ts = rv_arena_new<test_struct>(arena);
	REQUIRE(ts != RV_NULL);
	ts->a = 10;
	ts->b = 2.5;
	REQUIRE(ts->a == 10);
	REQUIRE(ts->b == 2.5);

	rv_arena_release(arena);
}

TEST_CASE("rv_arena_new_n_template", "[arena]") {
	rv_arena* arena = RAVE_NEW_ARENA(RV_ARENA_NO_FLAGS, 1024 * 1024, 4096, RV_NULL);

	rv_s32* arr = rv_arena_new_n<rv_s32>(arena, 10);
	REQUIRE(arr != RV_NULL);
	for (rv_s32 i = 0; i < 10; i++) {
		arr[i] = i * 2;
	}
	for (rv_s32 i = 0; i < 10; i++) {
		REQUIRE(arr[i] == i * 2);
	}

	rv_arena_release(arena);
}

TEST_CASE("rv_arena_push_n_templates", "[arena]") {
	rv_arena* arena = RAVE_NEW_ARENA(RV_ARENA_NO_FLAGS, 1024 * 1024, 4096, RV_NULL);

	rv_f32* zeroed = rv_arena_push_n<rv_f32>(arena, 20);
	REQUIRE(zeroed != RV_NULL);
	for (rv_s32 i = 0; i < 20; i++) {
		REQUIRE(zeroed[i] == 0.0f);
	}

	rv_f32* no_zero = rv_arena_push_array_no_zero<rv_f32>(arena, 20);
	REQUIRE(no_zero != RV_NULL);

	rv_arena_release(arena);
}

TEST_CASE("rv_arena_chaining_large_alloc", "[arena]") {
	rv_arena* arena =
		RAVE_NEW_ARENA(RV_ARENA_NO_FLAGS, 64 * 1024, 64 * 1024, RV_NULL);

	rv_u64 total
		= 64 * 1024 + 64 * 1024 + 4096;
	rv_u8* big = (rv_u8*)rv_arena_push(arena, total, 8, false);
	REQUIRE(big != RV_NULL);
	big[0]		 = 1;
	big[total - 1] = 2;
	REQUIRE(big[0] == 1);
	REQUIRE(big[total - 1] == 2);

	rv_arena_release(arena);
}

TEST_CASE("rv_arena_no_chain_flag", "[arena]") {
	rv_arena* arena =
		RAVE_NEW_ARENA(RV_ARENA_NO_CHAIN, 64 * 1024, 64 * 1024, RV_NULL);

	rv_u8* ok = (rv_u8*)rv_arena_push(arena, 1024, 8, false);
	REQUIRE(ok != RV_NULL);

	rv_u8* fail = (rv_u8*)rv_arena_push(arena, 1024 * 1024, 8, false);
	REQUIRE(fail == RV_NULL);

	rv_arena_release(arena);
}

TEST_CASE("rv_arena_backing_buffer", "[arena]") {
	rv_u64 buf_size = 1024 * 1024;
	void*  buffer   = rv_os_malloc(buf_size);
	REQUIRE(buffer != RV_NULL);

	rv_arena* arena = rv_arena_create(
		RV_ARENA_NO_FLAGS, buf_size, buf_size, buffer, __FILE__, __LINE__
	);
	REQUIRE(arena != RV_NULL);
	REQUIRE((void*)arena == buffer);

	rv_s32* p = (rv_s32*)rv_arena_push(arena, sizeof(rv_s32), alignof(rv_s32), false);
	REQUIRE(p != RV_NULL);
	*p = 99;
	REQUIRE(*p == 99);

	rv_arena_release(arena);
	rv_os_free(buffer);
}

TEST_CASE("rv_arena_pop_more_than_allocated", "[arena]") {
	rv_arena* arena = RAVE_NEW_ARENA(RV_ARENA_NO_FLAGS, 1024 * 1024, 4096, RV_NULL);

	rv_arena_push(arena, 100, 8, false);
	rv_u64 pos = rv_arena_get_pos(arena);

	rv_arena_pop(arena, 1000);
	REQUIRE(rv_arena_get_pos(arena) == pos);

	rv_arena_release(arena);
}

TEST_CASE("rv_arena_pop_zero", "[arena]") {
	rv_arena* arena = RAVE_NEW_ARENA(RV_ARENA_NO_FLAGS, 1024 * 1024, 4096, RV_NULL);

	rv_arena_push(arena, 64, 8, false);
	rv_u64 pos = rv_arena_get_pos(arena);

	rv_arena_pop(arena, 0);
	REQUIRE(rv_arena_get_pos(arena) == pos);

	rv_arena_release(arena);
}

TEST_CASE("rv_arena_clear_empty", "[arena]") {
	rv_arena* arena = RAVE_NEW_ARENA(RV_ARENA_NO_FLAGS, 1024 * 1024, 4096, RV_NULL);
	rv_u64 initial = rv_arena_get_pos(arena);
	rv_arena_clear(arena);
	REQUIRE(rv_arena_get_pos(arena) == initial);
	rv_arena_release(arena);
}

TEST_CASE("rv_arena_get_pos_initial", "[arena]") {
	rv_arena* arena = RAVE_NEW_ARENA(RV_ARENA_NO_FLAGS, 1024 * 1024, 4096, RV_NULL);
	REQUIRE(rv_arena_get_pos(arena) > 0);
	rv_arena_release(arena);
}

TEST_CASE("rv_arena_pop_to_zero", "[arena]") {
	rv_arena* arena = RAVE_NEW_ARENA(RV_ARENA_NO_FLAGS, 1024 * 1024, 4096, RV_NULL);

	rv_u64 initial = rv_arena_get_pos(arena);
	rv_arena_push(arena, 256, 8, false);
	rv_arena_push(arena, 256, 8, false);

	rv_arena_pop_to(arena, 0);
	REQUIRE(rv_arena_get_pos(arena) == initial);

	rv_arena_release(arena);
}

TEST_CASE("rv_arena_marker_across_chain", "[arena]") {
	rv_arena* arena =
		RAVE_NEW_ARENA(RV_ARENA_NO_FLAGS, 64 * 1024, 64 * 1024, RV_NULL);

	rv_arena_marker m1 = rv_arena_start_marker(arena);
	rv_arena_push(arena, 64 * 1024 - 128, 8, false);
	rv_arena_push(arena, 64 * 1024, 8, false);

	rv_arena_end_marker(arena, m1);
	REQUIRE(rv_arena_get_pos(arena) == m1.position);

	rv_arena_release(arena);
}

TEST_CASE("rv_arena_reuse_freed_blocks", "[arena]") {
	rv_arena* arena =
		RAVE_NEW_ARENA(RV_ARENA_NO_FLAGS, 64 * 1024, 64 * 1024, RV_NULL);

	rv_arena_push(arena, 64 * 1024, 8, false);
	rv_arena_push(arena, 64 * 1024, 8, false);
	rv_u64 pos = rv_arena_get_pos(arena);

	rv_arena_clear(arena);

	rv_arena_push(arena, 64 * 1024, 8, false);
	REQUIRE(rv_arena_get_pos(arena) < pos);

	rv_arena_release(arena);
}