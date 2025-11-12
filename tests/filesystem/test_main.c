#include "../../include/libd/testing.h"
#include "./test_filepath.c"
#include "./test_filepath_resolver.c"

TEST_MAIN

REGISTER(filepath_init);
REGISTER(filepath_string);

REGISTER(filepath_resolver_create_destroy);
REGISTER(filepath_resolver_tokenize);
REGISTER(filepath_resolver_expand);
REGISTER(filepath_resolver_normalize);
REGISTER(filepath_resolver_dump_to_filepath);

END_TEST_MAIN
