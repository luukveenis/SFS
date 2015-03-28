# SFS - A Simple File System

This is the final assignment in CSC 360. The assigned tasks perform a number
of operations on a FAT12 file system such as:
- reading and displaying file system information
- retrieving files from the file system
- adding new files to the file system (updating FAT table, etc)

## Compilation

The project provides a makefile to make compilation simple. Inside the project
directory, simply run the command:

  `make`

This will generate 4 executables:
- **diskinfo** which allows you to list information about a FAT12 disk
- **disklist** which lists all the items in the disk's root directory
- **diskget** which copies a file from the disk to the current working directory
- **diskput** which copies a file from the local file system to the disk
