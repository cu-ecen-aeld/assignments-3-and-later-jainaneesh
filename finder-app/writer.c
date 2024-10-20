#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    // Check if the number of arguments is correct
    if (argc != 3)
    {
        syslog(LOG_ERR, "Invalid number of arguments. Usage: %s <path_to_file> <write_to_file>", argv[0]);
        return 1;
    }

    const char *path_to_file = argv[1];
    const char *write_to_file = argv[2];

    // Open the file for writing
    FILE *file = fopen(path_to_file, "w");
    if (file == NULL)
    {
        syslog(LOG_ERR, "Failed to open file %s: %s", path_to_file, strerror(errno));
        return 1;
    }

    // Log the write operation
    syslog(LOG_DEBUG, "Writing %s to %s", write_to_file, path_to_file);

    // Write the string to the file
    if (fprintf(file, "%s", write_to_file) < 0)
    {
        syslog(LOG_ERR, "Failed to write to file %s: %s", path_to_file, strerror(errno));
        fclose(file);
        return 1;
    }

    // Close the file
    fclose(file);

    // Success message
    syslog(LOG_DEBUG, "Successfully wrote %s to %s", write_to_file, path_to_file);

    return 0;
}
