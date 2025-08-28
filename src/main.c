#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[])
{
    printf("Usage: %s -n -f <database file> \n", argv[0]);
    printf("\t -n - create new database file \n");
    printf("\t -f - (recquired) path to database file \n");
    return;
}

int main(int argc, char *argv[])
{
    printf("\n:::::START RUN:::::\n");
    char *filepath = NULL;
    char *addstring = NULL;

    bool newfile = false;
    int c = 0;
    int dbfd = -1;
    struct dbheader_t *header = NULL;

    struct employee_t *employees = NULL;

    while ((c = getopt(argc, argv, "nf:a:")) != -1)
    {
        switch (c)
        {
        case 'n':
            newfile = true;

            break;
        case 'f':
            filepath = optarg;
            break;
        case 'a':
            addstring = optarg;
            break;
        case '?':
            printf("Uknown option -%c\n", c);
            break;
        default:
            return -1;
        }
    }

    if (filepath == NULL)
    {
        printf("Filepath is a required argument\n");
        print_usage(argv);
        return 0;
    }

    if (newfile)
    {
        printf("main::NEWFILE\n");
        dbfd = create_db_file(filepath);
        if (dbfd == STATUS_ERROR)
        {
            printf("Unable to create database file\n");
            return -1;
        }

        if (create_db_header(&header) == STATUS_ERROR)
        {
            printf("Failed to create database header\n");
            return -1;
        }
    }
    else
    {
        printf("main::NOT NEWFILE\n");
        dbfd = open_db_file(filepath);
        if (dbfd == STATUS_ERROR)
        {
            printf("Unable to open database file\n");
            return -1;
        }
        printf("main::OPENED FD OK!\n");

        if (validate_db_header(dbfd, &header) == STATUS_ERROR)
        {
            printf("Failed to validate database header\n");
            return -1;
        }
        printf("main::VALIDATE DB HEADER OK!\n");
    }

    if (header == NULL)
    {
        printf("main::HEADER NULL!!!\n");
        return -1;
    }
    else
    {
        printf("main::HEADER NOT NULL!!!\n");
    }

    if (read_employees(dbfd, header, &employees) == STATUS_ERROR)
    {
        printf("main::error reading employees!\n");
        close(dbfd);
        return -1;
    }
    printf("main:: READ EMPLOYEES OK!\n");

    if (addstring)
    {
        printf("main::ADDSTRING!\n");
        if (header == NULL)
        {
            printf("main::inner HEADER NULL!!!\n");
            return -1;
        }
        else
        {
            printf("main::inner HEADER NOT NULL!!!\n");
        }

        header->count++;
        printf("main::HEADER COUNT: %d\n", header->count);

        employees = realloc(employees, (header->count * sizeof(struct employee_t)));
        if (employees == NULL)
        {
            printf("main::EMPLOYEES REALLOC FAILED!\n");
            return -1;
        }
        else
        {
            printf("main:: REALLOC OK!!!!!! %lu\n", header->count * sizeof(struct employee_t));
        }
        add_employee(header, employees, addstring);
    }
    if (output_file(dbfd, header, employees) == STATUS_ERROR)
    {
        close(dbfd);
        return -1;
    }
    close(dbfd);
    return 0;
}