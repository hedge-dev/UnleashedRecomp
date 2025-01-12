#pragma once

#include <cpu/guest_stack_var.h>
#include <kernel/function.h>

#define SWA_CONCAT2(x, y) x##y
#define SWA_CONCAT(x, y) SWA_CONCAT2(x, y)

#define SWA_INSERT_PADDING(length) \
    uint8_t SWA_CONCAT(pad, __LINE__)[length]

#define SWA_ASSERT_OFFSETOF(type, field, offset) \
    static_assert(offsetof(type, field) == offset)

#define SWA_ASSERT_SIZEOF(type, size) \
    static_assert(sizeof(type) == size)

#define SWA_VIRTUAL_FUNCTION(returnType, virtualIndex, ...) \
    GuestToHostFunction<returnType>(*(be<uint32_t>*)(g_memory.Translate(*(be<uint32_t>*)(this) + (4 * virtualIndex))), __VA_ARGS__)

struct swa_null_ctor {};
