#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/dirnav.h"

#define F_ERROR -256
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

typedef struct file_t {
	char fpath[1024];
	char fname[128];
	uint_64b fsize;
	struct file_t *left;
	struct file_t *right;
} file_t;

typedef struct str_t {  //linked list of strings
	char *str;  //this should point to file_node->fpath
	struct str_t *next;
} str_t;


int g_traverse_index;  //total number of files in dir
int g_duplicate_count;  //number of duplicate files
str_t *g_duplicate_list;


str_t* list_add(str_t *list, char *str)  //traverse till end of list and add new member, return ptr to it
{
	str_t *prev = NULL;
	if (list != NULL) {
		while (list != NULL) {
			prev = list;
			list = list->next;
		}
	}
	list = malloc(sizeof(str_t));
	list->str = str;
	list->next = NULL;
	if (prev != NULL) prev->next = list;
	return list;
}


int list_find(str_t *list, char *str)  //return index of str in list
{
	int index = 0;
	while (list != NULL) {
		if (strcmp(list->str, str) == 0) return index;
		list = list->next;
		index++;
	}
	return -1;  //not found
}


str_t* list_insert(str_t *list, char *str, int pos)  //if pos is more than list bound, add new member to end of list
{
	str_t *prev;
	while (pos-- >= 0) {
		prev = list;
		list = list->next;
		if (list == NULL) break;
	}
	str_t *new_node = malloc(sizeof(str_t));
	new_node->str = str;
	
	new_node->next = list;
	prev->next = new_node;

	return new_node;
}


void list_free(str_t *list)
{
	str_t *tmp;
	while (list != NULL) {
		tmp = list->next;
		free(list);
		list = tmp;
	}

	return;
}

//compare 2 file info structs
//priority: name -> size -> content, if higher priority fails -> will not check the rest
int fcmp(file_t *pfile1, file_t *pfile2, int mode)
{
	int bytes_read1, bytes_read2;
	bytes_read1 = bytes_read2 = 1;
	
	//if (!(pfile1->fpath) || !(pfile2->fpath)) return F_ERROR;  //sanity check.
	int ret = 0;
	if (mode & FCMP_NAME) {
		if (pfile1->fname[0] == '\0') strcpy(pfile1->fname, extract_file_name(pfile1->fpath));
		if (pfile2->fname[0] == '\0') strcpy(pfile2->fname, extract_file_name(pfile2->fpath));
		ret = strcmp(pfile1->fname, pfile2->fname);
	}
	if (mode & FCMP_SIZE) {
		if (ret == 0) {  //if files have not been compared or higher priority check was a match
			if (pfile1->fsize == 0) pfile1->fsize = file_get_size(pfile1->fpath);
			if (pfile2->fsize == 0) pfile2->fsize = file_get_size(pfile2->fpath);
			ret = pfile1->fsize - pfile2->fsize;
		}
	}
	/*
	if (mode & FCMP_CONTENT) {  //TODO: replace this with hash method?
		char buf1[1024];
		char buf2[1024];
		if (ret != 0) return ret;  //if previous checks failed then return
		FILE *fd1 = fopen(pfile1->fpath, "rb");
		FILE *fd2 = fopen(pfile2->fpath, "rb");
		if (fd1 == NULL || fd2 == NULL) return F_ERROR;  //file inaccessible.
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
	}*/
	return ret;
}


file_t* tree_traverse(file_t *parent, file_t *target, int mode)  //traverse tree and check for duplicates
{
	if (parent == NULL) {
		parent = target;
		parent->fsize = 0;  //init fsize, fcmp() will get actual file size
		parent->fname[0] = '\0';  //fcmp() will extract filename
		parent->left = NULL;
		parent->right = NULL;
	}
	else if (fcmp(parent, target, mode) == 0) {
		//handle duplicate file
		g_duplicate_count++;
		int pos = list_find(g_duplicate_list, parent->fpath);
		if (pos == -1) {  //first duplication, add a linebreak to output
			list_add(g_duplicate_list, " ");  //the linebreak
			list_add(g_duplicate_list, parent->fpath);
			list_add(g_duplicate_list, target->fpath);
		}
		else list_insert(g_duplicate_list, target->fpath, pos);
		
		//printf("%s\n", parent->fpath);  //original file's path
		//printf("%s\n", target->fpath);  //duplicate file's path
	}
	else if (fcmp(parent, target, mode) < 0) {
		parent->left = tree_traverse(parent->left, target, mode);
	}
	else if (fcmp(parent, target, mode) > 0) {
		parent->right = tree_traverse(parent->right, target, mode);
	}
	return parent;
}


void tree_free(file_t *parent)
{
	if (parent == NULL) return;
	if (parent->left != NULL) tree_free(parent->left);
	if (parent->right != NULL) tree_free(parent->right);
	free(parent);
	return;
}


//traverse dir and find all duplicate files within it
//O(nlogn) time complexity using binary tree dir structure with n = number of files in root dir
file_t* find_duplicates(file_t *root, char *dir, int mode)
{
	char full_entry_path[1024];
	DIR *dir_stream = opendir(dir);
	if (dir_stream == NULL) return NULL;
	int is_disk = dir_is_disk(dir);

	struct dirent *entry;
	while (entry = readdir(dir_stream)) {
		strcpy(full_entry_path, dir);
		if (!is_disk) strcat(full_entry_path, DIR_SEPARATOR);
		strcat(full_entry_path, entry->d_name);
		if (entry->d_type == DT_DIR) {  //entry is directory -> step inside it
			if (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0) continue;  //ignore stepback & current dir
			else {
				find_duplicates(root, full_entry_path, mode);  //step inside
			}
		}
		else if (entry->d_type == DT_REG) {  //if entry is a regular file -> put it in file tree and perform comparison.
			file_t *target_file = malloc(sizeof(file_t));
			/*if (target_file == NULL){
				puts("out of memory");
				return -1;
			}*/
			
			strcpy(target_file->fpath, full_entry_path);  //index fpath

			//will only modify root if root is NULL
			//in other words, tree_traverse() will only carry return ptr of node thru 1 recursion level
			//otherwise it will return the same pointer to root
			root = tree_traverse(root, target_file, mode);
			g_traverse_index++;
		}
	}
	closedir(dir_stream);
	return root;
}


void show_help(char *prog_name)
{
	printf(
		"program     : fdupchk\n"
		"description : check for duplicate files in a directory\n"
		"author      : sandwichdoge@gmail.com\n\n"
		"usage:\n"
		"%s <directory> [-n] [-s]\n"
		"-n: compare names\n"
		"-s: compare sizes\n"
		, prog_name);
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
		printf("please set at least 1 comparision parameter [-n][-s].\n");
		return 0;
	}
	if (!is_directory(search_dir)) {
		printf("target is not a directory\n");
		return 0;
	}
	
	g_traverse_index = 0;
	g_duplicate_count = 0;
	g_duplicate_list = list_add(NULL, "");  //init duplicate list

	file_t *root = NULL;
	root = find_duplicates(root, search_dir, mode);
	
	while (g_duplicate_list != NULL) {
		printf("%s\n", g_duplicate_list->str);
		g_duplicate_list = g_duplicate_list->next;
	}

	printf("found %d duplicate files from %d files in directory.\n", g_duplicate_count, g_traverse_index);
	tree_free(root);
	list_free(g_duplicate_list);
	return 0;
}
