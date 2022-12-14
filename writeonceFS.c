#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

// On mount library opens file that holds 'disk' and read all 4MB into memory, for the lifetime of process that mounted filesystem serve reads 
//       and writes based on image in memory 

// Should have data region, i-node region, and super block.

// Disk should support at least 50 files and 2MB of user data, disk should not shrink or grow beyond 4MB, if write cannot be done report error
// All operations should be done in 1K 'Disk Block' quanta. Only read or write to or from memory representing your 'disk' in 1k chunks. Engineer metadata and representations to take advantage of this 

// only write to hard-drive once when file is unmounted.
// Write out 4MB image of modified 'disk' to same file it came from, overwriting old information and preserving any changes made


int sizeof_filepartition = 50000;
int sizeof_inode_partition = 1000000;
int sizeof_disknode = 1000;

//Array of open file descriptors:
int* openfiledescriptors;

extern int errno;

//Memory of 4MB points to whatever is mounted
char* mem;
int init = 0;
char* wo_main_file;

typedef struct superblock{
    // struct inode* free_start;
    // struct inode* occupied_start;
    // int num_free_inodes;
    // int num_occupied_inodes;
    int sizeofsystem_remaining;
    struct wo_file* all_files_head;
    struct inode* inode_location;
    struct wo_file* wo_file_location;
    char* disk_block_location;
    int file_descriptor_counter;

}superblock;

typedef struct wo_file{
    // name of file
    char name[100];
    //FIle descriptor
    int fd;
    // Where the starting inode is
    struct inode* start;
    //Keep linked list of files?
    struct wo_file* next_file;
    //Can read
    int read;
    //Can write 
    int write;
    //If open
    int open;
}wo_file;



typedef struct inode{
    // Should have one for every 1K disk block and say whether or not it is in use or starting a file
    // Keeps track of how many of the 1K bytes were used in this block
    int bytes_used;
    //What disk block is pointed to in disk/mem
    char* diskpointed;
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
    fclose(fp);
    //If blank then setup inodes and such
    if (blank){
        //Sets up superblock at start of memory, it contains a list of free and occupied inodes as well as number and size of file system
        superblock* ptr = (superblock*)mem;
        // ptr->free_start = NULL;
        // ptr->occupied_start = NULL;
        ptr->all_files_head = NULL;
        // ptr->num_free_inodes = 0;
        // ptr->num_occupied_inodes = 0;
        ptr->inode_location = (inode*)(mem+sizeof(superblock));
        ptr->inode_location->next = NULL;
        ptr->wo_file_location = (wo_file*)(mem + sizeof_inode_partition);
        ptr->disk_block_location = (char*)(mem + sizeof_inode_partition + sizeof_filepartition);
        ptr->sizeofsystem_remaining = 4000000 - sizeof_inode_partition - sizeof_filepartition;
        // printf("Setup superbloc: %d, address: %p, %p, %p, %p\n", ptr->sizeofsystem_remaining, ptr, ptr->inode_location,ptr->wo_file_location , ptr->disk_block_location);
        // Set up first inode and increment both those locations, have inode pointing to old diskblock 1K, 72-1000000 is inodees, 1000000-1005000 is files, 1005000-4000000 is disk blocks
        
        // printf("Checking if new are correct? %p, %p\n", ptr->inode_location, ptr->disk_block_location);
        
    } else{
        FILE* file = fopen("test.txt", "r");
        superblock* ptr = (superblock*)mem;
        int d;
        int e;
        char str[1005];
        fgets(str, sizeof(str), file);
        fgets(str, sizeof(str), file);
        ptr->sizeofsystem_remaining = atoi(str);
        fgets(str, sizeof(str), file);
        ptr->file_descriptor_counter = atoi(str);
        ptr->all_files_head = NULL;
        ptr->disk_block_location = (char*)(mem + sizeof_inode_partition + sizeof_filepartition);
        ptr->inode_location = (inode*)(mem+sizeof(superblock));
        ptr->wo_file_location = (wo_file*)(mem + sizeof_inode_partition);

        while(fgets(str, sizeof(str), file) != NULL){
            str[strcspn(str, "\n")] = 0;
            if(strcmp(str, "NEWFILE_HERE!") == 0){
                wo_file* creatingfile;
                if(ptr->all_files_head == NULL){
                    //Checks if it is the first file
                    ptr->all_files_head = ptr->wo_file_location;
                    creatingfile = ptr->wo_file_location;
                    creatingfile->next_file = NULL;
                    creatingfile->start = NULL;
                    ptr->wo_file_location = (wo_file*)((char*)ptr->wo_file_location + sizeof(wo_file));
                    
                }else{
                    //Add file to end of file list if not first file
                    
                    creatingfile = ptr->all_files_head;
                    while(creatingfile->next_file != NULL){
                        creatingfile = creatingfile->next_file;
                    }
                    creatingfile->next_file = ptr->wo_file_location;
                    creatingfile = creatingfile->next_file;
                    creatingfile->next_file = NULL;
                    creatingfile->start = NULL;
                    ptr->wo_file_location = (wo_file*)((char*)ptr->wo_file_location + sizeof(wo_file));
                }
                // Asisgns all the file information to the file 
                fgets(str, sizeof(str), file);
                str[strcspn(str, "\n")] = 0; 
                strcpy(creatingfile->name, str);
                fgets(str, sizeof(str), file);
                str[strcspn(str, "\n")] = 0; 
                creatingfile->fd = atoi(str);
                fgets(str, sizeof(str), file);
                str[strcspn(str, "\n")] = 0;  
                creatingfile->read = atoi(str);
                fgets(str, sizeof(str), file);
                str[strcspn(str, "\n")] = 0;  
                creatingfile->write = atoi(str);
                fgets(str, sizeof(str), file);
                str[strcspn(str, "\n")] = 0;  
                creatingfile->open = atoi(str);
                //Now should be inodes and specifically how many bytes used in inode
                // printf("Read: %s, %d, %d, %d, %d\n", creatingfile->name, creatingfile->fd, creatingfile->read, creatingfile->write, creatingfile->open);

            }else{
                 // Should be size of disk inode ponting to
                str[strcspn(str, "\n")] = 0;  
                int sizeofdisk = atoi(str);
                // Go to last file: 
                wo_file* currentfile; 
                currentfile =  ptr->all_files_head;
                while(currentfile->next_file != NULL){
                        currentfile = currentfile->next_file;
                }
                // Check if start is NULL, if it is initialize else go to last one and initialize it there
                inode* currentinode;

                if(currentfile->start == NULL){
                    currentinode = ptr->inode_location;
                    ptr->inode_location = (inode*)((char*)ptr->inode_location + sizeof(inode));
                    currentfile->start = currentinode;
                    currentinode->next = NULL;
                    
                    
                }else{
                    currentinode = currentfile->start;
                    while(currentinode->next != NULL){
                        currentinode = currentinode->next;
                    }
                    currentinode->next = ptr->inode_location;
                    ptr->inode_location = (inode*)((char*)ptr->inode_location + sizeof(inode));
                    currentinode = currentinode->next;
                    currentinode->next = NULL;
                }
                // Should have the right current inode
                currentinode->bytes_used = sizeofdisk;
                // Set up disk block location
                currentinode->diskpointed = ptr->disk_block_location;
                ptr->disk_block_location = ptr->disk_block_location + sizeof_disknode;
                // Write the number of bytes to the disk block location
                // fread(currentinode->diskpointed, sizeof(char), currentinode->bytes_used, file);
                fgets(str, sizeof(str), file);
                str[strcspn(str, "\n")] = 0;  
                memcpy(currentinode->diskpointed, str, sizeof_disknode); 
            }
            // printf("\n");
        }
        fclose(file);
    
    }

    // mount is responsible for checking 'disk' to make sure it is properly formatted. If mount finds 'disk' is blank it should build any 
            // intital structures necessary for accessing 'disk' . If disk format is broken, report error and free structures built 
            // In case of dump of unmount assume data is recognizable form and if mount is called on that validate and parse file to load all contents
    
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
    //Write out superblock info 
    fprintf(file, "%d\n", ptr->sizeofsystem_remaining);
    fprintf(file, "%d\n", ptr->file_descriptor_counter);
    wo_file* current_file = ptr->all_files_head;
    while(current_file!=NULL){
        fprintf(file, "NEWFILE_HERE!\n");
        fprintf(file, "%s\n", current_file->name);
        fprintf(file, "%d\n", current_file->fd);
        fprintf(file, "%d\n", current_file->read);
        fprintf(file, "%d\n", current_file->write);
        fprintf(file, "%d\n", current_file->open);
        inode* current_inode = current_file->start;
        while(current_inode!=NULL){
            fprintf(file, "%d\n", current_inode->bytes_used);
            fwrite(current_inode->diskpointed, sizeof(char), current_inode->bytes_used, file);
            fprintf(file, "\n");
            current_inode = current_inode->next;
        }

        current_file = current_file->next_file;
    }

    //Should go through every file, in which should print out inode info, break line, then disk block size, break line, disk, continue till new file

    return 0;
}

void printfiles(){
    printf("PRINTING FILES\n");
    superblock* ptr = (superblock*)mem;
    wo_file* current_file = ptr->all_files_head;

    while(current_file!=NULL){
        printf("FD: %d, name: %s, address: %p, size %lu, open: %d\n", current_file->fd, current_file->name, current_file, sizeof(wo_file), current_file->open);

        inode* temp = current_file->start;
        while(temp!=NULL){
            printf("\t bytes: %d, disk: %p,\n", temp->bytes_used, temp->diskpointed);
            temp = temp->next;
        }
        current_file = current_file->next_file;
    }
}

int wo_create(char* filename, char* flags){
    
    // If it does not exist, create an entry for it and create and return a file descriptor for it. By default created files have all permissions(777)
    int errnum;
    
    superblock* ptr = (superblock*)mem;
    // If in file create mode, check to make sure file does not exist, if it does, return the appropriate error code and set errno
    int doesExist = 0;
    // CHECK IF allfiles head is null
    if(ptr->all_files_head == NULL){
        // printf("FirstFile!\n");
    }else{
        wo_file* temp = ptr->all_files_head;
        while(temp->next_file!=NULL){
            if(temp->name == filename){
                doesExist = 1;
            }
            temp = temp->next_file;
        }
    }
    
   
    if(doesExist){
        printf("Can't create file %s already exists\n", filename);
        errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        fprintf(stderr, "Can't create file it already exists %s\n", strerror(errnum));
        return -1;
    }

    wo_file* creatingFile = ptr->wo_file_location;
    strcpy(creatingFile->name, filename);
    printf("Did strcpy work? %s : %s\n", creatingFile->name, filename);
    creatingFile->next_file = NULL;
    ptr->wo_file_location = (wo_file*)((char*)ptr->wo_file_location + sizeof(wo_file));
    creatingFile->open = 1;
    creatingFile->read = 0;
    creatingFile->write = 0;
    creatingFile->start = NULL;
    // If it does exist, check permissions . If they are not compatible with the request, return the appropriate error code and set errno.
    // Permissions of form WO_RDONL
    if(strcmp(flags, "WO_RDONLY") == 0){ 
        creatingFile->read = 1;
    }else if(strcmp(flags, "WO_WRONLY") == 0){
        creatingFile->write = 1;
    }else if(strcmp(flags, "WO_RDWR") == 0){
        creatingFile->read = 1;
        creatingFile->write = 1;
    }else{
        printf("Flag is in incorrect format\n");
        errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        fprintf(stderr, "Flag is in incorrect format: %s\n", strerror(errnum));
        return -1;
    }
    creatingFile->fd = ptr->file_descriptor_counter;
    ptr->file_descriptor_counter += 1;
    //ADD FILE TO LIST
    if(ptr->all_files_head == NULL){
        ptr->all_files_head = creatingFile;
    }else{
        wo_file* temp = ptr->all_files_head;
        while(temp->next_file!=NULL){
            temp = temp->next_file;
        }
        temp->next_file = creatingFile;
    }
    return creatingFile->fd;
}

// Parameters should end in done to signal end of arguments
int wo_open(char* filename, ...){  
    va_list args;
    va_start(args, filename);
    char* mode;
    char* test = filename;
    char* flags;
    int counter = 1;
    while(strcmp(test, "done") != 0 || counter > 4){
        counter++;
        
        test = va_arg(args, char*);
        if(counter == 2){
            flags = test;
        }else if(counter == 3){
            mode = test;
        }
    }
    va_end(args);
    int errnum;
    if(counter > 4){
        printf("Make sure to end parameters with done or too many parameters\n");
        errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        fprintf(stderr, "Make sure to end parameters with done or too many parameters %s\n", strerror(errnum));
        return -1;
    }
    if(counter == 4){
        wo_create(filename, flags);
        return 0;
    }

    // On open, if not in file creation mode, attempt to find file
    superblock* ptr = (superblock*)mem;
    wo_file* current_file;
    wo_file* temp = ptr->all_files_head;
    int fileExists = 0;
    while(temp->next_file!=NULL){
        if((temp->name) == filename){
            current_file = temp;
            fileExists = 1;
        }
        temp = temp->next_file;
    }
    // If it does not exist, return the appropriate error code and set errno
    if(fileExists == 0){
        printf("file does not exist\n");
        errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        fprintf(stderr, "File doesn't exist: %s\n", strerror(errnum));
        return -1;
    }
    //Since file exists put it in open_files list
    current_file->open = 1;

    if(strcmp(flags, "WO_RDONLY") == 0){
        current_file->read = 1;
    }else if(strcmp(flags, "WO_WRONLY") == 0){
        current_file->write = 1;
    }else if(strcmp(flags, "WO_RDWR") == 0){
        current_file->read = 1;
        current_file->write = 1;
    }else{
        printf("Flag is in incorrect format\n");
        errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        fprintf(stderr, "Flag is in incorrect format: %s\n", strerror(errnum));
        return -1;
    }

    return 0;
}



int wo_read(int fd, void* buffer, int bytes){
    superblock* ptr = (superblock*)mem;
    int errnum;
    // Check if fd is valid
    int check_if_exists = 0;
    wo_file* current_file;
    wo_file* file_temp = ptr->all_files_head;
    while(file_temp->next_file!=NULL){
        
        if(file_temp->fd == fd && file_temp->open == 1 && file_temp->read == 1){    
            check_if_exists = 1;
            current_file = file_temp;
        }
        file_temp = file_temp->next_file;
    }
    if(!check_if_exists){
        printf("File descriptor does not exist or not in read mode: \n");
        errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        fprintf(stderr, "File descriptor does not exist or not in read mode%s\n", strerror(errnum));
        return -1;
    }
    if(current_file->start == NULL){
        //Nothing in file
        printf("NOTHING IN FILE TO READ\n");
        return -1;
    }
    int bytestocpy = bytes;
    int bytescopied = 0;
    inode* current_inode = current_file->start;

    while(bytestocpy > sizeof_disknode){
        memcpy(((char*)buffer) + bytescopied, current_inode->diskpointed, sizeof_disknode);
        bytescopied += sizeof_disknode;
        bytestocpy -= sizeof_disknode;
        current_inode = current_inode->next;
    }  
    //Now read fromn next disknode with remaning bytes
    memcpy(((char*)buffer) + bytescopied, current_inode->diskpointed, bytestocpy);

    return 0;
}

int wo_write(int fd, void* buffer, int bytes){
    // Start writing character using memcpy
    // If file needs disk block then check if file size available is > bytes rounded up then there is space then create inode pointing to disk block, put inode in file matching name
    superblock* ptr = (superblock*)mem;
    int errnum;
    // Check if fd is valid
    int check_if_exists = 0;
    wo_file* current_file;
    wo_file* file_temp = ptr->all_files_head;
    while(file_temp->next_file!=NULL){
        if(file_temp->fd == fd && file_temp->open == 1 && file_temp->write == 1){    
            check_if_exists = 1;
            current_file = file_temp;
        }
        file_temp = file_temp->next_file;
    }
    if(!check_if_exists){
        printf("File descriptor does not exist or not in write mode\n");
        errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        fprintf(stderr, "File descriptor does not exist or not in write mode%s\n", strerror(errnum));
        return -1;
    }
    //Check if disk nodes are necessary??
    if(ptr->sizeofsystem_remaining < bytes+1000){
        printf("NOT ENOUGH SPACE can't write");
    }
    inode* current_inode;
    if(current_file->start == NULL){
        // No inodes, first time writing, so create inode and disk it points to
        current_inode = ptr->inode_location;
        current_file->start = current_inode;
        current_inode->bytes_used = 0;
        current_inode->diskpointed = ptr->disk_block_location;
        current_inode->next = NULL;
        ptr->inode_location = (inode*)((char*)ptr->inode_location + sizeof(inode));

        ptr->disk_block_location = ptr->disk_block_location + sizeof_disknode;
        ptr->sizeofsystem_remaining -= sizeof_disknode;
        
    }else{
        //Finds last inode in list
        inode* temp_inode = current_file->start;
        while(temp_inode->next!=NULL){
            temp_inode = temp_inode->next;
        }
        current_inode = temp_inode;
    }
    int bytescounter = bytes;
    int bytescopied = 0;
    //Check that disk block has enough space 
    
    if(sizeof_disknode - current_inode->bytes_used < bytescounter){
        //Repeat until enough size is created
        while(sizeof_disknode - current_inode->bytes_used < bytescounter){
            // Not enough space in disk, so do memcpy for the remaning space then add new inode and disk
            int bytestocpy = sizeof_disknode - current_inode->bytes_used;
            bytescounter -= bytestocpy;
            bytescopied += bytestocpy;
            memcpy(current_inode->diskpointed, buffer, bytestocpy);
            current_inode->bytes_used += bytestocpy;
            current_inode->next = ptr->inode_location;
            current_inode = current_inode->next;
            current_inode->bytes_used = 0;
            current_inode->diskpointed = ptr->disk_block_location;
            current_inode->next = NULL;
            ptr->inode_location = (inode*)((char*)ptr->inode_location + sizeof(inode));

            ptr->disk_block_location = ptr->disk_block_location + sizeof_disknode;
            ptr->sizeofsystem_remaining -= sizeof_disknode;
        }
        //Now can memcpy
        current_inode->bytes_used += bytescounter;
        memcpy(current_inode->diskpointed, ((char*)buffer) + bytescopied, bytescounter);
        
    }else{
        memcpy(&current_inode->diskpointed[current_inode->bytes_used], buffer, bytes);
        current_inode->bytes_used += bytes;
    }

    return 0;

}

int wo_close(int fd){
    superblock* ptr = (superblock*)mem;
    int errnum;
    //Check if file is open in curent 
    int check_if_open = 0;
    wo_file* current_file;
    wo_file* temp = ptr->all_files_head;
    while(temp->next_file!=NULL){
        if(temp->fd == fd && temp->open == 1){    
            check_if_open = 1;
            current_file = temp;
        }
        temp = temp->next_file;
    }
    if(check_if_open == 1){
        current_file->open = 0;
        current_file->read = 0;
        current_file->write = 0;
    }else{
        printf("File is not open can't close\n");
        errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        fprintf(stderr, "File is not open can't close %s\n", strerror(errnum));
        return -1;
    }
    return 0;
}



int main(int argc, char* argv[]){
    // Malloc 4 MB and create file pass that in to mount
    char* memoryaddress = malloc(4000000);
    wo_mount("test.txt", memoryaddress);

    // wo_open("hi1", "WO_RDWR", "WO_CREAT", "done");
    // wo_open("wow", "WO_RDWR", "WO_CREAT", "done");
    // wo_open("amazing", "WO_RDWR", "WO_CREAT", "done");

    // wo_close(0);

    // wo_open("hi1", "WO_RDWR", "done");
    
    // char str[500] = "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc";
    // wo_write(0, str, 500);
    // char str2[3000] = "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc";
    // wo_write(1, str2, 3200);
    // wo_close(0);
    char* output = malloc(500);
    wo_read(0, output, 500);

    printfiles();
    printf("output: %s\n", output);
    wo_unmount(mem);

    free(memoryaddress);
    return 0;
}





