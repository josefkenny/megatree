typedef struct mt_branch mt_branch;
typedef struct mt_list mt_list;
struct mt_list {
    mt_list* prev;                 // The previous item, or NULL if this is the first item
    mt_list* next;                 // The next item, or NULL if this is the last item

    mt_branch* item;               // The branch itself
};
struct mt_branch {
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

};
extern int MT_ERRORS_ARE_FATAL;
extern int MT_ERROR_FLAG;
int __mt_check_error_flag();
int mt_error(const char *format,...);
extern size_t MT_CURRENT_NUM_BRANCHES;
extern size_t MT_MAX_ID;
size_t mt_find_max_id(mt_branch *root,size_t max_id,int max_depth);
void mt_update_max_id(mt_branch *root);
int mt_check_label_valid(char *new_label);
int mt_check_is_root(mt_branch *branch);
mt_branch *mt_check_branches_identical(mt_branch *branch_a,mt_branch *branch_b);
mt_branch *mt_get_parent(mt_branch *branch);
int mt_get_num_descendants(mt_branch *branch,size_t num_descendants,int max_depth);
int mt_get_num_children(mt_branch *branch);
mt_branch *mt_get_nth_child(mt_branch *branch,int n);
mt_branch *mt_get_first_child(mt_branch *branch);
mt_branch *mt_get_next_sibling(mt_branch *parent,mt_list *iterator);
int mt_get_children_as_pointer_array(mt_branch *branch,mt_list *iterator,mt_branch *out_pointer_array,int out_capacity);
mt_branch *mt_search_for_label(mt_branch *root,char *label);
mt_branch *mt_get_by_path(mt_branch *root,char *path);
int mt_check_path_exists(mt_branch *root,char *path);
void *mt_get_data_pointer(mt_branch branch);
int mt_get_data_copy(mt_branch branch,void *out_buffer,size_t out_capacity);
size_t mt_get_data_size(mt_branch branch);
size_t mt_get_childrens_data_size(mt_branch branch);
size_t mt_get_childrens_data_size_recursive(mt_branch branch);
char *mt_set_label(mt_branch *branch,char *new_label);
char *mt_set_data_type(mt_branch *branch,char *data_type);
mt_branch *mt_create_root();
mt_branch *mt_create_branch(mt_branch *parent,char *label);
mt_branch *mt_create_path(mt_branch *root,char *path);
int mt_set_data_copy(mt_branch *branch,void *data,size_t data_length);
int mt_set_data_pointer(mt_branch *branch,void *data,size_t data_length);
mt_branch *mt_delete_branch(mt_branch *branch);
int mt_copy_branch(mt_branch *to_copy,mt_branch *new_parent);
int mt_copy_branch_replace(mt_branch *to_copy,mt_branch *new_parent);
int mt_copy_branch_merge(mt_branch *to_copy,mt_branch *new_parent);
int mt_move_branch(mt_branch *to_move,mt_branch *new_parent);
int mt_move_branch_replace(mt_branch *to_move,mt_branch *new_parent);
int mt_move_branch_merge(mt_branch *to_move,mt_branch *new_parent);
size_t mt_get_tree_file_size(mt_branch *root);
int mt_write_tree_to_buffer(mt_branch *root,void *out_buffer,size_t out_capacity);
mt_branch *mt_load_tree_from_buffer(mt_branch *new_parent,void *in_buffer,size_t buffer_length);
void __mt_test_print_tree(mt_branch branch,int max_depth);
int __mt_rand(int min,int max);
int __mt_generate_random_data(void *out_buffer,size_t bytes);
int __mt_rand_string(char *out_buffer,size_t length);
__mt_test_log(char *to_log);
int __mt_strings_equal(char *string_a,char *string_b);
void __mt_assert(int condition,char *error_message);
#define INTERFACE 0
#define EXPORT_INTERFACE 0
#define LOCAL_INTERFACE 0
#define EXPORT
#define LOCAL static
#define PUBLIC
#define PRIVATE
#define PROTECTED
