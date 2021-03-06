#include <stdio.h>
#include <stdlib.h>

#include "const.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    if(validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);
    if(global_options & HELP_OPTION)
        USAGE(*argv, EXIT_SUCCESS);
    int input = global_options & 0xF;
    int output = (global_options >> 4) & 0xF;
    if ((input == 0x1) && (output == 0x2)){
        pgm_to_birp(stdin, stdout);
    }else if ((input == 0x1) && (output == 0x3)){
        pgm_to_ascii(stdin, stdout);
    }else if ((input == 0x2) && (output == 0x1)){
        birp_to_pgm(stdin, stdout);
    }else if ((input == 0x2) && (output == 0x2)){
        birp_to_birp(stdin, stdout);
    }else if ((input == 0x2) && (output == 0x3)){
        birp_to_ascii(stdin, stdout);
    }else{
        return EXIT_FAILURE;
    }
    // int index = 256;
    // printf("Printing bdd_nodes-------------------------------\n");
    // while ((bdd_nodes+index) -> level){
    //     BDD_NODE* n = bdd_nodes + index;
    //     printf("Index %d: %d %d %d\n",index, n->level, n->left, n->right);
    //     index += 1;
    // }
    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
