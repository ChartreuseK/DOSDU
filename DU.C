/*  DOSDU utility 0.1
 *   - 2023 Chartreuse Kitsune.
 *   Licensed under the 3BSD license, see LICENSE.TXT
 *
 *  A simple Disk Usage utility to list the size of files contained within a
 *  directory and all it's sub-directories. The first column shows the size
 *  in bytes, and the second column shows the full pathname of the folder.
 *  A summary of the current directory is specified at the end.
 *  The utility performs a depth first recursion into the filesystem and the
 *  order of the complete results will be all subfolders in directory order,
 *  while descending into subfolders.
 *
 *  Unlike the UNIX du utility, this utility can only take the name of a
 *  single folder as an argument. Using the /S option can allow you to see
 *  the usage of all folders within the specified directory, much like the
 *  UNIX: `du -s *` command would.
 *
 *  By default the utility displays the "true" size of the files, aka. how
 *  many bytes the file says it is long. However due to files being allocated
 *  in clusters the file may take up more space on the disk. For example, a
 *  1 byte long file, on a disk with a cluster size of 32,768 bytes will take
 *  up 32,768 bytes on the disk, as will a 1000 byte file or 10,000 byte
 *  file.  Using the /D option the DU utility will display the folder sizes
 *  rounded up to their cluster sizes on disk. This will give a much truer
 *  indication of where disk space is being used on the disk.
 *
 *  If you just want to see how much space a directory is using and not
 *  display anything about its subdirectories the /O option can be used.
 *  This will just display a summary of the specified directory.
 *
 * Written in Borland C++ 3.1, though should compile in earlier versions.
 * Requires dir.h for findfirst/findnext, curdir/curdisk, and getfat
 *          dos.h for the FA_* defines and more.
 * Developed on DOS for DOS.
 */

#include <stdio.h>
#include <dos.h>
#include <dir.h>
#include <string.h>
#include <ctype.h>

void usage(void);
long folder_usage(char *path, int summary);
long round_size(long size);
void display_human(long size);

/* Mmmm global variables */
long filecount, dircount;
long cluster_size;
/* Command option */
int roundcluster = 0;
int human = 0;


int main(int argc, char **argv)
{
	struct fatinfo fat;
	int drive;
    char curdir[MAXPATH+3];
    long total = 0;
    int i;
    int levels = -1;
    char *dirarg = NULL;

    if (argc > 1) {
        for (i = 1; i < argc; i++) {
       		if (argv[i][0] == '/') {
        		switch(toupper(argv[i][1])) {
            	case '?': /* Help */
            		usage();
                	return 0;
                case 'H': /* Human readible sizes */
                	human = 1;
                    break;
            	case 'D': /* Size on disk */
                	roundcluster = 1;
            		break;
                case 'S': /* Summary */
                	levels = 1;
                	break;
                case 'O': /* summary of Only this directory */
                	levels = 0;
                    break;
                default:
                	printf("Unknown option '%c'\n", argv[i][1]);
                    usage();
                    return 1;
                }
            }
            else {
            	/* Specifying the directory to check */
                dirarg = argv[i];
            }
        }
    }

	drive = getdisk();
   	sprintf(curdir, "%c:\\", 'A'+drive);
	getcurdir(0, curdir+3);

    if (dirarg && (strcmp(dirarg, ".") != 0)) {
    	/* User specifed a directory, turn it into a complete path */
        if (dirarg[0] == '\\') {
        	/* Absolute path on current drive */
            strcpy(curdir+3, dirarg+1);
        } else if (dirarg[1] == ':') {
        	/* Drive has possibly been specified, absolute path */
            curdir[0] = dirarg[0];
            /* Allow C:TEST type paths and C:\TEST */
            if (dirarg[2] == '\\')
			   	strcpy(curdir+3, dirarg+3);
            else
            	strcpy(curdir+3, dirarg+2);
        } else {
        	/* Relative path to current dir */
            if ((strlen(dirarg) + strlen(curdir) + 1) > MAXPATH) {
            	printf("Specified path is too long!\n");
                return 1;
            }
            strcat(curdir, "\\");
            strcat(curdir, dirarg);
        }

        /* Todo: Clean up any . or .. in path */
    }
	/* Now to clean up the end */
	i = strlen(curdir)-1;
    if (curdir[i] == '\\') {
       	curdir[i] = 0;
	}

    /* Get the drive's cluster size */
	getfat(drive+1, &fat);
    cluster_size = (long)fat.fi_bysec * (long)fat.fi_sclus;

    total = folder_usage(curdir, levels);

    if (!human)
    	printf("%-12ld\t", total);
    else
    	display_human(total);
    puts(".");

    printf("%ld files scanned, %ld directories\n", filecount, dircount);
    if (roundcluster)
    	printf("Disk cluster size is %ld\n", cluster_size);

	return 0;

}

void usage()
{
	puts("Usage:");
    puts("DU [/S] [/O] [/D] [path]");
    puts("\t/? - Displays the usage/help. (this)");
    puts("\t/H - Use human readible sizes kB/MB/GB, etc.");
    puts("\t/S - Displays the summary of directory and 1st level subdirs.");
    puts("\t/O - Displays the summary of only the specified directory.");
    puts("\t/D - Displays the size on disk, rather than actual file size.");
    puts("Version 0.1   - 2023 Chartreuse - 3BSD licensed");
    puts("");
}


long folder_usage(char *path, int levels)
{
    char search[MAXPATH+16];
    struct ffblk ent;
    long total = 0L;

    sprintf(search, "%s\\*.*", path);

	/* Try and find the files */
	if(findfirst(search, &ent, FA_DIREC) != 0)
        return 0L;
	/* Iterate over directory */
    do {
    	if (ent.ff_attrib == FA_DIREC) {
        	/* Ignore current and parent pointers */
    		if (!strcmp(ent.ff_name, ".") || !strcmp(ent.ff_name, ".."))
            	continue;

        	sprintf(search, "%s\\%s", path, ent.ff_name);

            ent.ff_fsize += folder_usage(search, levels>0 ? levels-1 : levels);
            /* Length of directory itself isn't counted here sadly
             * we'd need to do something lower level to know that
			 */
			if (levels != 0)  {
            	if (!human)
    				printf("%-12ld\t", ent.ff_fsize);
                else
                	display_human(ent.ff_fsize);
                printf("%s\\%s\n", path, ent.ff_name);
            }
            dircount++;
        }
        else {
        	if (roundcluster)
            	ent.ff_fsize = round_size(ent.ff_fsize);
        	filecount++;
        }
        total += ent.ff_fsize;
    } while (findnext(&ent) == 0);

	return total;
}

/**
 * Round a file size up to the cluster size of the drive
 **/
long round_size(long size)
{
	size = (size+cluster_size-1) / cluster_size;
    size *= cluster_size;
    return size;
}

/**
 * Display a file size in human readible units
 **/
void display_human(long size)
{
	static const char suffixes[] = "BkMGT";
    int level = 0;
    while(size > 1000) {
    	size /= 1000;
        level++;
    }
    printf("%ld%c\t", size, suffixes[level]);
}


