#ifndef _stringops_h
#define _stringops_h
#include "stringops.c"
/*IMPORTANT:
ALL functions below return pointers to allocated string addresses
that need to be freed after being used.
*/

char*  str_repeat(char* string, size_t count);	//self-explanatory.

char*  chr_repeat(const char _c, const size_t count);	//self-explanatory.

char*  str_right(char* string, const size_t count);	//return COUNT rightmost characters.

char*  str_left(char* string, const size_t count);	//return COUNT leftmost characters.

//return COUNT characters starting from 0-based POS.
char*  str_mid(char* string, const size_t pos, const size_t count);

char*  str_cat(char* str1, char* str2);	//return mem-allocated concatenation of 2 strings.

//return array of 2 characters, 1st character is _c, last character is NULL.
char*  chr_to_str(const char _c);

//return array of split strings whose ubound (aka number of occurrences) is in refOcc.
char** str_split(char* string, const char* delim, int *refOcc);	

//return a substring between str1 and str2 all belong in STRING.
char*  str_between(char* string, char* str1, char* str2);

//return a new string whose substrings equal to FINDSTR are replaced with REPLACEWITH.
//replaceAll == 0: only replace first occurrence.
char*  str_replace(char* string, char* findstr, char* replaceWith, int replaceAll);

//return 0-based position of a substring starting from START.
int    str_get_pos(char* string, char* substr, unsigned int start);

//return number of occurrences of SUBSTR inside STRING.
int    str_count(char* string, const char* substr);

//return number of lines in BUFFER, all lines are stored in OUT_ARRAY.
int buffer_read_to_array(char *buffer, char **out_array);
#endif // !stringops_h
