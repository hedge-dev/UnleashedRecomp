#pragma once

#include <cpu/guest_code.h>
#include <cpu/guest_stack_var.h>
#include <kernel/function.h>

#define SWA__CONCAT2(x, y) x##y
#define SWA_CONCAT2(x, y) _CONCAT(x, y)

#define SWA_INSERT_PADDING(length) \
    uint8_t SWA_CONCAT2(pad, __LINE__)[length]

#define SWA_VIRTUAL_FUNCTION(returnType, virtualIndex, ...) \
    GuestToHostFunction<returnType>(*(be<uint32_t>*)(g_memory.Translate(*(be<uint32_t>*)(this) + (4 * virtualIndex))), __VA_ARGS__)

struct swa_null_ctor {};
