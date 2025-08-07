#ifndef RAD_PARSER_H
#define RAD_PARSER_H

#define CHUNK_SIZE 1024

typedef struct {
    char chunk[ CHUNK_SIZE ];
    int start;
    int end;
} rad_parse;

void yaml_dry_run( const char event, const char *path );

void parse_keys( char *chunk , rad_parse *parse );

void move_keys( char *chunk, int start, int end );

#endif