#include "../include/libdane/filesystem.h"

#include <linux/limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct libd_filesystem_path_s {
  char path[PATH_MAX + 1];
};

libd_filesystem_result_e
libd_filesystem_path_create(libd_filesystem_path_o** out_path,
                            const char* path_string)
{
  //
  return RESULT_OK;
}

// FIX: This is mega bad and ugly.
void
expand_path_env_variables(char dest[PATH_MAX], const char* path)
{
  /*
   * If the expansion of the environment variables exceeds PATH_MAX
   * this will likely fill 'dest' with a broken path.
   */
  char* env_start = NULL;
  char* env_end = NULL;
  char temp_inspect[PATH_MAX] = {0};
  char temp_copy[PATH_MAX] = {0};
  char temp_env_var_name[PATH_MAX] = {0};
  char* temp_env_var_path = NULL;
  if (strlen(path) >= PATH_MAX) {
    return;  // leaving 1 spot for '\0' cuz why are your paths so long?
  }
  strcpy(temp_inspect, path);
  // loop until all environment variables are expanded;
  while (true) {
    // find start of environment variable '$';
    env_start = strchrnul(temp_inspect, '$');
    // if not found, exit loop;
    if (*env_start == '\0') {  // Assumes 'path' is null terminated
      break;
    }
    // find end of env variable '/' or '\0';
    env_end = env_start;
    env_end = strchrnul(env_end, '/');
    *env_start = '\0';  // setting up the copying
    *env_end = '\0';
    strcpy(temp_env_var_name, env_start + 1);
    // getenv the value
    temp_env_var_path = getenv(temp_env_var_name);
    if (temp_env_var_path == NULL) {
      fprintf(stderr,
              "ERROR | filesystem::expand_path_env_variables | environment "
              "variable='%s' not found\n",
              temp_env_var_name);
      break;  // env variable not found.
    }
    snprintf(temp_copy, PATH_MAX, "%s%s/%s", temp_inspect, temp_env_var_path,
             env_end + 1);
    // transfer to the buffer we inspect
    strcpy(temp_inspect, temp_copy);
  }
  // copy into dest;
  strcpy(dest, temp_inspect);
  if (dest[strlen(dest) - 1] == '/') {
    dest[strlen(dest) - 1] = '\0';
  }
}
