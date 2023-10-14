## DOSDU utility 0.1

2023 Chartreuse Kitsune.

Licensed under the 3BSD license, see LICENSE.TXT

------

A simple Disk Usage utility to list the size of files contained within a
directory and all it's sub-directories. The first column shows the size
in bytes, and the second column shows the full pathname of the folder.
A summary of the current directory is specified at the end.
The utility performs a depth first recursion into the filesystem and the
order of the complete results will be all subfolders in directory order,
while descending into subfolders.

Unlike the UNIX du utility, this utility can only take the name of a
single folder as an argument. Using the /S option can allow you to see
the usage of all folders within the specified directory, much like the
UNIX: `du -s` command would.

By default the utility displays the "true" size of the files, aka. how
many bytes the file says it is long. However due to files being allocated
in clusters the file may take up more space on the disk. For example, a
1 byte long file, on a disk with a cluster size of 32,768 bytes will take
up 32,768 bytes on the disk, as will a 1000 byte file or 10,000 byte
file.  Using the /D option the DU utility will display the folder sizes
rounded up to their cluster sizes on disk. This will give a much truer
indication of where disk space is being used on the disk.

If you just want to see how much space a directory is using and not
display anything about its subdirectories the /O option can be used.
This will just display a summary of the specified directory.

For human readible sizes the /H option may be used. This will take the
size to the nearest SI prefix of bytes, B for bytes, k for kilobytes,
M for megabytes, etc. These are using 1000 based units rather than 1024
to align more to how hard drives are normally sold. Currently the code
rounds the value down to the nearest whole unit.
 
Written in Borland C++ 3.1, though should compile in earlier versions.
Requires dir.h for findfirst/findnext, curdir/curdisk, and getfat
         dos.h for the FA_* defines and more.
Developed on DOS for DOS.
