

// Debugging macros



#define DEBUG(var)                                                                  \
  ({                                                                                \
    if (__builtin_types_compatible_p (__typeof__  (var), int))                      \
        printf("%s:%d (int) %s = %d\n", __FUNCTION__, __LINE__, #var, var);         \
    else if (__builtin_types_compatible_p (__typeof__  (var), char))                \
        printf("%s:%d (char) %s = '%c'\n", __FUNCTION__, __LINE__, #var, var);      \
    else if (__builtin_types_compatible_p (__typeof__  (var), char[]))              \
        printf("%s:%d (char[]) %s = \"%d\"\n", __FUNCTION__, __LINE__, #var, var);  \
    else if (__builtin_types_compatible_p (__typeof__  (var), float))               \
        printf("%s:%d (float) %s = %f\n", __FUNCTION__, __LINE__, #var, var);       \
    else if (__builtin_types_compatible_p (__typeof__  (var), void*))               \
        printf("%s:%d (void*) %s = %p\n", __FUNCTION__, __LINE__, #var, var);       \
    else if (__builtin_types_compatible_p (__typeof__  (var), size_t))              \
        printf("%s:%d (size_t) %s = %ld\n", __FUNCTION__, __LINE__, #var, var);     \
    else                                                                            \
        printf("%s:%d  %s = %ld  (unknown type, size=%d bytes, shown as int)\n", __FUNCTION__, __LINE__, #var, var, sizeof(var) );  \
  })


