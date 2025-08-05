#include "../includes/cli.h"
#include "../includes/ansi.h"
#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>

#define BUFFER_SIZE 256
#define CHUNK_SIZE 1024


void *yaml_dry_run( const char *event, const char *path ) {
    FILE *file;
    char buffer[ BUFFER_SIZE ];
    char chunk[ CHUNK_SIZE ];

    snprintf( buffer, sizeof( buffer ), "kubectl create cm policy --from-file=%s --dry-run=client -o yaml 2>&1", path );

    file = popen( buffer, "r" );
    if ( !file ) {
        fprintf(stderr, "\n%s: failed to execute dry-run command\n", PROGRAM);
        return;
    }
    puts("");
    while( fgets( chunk , sizeof( chunk ), file ) )  {
        printf("%s", chunk);
    }
    pclose( file );

}