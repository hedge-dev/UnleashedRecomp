#pragma once

#include <cpu/guest_code.h>
#include <kernel/function.h>

#define SWA__CONCAT2(x, y) x##y
#define SWA_CONCAT2(x, y) _CONCAT(x, y)

#define SWA_INSERT_PADDING(length) \
    uint8_t SWA_CONCAT2(pad, __LINE__)[length]

struct swa_null_ctor {};
