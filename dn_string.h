#ifndef DN_STRING
#define DN_STRING

#include "arena.h"
#include "string.h"
#include <stdbool.h>

typedef struct {
  const char* chars;
  unsigned int length;
} String;

/* TODO
   make this library arena independent, or use an internal arena
   but perhaps the user does not want to use arena's.
*/

char* String_cstring(Arena* arena, String str) {
  char* tmp_buffer;
  tmp_buffer = (char*) arena_alloc(arena, sizeof(char)*(str.length+1));
  memcpy(tmp_buffer, str.chars, str.length);
  tmp_buffer[str.length] = '\0';
  return tmp_buffer;
}

bool String_cmp(String str1, const char* cstr) {
  unsigned int i;
  for (i = 0; i < str1.length; i++) {
    if (cstr[i] == '\0') {
      return false;
    }
    if (str1.chars[i] != cstr[i]) {
      return false;
    }
  }
  return true;
} 

#endif
