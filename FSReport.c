#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

/**
 * print file properties.
 * Resources: CForWin - stat() function
 */
void print_file_properties(struct stat stats) {
    struct tm dt;

    // File permissions
    printf("\nFile access: ");
    if (stats.st_mode & R_OK) printf("read ");
    if (stats.st_mode & W_OK) printf("write ");
    if (stats.st_mode & X_OK) printf("execute");

    // File size
    printf("\nFile size: %d", stats.st_size);

    // Get file creation time in seconds and 
    // convert seconds to date and time format
    dt = *(gmtime(&stats.st_ctime));
    printf("\nCreated on: %d-%d-%d %d:%d:%d", dt.tm_mday, dt.tm_mon, dt.tm_year + 1900, 
                                              dt.tm_hour, dt.tm_min, dt.tm_sec);

    // File modification time
    dt = *(gmtime(&stats.st_mtime));
    printf("\nModified on: %d-%d-%d %d:%d:%d", dt.tm_mday, dt.tm_mon, dt.tm_year + 1900, 
                                              dt.tm_hour, dt.tm_min, dt.tm_sec);
}


int main() {
    struct stat stats;
    char path[200] = {'\0'};

    printf("Enter a path: ");
    scanf("%s", path);

    if (stat(path, &stats) == 0) { //successful
        print_file_properties(stats);
    } else {
        printf("Unable to get file properties.\nFile may not exist.\n");
    }

    return 0;
}
