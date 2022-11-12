#pragma once
// ********** Common Methods *****
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *mem_map(const char *path, size_t *p_size);

#ifdef __cplusplus
}
#endif