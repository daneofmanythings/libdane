#include "./platform_wrap.h"

#include "../../../include/libd/platform/filesystem.h"

bool
platform_is_env_var(const char* key)
{
  return libd_platform_filesystem_is_env_var(key);
}

enum libd_result
platform_env_var_get(
  char* out_val,
  const char* env_key,
  libd_filesystem_env_get_f env_getter)
{
  return libd_platform_filesystem_env_var_get(out_val, env_key, env_getter);
}

bool
plaform_is_component_value_valid(const char* value)
{
  return libd_plaform_filesystem_is_explicit_component_value_valid(value);
}
