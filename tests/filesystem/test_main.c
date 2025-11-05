#include "../../include/libd/testing.h"
#include "./filepath_resolver.c"
#include "./filesystem.c"

TEST_MAIN
// filepath resolver tests
REGISTER(filepath_resolver_create_destroy);
REGISTER(filepath_resolver_tokenize);
REGISTER(filepath_resolver_expand);

END_TEST_MAIN
