#ifndef CLI_H
#define CLI_H

typedef struct {
    int recursive;
    int files_arg_count;
    char **files;
} argument_p;

#define PROGRAM "rad"
#define VERSION "RAD 0.1.0"
#define DESC "Validate your Global Policy yaml files with EASE!"
#define CREATOR "Made by SQ1"

void parse_arguments(int argc, char **argv, argument_p *options);

#endif