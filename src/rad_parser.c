#include "../includes/cli.h"
#include "../includes/ansi.h"
#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>

#define BUFFER_SIZE 256
#define CHUNK_SIZE 1024

void parse_keys( char *chunk, int start, int end ) {

    memmove(chunk + end + strlen( (int) COLOR_RESET ), chunk + end, strlen( chunk + end ) + 1);
    memcpy(chunk + end , COLOR_RESET, strlen( (int) COLOR_RESET ));

    memmove(chunk + start + strlen( (int) COLOR_CYAN  ), chunk + start, strlen( chunk + start ) + 1);
    memcpy(chunk + start, COLOR_CYAN, strlen( (int) COLOR_CYAN  ));

}

void yaml_dry_run( const char *event, const char *path ) {
    FILE *file;
    char buffer[ BUFFER_SIZE ];
    char chunk[ CHUNK_SIZE ];
    char *pattern = "[A-Za-z0-9_.-]*:";
    char *line;

    regex_t regX;
    regmatch_t match[ BUFFER_SIZE ];
    int regX_silent_fail = 0;

    snprintf( buffer, sizeof( buffer ), "kubectl create cm policy --from-file=%s --dry-run=client -o yaml 2>&1", path );

    file = popen( buffer, "r" );
    if ( !file ) {
        fprintf(stderr, COLOR_RED "\n%s: failed to execute kubectl dry-run command" COLOR_RESET , PROGRAM);
        printf("\n");
        return;
    }

    if( regcomp( &regX, pattern, REG_ICASE | REG_EXTENDED ) != 0 ) {
        regX_silent_fail = 1;
        regfree( &regX );
    }

    puts("");
    while( fgets( chunk , sizeof( chunk ), file ) )  {

        if ( regX_silent_fail != 1 && event == 1 ) {
            if( !regexec( &regX, chunk, (size_t) BUFFER_SIZE , match, 0 ) ) {
                int len =  match[0].rm_eo - match[0].rm_so + 1;
                line = malloc( sizeof( chunk )  );
                snprintf( line, sizeof( chunk ), "%.*s", len, chunk + match[0].rm_so );
                parse_keys( chunk, match[0].rm_so, match[0].rm_eo );
            }
        }

        if( strstr( chunk, "not found" ) != NULL || strstr( chunk, "not recognized" ) != NULL ) {
            fprintf(stderr, COLOR_RED "%s: kubectl was not installed. Please configure your environment variable.\n" COLOR_RESET , PROGRAM);
            break;
        }
        printf("%s", chunk);

    }

    regfree( &regX );
    pclose( file );

}