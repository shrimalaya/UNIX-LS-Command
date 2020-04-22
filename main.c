#include "UnixLs.c"

int main(int argc, char* argv[]) {
    isInode = 0;    // Flag for inode
    isLong = 0;     // Flag for long list
    
    int cmd; // Options enteres in terminal
    int returnVal;  // Result from listDirectory()

    // Parse Command [il]
    // Learned getopt() from Linux man page
    // Parses options starting with '-' or '--'
    // Throws an error otherwise
    // 0 = successful, 1 = error, -1 = nothing more to read
    while((cmd = getopt(argc, argv, "il")) != -1) {
    	switch (cmd) {
    		case 'i':
    			isInode = 1;
    			break;
    		case 'l':
    			isLong = 1;
    			break;
    		default:
    			printf("Failed to detect a valid command\n");
    			return -1;
    	}
    }

    // if no directory argument passed, ls prints the current directory
    if(optind == argc) {
    	returnVal = listDirectory(".");
    	if(returnVal == -1) {
    		printf("[UnixLS] Error Accessing Path!\n");
    	}
    } else {
    	// Process in order: "." > ".." > "~" > rest
        
        // "."
    	if(find(".", argv, optind, argc) != -1) {
    		returnVal = listDirectory(".");
    		if(returnVal == -1) {
    			printf("[UnixLS] Error Accessing Path!\n");
    		}
    	}
        
        // ".."
    	if(find("..", argv, optind, argc) != -1) {
    		returnVal = listDirectory("..");
    		if(returnVal == -1) {
    			printf("[UnixLS] Error Accessing Path!\n");
    		}
    	}
        
        // "~"
    	if(find("~", argv, optind, argc) != -1) {
    		returnVal = listDirectory("~");
    		if(returnVal == -1) {
    			printf("[UnixLS] Error Accessing Path!\n");
    		}
    	}

    	// rest of the cases
    	for(int i = optind; i < argc; i++) {
    		if(strcmp(argv[i], ".") != 0 && strcmp(argv[i], "..") != 0 && strcmp(argv[i], "~") != 0) {
    			returnVal = listDirectory(argv[i]);
    			if(returnVal == -1) {
    				printf("[UnixLS] Error!\n");
    			}
    		}
    	} 
    }

    return 0;
}
