#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <grp.h>
#include <pwd.h>


/**
 * print file properties.
 * Resources: CForWin - stat() function
 */
void print_file_properties(struct stat stats, char filename[NAME_MAX]) {
    struct tm *dt;
    struct group *grp;
    struct passwd *pwd;

    //user name and group name
    pwd = getpwuid(stats.st_uid);
    grp = getgrgid(stats.st_gid);
    printf("%s (%s)\t", pwd->pw_name, grp->gr_name);

    //file inode
    printf("%d\t", stats.st_ino);

    //file permissions
    printf( (S_ISDIR(stats.st_mode)) ? "d" : "-");
    printf( (stats.st_mode & S_IRUSR) ? "r" : "-");
    printf( (stats.st_mode & S_IWUSR) ? "w" : "-");
    printf( (stats.st_mode & S_IXUSR) ? "x" : "-");
    printf( (stats.st_mode & S_IRGRP) ? "r" : "-");
    printf( (stats.st_mode & S_IWGRP) ? "w" : "-");
    printf( (stats.st_mode & S_IXGRP) ? "x" : "-");
    printf( (stats.st_mode & S_IROTH) ? "r" : "-");
    printf( (stats.st_mode & S_IWOTH) ? "w" : "-");
    printf( (stats.st_mode & S_IXOTH) ? "x" : "-");
    printf("\t");

    //file size
    printf("%d\t", stats.st_size);

    //file/directory name
    printf("%s\n", filename);

    //get file access time in seconds and convert seconds to date and time format
    dt = localtime(&stats.st_atime);
    printf("\t%s\t", asctime(dt));

    // printf("\t%d-%d-%d %d:%d:%d\t", dt.tm_mday, dt.tm_mon, dt.tm_year + 1900, 
    //                                           dt.tm_hour, dt.tm_min, dt.tm_sec);

    //file modification time
    dt = localtime(&stats.st_mtime);
    printf("%s\n", asctime(dt));
    // dt = gmtime(&stats.st_mtime);
    // printf("\t%d-%d-%d %d:%d:%d\n\n", dt.tm_mday, dt.tm_mon, dt.tm_year + 1900, 
    //                                           dt.tm_hour, dt.tm_min, dt.tm_sec);
}

typedef struct stat Stat;


void read_all_dir(char path[NAME_MAX], int level) {
    DIR *folder;
    Stat stats; 
    struct dirent *dir;
    char cur_path[PATH_MAX] = {'\0'};
    int num_paths = 0; //files and directories

    folder = opendir(path);
    if (folder == NULL) {
        printf("Error reading directory %s.\nPlease ensure you enter the full path name of the root directory.", cur_path);
        return;
    }

    //get num paths
    while (dir = readdir(folder)) {
        num_paths++;
    }
    closedir(folder); //close after reading all files in directory

    //create and populate 2D array
    folder = opendir(path);
    char **list_of_paths = calloc(num_paths, sizeof(char*));
    for (int i = 0; i < num_paths; i++) {
        dir = readdir(folder);
        list_of_paths[i] = calloc(strlen(dir->d_name) + 1, sizeof(char));
        strcpy(list_of_paths[i], dir->d_name);
    }
    closedir(folder); //close after reading all files in directory

    //bubble sort 2D array
    char *temp;
    for (int i = 0; i < num_paths-1; i++) {
        for (int j = i+1; j < num_paths; j++) {
            if (strcmp(list_of_paths[i], list_of_paths[j]) > 0) {
                temp = list_of_paths[i];
                list_of_paths[i] = list_of_paths[j];
                list_of_paths[j] = temp;
            }
        }
    }

    printf("\n\n\nPRINTING CONTENTS OF PATH: %s\nLEVEL %d\n", path, level);
    //printing directories only
    int header_printed = 0;
    for (int i = 0; i < num_paths - 1; i++) {
        strcpy(cur_path, path);
        //skip hidden files representing parent and current directory
        if (strcmp(list_of_paths[i], ".") == 0 || strcmp(list_of_paths[i], "..") == 0) continue;
        //add filename to path
        strcat(cur_path, "/");
        strcat(cur_path, list_of_paths[i]);
        if (stat(cur_path, &stats) != 0) printf("Unable to get directory properties for %s\n.", list_of_paths[i]); 
        if (!S_ISDIR(stats.st_mode)) continue;
        if (!header_printed) {
            printf("\nprinting subdirectories\n");
            header_printed = 1;
        }
        print_file_properties(stats, list_of_paths[i]);
        //printf("Dir name: %s\n", list_of_paths[i]);
    }

    //printing files only
    header_printed = 0;
    for (int i = 0; i < num_paths - 1; i++) {
        strcpy(cur_path, path);
        //skip hidden files representing parent and current directory
        if (strcmp(list_of_paths[i], ".") == 0 || strcmp(list_of_paths[i], "..") == 0) continue;
        //add filename to path
        strcat(cur_path, "/");
        strcat(cur_path, list_of_paths[i]);
        if (stat(cur_path, &stats) != 0) printf("Unable to get file properties for %s\n.", list_of_paths[i]); 
        if (S_ISDIR(stats.st_mode)) continue;
        if (!header_printed) {
            printf("\nprinting files\n");
            header_printed = 1;
        }
        print_file_properties(stats, list_of_paths[i]);        
        //printf("File: %s\n", list_of_paths[i]);
    }

    //recursively explore subdirectories
    for (int i = 0; i < num_paths - 1; i++) {
        strcpy(cur_path, path);
        //skip hidden files representing parent and current directory
        if (strcmp(list_of_paths[i], ".") == 0 || strcmp(list_of_paths[i], "..") == 0) continue;
        //add filename to path
        strcat(cur_path, "/");
        strcat(cur_path, list_of_paths[i]);
        if (stat(cur_path, &stats) != 0) printf("Unable to get file properties for %s\n.", list_of_paths[i]); 
        if (S_ISDIR(stats.st_mode)) read_all_dir(cur_path, level + 1);
    }
}


int main() {
    char path[200] = {'\0'};

    printf("Enter a path: ");
    scanf("%s", path);

    read_all_dir(path, 1);

    return 0;
}
