
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

    if (employees == NULL)
    {
        printf("parse::add_employee::employees NULL!!!!!!\n");
        return -1;
    }
    else
    {
        printf("parse::add_employee::employees NOT null!!!\n");
    }
    printf("parse::add_employee:: sizeof employees name %lu\n", sizeof(employees[header->count - 1].name));
    strncpy(employees[header->count - 1].name, name, sizeof(employees[header->count - 1].name));
    printf("parse::add_employee::strncpy name OK! = emp name = %s\n", employees[header->count - 1].name);

    printf("parse::add_employee:: sizeof employees address %lu\n", sizeof(employees[header->count - 1].address));
    strncpy(employees[header->count - 1].address, addr, sizeof(employees[header->count - 1].address));
    printf("parse::add_employee:: strncpy address OK! emp addr = %s\n", employees[header->count - 1].address);

    employees[header->count - 1].hours = atoi(hours);
    printf("parse::add_employee:: exiting add_employee()\n");

    return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbheader, struct employee_t **employees_out)
{
    if (fd < 0)
    {
        printf("parse::read_employee::got bad fd from user\n");
        return STATUS_ERROR;
    }

    if (dbheader == NULL)
    {
        printf("parse::read_employee::null dbheader\n");
        return STATUS_ERROR;
    }

    int count = dbheader->count;

    struct employee_t *employees = calloc(count, sizeof(struct employee_t));
    if (employees == NULL)
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
    if (dbheader == NULL)
    {
        printf("parse::output::file:: got null dbheader!");
        return STATUS_ERROR;
    }
    int realcount = dbheader->count;

    printf("parse::output_file:: realcount = %d", realcount);
    dbheader->magic = htonl(dbheader->magic);
    if (dbheader->magic == -1)
    {
        return -1;
    }
    dbheader->filesize = htonl(sizeof(struct dbheader_t) + (sizeof(struct employee_t) * realcount));
    dbheader->count = htons(dbheader->count);
    dbheader->version = htons(dbheader->version);

    if (lseek(fd, 0, SEEK_SET) == -1)
    {
        printf("perror::output_fle:: error lseek!");
        return STATUS_ERROR;
    }
    if (write(fd, dbheader, sizeof(struct dbheader_t)) == -1)
    {
        printf("parse::output_file:: error write!");
        return STATUS_ERROR;
    }
    for (int i = 0; i < realcount; i++)
    {
        printf("parse::output_file:: writing employees[%d]->name = %s\n", i, employees[i].name);
        employees[i].hours = htonl(employees[i].hours);
        write(fd, &employees[i], sizeof(struct employee_t));
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
