#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/dirnav.h"

#define FCMP_NAME 1
#define FCMP_SIZE 2
#define FCMP_CONTENT 4
#ifdef _WIN32
#define DIR_SEPARATOR "\\"
typedef unsigned long long uint_64b;
#else
#define DIR_SEPARATOR "/"
typedef unsigned long uint_64b;
#endif


typedef char byte;

typedef struct {
	char fpath[1024];
	char fname[128];
	uint_64b fsize;

} file_t;

int g_traverse_index;  //to index items in traverse_directory()

//store info of a file [NAME+SIZE] to a buffer struct.
int file_get_info(file_t *pfile_struct_buf, char *filepath, int mode)
{
	strcpy(pfile_struct_buf->fpath, filepath);

	if (mode & FCMP_NAME){
		char *tmp = extract_file_name(pfile_struct_buf->fpath);
		strcpy(pfile_struct_buf->fname, tmp);
	}
	else memset(pfile_struct_buf->fname, '\0', 1);

	if (mode & FCMP_SIZE) {
		pfile_struct_buf->fsize = file_get_size(pfile_struct_buf->fpath);
	}
	else pfile_struct_buf->fsize = 0;
	
	//TODO: add HASH method here to compare file content?
	return 0;
}


//compare 2 file info structs
int fcmp(file_t *pfile1, file_t *pfile2, int mode)  //return 1 or 0 (true or false)
{
	byte ret = 0;
	int bytes_read1, bytes_read2;
	bytes_read1 = bytes_read2 = 1;

	if (!(pfile1->fpath) || !(pfile2->fpath)) return 0;  //sanity check.

	if (mode & FCMP_NAME) {
		if (strcmp(pfile1->fname, pfile2->fname) == 0) ret |= FCMP_NAME;
	}
	if (mode & FCMP_SIZE) {
		//skip size check if both name and size are compared and namecheck failed.
		if (pfile1->fsize == pfile2->fsize) ret |= FCMP_SIZE;
		else return 0;  //different size -> obviously not a duplicate
	}
	
	if (mode & FCMP_CONTENT) {  //TODO: replace this with hash method?
		char buf1[1024];
		char buf2[1024];
		if (pfile1->fsize != pfile2->fsize) return 0;  //compare sizes again jic, return 0 if sizes differ.
		FILE *fd1 = fopen(pfile1->fpath, "rb");
		FILE *fd2 = fopen(pfile2->fpath, "rb");
		if (fd1 == NULL || fd2 == NULL) return 0;  //file inaccessible.
		while (1) {
			bytes_read1 = fread(buf1, 1, sizeof(buf1), fd1);
			bytes_read2 = fread(buf2, 1, sizeof(buf2), fd2);
			if (bytes_read1 < sizeof(buf1)) {  //truncate last segment
				memset(&buf1[bytes_read1], 0, sizeof(buf1) - bytes_read1);
			}
			if (bytes_read2 < sizeof(buf2)) {  //truncate last segment
				memset(&buf2[bytes_read2], 0, sizeof(buf2) - bytes_read2);
			}

			if (memcmp(buf1, buf2, bytes_read1) != 0 || bytes_read1 != bytes_read2) {  //difference found in file content.
				ret = 0;
				break;
			}
			if (bytes_read1 <= 0 || bytes_read2 <= 0) {  //end of file reached -> file content are the same.
				ret |= FCMP_CONTENT;
				break;
			}
		}
		fclose(fd1);
		fclose(fd2);
	}
	if ((ret ^ mode) == 0) {
		ret = 1;  //all checks are matches
	}
	else ret = 0;
	return ret;
}

//recursively find duplicates of file in file table.
//priority: name -> size -> content, if higher priority fails -> will not check the rest
int find_duplicates(file_t **file_table, int table_size, int mode)  
{
	byte is_dup = 0;
	byte is_firstdup = 1;  //for better formatting.
	file_t *file_current;
	for (int i = 0; i < table_size; ++i) {
		file_current = file_table[i];
		if (file_current->fpath[0] == '\0') continue;
		if (file_current->fsize == 0) file_current->fsize = file_get_size(file_current->fpath);
		is_firstdup = 1;
		for (int j = 0; j < table_size; ++j) {
			if (file_table[j]->fpath != NULL && file_table[j]->fpath[0] == '\0') continue;
			if (i == j) continue;  //do not compare with self.


			if (mode & FCMP_NAME){
				is_dup = fcmp(file_current, file_table[j], FCMP_NAME);
			}
			
			if (mode & FCMP_SIZE) {
				if (mode != FCMP_SIZE && is_dup == 0) is_dup = 0;  //if previous higher priority check exists and failed
				else {
					file_table[j]->fsize = file_get_size(file_table[j]->fpath);
					is_dup = fcmp(file_current, file_table[j], FCMP_SIZE);
				}
				
			}

			if (mode & FCMP_CONTENT) {
				if (mode != FCMP_CONTENT && is_dup == 0) is_dup = 0;  //if previous higher priority check exists and failed
				else {
					file_table[j]->fsize = file_get_size(file_table[j]->fpath);
					is_dup = fcmp(file_current, file_table[j], FCMP_CONTENT);
				}
			}


			if (is_dup) {
				if (is_firstdup) {
					printf("\n%s\n", file_current->fpath);
					is_firstdup = 0;
				}
				printf("%s\n", file_table[j]->fpath);
				memset(file_table[j]->fpath, '\0', 1);
			}
		}
	}
	return 0;
}

/*
//count number of files in directory, recursive. for low memory (<20MB) method only
unsigned int count_files_in_dir(char *dir)
{
	char full_entry_path[1024];
	DIR *dir_stream = opendir(dir);
	if (dir_stream == NULL) return 0;
	struct dirent *entry;
	int is_disk = dir_is_disk(dir);
	unsigned int count = 0;
	while (entry = readdir(dir_stream)) {
		strcpy(full_entry_path, dir);
		if (!is_disk) strcat(full_entry_path, DIR_SEPARATOR);
		strcat(full_entry_path, entry->d_name);
		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0) continue;
			else {
				count += count_files_in_dir(full_entry_path);
			}
		}
		else if (entry->d_type == DT_REG) {  //if entry is a regular file -> index it to file table.
			count++;
		}
	}
	closedir(dir_stream);
	return count;
}
*/

//traverse dir and list all files in an array of pointers to structs.
int traverse_dir(file_t **file_table, char *dir, int mode)
{
	char full_entry_path[1024];
	DIR *dir_stream = opendir(dir);
	if (dir_stream == NULL) return -1;
	struct dirent *entry;
	int is_disk = dir_is_disk(dir);
	while (entry = readdir(dir_stream)) {
		strcpy(full_entry_path, dir);
		if (!is_disk) strcat(full_entry_path, DIR_SEPARATOR);
		strcat(full_entry_path, entry->d_name);
		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0) continue;
			else {
				traverse_dir(file_table, full_entry_path, mode);
			}
		}
		else if (entry->d_type == DT_REG) {  //if entry is a regular file -> index it to file table.
			file_t *target_file = malloc(sizeof(file_t));
			/*if (target_file == NULL){
			puts("out of memory");
			return -1;
			}*/
			file_get_info(target_file, full_entry_path, mode);
			file_table[g_traverse_index] = target_file;
			g_traverse_index++;
		}
	}
	closedir(dir_stream);
	return g_traverse_index;
}


void show_help(char *prog_name)
{
	printf(
		"program     : fdupchk\n"
		"description : check for duplicate files in a directory\n"
		"author      : sandwichdoge@gmail.com\n\n"
		"usage:\n"
		"%s <directory> [-n] [-s] [-c]\n"
		"-n: compare names\n"
		"-s: compare sizes\n"
		"-c: compare content\n", prog_name);
}


int main(int argc, char **argv)
{
	if (argc < 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-h") == 0){
		show_help(argv[0]);
		return 0;
	}
	
	int mode = 0;
	char *search_dir;
	
	for (int i = 1; i < argc; ++i){  //load up params
		if (strcmp(argv[i], "-n") == 0) mode |= FCMP_NAME;
		else if (strcmp(argv[i], "-s") == 0) mode |= FCMP_SIZE;
		else if (strcmp(argv[i], "-c") == 0) mode |= FCMP_CONTENT;
		if (argv[i][0] != '-') search_dir = strip_trailing_separator(argv[i]);
	}
	
	if (mode == 0) {
		printf("please set at least 1 comparision parameter [-n][-s][-c].\n");
		return 0;
	}
	if (!is_directory(search_dir)) {
		printf("target is not a directory\n");
		return 0;
	}
	
	g_traverse_index = 0;

	/*count items to allocate mem accurately.
	skipped and alloc a fixed amount of mem for better performance.
	unsigned int items = count_files_in_dir(search_dir);
	printf("counted %d files in directory %s.\n", items, search_dir);*/

	file_t **file_table;
	file_table = malloc(2000000 * sizeof(file_t*));  //2million file struct pointers ~16MB ram - fixed mem method
	traverse_dir(file_table, search_dir, FCMP_NAME);  //only put file paths in file table

	printf("found %d files in directory.\n", g_traverse_index);
	find_duplicates(file_table, g_traverse_index, mode);
	//clean up
	printf("all done, cleaning up..\n");
	free(search_dir);
	for (int i = g_traverse_index - 1; i; --i){
		if (file_table[i] != NULL) free(file_table[i]);
	}
	free(file_table);
	
	return 0;
}
