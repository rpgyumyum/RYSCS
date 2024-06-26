#include "builtins.h"

int cd(char** args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "expected argument to \"cd\"\n");
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
        perror("lsh");
        }
    }
    return 1;
}

int help()
{
    printf("Type programs and args then hit enter.\n");
    printf("The following functions are builtin:\n");
    printf("\t- cd \n\t- help \n\t- exit \n\t- ls \n\t- pwd \n\t- rm \n\t- mv \n\t- cp \n\t- mkdir \n\t- rmdir \n\t- cat \n\t- touch \n\t- wc \n\t- grep \n\t- kill \n\t- chmod \n\t- clear");
    printf("\nEnjoy using the shell :)\n");
    return 1;

}

int shell_exit()
{
    return 0;
}

int ls(char** args)
{
    int ls_long = 0, show_hidden = 0;
    char cPath[FILENAME_MAX]; 
    if (getcwd(cPath, sizeof(cPath)) == NULL)
    {
        perror("ls error");
    }

    if (args[1] != NULL)
    {
        for (char* c = *++args; c; c=*++args) 
        {
            if (strcmp("-l", c) == 0)
            {
                ls_long = 1;
            }
            else if (strcmp("-a", c) == 0)
            {
                show_hidden = 1;
            } 
            else if (strcmp("-la", c) == 0 || strcmp("-al", c) == 0)
            {
                show_hidden = 1;
                ls_long = 1;
            }
            else if (opendir(cPath) != NULL) 
            {
                strcpy(cPath, c); 
            }
            else if (strcmp("-h", c) == 0)
            {
                printf("Usage: ls <args> <directory>\n");
                printf("\t-l --> show verbose output\n");
                printf("\t-a --> show hidden file\n");
                return 1;
            }
            else 
            {
                fprintf(stderr, "ls error: invalid arguments");
                return 1;
            }
        }
    }

	struct dirent *d;
	DIR *dh = opendir(cPath);
    if (dh == NULL) 
    {
        perror("error opening directory");
        return 1;
    }

	//while the next entry is not readable print directory files
	while ((d = readdir(dh)) != NULL)
	{
        //if the user didnt enable hidden skip files that begin with '.'
        if (d->d_name[0] == '.' && !show_hidden) continue;


            //if the user enabled long output print file info
        if (ls_long) 
        {
#ifdef _WIN32
        WIN32_FIND_DATA stats;

        HANDLE hFind = FindFirstFile(d->d_name, &stats);

        if (hFind != INVALID_HANDLE_VALUE) 
        {
            //size 
            int bytes = stats.nFileSizeLow;

            if (bytes < 1000) 
                printf(" %4dB", bytes);
            else if (bytes < 1000000) 
                printf(" %4dK", bytes / 1000);
            else if (bytes < 1000000000) 
                printf(" %4dM", bytes / 1000000);
            else 
                printf(" %4dG", bytes / 1000000000);
            
            SYSTEMTIME stUTC, stLocal;
            FILETIME ftCreate;

            SYSTEMTIME stCurrent;
            GetSystemTime(&stCurrent);

            // Extract the file creation time
            ftCreate = stats.ftCreationTime;

            // Convert the file time to local time
            FileTimeToSystemTime(&ftCreate, &stUTC);
            SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

            char* month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

            // Print the formatted creation time
            printf("%3d %3s", stLocal.wDay, month[stLocal.wMonth - 1]);
            if (stLocal.wYear == stCurrent.wYear)
                printf(" %2d:%02d", stLocal.wHour, stLocal.wMinute);
            else
                printf(" %5d", stLocal.wYear);

            FindClose(hFind);
        }
        else 
        {
            fprintf(stderr, "FindFirstFile failed (%lu)\n", GetLastError());
            return 1;
        }
#else
        struct stat stats;
        if (stat(d->d_name, &stats) == -1) 
        {
            perror("stat error");
            return 1;
        }

        //permissions
        mode_t perms = stats.st_mode;
        printf( (perms & S_IFDIR) ? "d" : "-");
        printf( (perms & S_IRUSR) ? "r" : "-");
        printf( (perms & S_IWUSR) ? "w" : "-");
        printf( (perms & S_IXUSR) ? "x" : "-");
        printf( (perms & S_IRGRP) ? "r" : "-");
        printf( (perms & S_IWGRP) ? "w" : "-");
        printf( (perms & S_IXGRP) ? "x" : "-");
        printf( (perms & S_IROTH) ? "r" : "-");
        printf( (perms & S_IWOTH) ? "w" : "-");
        printf( (perms & S_IXOTH) ? "x" : "-");

        //owner and group
        uid_t uid = stats.st_uid;
        gid_t gid = stats.st_gid;
        struct passwd *pwd;
        struct group *grp;

        if ((pwd = getpwuid(uid)) != NULL)
        {
            printf(" %-10s", pwd->pw_name);
        }

        if ((grp = getgrgid(gid)) != NULL)
        {
            printf(" %-10s", grp->gr_name);
        }          

        //size
        int bytes = stats.st_size;

        if (bytes < 1000) 
            printf(" %4dB", bytes);
        else if (bytes < 1000000) 
            printf(" %4dK", bytes / 1000);
        else if (bytes < 1000000000) 
            printf(" %4dM", bytes / 1000000);
        else 
            printf(" %4dG", bytes / 1000000000);

        //file creation time in seconds then convert to date and time format
        struct tm dt;
        time_t current_time;
        char* month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        dt = *(gmtime(&stats.st_mtime));
        time(&current_time);
        struct tm* curr_time_info = localtime(&current_time);

        printf(" %3d %3s", dt.tm_mday, month[dt.tm_mon]);
        if (curr_time_info->tm_year - dt.tm_year == 0)
            printf(" %2d:%02d", dt.tm_hour, dt.tm_min);
        else
            printf(" %5d", dt.tm_year + 1900);                  

#endif
        }
	    printf(" %s ", d->d_name);
        if(ls_long) printf("\n");

	}
    if(!ls_long) printf("\n");

    return 1;
}

int pwd()
{
    char cPath[FILENAME_MAX]; 
    if (getcwd(cPath, sizeof(cPath)) == NULL)
    {
        perror("pwd error");
    }
    printf("%s\n", cPath);
    return 1;
}

int rm(char** args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "expected argument to \"rm\"\n");
        return 1;
    }
    
    for (char* c = *++args; c; c=*++args) 
    {
        printf("removing %s\n", c);

        if (unlink(c) == -1) 
        {
            perror("remove error");
            return 1;
        }
    }
         
    return 1;
}

int mv(char** args)
{
    if (args[1] == NULL || args[2] == NULL)
    {
        fprintf(stderr, "usage: mv <old_name> <new_name>\n");
    }
    else if (rename(args[1], args[2]) == -1) 
    {
        perror("mv error");
        return 1;
    }

    return 1;
}

int cp(char** args) 
{    
    if (args[1] == NULL || args[2] == NULL)
    {
        fprintf(stderr, "usage:  <in_file> <out_file>\n");
        return 1;
    }
#ifdef _WIN32
    if(!CopyFile(
        args[1],
        args[2],
        0
    ))
    {
        fprintf(stderr, "copying failed (%lu)\n", GetLastError());
        return 1;
    }
#else 
    int in_file = open(args[1], O_RDONLY);
    if (in_file == -1) 
    {
        perror("error opening input file");
        return 1;
    }

    int out_file = open(args[2], O_WRONLY | O_CREAT | O_TRUNC, READ_PERMISSIONS); // rw-rw-rw-
    if (out_file == -1) 
    {
        perror("error opening output file");
        close(in_file); //close the input file descriptor before returning
        return 1;
    }

    struct stat stat_buf;
    if (fstat(in_file, &stat_buf) == -1) 
    {
        perror("error getting input file size");
        close(in_file);
        close(out_file);
        return 1;
    }

    off_t offset = 0;
    while (offset < stat_buf.st_size) 
    {
        ssize_t bytes_sent = sendfile(out_file, in_file, &offset, stat_buf.st_size - offset);
        if (bytes_sent == -1) 
        {
            perror("Error copying file");
            close(in_file);
            close(out_file);
            unlink(args[2]); //remove file we wrote to if error occurs
            return 1;
        }
    }

    close(in_file);
    close(out_file);
#endif

    return 1;
}


int shell_mkdir(char** args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "expected argument to \"mkdir\"\n");
    }

    for (char* c = *++args; c; c=*++args)
    {
        printf("creating %s\n", c);
#ifdef _WIN32
        if (mkdir(c) == -1)
        {
            fprintf(stderr, "making directory failed(%lu)\n", GetLastError());
            return 1;
        }
#else 
        if (mkdir(c, EXEC_PERMISSIONS) == -1)
        {
            perror("mkdir error");
            return 1;
        }
#endif
    }

    return 1;
}

int shell_rmdir(char **args) 
{
    if (args[1] == NULL) 
    {
        fprintf(stderr, "expected argument to \"rmdir\"\n");
        return 1;
    }

    int recursive = 0; 
    char *target_dir = NULL;

    // Check for "-r" option
    if (strcmp(args[1], "-r") == 0) 
    {
        if (args[2] == NULL) 
        {
            fprintf(stderr, "expected directory argument after \"-r\"\n");
            return 1;
        }
        recursive = 1;
        target_dir = args[2];
    } 
    else 
    {
        target_dir = args[1];
    }

    // Check if the directory exists
    if (opendir(target_dir) == NULL) 
    {
        perror("opendir error");
        return 1;
    }

    // if recursive option is set, iterate over the directory contents and delete recursively
    if (recursive) 
    {
        DIR *dir = opendir(target_dir);
        if (dir == NULL) 
        {
            perror("opendir error");
            return 1;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) 
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            // make full path
            char *path = malloc(strlen(target_dir) + strlen(entry->d_name) + 2);
            sprintf(path, "%s/%s", target_dir, entry->d_name);

            // check if the entry is a directory
            DIR *subdir = opendir(path);
            if (subdir != NULL) 
            {
                closedir(subdir);
                // Recursive call to delete the subdirectory
                char *args[] = {"rmdir", "-r", path, NULL};
                shell_rmdir(args);
            } 
            else 
            {
                // regular file
                if (remove(path) == -1) 
                {
                    perror("remove error");
                    return 1;
                }
            }
            free(path);
        }
        closedir(dir);
    }

    // Remove the target directory itself
    if (rmdir(target_dir) == -1) 
    {
        perror("rmdir error");
        return 1;
    }

    return 1;
}

int cat(char** args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "expected argument to \"cat\"\n");
    }

    for (char* c = *++args; c; c=*++args) 
    {
        FILE *file;
        file = fopen(c, "r");
        if(file == NULL) 
        {
            perror("file open error");
            return 1;
        }

        char buff[BUFSIZE];
        while(fgets(buff, BUFSIZE, file)) 
        {
          printf("%s", buff);
        }

        fclose(file);    
    }

    return 1;
}

int touch(char** args)
{    
    if (args[1] == NULL)
    {
        fprintf(stderr, "expected argument to \"touch\"\n");
    }

    for (char* c = *++args; c; c=*++args) 
    {
        printf("creating %s\n", c);
        int out_file = open(c, O_CREAT, READ_PERMISSIONS); // rw-rw-rw-

        if (out_file == -1) 
        {
            perror("Error creating file");
            return 1;
        }

        close(out_file);

    }

   return 1;
}

int wc(char** args) 
{
    if (args[1] == NULL) 
    {
        fprintf(stderr, "usage: wc <file>...\n");
        return 1; 
    }

    for (char* c = *++args; c; c = *++args) 
    {
        FILE* file = fopen(c, "r");
        if (file == NULL) 
        {
            fprintf(stderr, "Could not open file: %s\n", c);
            continue; 
        }

        long int lines = 0;
        long int words = 0;
        long int chars = 0;

        char buf[BUFSIZE];

        while (fgets(buf, BUFSIZE, file) != NULL) 
        {
            lines++;

            int in_word = 0; 

            for (int i = 0; buf[i] != '\0'; i++) 
            {
                if (buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\t') 
                {
                    if (in_word) 
                    {
                        in_word = 0;
                        words++;
                    }
                } 
                else 
                {
                    in_word = 1;
                }
                chars++;
            }
        }

        printf("lines: %ld words: %ld characters: %ld \t %s\n", lines, words, chars, c);
        
        fclose(file); 
    }

    return 1; 
}

int grep(char** args)
{
    if (args[1] == NULL || args[2] == NULL)
    {
        fprintf(stderr, "usage grep <search_string> <file>\n");
        return 1;
    }

    FILE* file = fopen(args[2], "r");
    if (file == NULL) 
    {
        fprintf(stderr, "error opening file %s\n", args[2]);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* content = (char*)malloc(file_size + 1); 
    if (content == NULL) 
    {
        fprintf(stderr, "memory allocation failed\n");
        fclose(file);
        return 1;
    }

    // Read the file content into the allocated memory
    size_t bytes_read = fread(content, 1, file_size, file);
    content[bytes_read] = '\0'; 

    fclose(file);
    
    //split into lines 
    int bufsize = TOK_BUFSIZE; 
    int pos = 0;
    char** lines = malloc(bufsize * sizeof(char*));
    char* conten_token;

    if (!lines)
    {
        fprintf(stderr, "allocation error when trying to spilt the lines\n");
        return 1;
    }

    conten_token = strtok(content, "\n");
    while (conten_token != NULL)
    {
        lines[pos] = conten_token;
        pos++;

        if (pos >= bufsize)
        {
            bufsize += TOK_BUFSIZE;
            lines = realloc(lines , bufsize * sizeof(char*));
            if (!lines)
            {
                fprintf(stderr, "reallocation error\n");
                exit(0);
            }
        }

        conten_token = strtok(NULL, "\n");
    }

    lines[pos] = NULL;

    
    //iterate over lines and print the line if substring is found
    int line_cnt = 0;
    for (char* c = *++lines; c; c=*++lines) 
    {
        if (strstr(c, args[1]) != NULL) 
        {
            printf("line %d:\t %s\n", line_cnt, c); 
        }

        line_cnt++;
    }
    
    return 1;
}

#ifdef __linux__
int shell_chmod(char** args)
{
    if (args[1] == NULL || args[2] == NULL)
    {
        fprintf(stderr, "usage chmod <permissions> <file>\n");
        return 1;
    } 
    else
    {
        char* permissionStr = args[1];
        while (*permissionStr != '\0') 
        {
            if (*permissionStr < '0' || *permissionStr > '7') 
            {
                fprintf(stderr, "Invalid permission: %s\n", args[1]);
                return 1;
            }
            permissionStr++;
        }
    }

    // convert octal input to correct mode_t numeric value
    mode_t permissions = strtol(args[1], NULL, 8);
    if (chmod(args[2], permissions) == -1) 
    {
        perror("chmod");
    }
    
    return 1;
}

int clear()
{
    //print ANSI escape codes to clear terminal (should work on all major enviroment)
    //  \x1b[3J: Clears the entire screen and deletes all lines in the scrollback buffer.
    //  \x1b[H: Moves the cursor to the top-left corner of the terminal.
    //  \x1b[2J: Clears the entire screen without affecting the scrollback buffer.
    printf("\x1b[3J\x1b[H\x1b[2J");
    return 1;
}

int shell_kill(char** args)
{ 
    if (args[1] == NULL)
    {
        fprintf(stderr, "usage kill <pid>n");
    }
    else if(kill(atoi(args[1]), SIGTERM) == -1)
    {
        perror("kill");
    }

    return 1;
}
#endif
