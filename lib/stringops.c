#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "stringops.h"

//All values returned from these functions need to be freed after they're used.


//O(n) because strlen() itself is O(n)
char* str_right(char* string, const size_t count)
{
	if (count >= strlen(string)) return string;
	return &string[strlen(string) - count];
}


//O(n), but O(1) can be achieved simply by setting string[count] to '\0'
char* str_left(char* string, const size_t count)
{
	if (count >= strlen(string)) return string;
	char* ret = malloc(count + 1);
	memcpy(ret, string, count);
	*(ret + count) = '\0';
	return ret;
}


char* str_mid(char* string, const size_t pos, const size_t count)
{
	if (count >= strlen(string)) return string;
	if (pos < 0 || pos > strlen(string)) return NULL;

	char* ret = malloc(count + 1);
	char* tmp = string;
	tmp += pos;
	memcpy(ret, tmp, count);
	*(ret + count) = '\0';
	return ret;
}


char* chr_repeat(const char _c, const size_t count)
{
	char *ret = malloc(count + 1);
	memset(ret, _c, count);
	*(ret + count) = '\0';
	return ret;
}


char* str_repeat(char* string, size_t count)
{
	int realCount = count + 1;
	char *ret = (char*)calloc(1, realCount * strlen(string) + 1);
	if (ret == NULL)
		return NULL;
	strcpy(ret, string);
	while (--realCount)
		strcat(ret, string);
	return ret;
}


int str_count(char* string, const char* substr)
{
	int count = 0;
	size_t substrSz = strlen(substr);
	for (int i = 0; string[i]; ++i)
	{
		char *tmp = str_mid(string, i, substrSz);
		if (strcmp(tmp, substr) == 0)
			++count;
		free(tmp);
	}
	
	return count;
}


char* str_cat(char* str1, char* str2)
{
	char *ret = malloc(strlen(str1) + strlen(str2) + 1);
	char *tmp = ret;
	memcpy(tmp, str1, strlen(str1));
	tmp += strlen(str1);
	memcpy(tmp, str2, strlen(str2));
	*(tmp + strlen(str2)) = '\0';
	return ret;
}

char* chr_to_str(const char _c)
{
	char* ret = malloc(2);
	*(ret) = _c;
	*(ret + 1) = '\0';
	return ret;
}

char** str_split(char* string, const char* delim, int *refOcc)
{
	//reinventing the wheel..could easy go with progressive string traverse method strstr().
	size_t delimSz = strlen(delim);
	size_t stringlen = strlen(string);
	int curpos = 0;
	int totrim = 0;
	size_t index = 0;
	size_t occurrences = str_count(string, delim);

	//remove redundant delimiters at the end.
	for (int i = 1; i < occurrences; ++i) {
		if (strcmp(str_mid(string, stringlen - delimSz*i, delimSz), delim) == 0) {
			occurrences -= 1;
			totrim = delimSz*i;
		}
		else
			break;
	}
	*refOcc = occurrences + 1;
	char* substr;
	char** tmp = malloc(occurrences * sizeof(char*));
	char* toCpy;
	for (int i = 0; string[i]; ++i)
	{
		substr = str_mid(string, i, delimSz);
		if (strcmp(substr, delim) == 0 || index == occurrences) {
			if (index == occurrences) {  //the rest of the string
				i = strlen(string);
			}
			tmp[index] = (char*)calloc(1, i - curpos + 1);
			toCpy = str_mid(string, curpos, i - curpos);
			if (strlen(toCpy) > 1) {
				memcpy(tmp[index], toCpy, i - curpos);
				*(tmp[index] + i - curpos) = '\0';
				++index;
				curpos = i + delimSz;
			}
		}

	}
	*(tmp[index - 1] + strlen(tmp[index - 1]) - totrim) = '\0';
	return tmp;
}



int str_get_pos(char* string, char* substr, unsigned int start)
{
	char* substrAddr = strstr((string + start), substr);
	if (substrAddr != NULL)
		return substrAddr - string;
	else
		return -1;  //not found
}


char* str_between(char* string, char* str1, char* str2)
{
	unsigned int str1Len = strlen(str1);
	unsigned int str1Pos = str_get_pos(string, str1, 0);
	unsigned int str2Pos = str_get_pos(string, str2, str1Pos + str1Len);
	if (str1Pos == -1 || str2Pos == -1)
		return "";
	unsigned int retLen = str2Pos - str1Pos - str1Len;
	char* ret = malloc(retLen + 1);
	ret = str_mid(string, str1Pos + str1Len, retLen);
	return ret;
}

char* str_replace(char* string, char* findstr, char* replaceWith, int replaceAll)
{
	int findLen = strlen(findstr);
	int replaceLen = strlen(replaceWith);
	int count = str_count(string, findstr);
	if (count == 0)
		return string;
	char *ret = calloc(1, strlen(string) + (replaceLen - findLen) * count + 1);
	if (ret == NULL){
		puts("calloc error\n");
		return "err";
	}
	char *tmp = ret;
	if (replaceAll == 0)
		count = 1;
	do {
		char *insertPos = strstr(string, findstr);
		int prevLen = insertPos - string;
		memcpy(tmp, string, prevLen);
		tmp += prevLen;
		memcpy(tmp, replaceWith, replaceLen);
		tmp += replaceLen;
		string += prevLen + findLen;
	} while (--count);
	strcpy(tmp, string);  //the rest of the string

	return ret;
}


int buffer_read_to_array(char *buffer, char **out_array)
{
	int start_pos = 0;
	int lines = str_count(buffer, "\n") + 1;

	for (int line = 0; line < lines; ++line) {
		char *line_next = strstr(buffer, "\n");
		int line_cur_len = line_next - buffer;

		if (line_next == NULL) {  //last line.
			line_cur_len = strlen(buffer);
		}

		out_array[line] = malloc(line_cur_len + 1);
		memcpy(out_array[line], buffer, line_cur_len);
		*(out_array[line] + line_cur_len) = '\0';

		start_pos += line_cur_len;
		if (line_next != NULL) buffer = line_next + 1;
	}

	return lines;
	//return number of lines read.
}
