#ifndef _DIRNAV_H
#define _DIRNAV_H
#endif
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#ifdef _WIN32
#include "dirent.h"
#define DIR_SEPARATOR "\\"
#else
#include <dirent.h>
#define DIR_SEPARATOR "/"
#endif


int is_directory(char* sPath)
{
    DIR* op;
    int ret;
    op = opendir(sPath);
    if (errno == 0)  //is valid dir
        ret = 1;
    else{
        ret = 0;
    } 
    closedir(op);
    return ret;
}

char c_separator()
{
	#ifdef _WIN32  //windows dir separator
		return '\\';
	#else
		return '/';
	#endif
}


char* extract_file_dir(char* sFilePath, int isLastSeparatorIncluded)
{
	char* ptr;
	ptr = strrchr(sFilePath, c_separator());
	if (isLastSeparatorIncluded == 1)
		++ptr;  //include last separator e.g. home/user/myfolder/
	size_t _len = strlen(ptr);
	size_t _retSz = strlen(sFilePath) - _len;
	char* ret = malloc(_retSz + 1);
	memcpy(ret, sFilePath, _retSz);
	*(ret+_retSz) = '\0';  //null terminate result string
	return ret;
}


char* extract_file_name(char* sFilePath)
{
	char *ptr = strrchr(sFilePath, c_separator());
	if (ptr == NULL) return NULL;
	ptr++;  //do not include last separator
	return ptr;
}


int dir_is_disk(char *dir)  //WIN32 compatibility wth microsoft
{
	if (strlen(dir) == 3 && dir[0] >= 'A' && dir[0] <= 'Z' && dir[1] == ':' && dir[2] == '\\') return 1;
	return 0;
}


char *strip_trailing_separator(char *string)
{
	if (dir_is_disk(string)) return string;
	int len = strlen(string);
	int i = len;
	while (string[i-1] == c_separator()){
		i--;
	}
	char *ret = malloc(len - i + 1);
	memcpy(ret, string, i);
	ret[i] = '\0';
	return ret;
}


void list_dir(char* path)
{
    //recursive
    printf("listing %s\n", path);
    DIR *hStream ;
    if (!(hStream = opendir(path)))
        return;
    struct dirent *entry;
    char* sFullEntryName;
    while ((entry = readdir(hStream))){
    sFullEntryName = malloc(strlen(entry->d_name) + strlen(path) + 2);
    strcpy(sFullEntryName, path);
    strcat(sFullEntryName, DIR_SEPARATOR);
    strcat(sFullEntryName, entry->d_name);
    if (entry->d_type == DT_DIR){
        if (strcmp(entry->d_name, "..") == 0 || (strcmp(entry->d_name, ".") == 0))
                continue;
        list_dir(sFullEntryName);
        free(sFullEntryName);
    }
    printf("%s\n", sFullEntryName);
    }
    closedir(hStream);
    return;
}


size_t file_get_size(char* sTargetFile)
{
	size_t Sz;
	FILE *_fp;
	_fp = fopen(sTargetFile, "rb");
	if (_fp == NULL)
	{
		printf("problem opening file: %s.\n", sTargetFile);
		return 0;
	}
	fseek(_fp, 0, SEEK_END);
	Sz = ftello(_fp);
	fclose(_fp);
	return Sz;  //in bytes
}
