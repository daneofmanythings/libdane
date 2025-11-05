#include "./platform_wrap.h"

// TODO:
bool
platform_is_env_var(const char* val)
{
  bool result = false;
  if (*val == '$') {
    result = true;
  }
  return result;
}

// TODO:
void
platform_env_var_mem_free(const char* env_var_str)
{
  return;
}
