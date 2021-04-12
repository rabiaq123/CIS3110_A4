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
#include <math.h>

/**
 * Name: Rabia Qureshi
 * Student #: 1046427
 * E-mail: rqureshi@uoguelph.ca
 */

typedef struct stat Stat; //so we can just use 'Stat' to declare a new stat struct


/**
 * print file or directory properties for -tree report type.
 * Resources: CForWin - stat() function
 */
void print_file_props_tree(Stat stats, char filename[NAME_MAX]) {
    struct tm *dt;
    struct group *grp;
    struct passwd *pwd;
    char *formatted_dt;

    //user name and group name
    pwd = getpwuid(stats.st_uid);
    grp = getgrgid(stats.st_gid);
    printf("%s (%s)\t", pwd->pw_name, grp->gr_name);

    //file/directory inode
    printf("%d\t", stats.st_ino);

    //file/directory permissions
    /*
    if true, first option gets printed; else second option to the right of the : gets printed
    */
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

    //file/directory name
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
 * @param full_path the root directory from which further recursive calls may be made
 * @param level level of directory at which recursive call is (starts at 1)
 * @param path name of file directory on its own
 */
void tree_report(char full_path[NAME_MAX], int level, char *path) {
    DIR *folder;
    Stat stats; 
    struct dirent *dir;
    char cur_path[PATH_MAX] = {'\0'}; //could be the root dir or subdir depending on recursive call
    int num_paths = 0; //includes files and directories

    folder = opendir(full_path);
    if (!folder) return;

    //get num paths
    while ((dir = readdir(folder)) != NULL) num_paths++;
    closedir(folder); //close after reading all files in directory

    /* 
    create and populate 2D array of the names of the paths in current directory only
    array will contain only the name of the directory/file itself
    */
    folder = opendir(full_path);
    char **list_of_paths = calloc(num_paths, sizeof(char*)); //paths in current directory
    for (int i = 0; i < num_paths; i++) {
        dir = readdir(folder);
        list_of_paths[i] = calloc(strlen(dir->d_name) + 1, sizeof(char));
        strcpy(list_of_paths[i], dir->d_name);
    }
    closedir(folder); //close after reading all files in directory

    //sort 2D array of paths in current directory in alphabetical order using bubble sort
    char *temp;
    for (int i = 0; i < num_paths; i++) {
        for (int j = i+1; j < num_paths; j++) {
            if (strcmp(list_of_paths[i], list_of_paths[j]) > 0) {
                temp = list_of_paths[i];
                list_of_paths[i] = list_of_paths[j];
                list_of_paths[j] = temp;
            }
        }
    }

    //if Level 1 then display the full path name otherwise just the directory name
    if (level == 1) printf("\nLevel %d: %s\n", level, full_path);
    else printf("\nLevel %d: %s\n", level, path);

    //printing directories only
    int dir_header_printed = 0;
    for (int i = 0; i < num_paths; i++) {
        //skip hidden files representing parent (..) and current (.) directory
        if (strcmp(list_of_paths[i], ".") == 0 || strcmp(list_of_paths[i], "..") == 0) continue;
        //add directory name to path to get attributes of file
        strcpy(cur_path, full_path);
        strcat(cur_path, "/");
        strcat(cur_path, list_of_paths[i]);
        //attempt to get attributes of directory
        if (stat(cur_path, &stats) != 0) printf("Unable to get directory properties for %s\n.", list_of_paths[i]); 
        if (!S_ISDIR(stats.st_mode)) continue;
        if (!dir_header_printed) {
            printf("Directories\n");
            dir_header_printed = 1;
        }
        print_file_props_tree(stats, list_of_paths[i]);
    }
    memset(cur_path, '\0', strlen(cur_path));

    //printing files only
    int files_header_printed = 0;
    for (int i = 0; i < num_paths; i++) {
        //skip hidden files representing parent (..) and current (.) directory
        if (strcmp(list_of_paths[i], ".") == 0 || strcmp(list_of_paths[i], "..") == 0) continue;
        //add filename to path
        strcpy(cur_path, full_path);
        strcat(cur_path, "/");
        strcat(cur_path, list_of_paths[i]);
        //attempt to get attributes of file
        if (stat(cur_path, &stats) != 0) printf("Unable to get file properties for %s\n.", list_of_paths[i]);
        if (S_ISDIR(stats.st_mode)) continue;
        if (!files_header_printed) {
            if (dir_header_printed) printf("\n"); //add space between directories and files sections if both exist
            printf("Files\n");
            files_header_printed = 1;
        }
        print_file_props_tree(stats, list_of_paths[i]);
    }
    memset(cur_path, '\0', strlen(cur_path));

    //recursively explore subdirectories
    for (int i = 0; i < num_paths; i++) {
        //skip hidden files representing parent (..) and current (.) directory
        if (strcmp(list_of_paths[i], ".") == 0 || strcmp(list_of_paths[i], "..") == 0) continue;
        //add filename or directory name to path
        strcpy(cur_path, full_path);
        strcat(cur_path, "/");
        strcat(cur_path, list_of_paths[i]);
        if (stat(cur_path, &stats) != 0) printf("Unable to get file properties for %s\n.", list_of_paths[i]); 
        if (S_ISDIR(stats.st_mode)) tree_report(cur_path, level + 1, list_of_paths[i]); //if directory, go a level deeper
    }
}


/**
 * print file or directory properties for -inode report type.
 */
void print_file_props_inode(Stat stats, char filename[NAME_MAX]) {
    struct tm *dt;
    struct group *grp;
    struct passwd *pwd;
    char *formatted_dt;

    //file/directory inode
    printf("%d:\t", stats.st_ino);

    //file/directory size
    printf("%d\t", stats.st_size);

    //number of 512B blocks allocated to the file/directory
    printf("%d\t", stats.st_blocks);

    //(file/directory size)/12
    printf("%.0f\t", ceil(stats.st_blocks/12));

    //file/directory name
    printf("%s\n", filename);
}


/**
 * inode report generation
 * read all directories and perform recursive calls until all subdirectories have been read
 * @param full_path the root directory from which further recursive calls may be made
 * @param level level of directory at which recursive call is (starts at 1)
 * @param path name of file directory on its own
 */
void inode_report(char full_path[NAME_MAX], int level, char *path) {
    DIR *folder;
    Stat stats; 
    struct dirent *dir;
    char cur_path[PATH_MAX] = {'\0'}; //could be the root dir or subdir depending on recursive call
    int num_paths = 0; //includes files and directories

    folder = opendir(full_path);
    if (!folder) return;

    //get num paths
    while ((dir = readdir(folder)) != NULL) num_paths++;
    closedir(folder); //close after reading all files/subdirectories in directory

    /* 
    create and populate 2D array of the names of the paths in current directory only
    array will contain only the name of the directory/file itself
    */
    folder = opendir(full_path);
    char **list_of_paths = calloc(num_paths, sizeof(char*)); //paths in current directory
    for (int i = 0; i < num_paths; i++) {
        dir = readdir(folder);
        list_of_paths[i] = calloc(strlen(dir->d_name) + 1, sizeof(char));
        strcpy(list_of_paths[i], dir->d_name);
    }
    closedir(folder); //close after reading all files in directory

    //get inode number for each path in list_of_paths[] array
    ino_t list_of_inodes[num_paths];
    for (int i = 0; i < num_paths; i++) {
        //skip hidden files representing parent (..) and current (.) directory
        if (strcmp(list_of_paths[i], ".") == 0 || strcmp(list_of_paths[i], "..") == 0) continue;
        //add directory name to path to get attributes of file/dir
        strcpy(cur_path, full_path);
        strcat(cur_path, "/");
        strcat(cur_path, list_of_paths[i]);
        //attempt to get attributes of file/dir
        if (stat(cur_path, &stats) != 0) printf("Unable to get directory properties for %s\n.", list_of_paths[i]);
        list_of_inodes[i] = stats.st_ino;
    }

    //sort 2D array of paths in current directory by inode number in ascending order using bubble sort
    ino_t temp_inode;
    char *temp_path;
    for (int i = 0; i < num_paths; i++) {
        for (int j = i+1; j < num_paths; j++) {
            if (list_of_inodes[i] > list_of_inodes[j]) {
                //sorting inodes
                temp_inode = list_of_inodes[i];
                list_of_inodes[i] = list_of_inodes[j];
                list_of_inodes[j] = temp_inode;
                //sorting corresponding paths
                temp_path = list_of_paths[i];
                list_of_paths[i] = list_of_paths[j];
                list_of_paths[j] = temp_path;
            }
        }
    }

    //if Level 1 then display the full path name otherwise just the directory name
    if (level == 1) printf("\nLevel %d Inodes: %s\n", level, full_path);
    else printf("\nLevel %d Inodes: %s\n", level, path);

    //printing inode information for paths in current directory
    for (int i = 0; i < num_paths; i++) {
        //skip hidden files representing parent (..) and current (.) directory
        if (strcmp(list_of_paths[i], ".") == 0 || strcmp(list_of_paths[i], "..") == 0) continue;
        //add directory name to path to get attributes of file
        strcpy(cur_path, full_path);
        strcat(cur_path, "/");
        strcat(cur_path, list_of_paths[i]);
        //attempt to get attributes of file
        if (stat(cur_path, &stats) != 0) printf("Unable to get directory properties for %s\n.", list_of_paths[i]);
        print_file_props_inode(stats, list_of_paths[i]);
    }
    memset(cur_path, '\0', strlen(cur_path));

    //recursively explore subdirectories
    for (int i = 0; i < num_paths; i++) {
        //skip hidden files representing parent (..) and current (.) directory
        if (strcmp(list_of_paths[i], ".") == 0 || strcmp(list_of_paths[i], "..") == 0) continue;
        //add filename or directory name to path
        strcpy(cur_path, full_path);
        strcat(cur_path, "/");
        strcat(cur_path, list_of_paths[i]);
        if (stat(cur_path, &stats) != 0) printf("Unable to get file properties for %s\n.", list_of_paths[i]); 
        if (S_ISDIR(stats.st_mode)) inode_report(cur_path, level + 1, list_of_paths[i]); //if directory, go a level deeper
    }
}


int main(int argc, char *argv[]) {
    //check for invalid command-line arguments
    if (argc != 3) {
        printf("Invalid input.\nEnter (1) report type and (2) the full path name of the root directory.\n");
        return -1;
    }

    char report_type[10] = {'\0'};
    char full_path[NAME_MAX] = {'\0'}, path[NAME_MAX] = {'\0'};

    //parse report type ()-tree or -inode) and full path of root directory
    strcpy(report_type, argv[1]);
    strcpy(full_path, argv[2]);

    if (strcmp(report_type, "-tree") == 0) tree_report(full_path, 1, path);
    if (strcmp(report_type, "-inode") == 0) inode_report(full_path, 1, path);

    return 0;
}
