#include <stdio.h>
#include <stdlib.h>
char* file_read(const char* filename)
{
  printf("%s\n", filename);
  FILE* input = fopen(filename, "rb");
  if(input == NULL){printf("HEY\n"); return NULL;}
 
  if(fseek(input, 0, SEEK_END) == -1) return NULL;
  long size = ftell(input);
  if(size == -1) return NULL;
  if(fseek(input, 0, SEEK_SET) == -1) return NULL;
 
  /*if using c-compiler: dont cast malloc's return value*/
  char *content = (char*) malloc( (size_t) size +1  ); 
  if(content == NULL) return NULL;
 
  fread(content, 1, (size_t)size, input);
  if(ferror(input)) {
    free(content);
    return NULL;
  }
 
  fclose(input);
  content[size] = '\0';
  return content;
}
