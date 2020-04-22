#define _XOPEN_SOURCE 500 // For FTW
#define _LARGEFILE64_SOURCE	// For FTW
#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE
#define NOPENFD 20	// For FTW
#define BUFFER_SIZE 1024

#include <stdio.h>
#include <stdlib.h>	
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <dirent.h>	// Directory operations
#include <ftw.h>	// file-tree walk
#include <sys/types.h>
#include <pwd.h>	// For User id
#include <grp.h>	// For Group id

static int isLong;   // Long
static int isInode;   // inode
static char currentDirectory[BUFFER_SIZE]; // Store the current filepath

// Find
int find(char *directoryArg, char **argv, int start, int end);

// Used in fn_nftw() to search for "." or ".." or hidden file
int checkSpecialArg(const char *fname);

// Function to use by nftw() in listDirectory() and print file paths
static int fn_nftw(const char *fpath, const struct stat *sb, const int typeflag , struct FTW *ftwbuf);

// Print the directory filepaths along with desired information as defined by 'l' or 'i'
int listDirectory(const char *directoryPath);

// Get a shortened version of the file name for better readability
void getFilename(char *fname, const char *fpath);

// R/W/E permissions
// https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
void const *printPermissions(__mode_t mode);
