#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

int main(int argc, char *argv[]) 
{
    if (argc > 3) {
        syslog(LOG_ERR, "Too many argument!");
        return EXIT_FAILURE;
    } else if (argc < 3) {
        syslog(LOG_ERR, "Missing argument!");
        return EXIT_FAILURE;
    }

    char* writeFile = argv[1];
    char* writeStr = argv[2];
    
    FILE *file = fopen(writeFile, "a");
    if (file == NULL) {
        syslog(LOG_ERR, "Error opening/creating file!");
        return EXIT_FAILURE;
    }

    fprintf(file, "%s", writeStr);
    syslog(LOG_DEBUG, "Writing %s to %s", writeStr, writeFile);

    fclose(file);

    return EXIT_SUCCESS;
}