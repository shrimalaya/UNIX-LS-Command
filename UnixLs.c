#include "UnixLs.h"

// Find the option argument in the command line input
int find(char *directoryArg, char **argv, int start, int end) {
	for(int i = 0; i < end; i++) {
		if(strcmp(argv[i], directoryArg) == 0) {
			return i;
		}
	}

	return -1;
}

// Special case defined by either a hidden direcroty or "." or ".."
int checkSpecialArg(const char *fname) {
    if(strcmp(fname, ".") == 0 || strcmp(fname, "..") == 0) {
        return 1;
    } else if(strlen(fname) > 0 && fname[0] == '.') {
        return 2;
    } else {
        return 0;
    }
}

// List the file path depending on the command entered by the user
int listDirectory(const char *directoryPath) {
	if(directoryPath == NULL) {
		return -1;
	}
	
	// Handle path leading to root (home directory)
	// Requires user id and password entry to access file path leading to home directory
	// Learned from stackoverflow/R Samuel Klatchko and linux man getpwuid
	if(strcmp(directoryPath, "~") >= 0) {
		struct passwd *pw = getpwuid(getuid());
		directoryPath = pw->pw_dir;
	}

	// copy the current directory path
	strcpy(currentDirectory, directoryPath);

	// Use file-tree walk for directoryPath because of issues with readdir() return values
	// Learned from Linux man ftw
	// Use FTW_PHYS flag to avoid symbolic links
	// 0 on success, -1 on error
	
	// Use FTW_ACTIONRETVAL to allow flags
	int returnVal = nftw(directoryPath, fn_nftw, NOPENFD, FTW_PHYS | FTW_ACTIONRETVAL);

	return returnVal;
}

// See the link below for FTW flag descriptions
// http://man7.org/linux/man-pages/man3/ftw.3.html
static int fn_nftw(const char *fpath, const struct stat *sb, const int typeflag, struct FTW *ftwbuf){
	char fname[BUFFER_SIZE];    // File name
	char date[20];  // To parse date data
	int returnVal;  // Result
	struct tm mTime;    // To parse time
	struct passwd *pw;  // For user id
	struct group *mGroup;   // Group id
	int fSize;      // File size in bytes
    
	// get filename from the filepath (so that it is displayed in short)
	getFilename(fname, fpath);

	returnVal = checkSpecialArg(fname); // Check for ".", "..", or hidden path
	if(returnVal == 1) {
        // 1 = "." or ".."
		return FTW_CONTINUE;
	} else if(returnVal == 2) {
        // 2 = hidden
		if(typeflag == FTW_D) {
            // Flag to avoid sending directory contents as arguments to fn_nftw()
			return FTW_SKIP_SUBTREE;
		} else {
            // Continue normally
			return FTW_CONTINUE;
		}
	}

	// Now, check for inode and longList options

	// -i: print the inode
	if(isInode) {
		printf("%-6lu ", sb->st_ino);
	}

	// -l: print the long file name
	// Long name includes rwe permissions, no. of hardlinks, userID, 
	// 			groupID, Size and latest timestamp of midification
	if(isLong) {
		// print permissions
		printPermissions(sb->st_mode);
		printf("%lu", sb->st_nlink);

		pw = getpwuid(sb->st_uid);
		mGroup = getgrgid(sb->st_gid);

		// Print user id
		if(pw == NULL) {
			printf(" %-8d",sb->st_uid);
		} else {
			printf(" %-8s", pw->pw_name);
		}

		// Print group id
		if(mGroup == NULL) {
			printf(" %-8d", sb->st_gid);
		} else {
			printf(" %-8s", mGroup->gr_name);
		}

		// Get file size (in bytes) and print it
		fSize = (int) sb->st_size;
		printf(" %-6d", fSize);

		// Latest date & time of modification

		// Don't use ctime() because it gives a single string
		localtime_r(&(sb->st_mtime), &mTime);
		// Format date and time
		size_t max = 20;
		const char* format = "%b %d %H:%M"; // b = month, d = date
		strftime(date, max, format, &mTime);
		printf(" %s ", date);
	}

	// Print name of the file now
	if(typeflag == FTW_SL) {
		if(isLong) {
            // Long = print the full details about the file
            
			char* tPath;
			size_t MAXLENGTH = 1024;
			ssize_t length;

			while(1) {
				tPath = malloc(MAXLENGTH + 1);
				if(tPath == NULL) {
					// Path not found
					return -1;
				}

				// readlink prints resolved symbolic links
				length = readlink(fpath, tPath, MAXLENGTH);
				if(length == (ssize_t) - 1) {
                    // free the target filepath
					free(tPath);
					return -1;
				}

				if(length >= (ssize_t) MAXLENGTH) {
					free(tPath);
					MAXLENGTH += 1024;
					continue;
				}

                // terminate the string with the NULL character
				tPath[length] = '\0';
				break;
			}

			printf("%s -> %s\n", fname, tPath);
			free(tPath);
		} else {
			printf("%s\n", fname);
		}
	} else if (typeflag == FTW_F) {
        // Flag indicates a regular file
		printf("%s\n", fname);
	} else if (typeflag == FTW_D || typeflag == FTW_DP) {
        // Either a directory or a directory with depth specified
		printf("%s\n", fname);
        // To preven objects of the directory from being passed as arguments to fn_nftw()
		return FTW_SKIP_SUBTREE;
	} else if (typeflag == FTW_DNR) {
        // Directory not Readable
		printf("%s [unreadable]\n", fname);
	} else {
        // Unknown file/dir path
		printf("%s unknown\n", fname);
	}
    
    // Continue normally
	return FTW_CONTINUE;

}

void getFilename(char *fname, const char *fpath) {
    if(strcmp(fpath, currentDirectory) == 0) {
        fname[0] = '.';
        fname[1] = '\0';
    } else {
        char *name = basename((char*) fpath); // basename ignores constants
        strcpy(fname, name);
    }
}

// From: https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
void const *printPermissions(__mode_t mode) {
	
    printf( (S_ISDIR(mode)) ? "d" : "-");

    // User permissions
    printf( (mode & S_IRUSR) ? "r" : "-");
    printf( (mode & S_IWUSR) ? "w" : "-");
    printf( (mode & S_IXUSR) ? "x" : "-");

    // Group permissions
    printf( (mode & S_IRGRP) ? "r" : "-");
    printf( (mode & S_IWGRP) ? "w" : "-");
    printf( (mode & S_IXGRP) ? "x" : "-");

    // Other permissions
    printf( (mode & S_IROTH) ? "r" : "-");
    printf( (mode & S_IWOTH) ? "w" : "-");
    printf( (mode & S_IXOTH) ? "x" : "-");
    printf(" ");
}
