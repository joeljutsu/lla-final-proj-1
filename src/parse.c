
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "parse.h"
#include "common.h"

int validate_db_header(int fd, struct dbheader_t **header_out)
{
    return 0;
}

int create_db_header(int fd, struct dbheader_t **header_out)
{
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL)
    {
        printf("calloc failed to create db header\n");
        return STATUS_ERROR;
    }
    header->version = 0x1;
    header->count = 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struct dbheader_t);

    *header_out = header;

    return STATUS_SUCCESS;
}
/*

int read_employees(int fd, struct dbheader_t *, struct employee_t **employees_out)
{
    return 0;
}
int output_file(int fd, struct dbheader_t *, struct employee_t **employees)
{

    return 0;
}
*/