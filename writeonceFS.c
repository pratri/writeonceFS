#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// On mount library opens file that holds 'disk' and read all 4MB into memory, for the lifetime of process that mounted filesystem serve reads 
//       and writes based on image in memory 

// Should have data region, i-node region, and super block.

// Disk should support at least 50 files and 2MB of user data, disk should not shrink or grow beyond 4MB, if write cannot be done report error
// All operations should be done in 1K 'Disk Block' quanta. Only read or write to or from memory representing your 'disk' in 1k chunks. Engineer metadata and representations to take advantage of this 

// only write to hard-drive once when file is unmounted.
// Write out 4MB image of modified 'disk' to same file it came from, overwriting old information and preserving any changes made

int freeDiskBlocks = 0;

//Array of open file descriptors:
int* openfiledescriptors;

extern int errno;

//Memory of 4MB points to whatever is mounted
char* mem;
int init = 0;
char* wo_main_file;

typedef struct superblock{
    struct inode* free_start;
    struct inode* occupied_start;
    int num_free_inodes;
    int num_occupied_inodes;
    int sizeofsystem;
    struct wo_file* head;

}superblock;

typedef struct wo_file{
    // name of file
    char* name;
    // Where the starting inode is
    struct inode* start;
    //Keep linked list of files?
    struct wo_file* next_file;
}wo_file;

typedef struct inode{
    // Should have one for every 1K disk block and say whether or not it is in use or starting a file
    // Keeps track of how many of the 1K bytes were used in this block
    int bytes_used;
    //What disk block is pointed to in disk/mem
    char* memorypointed;
    //What the next disk block is in the file
    struct inode* next; 
    
}inode;

   // On call will attempt to read in 'diskfile', 0 on success, negative number on error, on file access error should return number and set errno
int wo_mount(char* filename, void* memoryaddress){
    // filename: 4MB file in native os that holds your entire 'disk'
        // opens file and do init stuff
    // memoryaddress: an address to read your file into should be allocation of at least 4MB
    
    mem = memoryaddress;
    wo_main_file = filename;

    // Checks if the file/disk is blank or not
    int blank = 0;
    FILE* fp = fopen(filename, "r");

    int errnum;
    //ON file access error 
    if(fp == NULL){
        printf("file failed to open\n");
        errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        fprintf(stderr, "Error opening file: %s\n", strerror(errnum));
        return -1;
    }
    //Checks if file is empty
    if (NULL != fp) {
        fseek (fp, 0, SEEK_END);
        int size = ftell(fp);

        if (0 == size) {
            printf("file is empty\n");
            blank = 1;
        }
    }

    //If blank then setup inodes and such
    if (blank){
        //Sets up superblock at start of memory, it contains a list of free and occupied inodes as well as number and size of file system
        superblock* ptr = (superblock*)mem;
        ptr->free_start = NULL;
        ptr->occupied_start = NULL;
        ptr->head = NULL;
        ptr->num_free_inodes = 0;
        ptr->num_occupied_inodes = 0;
        ptr->sizeofsystem = sizeof(superblock);
        printf("Setup superbloc: %d, address: %p\n", ptr->sizeofsystem, &(ptr->sizeofsystem));
        
    } 

    // mount is responsible for checking 'disk' to make sure it is properly formatted. If mount finds 'disk' is blank it should build any 
            // intital structures necessary for accessing 'disk' . If disk format is broken, report error and free structures built 
            // In case of dump of unmount assume data is recognizable form and if mount is called on that validate and parse file to load all contents
    
    // The wo_mount() function only takes one disk in as a parameter. However, it is possible that we will use the output disk file from one test
    // and load it in as input for the next test. Your library should know if the file is already initialized and take proper action to either init or to parse.

    return 0;
}

int wo_unmount(void* memoryaddress){
    // On call will attempt to write out entire 'diskfile' should return 0 on success and on file access error should return number and set errno
    printf("UNMOUNTING\n");
    FILE* file = fopen(wo_main_file, "w");

    int errnum;
    if(file == NULL){
        printf("file failed to open\n");
        errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        fprintf(stderr, "Error opening file: %s\n", strerror(errnum));
        return -1;
    }
    // Also writes out files to file? how does multiple files write into one???? maybe write them all in one at a time with gap in between somehow??
    superblock* ptr = (superblock*)memoryaddress;
    fwrite("Start of superblock\n",sizeof(char), 20, file);
    printf("size %d\n", ptr->sizeofsystem);
    int size = ptr->sizeofsystem;
    fprintf(file, "%d", size);

    return 0;
}

// int wo_open( char* <filename>, <flags>, <mode> )???, when opening user specifies flag in which file is opened, file permission should be granted?
int wo_open(char* filename, char* flags){  
    // On open, if not in file creation mode, attempt to find file
   
    // If it does not exist, return the appropriate error code and set errno
    // If it does exist, check permissions . If they are not compatible with the request, return the appropriate error code and set errno.
    

    // If in file create mode, check to make sure file does not exist, if it does, return the appropriate error code and set errno
    // If it does not exist, create an entry for it and create and return a file descriptor for it. By default created files have all permissions(777)
    return 0;
}

int wo_create(char* filename){
    // If file doesn't exist check if enough size
    return 0;
}

int wo_read(int fd, void* buffer, int bytes){
    // fd valid file descriptor for filesystem 
    // buffer: a memory location to either write bytes to or read from 

    // On invocation check if given filedescriptor is valid (has entry in current table of open file descriptors), if not return with error and set errno
    //      If valid mark it as closed (remove from current table of open file descriptors)
    return 0;
}

int wo_write(int fd, void* buffer, int bytes){
    // Should extend file size if available????
    return 0;

}

int wo_close(int fd){
    return 0;
}



int main(int argc, char* argv[]){
    // Malloc 4 MB and create file pass that in to mount
    char* memoryaddress = malloc(4000000);
    printf("LOL ARLEAY?\n");
    wo_mount("test.txt", memoryaddress);

    wo_unmount(mem);

    free(memoryaddress);
    return 0;
}





