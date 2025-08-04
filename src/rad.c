#include "../includes/cli.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <yaml.h>

#define MAX_FN_LEN 256
#define FN_THRESHOLD 20
#define MAX_CHUNK_FILE 4096

#define INDENT " "
#define STRVAL(x) ((x) ? (char*)(x) : "")

const char *mem_err_alloc_message = "Error: This software needs enough memory space to work properly...";

void ErrorExit(const char *err_message) {
    puts( err_message );
    exit(1);
}

void memory_alloc_cleanup( int size, char **target ) {
    for(int i = 0; i < size; i++) {
        free( target[ i ] );
    }
    free( target );
}

int * count_digit( int level ) {

    int pad = 0;
    if ( level > 9 ) {
        while ( level > 0 ) {
            level /= 10;
            pad++;
        }
    }

    return pad;
}

char * indent( int pad ) {

    char *spaces = malloc( pad + 1 );
    spaces[ 0 ]  = '\0';
    if( !spaces ) { ErrorExit( mem_err_alloc_message ); }

    for ( int i = 0; i < pad; i++ ) {
        strcat( spaces, INDENT );
    }
    return spaces;

}


int count_space( const char *src ) {

    int total = 0, i = 0;

    while( src[ i ] == ' ') {
        total++; i++;
    }
    return total;
}

char * truncate_fn( char *fn, bool rtl, int len) {
    size_t total_char = strlen( fn );

    if ( !rtl ) {
    	if( len < total_char ) fn[len] = '\0';
    	return fn;
    }

    return ( (total_char >= len) ? ( fn + total_char - len ) : fn );

}

bool is_yml( const char *fn ) {
	size_t len = strlen( fn );
	return ( len >= 5 && ( strcmp( fn + len - 5, ".yaml" ) == 0 || strcmp( fn + len - 4, ".yml") == 0 ));
}

int is_readable( FILE *file ) {
    int chunk, ctr = 0, bin = 0;
    while ( (chunk = fgetc( file )) != EOF && ctr < 1024 ) {
        if( !(isspace( chunk ) || isprint( chunk ) ) ) {
            bin++;
            if ( bin > 20 ) {
                rewind( file );
                return 0;
            }
        }
        ctr++;
    }
    rewind( file );
    return 1;
}

bool is_file( const char *fn) {
    struct stat sb;
    return (stat ( fn, &sb ) == 0 && S_ISREG ( sb.st_mode ) );
}


int parse( FILE *file, yaml_parser_t *parser, int non_args_count, char *fn ) {

    yaml_event_t event;
    yaml_event_type_t event_type;

    char *chunk[ 1084 ];
    int currentline = 0;

    do {
        if ( !yaml_parser_parse( parser, &event ) ) {
            goto scan_line;
        }
        event_type = event.type;
        yaml_event_delete(&event);
    } while ( event_type != YAML_STREAM_END_EVENT );

    yaml_parser_delete(parser);
    puts("Success");
    fclose( file );
    exit(0);

scan_line:
    rewind( file );
    int line_height = 2, index = 0, min_line = 3;
    bool has_root = true;
    char **upper_bound = malloc( line_height * sizeof ( char *) );
    char **lower_bound = malloc( line_height * sizeof ( char * ) );
    char *err_line = malloc( MAX_CHUNK_FILE * sizeof ( char ) );

    if ( !err_line ) {
        ErrorExit( mem_err_alloc_message );
    }

    err_line[ 0 ] = '\0';

    for( int i =0; i < line_height; i ++ ) {
        upper_bound[ i ] = malloc( MAX_CHUNK_FILE );
        lower_bound[ i ] = malloc( MAX_CHUNK_FILE );

        if ( !upper_bound[ i ] ) {
            memory_alloc_cleanup( i, upper_bound );
            ErrorExit( mem_err_alloc_message );
        }

        if ( !lower_bound[ i ] ) {
            memory_alloc_cleanup( i, lower_bound );
            ErrorExit( mem_err_alloc_message );
        }

        upper_bound[ i] [ 0 ] = '\0';
        lower_bound[ i ][ 0 ] = '\0';
    }

    while( fgets( chunk , sizeof( chunk ), file ) ) {
        currentline++;

        if ( parser->problem_mark.line < min_line &&  currentline == parser->problem_mark.line) {
            has_root = false;
            strcat( err_line, chunk );
            break;
        }

        if ( currentline == ( parser->problem_mark.line + 3 ) ) break;

        if( currentline ==  parser->problem_mark.line ) {
            strcat( err_line, chunk );
            continue;
        }

        if( currentline == ( parser->problem_mark.line - 2 ) || currentline == ( parser->problem_mark.line - 1 ) ) {
            strcpy( upper_bound[ index % line_height ], chunk );
            index++;
            continue;
        }

        if( currentline == ( parser->problem_mark.line + 1 ) || currentline == ( parser->problem_mark.line + 2 ) ) {
            strcpy( lower_bound[ index % line_height ], chunk );
            index++;
            continue;
        }

    }

    int last_line = count_digit( parser->problem_mark.line + 2 );
    int head = count_digit( parser->problem_mark.line - 2 );

    puts("");

    if ( strlen ( fn ) >= 10 ) {
        fprintf(stderr, "Error from <%s...%s>: %s\n",truncate_fn( fn, false, 7), truncate_fn( fn, true, 10), parser->problem);
    } else {
        fprintf(stderr, "Error from <%s>: %s\n", truncate_fn( fn, true, strlen ( fn ) ), parser->problem);
    }
    fprintf(stderr, "Line: %d at column: %lu\n", parser->problem_mark.line, parser->problem_mark.column + 1);
    if ( non_args_count == 1 ) { puts(""); }
    if(!has_root) {
        printf("%d| %s", ( parser->problem_mark.line ), err_line);
    } else {
        if (non_args_count > 1 ) { puts("─────────────────────────────────────────────────"); }
        int j = 0;
        for( int i = line_height; i >= 1; i--) {
            do {
                int digit = count_digit( parser->problem_mark.line - i );
                if ( last_line == digit ) {
                    printf("%d| %s", (parser->problem_mark.line - i), upper_bound[j]);
                } else if ( last_line == 2 && digit == 0 ) {
                    printf("%s%d| %s", indent( last_line - ( digit + 1 ) ), (parser->problem_mark.line - i), upper_bound[j]);
                } else {
                    printf("%s%d| %s", indent( last_line - digit ), (parser->problem_mark.line - i), upper_bound[j]);

                }
                break;
            } while ( j <= i );
            j++;
        }

        int problem_mark = count_digit( parser->problem_mark.line );
        if ( last_line == problem_mark ) {
            printf("%d| %s", ( parser->problem_mark.line ), err_line);
        }  else if ( last_line == 2 && problem_mark == 0 ) {
            printf("%s%d| %s", indent( last_line - ( problem_mark + 1 ) ), (parser->problem_mark.line), err_line);
        } else {
            printf("%s%d| %s", indent( last_line - problem_mark ), (parser->problem_mark.line), err_line);
        }

        int space = count_space( err_line );
        if ( last_line == 0 ) { space += 1; }
        printf("%s", indent( space + last_line + 2 ) );
        for (int i = 1; i < ( strlen( err_line ) - space ); i++) {
            printf("~");
        }

        puts("");

        j = 0;
        for( int i = 1; i <= line_height; i++) {
            do {
                int digit = count_digit( parser->problem_mark.line + i );
                if ( last_line == digit ) {
                    printf("%d| %s", (parser->problem_mark.line + i), lower_bound[j]);
                } else if ( last_line == 2 && digit == 0 ) {
                    printf("%s%d| %s", indent( last_line - ( digit + 1) ), (parser->problem_mark.line + i), lower_bound[j]);
                } else {
                    printf("%s%d| %s", indent( last_line - digit ), (parser->problem_mark.line + i), lower_bound[j]);
                }
                break;
            } while ( j <= i );
            j++;
        }

    }

    if (non_args_count > 1 ) { puts("─────────────────────────────────────────────────"); }

    puts("");

    yaml_parser_delete(parser);

    memory_alloc_cleanup( line_height, upper_bound );
    memory_alloc_cleanup( line_height, lower_bound);
    free( err_line );
    fclose( file );

}

void err_files_mem( int ctr, char *file, char **invalid_files ) {

    invalid_files[ ctr ] = malloc( strlen( file ) + 1 );
    if( !invalid_files[ ctr ] ) {
        ErrorExit( mem_err_alloc_message );
    }
    invalid_files[ ctr ][ 0 ] = '\0';
    strcpy( invalid_files[ ctr ], file );
}


int
main( int argc, char *argv[ ] ) {

    argument_p args;
    parse_arguments( argc, argv, &args );

    char **invalid_files = malloc(args.files_arg_count * sizeof(char *));
    int ctr = 0;

    if ( invalid_files == NULL ) {
        ErrorExit( mem_err_alloc_message );
    }

    for( int i = 0; i < args.files_arg_count; i++ ) {

        if ( strlen( args.files [ i ] ) >= MAX_FN_LEN) {
            printf( "\nFilename is too long, might help if you use relative path instead!\n" );
            printf( "Skipping %s....(trunc) for now!\n", truncate_fn( args.files[ i ], false, 25 ) );
            continue;
        }

        if ( is_yml( args.files[ i ] ) && is_file( args.files[ i ]  )) {

            yaml_parser_t parser;
            yaml_parser_initialize(&parser);
            FILE *file = fopen(args.files[ i ], "r");

            if( !file || !is_readable( file ) ) {
                err_files_mem( ctr, args.files[ i ], invalid_files );
                ctr++;
                fclose( file );
                continue;
            }

            yaml_parser_set_input_file( &parser, file );
            parse( file, &parser, args.files_arg_count, args.files[ i ] );
            yaml_parser_delete( &parser );

        } else {
            err_files_mem( ctr, args.files[ i ], invalid_files );
            ctr++;
        }

    }

    puts("");
    if (ctr > 0) {
        printf("Error: Found %d total invalid %s. Check the path and make sure it is a valid yaml file\n", ctr, ((ctr > 1) ? "files" : "file"));
        for ( int i = 0; i < ctr; i++ ) {
            if ( strlen ( invalid_files[ i ] ) >= 10 ) {
                fprintf(stderr, ">> %s...%s\n",truncate_fn( invalid_files[ i ], false, 7), truncate_fn( invalid_files[ i ], true, 10));
            } else {
                fprintf(stderr, ">> %s\n", truncate_fn( invalid_files[ i ], true, strlen ( invalid_files[ i ] ) ));
            }

        }
    }
    puts("");
    memory_alloc_cleanup(ctr, invalid_files);

    return 0;
}