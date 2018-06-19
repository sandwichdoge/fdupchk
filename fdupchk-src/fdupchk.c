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

typedef struct file_t{
	char fpath[1024];
	char fname[128];
	uint_64b fsize;
	struct file_t *left;
	struct file_t *right;

} file_t;

typedef struct str_t{
	char str[1024];
	struct str_t *next;
}

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
	
	pfile_struct_buf->left = NULL;
	pfile_struct_buf->right = NULL;
	//TODO: add HASH method here to compare file content?
	return 0;
}


//compare 2 file info structs
//priority: name -> size -> content, if higher priority fails -> will not check the rest

int fcmp(file_t *pfile1, file_t *pfile2, int mode)  //return 1 or 0 (true or false)
{
	
	int bytes_read1, bytes_read2;
	bytes_read1 = bytes_read2 = 1;
	
	//if (!(pfile1->fpath) || !(pfile2->fpath)) return ERROR;  //sanity check.
	int ret = 0;
	if (mode & FCMP_NAME) {
		ret = strcmp(pfile1->fname, pfile2->fname);
	}
	if (mode & FCMP_SIZE) {
		if (ret == 0) {  //if files have not been compared or higher priority check was a match
			ret = pfile1->fsize - pfile2->fsize;
		}
	}
	
	if (mode & FCMP_CONTENT) {  //TODO: replace this with hash method?
		char buf1[1024];
		char buf2[1024];
		if (pfile1->fsize != pfile2->fsize) return -1;  //compare sizes again jic, return 0 if sizes differ.
		FILE *fd1 = fopen(pfile1->fpath, "rb");
		FILE *fd2 = fopen(pfile2->fpath, "rb");
		if (fd1 == NULL || fd2 == NULL) return -1;  //file inaccessible.
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
				ret = -1;
				break;
			}
			if (bytes_read1 <= 0 || bytes_read2 <= 0) {  //end of file reached -> file content are the same.
				ret = 0;
				break;
			}
		}
		fclose(fd1);
		fclose(fd2);
	}
	return ret;
}


file_t* traverse_tree(file_t *parent, file_t *target, int mode)  //traverse tree and check for duplicates
{
	if (parent == NULL) {
		parent = target;
	}
	else if (fcmp(parent, target, mode) == 0) {
		//handle duplicate file
		//TODO: something about formatting
		//save them to an array to write somewhere later?
		printf("%s\n", parent->fpath);  //original file's path
		printf("%s\n", target->fpath);  //duplicate file's path
	}
	else if (fcmp(parent, target, mode) < 0) {
		parent->left = traverse_tree(parent->left, target, mode);
	}
	else if (fcmp(parent, target, mode) > 0) {
		parent->right = traverse_tree(parent->right, target, mode);
	}
	return parent;
}

//traverse dir and find all duplicate files within it
//O(nlogn) time complexity using binary tree dir structure with n = number of files in root dir
int traverse_dir(file_t *root, char *dir, int mode)
{
	char full_entry_path[1024];
	DIR *dir_stream = opendir(dir);
	if (dir_stream == NULL) return -1;
	int is_disk = dir_is_disk(dir);

	struct dirent *entry;
	while (entry = readdir(dir_stream)) {
		strcpy(full_entry_path, dir);
		if (!is_disk) strcat(full_entry_path, DIR_SEPARATOR);
		strcat(full_entry_path, entry->d_name);
		if (entry->d_type == DT_DIR) {  //entry is directory -> step inside it
			if (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0) continue;  //ignore stepback & current dir
			else {
				traverse_dir(root, full_entry_path, mode);  //step inside
			}
		}
		else if (entry->d_type == DT_REG) {  //if entry is a regular file -> put it in file tree and perform comparison.
			file_t *target_file = malloc(sizeof(file_t));
			/*if (target_file == NULL){
				puts("out of memory");
				return -1;
			}*/
			file_get_info(target_file, full_entry_path, mode);

			//will only modify root if root is NULL, then root stays the same
			//in other words, traverse_tree() only carries node return ptr thru 1 recursion level
			root = traverse_tree(root, target_file, mode);
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
	
	for (int i = 1; i < argc; i++){  //load up params
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

	file_t *root = NULL;
	traverse_dir(root, search_dir, mode);

	printf("from totally %d files in directory.\n", g_traverse_index);
	
	if (root != NULL) free(root);
	return 0;
}
