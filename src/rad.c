#include "../includes/cli.h"
#include <stdlib.h>
#include <stdio.h>

int
main( int argc, char *argv[ ] ) {

    argument_p args;
    parse_arguments( argc, argv, &args );

    return 0;
}