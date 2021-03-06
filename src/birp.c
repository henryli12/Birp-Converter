/*
 * BIRP: Binary decision diagram Image RePresentation
 */

#include "image.h"
#include "bdd.h"
#include "const.h"
#include "debug.h"

int pgm_to_birp(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    int width;
    int height;
    int err = img_read_pgm(in, &width, &height, raster_data, RASTER_SIZE_MAX);
    if(err){
        return -1;
    }
    BDD_NODE *n = bdd_from_raster(width, height, raster_data);
    if(n){
        img_write_birp(n, width, height, out);
        return 0;
    }
    return -1;
}

int birp_to_pgm(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    int width;
    int height;
    BDD_NODE *n = img_read_birp(in, &width, &height);
    if(n){
        bdd_to_raster(n ,width, height, raster_data);
        img_write_pgm(raster_data, width, height, out);
        return 0;
    }
    return -1;
}

unsigned char bdd_map_helper(unsigned char c){
    return (global_options >> ((int)(c) * 4)) & 0xF;
}

int birp_to_birp(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    int width;
    int height;
    BDD_NODE *n = img_read_birp(in, &width, &height);
    // printf("return node %d %d %d", n->level, n->left, n->right);
    if(n){
        n = bdd_map(n, bdd_map_helper);
        int factor;
        // If zooming, adjust height and width
        if(((global_options >> 8) & 0xF) == 0x3){
            factor = global_options >> 16;
            if(factor > 16){
                factor -= 1;
                factor = factor ^ 0xFF;
                int multiple = 1 << factor;
                height = height/multiple + (height%multiple != 0);
                width = width/multiple + (width%multiple != 0);
                // height /= factor;
                // width >>= factor;
                height = height < 1 ? 1: height;
                width = width < 1? 1 : width; 
            }else{
                height <<= factor;
                width <<= factor;
            }
        }
        img_write_birp(n, width, height, out);
        return 0;
    }
    return -1;
}

int pgm_to_ascii(FILE *in, FILE *out) {
    int width;
    int height;
    if(!img_read_pgm(in, &width, &height, raster_data, RASTER_SIZE_MAX)){
        for(int i = 0; i < height; i++) {
            for(int j = 0; j < width; j++) {
                int value = *(raster_data+(i * width + j));
                if (value >= 0300){
                    fputc(64, out);
                }else if(value >= 0200 && value < 0300){
                    fputc(42, out);
                }else if(value >= 0100 && value < 0200){
                    fputc(46, out);
                }else if(value < 0100){
                    fputc(32, out);
                }
            }
            fputc(10, out);
        }
        return 0;
    }
    return -1;
}

int birp_to_ascii(FILE *in, FILE *out) {
    int width;
    int height;
    BDD_NODE *n = img_read_birp(in, &width, &height);
    if(n){
        for(int i = 0; i < height; i++) {
            for(int j = 0; j < width; j++) {
                int value = bdd_apply(n, i, j);
                if (value >= 0300){
                    fputc(64, out);
                }else if(value >= 0200 && value < 0300){
                    fputc(42, out);
                }else if(value >= 0100 && value < 0200){
                    fputc(46, out);
                }else if(value < 0100){
                    fputc(32, out);
                }
            }
            fputc(10, out);
        }
        return 0;
    }
    return -1;
}

// Check two strings are equal
int equal(char *x, char *y){
   while(*x == *y){
        if (*x == '\0' || *y == '\0'){
            break;
        }
        x++;
        y++;
    }
    if(*x == '\0' && *y == '\0'){
        return 1;
    }else{
        return 0;
    }
}

// Convert string to int
int str_to_int(char *x){
    int out = 0;
    int temp = 0;
    while(*x != '\0'){
        temp = (int)*x - 48;
        if (temp >= 0 && temp <= 9){
            out *= 10;
            out += temp;
        }else{
            return -1;
        }
        x++;
    }
    return out;
}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specifed will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere int the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */
int validargs(int argc, char **argv) {
    // TO BE IMPLEMENTED
    if(argc == 1){
        global_options = 0x00000022;
        return 0;
    }
    // Check for -h
    if(equal(*(argv+1), "-h")){
        global_options = 0x80000000;
        return 0;
    }
    // Initial settings
    global_options = 0x00000000;
    int haveInput = 0;
    int haveOutput = 0;
    int optionalFlag = 1;
    int i = 1;
    // Check for -i and -o
    // Make sure the given formats are valid
    // Update global options
    while (i < argc){
        char* flag = *(argv+i);
        if(equal(flag, "-i") && (i+1) < argc){
            if(haveInput){
                global_options = 0x00000000;
                return -1;
            }
            haveInput = 1;
            char* type = *(argv+i+1);
            if(equal(type, "pgm")){
                global_options = global_options + 0x00000001;
                optionalFlag = 0;
            }else if (equal(type, "birp")){
                global_options = global_options + 0x00000002;
            }else{
                global_options = 0x00000000;
                return -1;
            }
        }else if(equal(flag, "-o") && (i+1) < argc){
            if(haveOutput){
                global_options = 0x00000000;
                return -1;
            }
            haveOutput = 1;
            char* type = *(argv+i+1);
            if(equal(type, "pgm")){
                global_options = global_options + 0x00000010;
                optionalFlag = 0;
            }else if (equal(type, "birp")){
                global_options = global_options + 0x00000020;
            }else if (equal(type, "ascii")){
                global_options = global_options + 0x00000030;
                optionalFlag = 0;
            }else{
                global_options = 0x00000000;
                return -1;
            }
        }else{
            break;
        }
        i += 2;
    }
    // Setting default input/output if not provided
    if(!haveInput && !haveOutput){
        global_options = global_options + 0x00000022;
    }else if(!haveInput && haveOutput){
        global_options = global_options + 0x00000002;
    }else if(haveInput && !haveOutput){
        global_options = global_options + 0x00000020;
    }
    // Check for optional flag
    // Update global_options
    if (i < argc && optionalFlag){
        char* flag = *(argv+i);
        if(equal(flag, "-n")){
            global_options = global_options + 0x00000100;
            i++;
        }else if(equal(flag, "-r")){
            global_options = global_options + 0x00000400;
            i++;
        }else if(equal(flag, "-t") && (i+1) < argc){
            global_options = global_options + 0x00000200;
            int threshold = str_to_int(*(argv+i+1));
            if (threshold >= 0 && threshold <= 255){
                threshold = threshold  << 16;
                global_options = global_options + threshold;
                i += 2;
            }else{
                global_options = 0x00000000;
                return -1;
            }
        }else if(equal(flag, "-z") && (i+1) < argc){
            global_options = global_options + 0x00000300;
            int factor = str_to_int(*(argv+i+1));
            if (factor >= 0 && factor <= 16){
                factor = (factor ^ 0xFF) + 1;
                factor = factor << 16;
                global_options = global_options + factor;
                i += 2;
            }else{
                global_options = 0x00000000;
                return -1;
            }
        }else if(equal(flag, "-Z") && (i+1) < argc){
            global_options = global_options + 0x00000300;
            int factor = str_to_int(*(argv+i+1));
            if (factor >= 0 && factor <= 16){
                factor = factor << 16;
                global_options = global_options + factor;
                i += 2;
            }else{
                global_options = 0x00000000;
                return -1;
            }
        }else{
            global_options = 0x00000000;
            return -1;
        }
    }
    // If there are still flags remaining, args is invalid
    if(i < argc){
        global_options = 0x00000000;
        return -1;
    }
    return 0;
}