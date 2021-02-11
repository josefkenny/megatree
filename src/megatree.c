/*
TODO:
Useful features:
-   A macro and/or function to easily add any single variable or struct to a branch
        e.g. mt_add_variable(mt_branch* parent, VARIABLE, array_length)
    It should automatically detect the variable name and put this in the 'label' field
    It should automatically detect the variable type and put this in the `data_type` field
    It should automatically detect 

    There could be a 'copy' version and also a 'pointer' version
    In the case of the `pointer` version
   
-   An error system. e.g. get_last_error_type, get_last_error_message

-   A way of finding megatree branches by the pointer their `data` is assigned to

-   A list of transactions

   
Things that could potentially speed up the megatree:
-   A path string ->  maybe stored as a hashmap/binary tree for effecient text searching  

*/

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "debug.h"

#include "common.h"



#define ________MEGATREE_STRUCTS

#if INTERFACE
typedef struct mt_branch            // Main data unit
{
    mt_branch* parent;              // The parent of this branch
    mt_list* first_child;           // The first child of this branch, of NULL if there are no children
    mt_list* last_child;            // The last child of this branch, or NULL if there are no children

    size_t id;                      // This branch's id. Should be unique. Can be used instead of labels in paths e.g. {<id>}
    char* label;                    // A label identifying this branch. Not necessarily unique among siblings.
                                    // Cannot be blank or include the following characters: '/', ' ', '{', '}'

    char* data_type;                // Optional string identifying the type of data. Can be anything: NULL, an empty string, etc.
                                    // Can even be set to the parent's data type, in the case of branches containing

    size_t data_size;               // The size of `data` in bytes
    void* data;                     // Pointer to a buffer containing the data. Should always be exactly `data_size` bytes long

} mt_branch;


typedef struct mt_list             // Doubly-linked list containing the children of each `mt_branch`
{
    mt_list* prev;                 // The previous item, or NULL if this is the first item
    mt_list* next;                 // The next item, or NULL if this is the last item

    mt_branch* item;               // The branch itself
} mt_list;
#endif

#define ________ERROR_HANDLING


int MT_ERRORS_ARE_FATAL = 1;    // If set to 1, then terminate program if any error occurs

int MT_ERROR_FLAG = 0;          // If 1, an error has 

// When `MT_ERRORS_ARE_FATAL` is disabled, the mt_error_flag is set to 1 if an mt_error has occurred
// during the execution of a function. This function `check_error_flag()` returns the result of that
// flag and also clears it by setting it back to 0, allowing the function to detect an error condition
// and terminate in a safe way.
int __mt_check_error_flag()
{
    int temp_error_flag = MT_ERROR_FLAG;
    MT_ERROR_FLAG = 0;
    return temp_error_flag;
}

// Called when there is an error message to display
int mt_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf("*** Megatree error: %s\n", format, args);
    va_end(args);
    if(MT_ERRORS_ARE_FATAL)
    {
        printf("MT_ERRORS_ARE_FATAL is set to 1, so the program will now terminate.\n");
        exit(0);
    }
    MT_ERROR_FLAG = 1;
}


#define ________HOUSEKEEPING

// Keeps track of the total number of branches in the megatree
// Should be incremented every time a branch is added and decremented every time a branch is removed
size_t MT_CURRENT_NUM_BRANCHES = 0;

// This counter holds the maximum ID value in the entire megatree
// This may be higher than the number of nodes in the case of 
// In any case in which it may become desynchronised with the actual number, `mt_find_max_id` must be run
size_t MT_MAX_ID = 0;

// Recursively traverse the entire tree, starting at root, to find the highest `id` field
// 
// `root`       The root node to start searching from
// `max_id`     The maximum `id` field currently known (for recursive use)
// `max_depth`  The maximum depth to traverse to. If -1, go as deep as necessary (no limit on depth)
// 
// Returns:     The maximum `id` field found in the entire tree starting at `root`
size_t mt_find_max_id(mt_branch* root, size_t max_id, int max_depth)
{
    if(max_depth == 0) return max_id; // Stop if we have already reached max_depth
    
    // Check this node
    if(root->id > max_id) max_id = root->id; 

    // Recursively check deeper nodes
    mt_list* list_position = root->first_child;
    mt_branch* this_child = list_position->item;

    while(list_position != NULL && this_child != NULL) 
    {
        int new_depth = max_depth; // If max_depth is -1, ignore depth limit
        if(max_depth != -1) new_depth = max_depth - 1; 

        max_id = mt_find_max_id(this_child, max_id, new_depth);
        list_position = list_position->next;
        this_child = list_position->item;
    }

    return max_id;
}

// Finds the highest extant `id` field across the tree starting at `branch` and sets MT_MAX_ID to it
void mt_update_max_id(mt_branch* root) { MT_MAX_ID = mt_find_max_id(root, 0, -1); }



#define ________DATA_VALIDATION

// Checks whether the provided label is a valid
//
// Returns:     1 if the label is valid, 0 if it is invalid
int mt_check_label_valid(char* new_label)
{
    int i = 0;
    while(new_label[i] != 0)
    {
        if(new_label[i] == ' ' || new_label[i] == '/'   // ' ' is a general separator, '/' is the path separator, 
        || new_label[i] == '{' || new_label[i] == '}')  // and {  } are used to denote referencing using IDs rather than labels
        {
            return 0;
        }

        i++;
    }

    return 1;
}

// Check whether `branch` is the root node
// Returns:      1 if `branch` is the root node, or 0 if not
int mt_check_is_root(mt_branch* branch)
{
    if (branch->parent == NULL) return 1;
    return 0;
}


// Check whether these two branches (and all their sub-branches and data fields) are identical
// 
// `branch_a`   Branch to compare to `branch_b`
// `branch_b`   Branch to compare to `branch_a`
//
// Returns:     NULL if the branches are identical,
//              or a pointer to the first branch found in `branch_a` that does not match its `branch_b` counterpart
//              (you can run the function again with the branches swapped to find said non-matching counterpart)
mt_branch* mt_check_branches_identical(mt_branch* branch_a, mt_branch* branch_b)
{

}




#define ________GET_BRANCHES

// Get the parent of the specified branch
//
// `branch`     The branch to find the parent of
//
// Returns:     A pointer to the parent of `branch`, or NULL if there is none (e.g. the branch is the root)
mt_branch* mt_get_parent(mt_branch* branch)
{
    if (branch == NULL)  mt_error("Attempted to get the parent of a branch which is a null pointer"); 
    if (__mt_check_error_flag()) return 0;

    return branch->parent;
}



// Get the total number of recursive sub-branches (all descendants) of `branch`
//
// `branch`             The branch to start searching at
// `num_descendants`    Set to 0 -- Start counting at this number (for recursive use)
// `max_depth`          The maximum depth to traverse to. If -1, go as deep as necessary (no limit on depth)
//
//  Returns:            The total number of branches that descend from this branch
int mt_get_num_descendants(mt_branch* branch, size_t num_descendants, int max_depth)
{
    if(max_depth == 0) return num_descendants; // Stop if we have already reached max_depth

    // Recursively check deeper nodes
    mt_list* list_position = branch->first_child;
    mt_branch* this_child = list_position->item;

    while(list_position != NULL && this_child != NULL) 
    {
        int new_depth = max_depth; // If max_depth is -1, ignore depth limit
        if(max_depth != -1) new_depth = max_depth - 1; 

        num_descendants++; // Count this node
        num_descendants = mt_get_num_descendants(this_child, num_descendants, new_depth);
        list_position = list_position->next;
        this_child = list_position->item;
    }

    return num_descendants;
}

// Get the number of direct children that `branch` has
int mt_get_num_children(mt_branch* branch)
{
    mt_get_num_descendants(branch, 0, 1);
}


// Get the nth child of `branch`
//
// `branch`     The branch to look for children in
//
// Returns:     A pointer to the parent of `branch`, or NULL if there are less than `n` children
mt_branch* mt_get_nth_child(mt_branch* branch, int n)
{

}

// Get the first child of `branch`
//
// Returns:     A pointer to the first child, or NULL if there are no children
mt_branch* mt_get_first_child(mt_branch* branch)
{

}

// Iterate through the children of `parent` by maintaining an `iterator` pointer
// 
// If `iterator` is null, returns the first child of `parent` and sets `iterator` to the new sibling position
// If `iterator` is non-null, uses it to find the next sibling in the list
// 
// Returns: the first child of `parent`, 
//          or the next child pointed to by `iterator`,
//          or NULL if there are no more children
// 
// Also modifies `iterator` each time. Please pass it in the next time you call!
mt_branch* mt_get_next_sibling(mt_branch* parent, mt_list* iterator)
{

}


// Retrieve pointers to every child of the specified `branch` and write them into `out_pointer_array`
// This can be done in batches, for instance, when `out_pointer_array` is smaller than the total number
// of children belonging to this branch. If `iterator` is provided, it is modified each time to allow
// multiple sequential reads of pointers.
// 
// `branch`             The branch to retrieve the children of
// `iterator`           The iterator from a previous execution of this function, or NULL to start from the first child
// `out_pointer_array`  An array of pointers to branches written out by the function. Unused elements are filled with `NULL`
// `out_capacity`       The number of pointers that can fit in `out_pointer_array`
//   
// Returns:             The number of branch pointers written to `out_pointer_array` in this batch
//
// Also modifies `iterator` each time. Please pass it in the next time you call!
int mt_get_children_as_pointer_array(mt_branch* branch, mt_list* iterator, mt_branch* out_pointer_array, int out_capacity)
{
    
    // Remember to pad up to `out_capacity` with NULLs
}




#define ________SEARCH

// Find the first occurrence of a branch descending from `root` with the label `label`
mt_branch* mt_search_for_label(mt_branch* root, char* label)
{


}




// Return the branch pointed to by `path`, relative to the branch `root`
//
// Path format:   root/label/another_label/{12}/{132}/final_label
mt_branch* mt_get_by_path(mt_branch* root, char* path)
{
    if(path == NULL) return NULL;

    mt_branch* current_branch = root;

    // TODO: This is where we check in the cache first, when we have one

    // Iterate through each token in the path
    for (char *label = strtok(path," "); label != NULL; label = strtok(NULL, "/"))
    {
        // If label starts with { and ends with }
        int starts_with = (label[0] == '{');
        int i=0;
        while(label[i] != 0) { i++; }    // Go to the end of the string
        int ends_with = (label[i-1] == '}');

        if(starts_with && ends_with)
        {
            size_t id_to_find = atoi(label);    // Try to parse the string inside into an integer

            if(id_to_find != 0) // If it's valid, try to find a child of the current branch with a matching ID
            {   // If found, set current_branch to the matching branch and continue loop
                
                
            }   
        }
        else
        {

        }
        
        // Else, try to find a child of the current branch with a matching label
        

        // If found, set current_branch to the matching branch and continue loop

        
        // If not found, return NULL
    }

    // TODO: This is where we add the path to the path cache, when  we have one

    return current_branch;
}

// Check whether the branch pointed to by `path` exists
// Returns:     1 if found, 0 if not found
int mt_check_path_exists(mt_branch* root, char* path)
{
    if(mt_get_by_path(root, path) != NULL) return 1;
    return 0;
}





#define ________GET_DATA


// Get a pointer to the data belonging to `branch`
//
// Returns:     A pointer to the data or NULL if there is no data or if `branch` doesn't exist
void* mt_get_data_pointer(mt_branch branch)
{

}

// Copies the data belonging to `branch` into `out_buffer`, up to a maximum of `out_capacity` bytes
//
// Returns:     The number of bytes copied; 0 if there is no data or e.g. `branch` doesn't exist
int mt_get_data_copy(mt_branch branch, void* out_buffer, size_t out_capacity)
{
    
}


// Get the size of the data belonging to `branch`
// This only includes data fields, and not any tree metadata
//
// Returns:     the data size, in bytes, or 0 if there is no data or if the branch doesn't exist
size_t mt_get_data_size(mt_branch branch)
{

}

// Get the total size of the data belonging to the direct children of `branch`
// This only includes data fields, and not any tree metadata
//
// Returns:     the data size, in bytes, or 0 if there is no data or if there are no children
size_t mt_get_childrens_data_size(mt_branch branch)
{

}

// Get the total size of the data belonging to the direct children of `branch`
// This only includes data fields, and not any tree metadata
//
// Returns:     the data size, in bytes, or 0 if there is no data or if there are no children
size_t mt_get_childrens_data_size_recursive(mt_branch branch)
{

}




#define ________EDIT

// Sets the label for the specified Megatree branch.
// 
// `branch`     The branch to set the label of
// `new_label`  The label to set it to
//
// Returns:     A pointer to the new label string 
char* mt_set_label(mt_branch* branch, char* new_label)
{
    if (new_label == NULL)  mt_error("Attempted to set a label to a string which is a null pointer"); 
    if (new_label[0] == 0)  mt_error("Attempted to set a label to a string which is empty"); 
    if (branch == NULL)     mt_error("Attempted to set the label '%s' to a branch which is a null pointer"); 
    if (!mt_check_label_valid) mt_error("Attempted to set the label '%s', which contains disallowed characters"); 
    if (__mt_check_error_flag()) return 0;

    if (branch->label != NULL) free(branch->label); // Check whether there is already a label, and free it if needed

    branch->label = strdup(new_label);              // Create the new label
    return branch->label;
}

// Sets the data_type string for the specified branch
// 
// `branch`     The branch to set the data_type string of
// `new_label`  The data_type string to set it to
// 
// Returns:     A pointer to the new data type string
char* mt_set_data_type(mt_branch* branch, char* data_type)
{
    if (branch == NULL)     mt_error("Attempted to set the data type '%s' to a branch which is a null pointer"); 
    if (!mt_check_label_valid) mt_error("Attempted to set the data type '%s', which contains disallowed characters"); 
    if (__mt_check_error_flag()) return 0;

    if (branch->data_type != NULL) free(branch->data_type); // Check whether there is already a data type, and free it if needed

    branch->data_type = strdup(data_type);
    return branch->data_type;
}

// Create a new root megatree branch node.
//
// Returns:     A pointer to the new branch
mt_branch* mt_create_root()
{
    mt_branch* new_root = calloc( sizeof *new_root, 0 );

    DEBUG(sizeof *new_root);

    new_root->parent = NULL;
    new_root->first_child = NULL;
    mt_set_label(&new_root, "root");
    new_root->id = 0;

    new_root->data_size = 0;
    new_root->data = NULL;

    return &new_root;
}



// Create a new branch on the tree at 
//
//
//
//
//
mt_branch* mt_create_branch(mt_branch* parent, char* label)
{
    

}

// Create a tree structure matching the specified path string
// leaving any existing branches and data unchanged
//
// `root`       The 
// `path`       The path to create
//
//  Returns:    The deepest branch node created (i.e. the final element in the path string)
mt_branch* mt_create_path(mt_branch* root, char* path)
{
    // Make sure to disallow labels with illegal characters
}



// Copy data into the data field of `branch` from the buffer `data`
// 
// `branch`         The branch to copy data into
// `data`           A pointer to the buffer of data to be copied
// `data_length`    The number of bytes to copy from `data`
// 
// Returns:     The number of bytes copied, or 0 if an error occurred
int mt_set_data_copy(mt_branch* branch, void* data, size_t data_length)
{
    // Dispose of any existing data by freeing it

}


// Link the data field for `branch` by setting it to a pointer 
// to an already-existing buffer somewhere in memory
// 
// `branch`         The branch to link data to
// `data`           A pointer to the buffer of data to be linked
// `data_length`    The data length of the 
// 
// Returns:     1 if success, 0 if error
int mt_set_data_pointer(mt_branch* branch, void* data, size_t data_length)
{
    // Dispose of any existing data by freeing it


}

#define ________DELETE



// Delete the entire branch and its sub-branches
//
// `branch`     The branch to delete (along with sub-branches)
// 
// Returns:     The parent of the deleted branch, or NULL if failure
mt_branch* mt_delete_branch(mt_branch* branch)
{

}


#define ________COPY


// Copy an entire branch and any sub-branches to a different location
// leaving any branches with the same names as their siblings as duplicates
// 
// `to_copy`    The branch to be copied (along with sub-branches)
// `new_parent` The branch to become the new parent of `to_copy`
// 
// Returns:     1 if success, 0 if failure
int mt_copy_branch(mt_branch* to_copy, mt_branch* new_parent)
{
    

}

// Copy an entire branch and any sub-branches to a different location
// replacing any existing top-level branch with the same label as the
// incoming branch `to_copy`, and deleting all of that branch's sub-branches
// 
// `to_copy`    The branch to be copied (along with sub-branches)
// `new_parent` The branch to become the new parent of `to_copy`
// 
// Returns:     1 if success, 0 if failure
int mt_copy_branch_replace(mt_branch* to_copy, mt_branch* new_parent)
{
    

}

// Copy an entire branch and any sub-branches to a different location
// replacing the data of any branches that have the same labels and locations as
// those being copied, but keeping all children intact
// 
// `to_copy`    The branch to be copied (along with sub-branches)
// `new_parent` The branch to become the new parent of `to_copy`
// 
// Returns:     1 if success, 0 if failure
int mt_copy_branch_merge(mt_branch* to_copy, mt_branch* new_parent)
{
    

}

#define ________MOVE


// Move an entire branch and any sub-branches to a different location
// leaving any branches with the same names as their siblings as duplicates
// 
// `to_move`    The branch to be moved (along with sub-branches)
// `new_parent` The branch to become the new parent of `to_move`
// 
// Returns:     1 if success, 0 if failure
int mt_move_branch(mt_branch* to_move, mt_branch* new_parent)
{
    // Copy using mt_copy_branch then delete original
}


// Move an entire branch and any sub-branches to a different location
// replacing any existing top-level branch with the same label as the
// incoming branch `to_move`, and deleting all of that branch's sub-branches
// 
// `to_move`    The branch to be moved (along with sub-branches)
// `new_parent` The branch to become the new parent of `to_move`
// 
// Returns:     1 if success, 0 if failure
int mt_move_branch_replace(mt_branch* to_move, mt_branch* new_parent)
{
    // Copy using mt_copy_branch_replace then delete original 
}

// Move an entire branch and any sub-branches to a different location
// replacing the data of any branches that have the same labels and locations as
// those being moved, but keeping all children intact
// 
// `to_move`    The branch to be moved (along with sub-branches)
// `new_parent` The branch to become the new parent of `to_move`
// 
// Returns:     1 if success, 0 if failure
int mt_move_branch_merge(mt_branch* to_move, mt_branch* new_parent)
{
    // Copy using mt_copy_branch_merge then delete original
}




#define ________SERIALIZATION


// Calculate the total size in bytes of the tree, if serialised,
// by performing a simulated serialisation (no bytes are actually written)
// This is useful for allocating a buffer to write
// 
// `root`       The branch to start serialising at
// 
// Returns:     The total size of the tree in bytes
size_t mt_get_tree_file_size(mt_branch* root)
{

}

// Write the (sub-)tree starting at `root` to the buffer `out_buffer`, up to a maximum of `out_capacity` bytes
// 
// Returns:     The number of bytes written, or 0 if error
int mt_write_tree_to_buffer(mt_branch* root, void* out_buffer, size_t out_capacity)
{

}

// Read the (sub-)tree from the buffer `in_buffer`, up to a maximum of `buffer_length` bytes,
// re-create the tree structure including all data fields, and attatch this tree as a child of
// the branch `new_parent`.
// 
// Returns:  a pointer to the root of the newly-loaded tree that has been attatched to `new_parent`
//           or NULL if there was an error.
mt_branch* mt_load_tree_from_buffer(mt_branch* new_parent, void* in_buffer, size_t buffer_length)
{

   
}
