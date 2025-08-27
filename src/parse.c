
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/stat.h>

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

    dbheader->magic = htonl(dbheader->magic);
    dbheader->filesize = htonl(sizeof(struct dbheader_t) + sizeof(struct employee_t) * realcount);
    dbheader->count = htons(dbheader->count);
    dbheader->version = htons(dbheader->version);

    printf("output_file magic: %d\n", dbheader->magic);

    lseek(fd, 0, SEEK_SET);
    write(fd, dbheader, sizeof(struct dbheader_t));
    printf("output_file() STATUS_SUCCESS");

    return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut)
{
    if (fd < 0)
    {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    printf("validate_db_header fd ok!\n");

    struct dbheader_t *dbheader = calloc(1, sizeof(struct dbheader_t));
    if (dbheader == NULL)
    {
        printf("calloc failed to create db header\n");
        return STATUS_ERROR;
    }

    printf("validate_db_header dbheader callok() ok!\n");
    printf("size of struct dbheader_t %ld\n", sizeof(struct dbheader_t));
    if (read(fd, dbheader, sizeof(struct dbheader_t) != sizeof(struct dbheader_t)))
    {
        perror("read");
        free(dbheader);
        return STATUS_ERROR;
    }
    printf("validate_db_header dbheader read ok\n");
    printf("1) validate_db_header() dbheader->magic = %d\n", dbheader->magic);
    printf("validate_db_header() dbheader->version = %d\n", dbheader->version);

    dbheader->version = ntohs(dbheader->version);
    dbheader->count = ntohs(dbheader->count);
    dbheader->magic = ntohl(dbheader->magic);
    dbheader->filesize = ntohl(dbheader->filesize);

    printf("2) validate_db_header() dbheader->magic = %d\n", dbheader->magic);
    printf("HEADER MAGIC: %d\n", HEADER_MAGIC);
    if (dbheader->magic != ntohl(HEADER_MAGIC))
    {
        printf("Improper header magic\n");
        free(dbheader);
        return STATUS_ERROR;
    }

    printf("header version = %d", dbheader->version);
    if (dbheader->version != 0x1)
    {
        printf("Improper header version\n");
        free(dbheader);
        return STATUS_ERROR;
    }

    struct stat dbstat = {0};
    fstat(fd, &dbstat);
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