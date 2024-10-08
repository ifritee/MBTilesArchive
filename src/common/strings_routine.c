#include "strings_routine.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
  const char *start;
  size_t len;
} token;

StringArray split(const char *str, char sep)
{
  StringArray data;
//  char **array;
  unsigned int start = 0, stop, toks = 0, t;
  token *tokens = malloc((strlen(str) + 1) * sizeof(token));
  for (stop = 0; str[stop]; stop++) {
    if (str[stop] == sep) {
      tokens[toks].start = str + start;
      tokens[toks].len = stop - start;
      toks++;
      start = stop + 1;
    }
  }
  /* Mop up the last token */
  tokens[toks].start = str + start;
  tokens[toks].len = stop - start;
  toks++;
  data.strings = malloc((toks + 1) * sizeof(char*));
  for (t = 0; t < toks; t++) {
    /* Calloc makes it nul-terminated */
    char *token = calloc(tokens[t].len + 1, 1);
    memcpy(token, tokens[t].start, tokens[t].len);
    data.strings[t] = token;
  }
  /* Add a sentinel */
  data.strings[t] = NULL;
  free(tokens);
  data.count = toks;
  return data;
}

char * append_string(const char *oldString, char *newString)
{
  size_t oldLength = strlen(oldString);
  size_t newLength = strlen(newString);
  size_t len = oldLength + newLength + 1;
  char * out = (char *)malloc(len);
  strncpy(out, oldString, oldLength + 1);
  strncpy(out + oldLength, newString, newLength + 1);
  return out;
}

void freeStrings(StringArray *data)
{
  int i;
  for (i = 0; i < data->count; ++i) {
    free(data->strings[i]);
  }
  if (data->count > 0) { free(data->strings); }
  data->count = 0;
}
