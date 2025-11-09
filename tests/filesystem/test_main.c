#include "../../include/libd/testing.h"
#include "./test_filepath_resolver.c"
#include "./test_filesystem.c"

TEST_MAIN
// filepath resolver tests
REGISTER(filepath_resolver_create_destroy);
REGISTER(filepath_resolver_tokenize);
REGISTER(filepath_resolver_expand);
REGISTER(filepath_resolver_normalize);
REGISTER(filepath_resolver_dump_to_filepath);

END_TEST_MAIN
