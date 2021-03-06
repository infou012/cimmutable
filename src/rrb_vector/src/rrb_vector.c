#include "rrb_vector.h"

#define DEBUG 0

/* Functions used before defintions. */
rrb_t* add_leaf(rrb_t* rrb, imc_data_t* data, int where);
rrb_t* add_node(rrb_t* rrb, imc_data_t* data, bool meta);
rrb_t* update_leaf(rrb_t* rrb, int  where, imc_data_t* data);
rrb_t* update_node(rrb_t* rrb, int* index, imc_data_t* data, bool meta);
rrb_t* pop_node(rrb_t* rrb, imc_data_t** data, int* index, bool meta);
rrb_t* pop_data(rrb_t* rrb, imc_data_t** data, int* index, bool meta);
int split_leaf(const rrb_t* rrb, rrb_t** left, rrb_t** right, int* index, bool meta);
int split_node(const rrb_t* rrb, rrb_t** left, rrb_t** right, int* index, bool meta);
void consolidate_tree(rrb_t** rrb, bool top);

/** Creates an empty array of nodes into rrb. */
void make_nodes(rrb_t* rrb) {
    debug_print("make_nodes, beginning\n");
    rrb->nodes.child = calloc(32, sizeof *rrb->nodes.child);
    debug_print("make_nodes, end\n");
}

/** Creates an empty array of leafs into rrb. */
void make_datas(rrb_t* rrb) {
    debug_print("make_datas, beginning\n");
    rrb->nodes.leaf = calloc(32, sizeof *rrb->nodes.leaf);
    debug_print("make_datas, end\n");
}

int* make_meta() {
    return calloc(32, sizeof (int* ));
}

/** Creates an empty RRB-Tree. Top is used in order to determine if it contains
* leafs or RRB-Nodes. */
rrb_t* create(bool top) {
    debug_print("create, beginning\n");
    rrb_t* rrb = malloc(sizeof *rrb);
    rrb->ref = 1;
    rrb->level = 1;
    rrb->full = false;
    rrb->meta = NULL;
    rrb->elements = 0;
    if (top == true) {
        make_nodes(rrb);
    } else {
        make_datas(rrb);
    }
    debug_print("create, end\n");
    return rrb;
}

/** Creates a RRB-Tree with datas. */
rrb_t* create_w_leafs() {
    debug_print("create_w_leafs\n");
    return create(false);
}

/** Creates a RRB-Tree with nodes. */
rrb_t* create_w_nodes() {
    debug_print("create_w_nodes\n");
    return create(true);
}

/** Checks if rrb contains data. */
bool contains_leafs(const rrb_t* rrb) {
    debug_print("contains_leafs\n");
    return rrb->level == 1;
}

/** Checks if rrb contains nodes. */
bool contains_nodes(const rrb_t* rrb) {
    debug_print("contains_nodes\n");
    return rrb->level > 1;
}

/** Creates an empty RRB-Vector. */
rrb_t* rrb_create() {
    debug_print("rrb_create\n");
    return create_w_leafs();
}

/** Frees a RRB-Tree. */
void free_rrb(rrb_t* rrb) {
    debug_print("free_rrb, beginning\n");
    if (rrb->meta != NULL) {
        free(rrb->meta);
    }
    if (contains_leafs(rrb)) {
        free(rrb->nodes.leaf);
    } else {
        free(rrb->nodes.child);
    }
    free(rrb);
    debug_print("free_rrb, end\n");
}

/** Increases references to the tree. */
rrb_t* inc_ref(rrb_t* rrb) {
    debug_print("inc_ref, beginning\n");
    rrb->ref += 1;
    debug_print("inc_ref, end\n");
    return rrb;
}

/** Decreases references to the tree. */
void dec_ref(rrb_t* rrb) {
    debug_print("dec_ref, beginning\n");
    rrb->ref -= 1;
    if (rrb->ref == 0) {
        if (contains_nodes(rrb)) {
            for (int i = 0; i < 32; i++) {
                if (rrb->nodes.child[i] != NULL) {
                    dec_ref(rrb->nodes.child[i]);
                }
            }
        }
        free_rrb(rrb);
    }
    debug_print("dec_ref, end\n");
}

/** Checks if a RRB-Tree is full. */
bool is_full(const rrb_t* rrb) {
    debug_print("is_full\n");
    return rrb->full == true;
}

/** Easily clone the informations. */
void clone_info(rrb_t* clone, const rrb_t* src) {
    debug_print("clone_info, beginning\n");
    clone->full = src->full;
    clone->level = src->level;
    clone->elements = src->elements;
    debug_print("clone_info, end\n");
}

/** Easily clone the meta section. */
void clone_meta(rrb_t* clone, const rrb_t* src) {
    debug_print("clone_meta, beginning\n");
    if (src->meta != NULL) {
        clone->meta = malloc(sizeof *clone->meta * 32);
        for (int i = 0; i < 32; i++) {
            clone->meta[i] = src->meta[i];
        }
    } else {
        clone->meta = NULL;
    }
    debug_print("clone_meta, end\n");
}

/** Easily clone the nodes section. */
void clone_nodes(rrb_t* clone, const rrb_t* src) {
    debug_print("clone_nodes, beginning\n");
    if (contains_leafs(src)) {
        clone->nodes.leaf = malloc(sizeof *clone->nodes.leaf * 32);
        for (int i = 0; i < 32; i++) {
            clone->nodes.leaf[i] = src->nodes.leaf[i];
        }
    } else {
        clone->nodes.child = malloc(sizeof *clone->nodes.child * 32);
        for (int i = 0; i < 32; i++) {
            clone->nodes.child[i] = src->nodes.child[i];
            if (clone->nodes.child[i] != NULL) {
                inc_ref(clone->nodes.child[i]);
            }
        }
    }
    debug_print("clone_nodes, end\n");
}

/** Copies the node as is. */
rrb_t* copy_node(const rrb_t* src) {
    debug_print("copy_node, beginning\n");
    rrb_t* clone = malloc(sizeof *clone);
    clone->ref = 1;
    clone_info(clone, src);
    clone_meta(clone, src);
    clone_nodes(clone, src);
    debug_print("copy_node, end\n");
    return clone;
}

/** Inserts the data at the desired level. */
rrb_t* create_child(int level, imc_data_t* data) {
    debug_print("create_level, beginning\n");
    if (level == 1) {
        rrb_t* rrb = create_w_leafs();
        rrb->nodes.leaf[0] = data;
        rrb->elements = 1;
        rrb->level = level;
        debug_print("create_level, end\n");
        return rrb;
    } else {
        rrb_t* rrb = create_w_nodes();
        rrb->nodes.child[0] = create_child(level - 1, data);
        rrb->elements = 1;
        rrb->level = level;
        debug_print("create_level, end\n");
        return rrb;
    }
}

/** Adds a node, parent of the tree, and insert
  * the data at the correct level. */
rrb_t* add_as_parent_to(rrb_t* child, imc_data_t* data) {
    debug_print("add_as_parent_to, beginning\n");
    rrb_t* parent = create_w_nodes();
    parent->level = child->level + 1;
    parent->nodes.child[0] = inc_ref(child);
    parent->nodes.child[1] = create_child(child->level, data);
    parent->elements = rrb_size(child) + 1;
    if (contains_nodes(child) && child->meta != NULL) {
        parent->meta = malloc(sizeof *parent->meta * 32);
        parent->meta[0] = rrb_size(child);
        parent->meta[1] = rrb_size(child) + 1;
    }
    debug_print("add_as_parent_to, end\n");
    return parent;
}

/** Calc the position according to the formula of research paper. */
int calc_position(int index, int level) {
    debug_print("calc_position\n");
    return (index >> (5 * (level - 1))) & 31;
}

/** Finds the correct place to insert the new data. */
int place_to_insert(const rrb_t* rrb, bool meta) {
    debug_print("place_to_insert, beginning\n");
    if (rrb->meta == NULL || meta == false) {
        return calc_position(rrb->elements, rrb->level);
    }

    for (int i = 0; i < 32; i++) {
        if (contains_nodes(rrb)) {
            if (rrb->nodes.child[i] == NULL || !is_full(rrb->nodes.child[i])) {
                debug_print("place_to_insert, leafs\n");
                return i;
            }
        } else {
            if (rrb->nodes.leaf[i] == NULL) {
                debug_print("place_to_insert, node\n");
                return i;
            }
        }
    }

    debug_print("place_to_insert, default case\n");
    return -1;
}

/** Adjusts the index in case of existing meta. */
int check_meta_index(const rrb_t* rrb, int* index) {
    debug_print("check_meta_index, beginning\n");
    debug_args("check_meta_index, index: %d\n", *index);
    for (int i = 0; i < 32; i++) {
        if (contains_nodes(rrb)) {
            if ((unsigned int) (*index) < rrb_size(rrb->nodes.child[i])) {
                return i;
            } else {
                *index -= rrb_size(rrb->nodes.child[i]);
            }
        } else {
            if (*index == 0) {
                return i;
            } else {
                *index -= 1;
            }
        }
    }
    return -1;
}

/** Gets the index needed to look at the right level. */
int place_to_look(const rrb_t* rrb, int* index, bool meta) {
    debug_print("place_to_look, beginning\n");
    if (rrb->meta != NULL || meta == true) {
        debug_print("place_to_look, check_meta_index\n");
        return check_meta_index(rrb, index);
    } else {
        debug_print("place_to_look, meta null\n");
        return calc_position(*index, rrb->level);
    }
}

/** Copies if the node exists, else creates it at the correct level. */
rrb_t* create_clone(const rrb_t* src, int level) {
    debug_print("create_clone, beginning\n");
    rrb_t* clone;
    if (src == NULL) {
        if (level == 1) {
            clone = create_w_leafs();
        } else {
            clone = create_w_nodes();
        }
        clone->level = level;
    } else {
        clone = copy_node(src);
    }
    debug_print("create_clone, end\n");
    return clone;
}

/** Adds a node to a non fully tree. */
rrb_t* add(rrb_t* clone, imc_data_t* data, bool meta) {
    debug_print("add, beginning\n");
    if (contains_leafs(clone)) {
        debug_print("add, leafs\n");
        return add_leaf(clone, data, place_to_insert(clone, meta));
    } else {
        debug_print("add, node\n");
        return add_node(clone, data, meta);
    }
}

/** Easily add a data to a tree leafs. */
rrb_t* add_leaf(rrb_t* rrb, imc_data_t* data, int where) {
    debug_print("add_leaf, beginnning\n");
    rrb->nodes.leaf[where] = data;
    if (where == 31) {
        rrb->full = true;
    }
    rrb->elements += 1;
    debug_print("add_leaf, end\n");
    return rrb;
}

/** Easily adds a data to a tree node. */
rrb_t* add_node(rrb_t* rrb, imc_data_t* data, bool meta) {
    debug_print("add_node, beginning\n");
    int where = place_to_insert(rrb, meta);
    rrb_t* clone = create_clone(rrb->nodes.child[where], rrb->level - 1);
    rrb_t* child = rrb->nodes.child[where];
    rrb->nodes.child[where] = add(clone, data, meta);
    if (child != NULL) {
        dec_ref(child);
    }
    if (where == 31 && is_full(rrb->nodes.child[where])) {
        rrb->full = true;
    }
    rrb->elements += 1;
    debug_print("add_node, end\n");
    return rrb;
}

/** Adds a data to the tree, and returns a new version of the tree. */
rrb_t* rrb_push(rrb_t* rrb, imc_data_t* data) {
    debug_print("rrb_push, beginning\n");
    if (is_full(rrb)) {
        debug_print("rrb_push, full\n");
        return add_as_parent_to(rrb, data);
    } else {
        debug_print("rrb_push, not full\n");
        rrb_t* clone = create_clone(rrb, rrb->level);
        if (rrb->meta == NULL) {
            return add(clone, data, false);
        } else {
            return add(clone, data, true);
        }
    }
}

/** Gets the size of rrb. */
size_t rrb_size(const rrb_t* rrb) {
    debug_print("rrb_size, beginning\n");
    if (rrb == NULL) {
        debug_print("rrb_size, null\n");
        return -1;
    } else {
        debug_print("rrb_size, not null\n");
        return rrb->elements;
    }
}

/** Looks for a data into the tree. */
imc_data_t* lookup(const rrb_t* rrb, int* index, bool meta) {
    debug_print("lookup, beginning\n");
    int position = place_to_look(rrb, index, meta);
    debug_args("lookup, position: %d\n", position);
    if (contains_nodes(rrb)) {
        debug_print("lookup, nodes\n");
        return lookup(rrb->nodes.child[position], index, meta);
    } else {
        debug_print("lookup, leafs\n");
        return rrb->nodes.leaf[position];
    }

}

/** Checks if the index is correct then looks for a data into the tree. */
imc_data_t* rrb_lookup(const rrb_t* rrb, int index) {
    debug_print("rrb_lookup, beginning\n");
    if ((size_t) index >= rrb_size(rrb)) {
        debug_print("rrb_lookup, no index\n");
        return NULL;
    } else {
        debug_print("rrb_lookup, lookup\n");
        if (rrb->meta != NULL) {
            return lookup(rrb, &index, true);
        } else {
            return lookup(rrb, &index, false);
        }
    }
}

/** Unref rrb and free it automatically if needed. */
void rrb_unref(rrb_t* rrb) {
    debug_print("rrb_unref, beginning\n");
    dec_ref(rrb);
    debug_print("rrb_unref, end\n");
}

/** Updates the tree by changing the data at index by data. */
rrb_t* update(const rrb_t* rrb, int* index, imc_data_t* data, bool meta) {
    debug_print("update, beginning\n");
    rrb_t* clone = create_clone(rrb, rrb->level);
    if (contains_leafs(clone)) {
        debug_print("update, leafs\n");
        return update_leaf(clone, place_to_look(clone, index, meta), data);
    } else {
        debug_print("update, node\n");
        return update_node(clone, index, data, meta);
    }
}

/** Easily updates a data in a tree leaf. */
rrb_t* update_leaf(rrb_t* rrb, int where, imc_data_t* data) {
    debug_print("update_leaf, beginnning\n");
    rrb->nodes.leaf[where] = data;
    debug_print("add_leaf, end\n");
    return rrb;
}

/** Easily updates a data in a tree node. */
rrb_t* update_node(rrb_t* rrb, int* index, imc_data_t* data, bool meta) {
    debug_print("update_node, beginning\n");
    int where = place_to_look(rrb, index, meta);
    dec_ref(rrb->nodes.child[where]);
    rrb->nodes.child[where] = update(rrb->nodes.child[where], index, data, meta);
    debug_print("update_node, end\n");
    return rrb;
}

/** Checks if index is inside the tree, and update the data at index by data. */
rrb_t* rrb_update(const rrb_t* rrb, int index, imc_data_t* data) {
    debug_print("rrb_update, beginning\n");
    if ((size_t) index >= rrb_size(rrb)) {
        debug_print("rrb_lookup, no index\n");
        return NULL;
    } else {
        debug_print("rrb_update, update\n");
        if (rrb->meta != NULL) {
            return update(rrb, &index, data, true);
        } else {
            return update(rrb, &index, data, false);
        }
    }
}

/** Pop the last element for the tree, and put the data into data. */
rrb_t* pop(const rrb_t* rrb, imc_data_t** data, int* index, bool meta) {
    debug_print("pop, beginning\n");
    rrb_t* clone = create_clone(rrb, rrb->level);
    if (contains_nodes(rrb)) {
        rrb_t* root = pop_node(clone, data, index, meta);
        if (root->nodes.child[1] == NULL) {
            rrb_t* child = inc_ref(root->nodes.child[0]);
            dec_ref(root);
            return child;
        } else {
            return root;
        }
    } else {
        return pop_data(clone, data, index, meta);
    }
}

/** Convenient way to pop data from leafs. */
rrb_t* pop_data(rrb_t* rrb, imc_data_t** data, int* index, bool meta) {
    int position = place_to_look(rrb, index, meta);
    *data = rrb->nodes.leaf[position];
    rrb->nodes.leaf[position] = NULL;
    rrb->elements -= 1;
    if (rrb_size(rrb) == 0) {
        dec_ref(rrb);
        return NULL;
    } else {
        return rrb;
    }
}

/** Convenient way to pop data from nodes. */
rrb_t* pop_node(rrb_t* rrb, imc_data_t** data, int* index, bool meta) {
    int position = place_to_look(rrb, index, meta);
    rrb_t* child = pop(rrb->nodes.child[position], data, index, meta);
    dec_ref(rrb->nodes.child[position]);
    if (rrb_size(child) == 0) {
        dec_ref(child);
        rrb->nodes.child[position] = NULL;
    } else {
        rrb->nodes.child[position] = child;
    }
    rrb->elements -= 1;
    return rrb;
}

/** Checks if the meta is NULL and pop the last data from the tree. */
rrb_t* rrb_pop(rrb_t* rrb, imc_data_t** data) {
    debug_print("rrb_pop, beginning\n");
    int index = rrb_size(rrb) - 1;
    // Check if there's at least an element in the tree.
    if (index == -1) {
        return NULL;
    }
    if (rrb->meta == NULL) {
        debug_print("rrb_pop, meta null\n");
        return pop(rrb, data, &index, false);
    } else {
        debug_print("rrb_pop, meta\n");
        return pop(rrb, data, &index, true);
    }
}

/** Inits both left and right trees, according to the RRB-Tree provided. */
void init_left_right(const rrb_t* rrb, rrb_t** left, rrb_t** right, bool leafs) {
    if (leafs == false) {
        *left  = create_w_nodes();
        *right = create_w_nodes();
        (*left )->level = rrb->level;
        (*right)->level = rrb->level;
        (*left )->elements += 1;
        (*right)->elements += 1;
    } else {
        *left  = create_w_leafs();
        *right = create_w_leafs();
    }
}

/** Calculus the maximum size of a tree at its level. */
size_t calc_size(int level) {
    return 32 << (5 * (level - 2));
}

/** Finds if the tree needs a meta section or not. */
bool find_if_meta(const rrb_t* rrb) {
    if (rrb_size(rrb) == 0) {
        return false;
    }

    for (int i = 1; i < 32; i++) {
        if (rrb->nodes.child[i] == NULL) {
            return false;
        } else if (rrb_size(rrb->nodes.child[i - 1]) < calc_size(rrb->level - 2)) {
            return true;
        }
    }
    return false;
}

/** Checks and set the size correctly. */
void set_proper_size(rrb_t* rrb) {
    for (int i = 0; i < 32; i ++) {
        consolidate_tree(&(rrb->nodes.child[i]), false);
        if (rrb->nodes.child[i] != NULL) {
            rrb->elements += rrb_size(rrb->nodes.child[i]);
        }
    }
}

/** Checks and creates meta if needed. */
void set_proper_meta(rrb_t* rrb) {
    if (find_if_meta(rrb) == true) {
        rrb->meta = make_meta();
        rrb->meta[0] = rrb_size(rrb->nodes.child[0]);
        for (int i = 1; i < 32; i ++) {
            if (rrb->nodes.child[i] != NULL) {
                rrb->meta[i] = rrb_size(rrb->nodes.child[i]) + rrb->meta[i - 1];
            }
        }
    }
}

/** Takes a tree just splitted and not well defined, and define everything as
  * it should be, i.e., setting proper sizes and creating meta if needed. */
void consolidate_tree(rrb_t** rrb, bool top) {
    if (*rrb != NULL && contains_nodes(*rrb)) {
        if (rrb_size(*rrb) == 1 && top == true) {
            rrb_t* temp = inc_ref((*rrb)->nodes.child[0]);
            dec_ref(*rrb);
            *rrb = temp;
            return consolidate_tree(rrb, true);
        }

        (*rrb)->elements = 0;
        if (contains_nodes(*rrb)) {
            set_proper_size(*rrb);
            set_proper_meta(*rrb);
        }
    }
}

/** Shortcut, and nicer to read in rrb_split. */
void consolidate_trees(rrb_t** left, rrb_t** right) {
    consolidate_tree(left, true);
    consolidate_tree(right, true);
}

/** Splits the tree. If the tree is not relaxed, left will not be relaxed.
  * right will always be relaxed, due to the nature of the split.
  * Rearranging the tree to make it balanced is possible, but really costly,
  * as it involves a lot of memory moves from different nodes and leaf in the
  * tree. It could be a possible improvement to provide different versions:
  * split and split_balanced, which splits the tree, then do a balancing. */
int split(const rrb_t* rrb, rrb_t** left, rrb_t** right,
    int* index, bool meta) {
    if (contains_leafs(rrb)) {
        return split_leaf(rrb, left, right, index, meta);
    } else {
        return split_node(rrb, left, right, index, meta);
    }
}

/** Convenient way to split a leaf. */
int split_leaf(const rrb_t* rrb, rrb_t** left, rrb_t** right,
    int* index, bool meta) {
    int where = place_to_look(rrb, index, meta);
    init_left_right(rrb, left, right, true);
    for (int i = 0; i < where; i++) {
        (*left)->nodes.leaf[i] = rrb->nodes.leaf[i];
        (*left)->elements += 1;
    }
    for (int i = where, j = 0; i < 32; i++, j++) {
        (*right)->nodes.leaf[j] = rrb->nodes.leaf[i];
        (*right)->elements += 1;
    }
    return 1;
}

/** Convenient way to split a node. */
int split_node(const rrb_t* rrb, rrb_t** left, rrb_t** right,
    int* index, bool meta) {
    int where = place_to_look(rrb, index, meta);
    init_left_right(rrb, left, right, false);
    for (int i = 0; i < where; i++) {
        (*left)->nodes.child[i] = rrb->nodes.child[i];
        inc_ref(rrb->nodes.child[i]);
        (*left)->elements += 1;
    }
    for (int i = where + 1, j = 1; i < 32; i++, j++) {
        (*right)->nodes.child[j] = rrb->nodes.child[i];
        if (rrb->nodes.child[i] != NULL) {
            inc_ref(rrb->nodes.child[i]);
            (*right)->elements += 1;
        }
    }
    return split(rrb->nodes.child[where], &((*left)->nodes.child[where]),
        &((*right)->nodes.child[0]), index, meta);
}

/** Splits the RRB-Tree into two trees and stores both parts into left and
  * right. Provides the index to indicates the cut. Element pointed by the index
  * will be contained in right resulting tree. */
int rrb_split(const rrb_t* rrb, rrb_t** left, rrb_t** right, int index) {
    debug_print("rrb_split, beginning\n");
    int value = 0;
    if ((size_t) ++index > rrb_size(rrb)) {
        *left  = NULL;
        *right = NULL;
        return value;
    }

    if (rrb->meta == NULL) {
        debug_print("rrb_split, meta null\n");
        value = split(rrb, left, right, &index, false);
    } else {
        debug_print("rrb_split, meta\n");
        value = split(rrb, left, right, &index, true);
    }
    consolidate_trees(left, right);
    return value;
}

/** Convenient way to init merge leaves. */
rrb_t* init_merge_leaves(rrb_t* left, rrb_t* right) {
    rrb_t* parent = create_w_nodes();
    parent->level = 2;
    parent->nodes.child[0] = left;
    parent->nodes.child[1] = right;
    parent->elements = rrb_size(left) + rrb_size(right);
    return parent;
}

/** Finds last index used in the nodes array. */
int find_last_index(const rrb_t* rrb) {
    for (int i = 0; i < 32; i++) {
        if (rrb->nodes.child[i] == NULL) {
            return i - 1;
        }
    }
    return 31;
}

/** Balances a data tree. */
void move_datas(rrb_t* left, rrb_t* right) {
    int i, j; size_t size = rrb_size(right);
    for (i = rrb_size(left), j = 0; i < 32 && (size_t) j < size; i++, j++) {
        left->nodes.leaf[i] = right->nodes.leaf[j];
        left->elements  += 1;
        right->elements -= 1;
    }

    for (i = 0; j < 32; i++, j++) {
        right->nodes.leaf[i] = right->nodes.leaf[j];
        right->nodes.leaf[j] = NULL;
    }
}

/** Merges leaves nodes. */
rrb_t* merge_leaves(rrb_t* left, rrb_t* right) {
    rrb_t* parent = init_merge_leaves(left, right);
    rrb_t* n_left  = parent->nodes.child[0];
    rrb_t* n_right = parent->nodes.child[1];
    if (!is_full(n_left)) {
        move_datas(n_left, n_right);
        if (n_right->elements == 0) {
            dec_ref(n_right);
            parent->nodes.child[1] = NULL;
        }
    }
    return parent;
}

/** Deletes first elem from right, and last elem in left. */
void delete_first_n_last(rrb_t* left, rrb_t* right, int last) {
    dec_ref(left->nodes.child[last]);
    dec_ref(right->nodes.child[0]);
    left->nodes.child[last] = NULL;
    for (int i = 0; i < 31; i++) {
        right->nodes.child[i] = right->nodes.child[i + 1];
    }
    right->nodes.child[31] = NULL;
}

/** Balances a child tree. */
void move_nodes(rrb_t* left, rrb_t* right) {
    int i, j; size_t size = rrb_size(right);
    for (i = rrb_size(left), j = 0; i < 32 && (size_t) j < size; i++, j++) {
        left->nodes.child[i] = right->nodes.child[j];
        left->elements  += rrb_size(left->nodes.child[i]);
        right->elements -= rrb_size(left->nodes.child[i]);
    }

    for (i = 0; j < 32; i++, j++) {
        right->nodes.child[i] = right->nodes.child[j];
        right->nodes.child[j] = NULL;
    }
}

/** Merges three RRB-Tree and balances the result. */
rrb_t* merge_balance(rrb_t* left, rrb_t* merged, rrb_t* right) {
    int last = find_last_index(left);
    if (last < 31) {
        move_nodes(left, merged);
    }
    move_nodes(merged, right);
    rrb_t* parent = create_w_nodes();
    parent->level = left->level + 1;
    parent->nodes.child[0] = left;
    parent->nodes.child[1] = merged;
    if (right->nodes.child[0] != NULL) {
        parent->nodes.child[2] = right;
        parent->elements = rrb_size(left) + rrb_size(merged) + rrb_size(right);
    } else {
        dec_ref(right);
        parent->elements = rrb_size(left) + rrb_size(merged);
    }
    return parent;
}

/** Merges two RRB-Tree into one. */
rrb_t* merge(const rrb_t* left, const rrb_t* right) {
    rrb_t* n_left  = copy_node(left);
    rrb_t* n_right = copy_node(right);
    if (contains_leafs(n_left)) {
        return merge_leaves(n_left, n_right);
    } else {
        int last = find_last_index(n_left);
        rrb_t* merged = merge(n_left->nodes.child[last], n_right->nodes.child[0]);
        delete_first_n_last(n_left, n_right, last);
        return merge_balance(n_left, merged, n_right);
    }
}

/** Creates a parent for a node. */
rrb_t* create_parent(const rrb_t* child) {
    rrb_t* n_child = copy_node(child);
    rrb_t* parent = create_w_nodes();
    parent->level = n_child->level + 1;
    parent->nodes.child[0] = n_child;
    parent->elements = rrb_size(n_child);
    n_child->full = true;
    return parent;
}

/** Adds a child to a node. */
void add_child(rrb_t* parent, rrb_t* child, bool first) {
    int index = first == false ? 1 : 0;
    if (child->level == parent->level - 1) {
        inc_ref(child);
        parent->nodes.child[index] = child;
    } else {
        parent->nodes.child[index] = create_w_nodes();
        parent->nodes.child[index]->level = parent->level - 1;
        add_child(parent->nodes.child[index], child, true);
    }
    parent->elements += rrb_size(child);
}

/** Adds child as a node. */
rrb_t* add_as_child(const rrb_t* parent, rrb_t* child) {
    int last = find_last_index(parent);
    if (last == 31) {
        rrb_t* n_parent = create_parent(parent);
        add_child(n_parent, child, false);
        return n_parent;
    }

    rrb_t* n_parent = copy_node(parent);
    n_parent->nodes.child[last]->full = true;
    n_parent->elements += rrb_size(child);
    if (child->level == parent->level - 1) {
        n_parent->nodes.child[last + 1] = child;
        inc_ref(child);
    } else {
        n_parent->nodes.child[last + 1] = create_w_nodes();
        n_parent->nodes.child[last + 1]->level = parent->level - 1;
        add_child(parent->nodes.child[last + 1], child, true);
    }
    return n_parent;
}

/** Merges two RRB-Tree into one, and returns it. */
rrb_t* rrb_merge(rrb_t* left, rrb_t* right) {
    if (left->level > right->level) {
        return add_as_child(left, right);
    } else if (left->level < right->level) {
        return add_as_child(right, left);
    }

    rrb_t* merged = merge(left, right);
    if (merged->nodes.child[1] == NULL) {
        rrb_t* temp = merged->nodes.child[0];
        inc_ref(temp);
        dec_ref(merged);
        merged = temp;
    }
    consolidate_tree(&merged, true);
    return merged;
}
