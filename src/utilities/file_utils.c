#include "utilities/file_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t get_file_size(FILE *f) {
	fseek(f, 0, SEEK_END);
	size_t len = ftell(f);
	rewind(f);
    return len;
}

#ifdef __cplusplus
} // extern "C"
#endif
