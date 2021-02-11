#include <stdlib.h>
#include <time.h>    // Only needed for tests
#include <stdio.h>

#include "debug.h"

#include "common.h"


#define ________TEST


// Prints a visual representation of the Megatree starting from `branch`
void __mt_test_print_tree(mt_branch branch, int max_depth)
{
    // Print this node

}


// Returns a random integer between `min` (inclusive) and `max` (non-inclusive)
int __mt_rand(int min, int max)
{
    if(min > max) // Swap if the wrong way round for some godforsaken reason
    {
        int tmp = min; 
        min = max; 
        max = tmp;
    }
    return (rand() % (max-min) ) + min;
}

// Generate a buffer of random test data
//
// `out_buffer`    The buffer to write data into (must be big enough to hold `bytes` bytes)
// `bytes`         The number of bytes to write
// 
// Return:      The number of bytes written
int __mt_generate_random_data(void* out_buffer, size_t bytes)
{
    char *ch = out_buffer;
    int i;
    for(i=0; i<bytes; i++) *(ch + i) = __mt_rand(0, 255);
    return i;
}

// Generate a random string up to length `length` consisting of alphanumeric characters
//
// `out_buffer`             The existing string buffer to write to (must have space for `length` characters)
// `length`                 The number of characters to write
// 
// Return:      The number of bytes written
int __mt_rand_string(char* out_buffer, size_t length)
{
    int i;
    for(i=0; i<length-1; i++) *(out_buffer + i) = __mt_rand(48, 122); // '0' to 'z'

    out_buffer[length] = 0; // Null-terminated
    return i;
}

__mt_test_log(char* to_log)
{
    printf("%s\n", to_log);
}

// Check if the input strings are identical
//
// Return 1 if identical, 0 if not
int __mt_strings_equal(char* string_a, char* string_b)
{
    if(strcmp(string_a,string_b)==0) return 1;
    return 0;
}

void __mt_assert(int condition, char* error_message)
{
    if(!condition)
    {    // Always terminate after a failed assert
         MT_ERRORS_ARE_FATAL = 1; 
         mt_error(error_message);
    }
}




// A sort-of reasonably ok test of most, or all of the megatree code
int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("Running megatree tests...\n");
    
    srand(time(NULL));    // Make the tests fuzzier. Fluffier.
    
    // -------- Create data
    __mt_test_log(" Create root node ");

    mt_branch* root = mt_create_root();

    __mt_test_log(" Finished creating root ");

    __mt_test_log("Create first level branches");
    int first_level_branches_to_create = __mt_rand(1, 18);
    mt_branch* last_first_level_node;
    for(int i=0; i<first_level_branches_to_create; i++)
    {
        char label[8]; __mt_rand_string(&label, 8);
        last_first_level_node = mt_create_branch(root, label);
    }

    __mt_test_log(" Add sub-branches to last node of first level branches");
    int second_level_branches_to_create = __mt_rand(1, 18);
    mt_branch* last_second_level_node;
    for(int i=0; i<second_level_branches_to_create; i++)
    {
        char label[8]; __mt_rand_string(&label, 8);
        last_second_level_node = mt_create_branch(last_first_level_node, label);
    }

    __mt_test_log(" Set the label of an existing branch");
    mt_set_label(last_first_level_node, "last_first_level");
    __mt_test_log(" Verify correct");
    __mt_assert(__mt_strings_equal(last_first_level_node->label, "last_first_level"), "Label not set correctly");


    __mt_test_log(" Set the data type of an existing branch");
    mt_set_data_type(last_second_level_node, "test");
    __mt_test_log(" Verify correct");
    __mt_assert(__mt_strings_equal(last_second_level_node->data_type, "test"), "Data type not set correctly");

    __mt_test_log(" Change the data type of an existing branch");
    mt_set_data_type(last_second_level_node, "changed");
    __mt_test_log(" Verify correct");
    __mt_assert(__mt_strings_equal(last_second_level_node->data_type, "changed"), "Data type not set correctly");

    
    __mt_test_log(" Test creating branches based on a path");
    mt_create_path(root, "creating_path_test/creating_path_test2/test");

    __mt_test_log(" Create branches based on a path that includes spaces");
    mt_create_path(root, "creating_path_test/spaces used/test with spaces");    

    // Create branches based on a path that includes leading and trailing spaces (should be stripped)
    mt_create_path(root, "   creating_path_test/leading");    
    mt_create_path(root, "   creating_path_test/trailing   ");    
    mt_create_path(root, "   creating_path_test/leading and trailing  ");   
    
    __mt_test_log(" Create branches based on a path that includes an existing path");
    mt_create_path(root, "creating_path_test/creating_path_test2/extend_existing_path");

    __mt_test_log(" Create branches based on a path that includes leading and trailing slashes (should be stripped)");
    mt_create_path(root, "/creating_path_test/creating_path_test3/test");
    mt_create_path(root, "creating_path_test/creating_path_test4/test//");
    mt_create_path(root, "//creating_path_test/creating_path_test5/test/");
    mt_create_path(root, "/creating_path_test/creating_path_test6/test///");

    __mt_test_log(" Create branches based on a path that includes multiple adjacent slashes (should be collapsed to 1 slash)");
    mt_create_path(root, "/creating_path_test///creating_path_test8/test");
    mt_create_path(root, "/creating_path_test///creating_path_test8/test//test//test/");

    __mt_test_log(" Create branches based on a path that includes invalid characters");
    mt_create_path(root, "/creating_path_test/{134}");
    mt_create_path(root, "/creating_path_test/{invalid characters}");

    __mt_test_log(" Create branches based on a path (very long)");
    mt_create_path(root, "/very_long_path/j/j/j/j/j/j/j/j/j/j/j/j/j/j/j/j/j/j/j/j/j/j/j/j/j/j/j/j/j/");



    __mt_test_log(" Generate test data - exactly 1 million bytes");
    char* test_data = malloc( sizeof(test_data) * 1000000 );
    __mt_generate_random_data(test_data, 1000000);
    
    mt_branch* data_test_branch;

    __mt_test_log(" Add data to a new branch by copying it from a buffer");
    data_test_branch = mt_create_path(root, "/test/data_insertion/copied_from_buffer_89_bytes");
    mt_set_data_copy(data_test_branch, test_data, 89); // Copy 89 bytes

    __mt_test_log(" Add data to a new branch by copying it from a buffer (0 bytes)");
    data_test_branch = mt_create_path(root, "/test/data_insertion/copied_from_buffer_empty");
    mt_set_data_copy(data_test_branch, test_data, 0); 

    __mt_test_log(" Add data to a new branch by copying it from a buffer (very small amount of data like 1 byte)");
    data_test_branch = mt_create_path(root, "/test/data_insertion/copied_from_buffer_1_byte");
    mt_set_data_copy(data_test_branch, test_data, 1); 

    __mt_test_log(" Add data to a new branch by copying it from a buffer (1 kilobyte)");
    data_test_branch = mt_create_path(root, "/test/data_insertion/copied_from_buffer_1k");
    mt_set_data_copy(data_test_branch, test_data, 1000); 

    __mt_test_log(" Add data to a new branch by copying it from a buffer (1 megabyte)");
    data_test_branch = mt_create_path(root, "/test/data_insertion/copied_from_buffer_1_megabyte");
    mt_set_data_copy(data_test_branch, test_data, 1000000); 


    __mt_test_log(" Add data to a branch by assigning a pointer to it (1M)");
    data_test_branch = mt_create_path(root, "/test/data_insertion/pointer_1_megabyte"); 
    mt_set_data_pointer(data_test_branch, test_data, 1000000);

    __mt_test_log(" Add data to a branch by assigning a pointer to it (3 bytes) ");
    data_test_branch = mt_create_path(root, "/test/data_insertion/pointer_3_bytes");
    mt_set_data_pointer(data_test_branch, test_data, 3);

    // TODO: implement a mt_check_path_exists and also write a test for it
    //       Then use it to check all of these path operations

    // Also write a __mt_check_buffers_identical(void* buffer_a, void* buffer_b, size_t length)
    //      and use it to check that the data copying is good





    // Replace the data in a branch with new data by copying it from a buffer
    // Replace the data in a branch with new data by assigning a pointer to it

    // Remove data from a branch



    // -------- Retrieve data

    // Search for a label
    // Retrieve a branch based on a path
    // Try to retrieve an invalid path
    // Try to retrieve a path that does not exist

    // Get the size of a branch's data
    // Get a branch's data as a pointer
    // Get a branch's data by copying it into a buffer

    // Get the size of the data belonging to a branch's children
    // Get the size of the data belonging to all a branch's descendants


    // -------- Delete branches

    // Delete a single branch
    // Verify the memory has been freed


    // -------- Copy branches
    
    // Copy a branch, ignoring duplicates
    // Check duplicates now exist

    // Copy a branch, replacing top-level duplicates
    // Check no duplicates exist, and any original deeper-level items have been removed

    // Copy a branch, merging duplicates
    // Check duplicates exist


    // -------- Move branches
    // Copy a branch, ignoring duplicates
    // Check original has been removed


    // -------- Save and load
    // Test mt_get_tree_file_size
    // Assign buffer

    // Test writing to a buffer
    // Is this the same as the result from mt_get_tree_file_size?



    // -------- Housekeeping
    // Test finding the maximum ID (mt_find_max_id)
    // Test updating the maximum ID (mt_update_max_id)

    // Add a node, and see if the maximum id is updated
    // Delete a node, and see if the maximum id is updated


    // -------- Test data validation
    // Test invalid label

    // Test attempt to delete root node



    return 0;
}
