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


typedef struct stat Stat; //so we can just use 'Stat' to declare a new stat struct


/**
 * print file or directory properties.
 * Resources: CForWin - stat() function
 */
void print_file_properties(struct stat stats, char filename[NAME_MAX]) {
    struct tm *dt;
    struct group *grp;
    struct passwd *pwd;
    char *formatted_dt;

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

    //file/directory size
    printf("%d\t", stats.st_size);

    //file/directory nam
    printf("%s\n", filename);

    //get file/directory access time 
    dt = localtime(&stats.st_atime);
    formatted_dt = asctime(dt);
    formatted_dt[strlen(formatted_dt) - 1] = '\0'; //remove trailing newline that is added by default
    printf("\t%s\t", formatted_dt);

    //file/directory modification time
    dt = localtime(&stats.st_mtime);
    formatted_dt = asctime(dt);
    formatted_dt[strlen(formatted_dt) - 1] = '\0'; //remove trailing newline that is added by default
    printf("%s\n", formatted_dt);
}


/**
 * tree report generation
 * read all directories and perform recursive calls until all subdirectories have been read
 * @param path the root directory from which further recursive calls may be made
 * @param level level of directory at which recursive call is (starts at 1)
 */
void tree_report(char path[NAME_MAX], int level) {
    DIR *folder;
    Stat stats; 
    struct dirent *dir;
    char cur_path[PATH_MAX] = {'\0'}; //could be the root dir or subdir depending on recursive call
    int num_paths = 0; //includes files and directories

    folder = opendir(path);
    if (!folder) return;

    //get num paths
    while ((dir = readdir(folder)) != NULL) num_paths++;
    closedir(folder); //close after reading all files in directory

    /* 
    create and populate 2D array of the names of the paths in current directory only
    array will contain only the name of the directory/file itself
    */
    folder = opendir(path);
    char **list_of_paths = calloc(num_paths, sizeof(char*)); //paths in current directory
    for (int i = 0; i < num_paths; i++) {
        dir = readdir(folder);
        list_of_paths[i] = calloc(strlen(dir->d_name) + 1, sizeof(char));
        strcpy(list_of_paths[i], dir->d_name);
    }
    closedir(folder); //close after reading all files in directory

    //sort 2D array of paths in current directory in alphabetical order using bubble sort
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

    printf("\nLevel %d: %s\n", level, path);
    //printing directories only
    int header_printed = 0;
    for (int i = 0; i < num_paths - 1; i++) {
        //skip hidden files representing parent (..) and current (.) directory
        if (strcmp(list_of_paths[i], ".") == 0 || strcmp(list_of_paths[i], "..") == 0) continue;
        //add directory name to path to get attributes of file
        strcpy(cur_path, path);
        strcat(cur_path, "/");
        strcat(cur_path, list_of_paths[i]);
        //attempt to get attributes of file
        if (stat(cur_path, &stats) != 0) printf("Unable to get directory properties for %s\n.", list_of_paths[i]); 
        if (!S_ISDIR(stats.st_mode)) continue;
        if (!header_printed) {
            printf("Directories\n");
            header_printed = 1;
        }
        print_file_properties(stats, list_of_paths[i]);
    }
    memset(cur_path, '\0', strlen(cur_path));

    //printing files only
    header_printed = 0;
    for (int i = 0; i < num_paths - 1; i++) {
        //skip hidden files representing parent (..) and current (.) directory
        if (strcmp(list_of_paths[i], ".") == 0 || strcmp(list_of_paths[i], "..") == 0) continue;
        //add filename to path
        strcpy(cur_path, path);
        strcat(cur_path, "/");
        strcat(cur_path, list_of_paths[i]);
        if (stat(cur_path, &stats) != 0) printf("Unable to get file properties for %s\n.", list_of_paths[i]); 
        if (S_ISDIR(stats.st_mode)) continue;
        if (!header_printed) {
            printf("Files\n");
            header_printed = 1;
        }
        print_file_properties(stats, list_of_paths[i]);        
    }
    memset(cur_path, '\0', strlen(cur_path));

    //recursively explore subdirectories
    for (int i = 0; i < num_paths - 1; i++) {
        //skip hidden files representing parent (..) and current (.) directory
        if (strcmp(list_of_paths[i], ".") == 0 || strcmp(list_of_paths[i], "..") == 0) continue;
        //add filename or directory name to path
        strcpy(cur_path, path);
        strcat(cur_path, "/");
        strcat(cur_path, list_of_paths[i]);
        if (stat(cur_path, &stats) != 0) printf("Unable to get file properties for %s\n.", list_of_paths[i]); 
        if (S_ISDIR(stats.st_mode)) tree_report(cur_path, level + 1); //if path is directory, go a level deeper
    }
}


int main(int argc, char *argv[]) {
    //check for invalid command-line arguments
    if (argc != 3) {
        printf("Invalid input.\nEnter (1) report type and (2) the full path name of the root directory.\n");
        return -1;
    }

    char report_type[10] = {'\0'};
    char path[NAME_MAX] = {'\0'};

    //parse report type ()-tree or -inode) and full path of root directory
    strcpy(report_type, argv[1]);
    strcpy(path, argv[2]);

    if (strcmp(report_type, "-tree") == 0) tree_report(path, 1);

    return 0;
}
