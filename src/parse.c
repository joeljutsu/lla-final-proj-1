
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include "parse.h"
#include "common.h"

int read_employees(int fd, struct dbheader_t *dbheader, struct employee_t **employees_out)
{
    if (fd < 0)
    {
        printf("got bad fd from user\n");
        return STATUS_ERROR;
    }

    if (dbheader == NULL)
    {
        printf("null dbheader\n");
        return STATUS_ERROR;
    }

    int count = dbheader->count;

    struct employee_t *employees = calloc(count, sizeof(struct employee_t));
    if (employees == NULL)
    {
        printf("Calloc failed on employees\n");
        return STATUS_ERROR;
    }

    if (read(fd, employees, count * sizeof(struct employee_t)) == -1)
    {
        printf("read failed!\n");
        return STATUS_ERROR;
    }

    for (int i = 0; i < count; i++)
    {
        employees[i].hours = ntohl(employees[i].hours);
        // employees[i].address = ntohl(employees[i].address);
    }

    *employees_out = employees;

    return STATUS_SUCCESS;
}

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

    dbheader->magic = htonl(dbheader->magic);
    if (dbheader->magic == -1)
    {
        return -1;
    }

    dbheader->filesize = htonl(sizeof(struct dbheader_t) + sizeof(struct employee_t) * realcount);

    if (dbheader->filesize == -1)
    {
        return -1;
    }

    dbheader->count = htons(dbheader->filesize);

    // if (dbheader->count == -1)
    // {
    //     return -1;
    // }

    dbheader->version = htons(dbheader->version);

    // if (dbheader->version == -1)
    // {
    //     return -1;
    // }

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

    // if (dbheader->version == -1)
    // {
    //     return -1;
    // }

    dbheader->count = ntohs(dbheader->count);

    // if (dbheader->count == -1)
    // {
    //     return -1;
    // }

    dbheader->magic = ntohl(dbheader->magic); // dbheader->magic; //

    if (dbheader->magic == -1)
    {
        return -1;
    }

    dbheader->filesize = ntohl(dbheader->filesize);

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

    if (dbheader->filesize != dbstat.st_size)
    {
        printf("Corrupted database \n");
        free(dbheader);
        return STATUS_ERROR;
    }
    *headerOut = dbheader;

    return STATUS_SUCCESS;
}

int create_db_header(struct dbheader_t **header_out)
{
    if (header_out == NULL)
    {
        return -1;
    }
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
