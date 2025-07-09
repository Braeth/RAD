#include "../includes/cli.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#ifdef __linux__
#include <getopt.h>
#else
#include <unistd.h>
#endif


void print_banner() {
    printf("\n"
        "  ██████╗  █████╗ ██████╗  \n"
        "  ██╔══██╗██╔══██╗██╔══██╗ \n"
        "  ██████╔╝███████║██║  ██║ \n"
        "  ██╔══██╗██╔══██║██║  ██║ \n"
        "  ██║  ██║██║  ██║██████╔╝ \n"
        "  ╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝  \n"
        "                           \n");
        printf("  >>>   %s   <<<\n", DESC);
        printf( "%s\nVer: %s\n", CREATOR, VERSION);
}

void print_help() {
    printf("\n"
           "Usage: %s YAML_FILE | %s [OPTIONS] YAML_FILE [YAML_FILE...]\n"
           "  General Options:\n"
           "    -h, --help                 Print this help text and exit\n"
           "    -v, --version              Software version\n"
           "    -r, --recursive            Validate multiple yaml files\n\n"
           "e.g: $%s <path-to>/policy.yaml\n", PROGRAM, PROGRAM, PROGRAM);
}

void parse_arguments(int argc, char **argv, argument_p *options) {

    memset( options, 0, sizeof( argument_p ) );
    opterr = 0;

    if ( argc == 1) {
        print_banner();
        exit(0);
    }

    int opt;
    int option_index = 0;

    static struct option long_options[] = {{"version", no_argument, 0, 'v'},
                                           {"help",    no_argument, 0, 'h'},
                                           {"recursive", no_argument, 0, 'r'},
                                           {0, 0, 0, 0}};

    while( (opt = getopt_long(argc, argv, "vhr", long_options, &option_index)) != -1 ) {

        switch( opt ) {
        case 'v':
            printf("%s\n", VERSION);
            exit(0);
        case 'h':
            print_help();
            exit(0);
        case 'r':
            options->recursive=1;
            break;
        default:
            printf("\nUsage: %s YAML_FILE | %s [OPTIONS] YAML_FILE [YAML_FILE...]\n\n", PROGRAM, PROGRAM);
            fprintf(stderr, "%s: error: no such option: %s\n", PROGRAM, argv[1]);
            exit(1);
        }

    }

    options->files_arg_count = argc - optind;

    if( options->files_arg_count == 0 ) {
        printf("\nUsage: %s YAML_FILE | %s [OPTIONS] YAML_FILE [YAML_FILE...]\n", PROGRAM, PROGRAM);
        fprintf(stderr, "\n%s: error: no yaml file specified\n", PROGRAM);
        exit(1);
    } else options->files = &argv[optind];

    if( options->files_arg_count > 1 && !options->recursive ) {
        printf("\nUsage: %s YAML_FILE | %s [OPTIONS] YAML_FILE [YAML_FILE...]\n\n", PROGRAM, PROGRAM);
        fprintf(stderr, "%s: error: Ambiguous option\n", PROGRAM);
        exit(1);
    }

}
