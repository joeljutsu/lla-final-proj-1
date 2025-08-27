
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
    int realcount = dbheader->count;
    printf("realcount: %d\n", realcount);
    printf("sizeof employee_t:  %ld\n", sizeof(struct employee_t));
    printf("filesize computed: %ld\n", sizeof(struct dbheader_t) + sizeof(struct employee_t) * realcount);

    dbheader->magic = htonl(dbheader->magic);
    printf("output_file dbheader->magic: %d\n", dbheader->magic);
    dbheader->filesize = htonl(sizeof(struct dbheader_t) + sizeof(struct employee_t) * realcount);
    dbheader->count = htons(dbheader->filesize);
    printf("output_file dbheader->count: %d\n", dbheader->count);

    printf("output_file dbheader->version: %d\n", dbheader->version);
    dbheader->version = htons(dbheader->version);
    printf("output_file dbheader->version: %d\n", dbheader->version);

    lseek(fd, 0, SEEK_SET);
    write(fd, dbheader, sizeof(struct dbheader_t));

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

    struct dbheader_t *dbheader = calloc(1, sizeof(struct dbheader_t));

    if (dbheader == NULL)
    {
        printf("calloc failed to create db header\n");
        return STATUS_ERROR;
    }
    lseek(fd, 0, SEEK_SET);

    if (read(fd, dbheader, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t))
    {
        printf("ERROR!!!");
        perror("read");
        free(dbheader);
        return STATUS_ERROR;
    }
    printf("not error!!!");
    printf("1:: validate_db_header dbheader->version: %d\n", dbheader->version);
    dbheader->version = ntohs(dbheader->version);
    printf("2::validate_db_header dbheader->version: %d\n", dbheader->version);

    // dbheader->version = 1;
    dbheader->count = ntohs(dbheader->count);
    dbheader->magic = ntohl(dbheader->magic); // dbheader->magic; //
    dbheader->filesize = ntohl(dbheader->filesize);

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
    fstat(fd, &dbstat);
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