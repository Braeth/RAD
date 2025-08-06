#include "../includes/cli.h"
#include "../includes/ansi.h"
#include "../includes/rad_parser.h"
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

int * long_row( int last_line, char **lines, int size ) {
    size_t len;
    char *curr = malloc( 1024 * sizeof( char ) );
    if ( !curr ) { ErrorExit( mem_err_alloc_message ); }
    curr[ 0 ] = '\0';

    for ( int i = 0; i <= size; i++ ) {
        len = strlen( curr );
        if ( len == 0 ){
            strcpy( curr, lines[ i ] );
        }

        if( len < strlen( lines[ i ] + 1 ) ) {
            strcpy( curr, lines[ i ] );
        }

    }

    size_t total = strlen( curr );
    free( curr );
    return ( total + last_line + 2 );
}

void draw_lines ( int last_line, char **segment, int row_height ) {
    int len = long_row( last_line, segment, ( row_height ) );
    while ( len > 0 ) {
        printf("─");
        len--;
    }
    puts("");
}

void draw_lines_single ( int len ) {
    while ( len > 0 ) {
        printf("─");
        len--;
    }
    puts("");
}

void draw_lock( int len ) {
    for( int i = 0; i < len; i++) {
        printf("%s", INDENT);
    }
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

void draw_table( int width, int rows, char **valid_files ) {

    for (int i = 0; i < rows; i++ ) {
        draw_lines_single (  width + 15 );
        printf("| ..%s%s|", valid_files[ i ], indent( width - strlen( valid_files[ i ] ) ));
        printf(COLOR_GREEN "  VALID  ");
        printf(COLOR_RESET "|\n");
    }
    draw_lines_single (  width + 15 );
    printf("\n");
}

int count_space( const char *src ) {

    int total = 0, i = 0;

    while( src[ i ] == ' ') {
        total++; i++;
    }
    return total;
}

void remove_newline( char * line ) {
    size_t len = strlen( line );
    if( len > 0 && line[ len - 1 ] == '\n' )
        line[ len - 1 ] = '\0';
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
            if ( bin > 10 ) {
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
    fclose( file );
    return strlen( fn );

scan_line:
    rewind( file );
    int line_height = 2, index = 0, min_line = 3;
    bool has_root = true;
    char **upper_bound = malloc( line_height * sizeof ( char *) );
    char **lower_bound = malloc( line_height * sizeof ( char * ) );
    char *err_line = malloc( MAX_CHUNK_FILE * sizeof ( char ) );
    char **segment = malloc( ( line_height + line_height + 1 ) * sizeof ( char * ) );

    if ( !err_line || !segment ) { ErrorExit( mem_err_alloc_message ); }

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

    int row_height = line_height + line_height;
    for (int i  = 0; i <= row_height; i++) {
        if( i <= 1 ) {
            segment[ i ] = strdup( upper_bound[ i ] );
            continue;
        }
        if ( i == 4) {
            segment[ i ] = strdup( err_line );
            break;
        }
        segment[ i ] = strdup( lower_bound[ i - line_height ] );
    }

    if ( strlen ( fn ) >= 15 ) {
        fprintf(stderr, COLOR_RED "Error from <%s...%s>: %s\n", truncate_fn( fn, false, 10), truncate_fn( fn, true, 15), parser->problem);
    } else {
        fprintf(stderr, COLOR_RED "Error from <%s>: %s\n", truncate_fn( fn, true, strlen ( fn ) ), parser->problem);
    }
    fprintf(stderr, "Line: %d at column: %lu\n" COLOR_RESET , parser->problem_mark.line, parser->problem_mark.column + 1);

    if(!has_root) {
        printf("\n%d| %s", ( parser->problem_mark.line ), err_line);
    } else {

        draw_lines( last_line, segment, row_height );
        int total_indentation = long_row( last_line, segment, row_height );

        int j = 0;
        for( int i = line_height; i >= 1; i--) {
            do {
                remove_newline( upper_bound[j] );
                int digit = count_digit( parser->problem_mark.line - i );
                if ( last_line == digit ) {
                    printf("%d| %s", (parser->problem_mark.line - i), upper_bound[j]);
                } else if ( last_line == 2 && digit == 0 ) {
                    printf("%s%d| %s", indent( last_line - ( digit + 1 ) ), (parser->problem_mark.line - i), upper_bound[j]);
                } else {
                    printf("%s%d| %s", indent( last_line - digit ), (parser->problem_mark.line - i), upper_bound[j]);
                }
                draw_lock( total_indentation - strlen( upper_bound[j] ) - last_line - 2 );
                printf("|\n");
                break;
            } while ( j <= i );
            j++;
        }

        int problem_mark = count_digit( parser->problem_mark.line );
        remove_newline( err_line );
        if ( last_line == problem_mark ) {
            printf( COLOR_RED "%d| %s" , ( parser->problem_mark.line ), err_line);
        }  else if ( last_line == 2 && problem_mark == 0 ) {
            printf( COLOR_RED "%s%d| %s" , indent( last_line - ( problem_mark + 1 ) ), (parser->problem_mark.line), err_line);
        } else {
            printf( COLOR_RED "%s%d| %s", indent( last_line - problem_mark ), (parser->problem_mark.line), err_line);
        }
        draw_lock( total_indentation - strlen( err_line ) - last_line - 2 );
        printf("|\n" COLOR_RESET );

        int space = count_space( err_line );
        if ( last_line == 0 ) { printf( INDENT ); }
        printf("%s", indent( space + last_line + 2 ) );
        for (int i = 1; i < ( strlen( err_line ) - space ); i++) {
            printf( COLOR_RED "~");
        }
        printf("%s|\n" COLOR_RESET , indent ( total_indentation - strlen( err_line ) - last_line -1 ));

        j = 0;
        for( int i = 1; i <= line_height; i++) {
            do {
                remove_newline( lower_bound[j] );
                int digit = count_digit( parser->problem_mark.line + i );
                if ( last_line == digit ) {
                    printf("%d| %s", (parser->problem_mark.line + i), lower_bound[j]);
                } else if ( last_line == 2 && digit == 0 ) {
                    printf("%s%d| %s", indent( last_line - ( digit + 1) ), (parser->problem_mark.line + i), lower_bound[j]);
                } else {
                    printf("%s%d| %s", indent( last_line - digit ), (parser->problem_mark.line + i), lower_bound[j]);
                }
                draw_lock( total_indentation - strlen( lower_bound[j] ) - last_line - 2 );
                printf("|\n");
                break;
            } while ( j <= i );
            j++;
        }

    }

    draw_lines( last_line, segment, row_height );

    yaml_parser_delete(parser);

    memory_alloc_cleanup( line_height, upper_bound );
    memory_alloc_cleanup( line_height, lower_bound);
    memory_alloc_cleanup( row_height, segment );
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
    enable_virtual_processing();
    argument_p args;
    parse_arguments( argc, argv, &args );

    char **invalid_files = malloc(args.files_arg_count * sizeof(char *));
    char **valid_files = malloc(args.files_arg_count * sizeof(char *));
    int ctr = 0, valid_ctr = 0;

    if ( invalid_files == NULL || valid_files == NULL ) {
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
            int res = parse( file, &parser, args.files_arg_count, args.files[ i ] );
            yaml_parser_delete( &parser );

            if (res > 0) {
                if( strlen( args.files[ i ] ) > 15 ) {
                    valid_files[ valid_ctr ] = strdup( truncate_fn( args.files[ i ] , true, 15) );
                } else {
                    valid_files[ valid_ctr ] = strdup( args.files[ i ] );
                }
                valid_ctr++;
            }

            if ( res > 0 && args.dry_run ) {
                yaml_dry_run( args.colorized , args.files[ i ] );
            }



        } else {
            err_files_mem( ctr, args.files[ i ], invalid_files );
            ctr++;
        }

    }


    if (ctr > 0) {
        puts("");
        printf( COLOR_RED "Error: Found %d total invalid %s. Check the path and make sure it is a valid yaml file" COLOR_RESET , ctr, ((ctr > 1) ? "files" : "file"));
        printf("\n");
        for ( int i = 0; i < ctr; i++ ) {
            if ( strlen ( invalid_files[ i ] ) >= 10 ) {
                fprintf(stderr, ">> %s...%s\n",truncate_fn( invalid_files[ i ], false, 7), truncate_fn( invalid_files[ i ], true, 10));
            } else {
                fprintf(stderr, ">> %s\n", truncate_fn( invalid_files[ i ], true, strlen ( invalid_files[ i ] ) ));
            }

        }
    }
    puts("");

    if ( valid_ctr > 0 && !args.dry_run) {
        printf( COLOR_BLUE "INFO: " COLOR_RESET);
        printf("SUCCESS! %d %s %s VALID!\n", valid_ctr, ((valid_ctr > 1) ? "files": "file"), ((valid_ctr > 1) ? "are": "is") );


        int total_lines = long_row( 1, valid_files , valid_ctr - 1 );
        draw_table( total_lines, valid_ctr, valid_files );
    }

    memory_alloc_cleanup(ctr, invalid_files);
    memory_alloc_cleanup(valid_ctr, valid_files);

    return 0;
}