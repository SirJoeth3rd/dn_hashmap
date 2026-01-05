#ifndef DN_STRING
#define DN_STRING

#include "arena.h"
#include "string.h"
#include <stdbool.h>
#include <assert.h>

#define TO_STRING(charbuf) (String){.chars = charbuf, .length = strlen(charbuf)}

typedef struct {
  char* chars;
  unsigned int length;
} String;

/* TODO
   make this library arena independent, or use an internal arena
   but perhaps the user does not want to use arena's.
*/

char* String_cstring(Arena*, String);
String String_from_cstring(char*);
bool String_cmp(String, const char*);

char* String_cstring(Arena* arena, String str) {
  char* tmp_buffer;
  if (arena) {
    tmp_buffer = (char*) arena_alloc(arena, sizeof(char)*(str.length+1));
  } else {
#if defined(NDEBUG)
    tmp_buffer = (char*) malloc(sizeof(char)*(str.length+1));
#else
    assert(("String_cstring requires arena to put chars in", arena));
#endif
  }
  memcpy(tmp_buffer, str.chars, str.length);
  tmp_buffer[str.length] = '\0';
  return tmp_buffer;
}

String String_from_cstring(char* chars) {
  String str;
  str.chars = chars;
  str.length = strlen(chars);
  return str;
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

void String_cpy_out(String str, char* buffer) {
  memcpy(buffer, str.chars, str.length);
}

String String_cpy_in(String str, char* buffer) {
  unsigned int buffer_len;
  buffer_len = strlen(buffer);
  if (buffer_len > str.length) {
    // TODO: error here
  }
  strcpy(str.chars, buffer);
  return str;
}

String String_append(String str, String app) {
  //TODO: fix (if my underlying chars is empty on the end just use that instead)
  String new_string;
  new_string.length = str.length + app.length;
  new_string.chars = malloc(sizeof(char)*new_string.length);
  strncpy(new_string.chars, str.chars, str.length);
  strncpy(&new_string.chars[str.length], app.chars, app.length);
  return new_string;
}

String String_subslice(String str, int start, int end) {
  String new_string;
  //TODO: dew it
}

#endif
