#include "../includes/cli.h"
#include "../includes/ansi.h"
#include "../includes/rad_parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 256
#define CHUNK_SIZE 1024


void move_keys( char *chunk, int start, int end ) {

    memmove(chunk + end + strlen( (int) COLOR_RESET ), chunk + end, strlen( chunk + end ) + 1);
    memcpy(chunk + end , COLOR_RESET, strlen( (int) COLOR_RESET ));

    memmove(chunk + start + strlen( (int) COLOR_CYAN  ), chunk + start, strlen( chunk + start ) + 1);
    memcpy(chunk + start, COLOR_CYAN, strlen( (int) COLOR_CYAN  ));

}

void parse_keys( char *chunk , rad_parse *parse ) {
    memset( parse, 0, sizeof( rad_parse ));
    while( chunk[ parse->start ] == ' ' || chunk[ parse->start ] == '\t' || chunk[ parse->start ] == '-' ) {
        parse->start++;
    }

    char *splitter  = strchr( chunk + parse->start, ':' );
    int len;
    if( splitter ) {
        len = splitter - (chunk + parse->start);
        strncpy( parse->chunk, chunk + parse->start, len);
        parse->chunk[ len ] = '\0';
        parse->end = parse->start + len;
    }

}

void yaml_dry_run( const char *event, const char *path ) {
    rad_parse parse;
    FILE *file;
    char buffer[ BUFFER_SIZE ];
    char chunk[ CHUNK_SIZE ];

    snprintf( buffer, sizeof( buffer ), "kubectl create cm policy --from-file=%s --dry-run=client -o yaml 2>&1", path );

    file = popen( buffer, "r" );
    if ( !file ) {
        fprintf(stderr, COLOR_RED "\n%s: failed to execute kubectl dry-run command" COLOR_RESET , PROGRAM);
        printf("\n");
        return;
    }

    puts("");
    while( fgets( chunk , sizeof( chunk ), file ) )  {

        if ( event == 1 ) {
            parse_keys( chunk, &parse );
            move_keys( chunk, parse.start, parse.end );
        }

        if( strstr( chunk, "not found" ) != NULL || strstr( chunk, "not recognized" ) != NULL ) {
            fprintf(stderr, COLOR_RED "%s: kubectl was not installed. Please configure your environment variable.\n" COLOR_RESET , PROGRAM);
            break;
        }
        printf("%s", chunk);

    }

    pclose( file );

}
