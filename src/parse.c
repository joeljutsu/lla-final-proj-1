
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include "parse.h"
#include "common.h"

int add_employee(struct dbheader_t *header, struct employee_t *employees, char *addstring)
{
    printf("parse::ADD EMPLOYEE: %s\n", addstring);

    char *name = strtok(addstring, ",");
    char *addr = strtok(NULL, ",");
    char *hours = strtok(NULL, ",");

    printf("parse::add_employee:: HELLO %s %s %s\n", name, addr, hours);

    strncpy(employees[header->count - 1].name, name, sizeof(employees[header->count - 1].name));
    strncpy(employees[header->count - 1].address, addr, sizeof(employees[header->count - 1].address));

    employees[header->count - 1].hours = atoi(hours);

    return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbheader, struct employee_t **employees_out)
{
    if (fd < 0)
    {
        printf("parse::read_employee::got bad fd from user\n");
        return STATUS_ERROR;
    }

    int count = dbheader->count;

    struct employee_t *employees = calloc(count, sizeof(struct employee_t));
    if (employees == (void *)-1)
    {
        printf("parse::read_employee:: Calloc failed on employees\n");
        return STATUS_ERROR;
    }

    if (read(fd, employees, count * sizeof(struct employee_t)) == -1)
    {
        printf("parse::read_employee:: read failed!\n");
        return STATUS_ERROR;
    }

    for (int i = 0; i < count; i++)
    {
        employees[i].hours = ntohl(employees[i].hours);
        printf("parse::read_employee:: setting employee[%d] hours = %d\n", i, employees[i].hours);
    }

    *employees_out = employees;

    return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbheader, struct employee_t *employees)
{
    printf("OUTPUT_FILE...\n");
    if (fd < 0)
    {
        printf("parse::output_file:: Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    int realcount = dbheader->count;

    dbheader->magic = htonl(dbheader->magic);
    dbheader->filesize = htonl(sizeof(struct dbheader_t) + (sizeof(struct employee_t) * realcount));
    dbheader->count = htons(dbheader->count);
    dbheader->version = htons(dbheader->version);

    if (lseek(fd, 0, SEEK_SET) == -1)
    {
        printf("perror::output_fle:: error lseek!");
        return STATUS_ERROR;
    }
    printf("output_file:: lseek OK!!!\n");
    if (write(fd, dbheader, sizeof(struct dbheader_t)) == -1)
    {
        printf("parse::output_file:: error write!");
        return STATUS_ERROR;
    }
    printf("output_file:: write() OK!!!\n");

    for (int i = 0; i < realcount; i++)
    {
        employees[i].hours = htonl(employees[i].hours);
        if (write(fd, &employees[i], sizeof(struct employee_t)) == -1)
        {
            printf("write add emp ERROR!\n");
            return STATUS_ERROR;
        }
    }

    // close(fd);

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
    if (dbheader == (void *)-1)
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
    printf("validate_db_header:: filesize OK!!!\n");
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
