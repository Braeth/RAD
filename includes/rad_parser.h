#ifndef RAD_PARSER_H
#define RAD_PARSER_H

void yaml_dry_run( const char *event, const char *path );
void parse_keys( char *chunk, int start, int end );

#endif