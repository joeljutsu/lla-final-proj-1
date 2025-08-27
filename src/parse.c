
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include "parse.h"
#include "common.h"

int output_file(int fd, struct dbheader_t *dbheader, struct employee_t **employees)
{
    if (fd < 0)
    {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }
    if (dbheader == NULL)
    {
        printf("got null dbheader!");
        return STATUS_ERROR;
    }
    int realcount = dbheader->count;
    printf("output_file::realcount: %d\n", realcount);
    printf("output_file::sizeof employee_t:  %ld\n", sizeof(struct employee_t));
    printf("output_file::filesize computed: %ld\n", sizeof(struct dbheader_t) + sizeof(struct employee_t) * realcount);

    dbheader->magic = htonl(dbheader->magic);
    if (dbheader->magic == -1)
    {
        return -1;
    }
    printf("output_file:: dbheader->magic: %d\n", dbheader->magic);

    printf("1)output_file:: dbheader->filesize: %d\n", dbheader->filesize);
    dbheader->filesize = htonl(sizeof(struct dbheader_t) + sizeof(struct employee_t) * realcount);
    printf("2)output_file:: dbheader->filesize: %d\n", dbheader->filesize);
    if (dbheader->filesize == -1)
    {
        return -1;
    }

    dbheader->count = htons(dbheader->filesize);
    printf("output_file:: dbheader->count: %d\n", dbheader->count);
    if (dbheader->count == -1)
    {
        return -1;
    }
    printf("1)output_file:: dbheader->version: %d\n", dbheader->version);
    dbheader->version = htons(dbheader->version);
    printf("2)output_file:: dbheader->version: %d\n", dbheader->version);
    if (dbheader->version == -1)
    {
        return -1;
    }

    if (lseek(fd, 0, SEEK_SET) == -1)
    {
        printf("error lseek!");
        return STATUS_ERROR;
    }
    if (write(fd, dbheader, sizeof(struct dbheader_t)) == -1)
    {
        printf("error write!");
        return STATUS_ERROR;
    }

    close(fd);

    return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut)
{
    printf("fd: %d\n", fd);
    if (fd < 0)
    {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    if (headerOut == NULL)
    {
        return -1;
    }

    struct dbheader_t *dbheader = calloc(1, sizeof(struct dbheader_t));

    if (dbheader == NULL)
    {
        printf("calloc failed to create db header\n");
        return STATUS_ERROR;
    }

    if (read(fd, dbheader, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t))
    {
        printf("ERROR!!!");
        perror("read");
        free(dbheader);
        return STATUS_ERROR;
    }

    dbheader->version = ntohs(dbheader->version);
    printf("validate_db_header:: dbheader->version: %d\n", dbheader->version);
    if (dbheader->version == -1)
    {
        return -1;
    }

    dbheader->count = ntohs(dbheader->count);
    printf("validate_db_header:: dbheader->count: %d\n", dbheader->count);
    if (dbheader->count == -1)
    {
        return -1;
    }

    printf("1)validate_db_header:: dbheader->magic: %d\n", dbheader->magic);
    dbheader->magic = ntohl(dbheader->magic); // dbheader->magic; //
    printf("2)validate_db_header:: dbheader->magic: %d\n", dbheader->magic);
    if (dbheader->magic == -1)
    {
        return -1;
    }
    // printf("true/false? %d\n", dbheader->magic != HEADER_MAGIC);

    dbheader->filesize = ntohl(dbheader->filesize);
    printf("validate_db_header:: dbheader->filesize: %d\n", dbheader->filesize);
    if (dbheader->filesize == -1)
    {
        return -1;
    }

    if (dbheader->magic != HEADER_MAGIC)
    {
        printf("Improper header magic\n");
        free(dbheader);
        return STATUS_ERROR;
    }

    if (dbheader->version != 1)
    {
        printf("Improper header version\n");
        free(dbheader);
        return STATUS_ERROR;
    }

    struct stat dbstat = {0};
    if (fstat(fd, &dbstat) == -1)
    {
        printf("error fstat");
        return STATUS_ERROR;
    }
    printf("dbstat st_size: %ld\n", dbstat.st_size);

    printf("validate_db_header filesize?: %d\n", dbheader->filesize);

    if (dbheader->filesize != dbstat.st_size)
    {
        printf("Corrupted database \n");
        free(dbheader);
        return STATUS_ERROR;
    }
    *headerOut = dbheader;
    printf("validate_db_header() SUCCESS\n");
    return STATUS_SUCCESS;
}

int create_db_header(struct dbheader_t **header_out)
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
    printf("create_db_header() SUCCESS\n");

    return STATUS_SUCCESS;
}

/*

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
int read_employees(int fd, struct dbheader_t *, struct employee_t **employees_out)
{
    return 0;
}
*/