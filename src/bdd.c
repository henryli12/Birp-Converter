#include <stdlib.h>
#include <stdio.h>

#include "bdd.h"
#include "debug.h"

/*
 * Macros that take a pointer to a BDD node and obtain pointers to its left
 * and right child nodes, taking into account the fact that a node N at level l
 * also implicitly represents nodes at levels l' > l whose left and right children
 * are equal (to N).
 *
 * You might find it useful to define macros to do other commonly occurring things;
 * such as converting between BDD node pointers and indices in the BDD node table.
 */
#define LEFT(np, l) ((l) > (np)->level ? (np) : bdd_nodes + (np)->left)
#define RIGHT(np, l) ((l) > (np)->level ? (np) : bdd_nodes + (np)->right)

// Keep track of highest index bdd node
int bdd_nodes_index = 255;

int bdd_min_level(int w, int h) {
    int min_level = 0;
    int n = w < h ? h : w;
    int perfect = 1;
    while(n > 1){
        if(n&1){
            perfect = 0;
        }
        min_level++;
        n >>= 1;
    }
    if(perfect){
        return min_level * 2;
    }else{
        return (min_level + 1) * 2;
    }
}

/**
 * Look up, in the node table, a BDD node having the specified level and children,
 * inserting a new node if a matching node does not already exist.
 * The returned value is the index of the existing node or of the newly inserted node.
 *
 * The function aborts if the arguments passed are out-of-bounds.
 */
int bdd_lookup(int level, int left, int right) {
    // TO BE IMPLEMENTED
    if(level < 0 || level > BDD_LEVELS_MAX || left < 0 || left > BDD_NODES_MAX || right < 0 || right > BDD_NODES_MAX){
        abort();
    }
    if(left == right){
        // printf("Same children: %d\n", left);
        return left;
    }
    int hash_key = (level + left + right) % BDD_NODES_MAX;
    // Look for node
    while (*(bdd_hash_map + hash_key)){
        BDD_NODE *n = *(bdd_hash_map + hash_key);
        if(n->level == level && n->left == left && n->right == right){
            int index = n - bdd_nodes;
            // printf("Found node: %d %d %d at index %d\n", n->level, n->left, n->right, index);
            return index;
        }
        hash_key = (hash_key + level) % BDD_NODES_MAX;
    }
    // Node doesn't exist, create new node
    BDD_NODE n;
    n.level = level;
    n.left = left;
    n.right = right;
    bdd_nodes_index ++;
    *(bdd_nodes + bdd_nodes_index) = n;
    *(bdd_hash_map + hash_key) = (bdd_nodes + bdd_nodes_index);
    // printf("Added node: %d %d %d at index %d\n", (bdd_nodes + bdd_nodes_index) -> level, (bdd_nodes + bdd_nodes_index) -> left, (bdd_nodes + bdd_nodes_index) -> right, bdd_nodes_index);
    return bdd_nodes_index;
}

int bdd_from_raster_helper(int r_low, int r_high, int c_low, int c_high, unsigned char *raster, int direction, int level, int width){
    // printf("%d %d %d %d %d\n", level, r_low, r_high, c_low, c_high);
    if(level == 0){
        int index = *(raster + (r_low * width + c_low));
        // printf("index: %d\n", index);
        return index;
    }
    if(r_low == r_high && c_low == c_high){
        int index = *(raster + (r_low * width + c_low));
        // printf("index: %d\n", index);
        return index;
    }
    int mid;
    int left, right;
    if(direction == 0){
        mid = r_low + (r_high - r_low)/2;
        left = bdd_from_raster_helper(r_low, mid, c_low, c_high, raster, (direction+1)%2, level-1, width);
        right = bdd_from_raster_helper(mid, r_high, c_low, c_high, raster, (direction+1)%2, level-1, width);
        // printf("level: %d left: %3d right: %3d %d %d %d %d %d %d\n", level, left, right, r_low, r_high, c_low, c_high, mid, direction);
        return bdd_lookup(level, left, right);
    }else if(direction == 1){
        mid = c_low + (c_high - c_low)/2;
        left = bdd_from_raster_helper(r_low, r_high, c_low, mid, raster, (direction+1)%2, level-1, width);
        right = bdd_from_raster_helper(r_low, r_high, mid, c_high, raster, (direction+1)%2, level-1, width);
        // printf("level: %d left: %3d right: %3d %d %d %d %d %d %d\n", level, left, right, r_low, r_high, c_low, c_high, mid, direction);
        return bdd_lookup(level, left, right);
    }
    return -1;
}

BDD_NODE *bdd_from_raster(int w, int h, unsigned char *raster) {
    // TO BE IMPLEMENTED
    int index = bdd_from_raster_helper(0, h, 0 , w, raster, 0, bdd_min_level(w, h), w);
    BDD_NODE *n = bdd_nodes + index;
    // printf("bdd_from_raster node: %d %d %d\n", n->level, n->left, n->right);
    if(n){
        return n;
    }
    return NULL;
}

void bdd_to_raster(BDD_NODE *node, int w, int h, unsigned char *raster) {
    // TO BE IMPLEMENTED
    unsigned char *dp = raster;
    for(int i = 0; i < h; i++) {
        for(int j = 0; j < w; j++) {
                *dp++ = bdd_apply(node, i, j);
        }
    }
}

void input_child_serial(int serial, FILE *out){
    int i = 0;
    while(i < 4){
        int c = serial & 0377;
        fputc(c, out);
        serial >>= 8;
        i += 1;
    }
}

int bdd_serialize_helper(BDD_NODE *node, int *serial, FILE *out, int *visited){
    int index = node - bdd_nodes;
    // Check visited
    if(*(visited+index) > 0){
        return 0;
    }
    // Check level 0
    if(node->level == 0){
        fprintf(out, "@%c", index);
        *(visited+index) = *serial;
        *serial += 1;
        return 0;
    }
    // Traverse left and right
    bdd_serialize_helper((bdd_nodes + node->left), serial, out, visited);
    bdd_serialize_helper((bdd_nodes + node->right), serial, out, visited);
    // Add current node
    char level = (char)(node->level + 64);
    int left = *(visited + node->left);
    int right = *(visited + node->right);
    fputc((node->level + 64), out);
    input_child_serial(left, out);
    input_child_serial(right, out);
    *(visited+index) = *serial;
    *serial += 1;
    return 0;
}

int bdd_serialize(BDD_NODE *node, FILE *out) {
    // TO BE IMPLEMENTED
    int serial = 1;
    int visited[BDD_NODES_MAX] = {0};
    bdd_serialize_helper(node, &serial, out, visited);
    return 0;
}

void get_serial(FILE *in, int *serial){
    int j = 0;
    int temp = 0;
    int c;
    while(j < 4){
        c = fgetc(in);
        c <<= (8*j);
        temp += c;
        j += 1;
    }
    *(serial) = temp;
}

BDD_NODE *bdd_deserialize(FILE *in) {
    int c;
    int indices[BDD_NODES_MAX] = {0};
    int serial = 1;
    int j;
    int level, left, right;
    while ((c = fgetc(in)) != EOF){
        if(c == 0100){
            c = fgetc(in);
            *(indices + serial) = c;
        }else if(c > 0100){
            level = c - 64;
            get_serial(in, &left);
            get_serial(in, &right);
            left = *(indices + left);
            right = *(indices + right);
            int index = bdd_lookup(level, left, right);
            *(indices + serial) = index;
        }
        serial += 1;
    }
    BDD_NODE *n = bdd_nodes + *(indices + serial - 1);
    return n;
}

unsigned char bdd_apply_helper(BDD_NODE *node, int r, int c){
    int level = node -> level;
    if(level == 0){
        return node - bdd_nodes;
    }
    int bit;
    int side;
    if(level % 2 == 1){
        bit = (level - 1) / 2;
        // printf("c:%d bit:%d ", c, bit);
        side = c >> bit;
        side = side & 1;
        // printf("side:%d\n", side);
        if(side){
            return bdd_apply_helper((bdd_nodes + node->right), r ,c);
        }else{
            return bdd_apply_helper((bdd_nodes + node->left), r, c);
        }
    }else{
        bit = (level - 2) / 2;
        // printf("r:%d bit:%d ", r, bit);
        side = r >> bit;
        side = side & 1;
        // printf("side:%d\n", side);
        if(side){
            return bdd_apply_helper((bdd_nodes + node->right), r ,c);
        }else{
            return bdd_apply_helper((bdd_nodes + node->left), r, c);
        }
    }
}

unsigned char bdd_apply(BDD_NODE *node, int r, int c) {
    // TO BE IMPLEMENTED
    return bdd_apply_helper(node, r, c);
}

int get_option_arg(unsigned char (*func)(unsigned char)){
    int arg = 0;
    for(int i = 0; i < 2; i++){
        arg += (func(i+4) << (i*4));
    }
    return arg;
}
int bdd_negate_helper(BDD_NODE *node){
    if(node->level == 0){
        return 255 - (node - bdd_nodes);
    }
    int left = bdd_negate_helper((bdd_nodes + node->left));
    int right = bdd_negate_helper((bdd_nodes + node->right));
    return bdd_lookup(node->level, left, right);
}

BDD_NODE *bdd_negate(BDD_NODE *node){
    int index = bdd_negate_helper(node);
    return bdd_nodes + index;
    // return node;
}

int bdd_filter_helper(BDD_NODE *node, int threshold){
    if(node->level == 0){
        return ((node - bdd_nodes) >= threshold) ? 255 : 0;
    }
    int left = bdd_filter_helper((bdd_nodes + node->left), threshold);
    int right = bdd_filter_helper((bdd_nodes + node->right), threshold);
    return bdd_lookup(node->level, left, right);
}

BDD_NODE *bdd_filter(BDD_NODE *node, int threshold){
    int index = bdd_filter_helper(node, threshold);
    // TO BE IMPLEMENTED
    return bdd_nodes + index;
}

BDD_NODE *bdd_map(BDD_NODE *node, unsigned char (*func)(unsigned char)) {
    // TO BE IMPLEMENTED
    int option = func(2);
    if(option == 0x0){
        return node;
    }else if(option == 0x1){
        return bdd_negate(node);
    }else if(option == 0x2){
        int threshold = get_option_arg(func);
        return bdd_filter(node, threshold);
    }else if(option == 0x3){
        int factor = get_option_arg(func);
        return bdd_zoom(node, node->level, factor);
    }else{
        return bdd_rotate(node, node->level);
    }
}

BDD_NODE *bdd_rotate(BDD_NODE *node, int level) {
    // TO BE IMPLEMENTED
    return node;
}

int bdd_zoom_in_helper(BDD_NODE *node, int level, int factor){
    level = node->level + 2*factor;
    if(node->level == 0){
        return (node - bdd_nodes);
    }
    int left = bdd_zoom_in_helper((bdd_nodes + node->left), level, factor);
    int right = bdd_zoom_in_helper((bdd_nodes + node->right), level, factor);
    return bdd_lookup(level, left, right);
}

int bdd_zoom_out_helper(BDD_NODE *node, int level, int factor){
    level = node->level - 2*factor;
    if(level <= 0){
        return (node - bdd_nodes) == 0 ? 0 : 255;
    }
    int left = bdd_zoom_out_helper((bdd_nodes + node->left), level, factor);
    int right = bdd_zoom_out_helper((bdd_nodes + node->right), level, factor);
    return bdd_lookup(level, left, right);
}

BDD_NODE *bdd_zoom(BDD_NODE *node, int level, int factor) {
    int index;
    if(factor > 16){
        factor -= 1;
        factor = factor ^ 0xFF;
        index = bdd_zoom_out_helper(node, level, factor);
    }else{
        index = bdd_zoom_in_helper(node, level, factor);
    }
    return bdd_nodes + index;
}