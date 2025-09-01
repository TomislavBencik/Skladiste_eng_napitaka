#ifndef INVENTORY_H
#define INVENTORY_H

#include <stddef.h> // for size_t

typedef struct {
    int id;
    char brand[32];
    char flavour[32];
    int quantity;
    float price;
} InventoryItem;

#define MAX_ITEMS 100

// Macro function example (for SQUARE)
#define SQUARE(x) ((x)*(x))

// Menu enum for main menu
typedef enum {
    MENU_VIEW,
    MENU_EDIT,
    MENU_DELETE,
    MENU_ADD,
    MENU_EXIT,
    MENU_INVALID
} MenuOption;

// Submenu enum
typedef enum {
    SUBMENU_NONE,
    SUBMENU_STATS
} SubMenuOption;

// Extern global variable for inventory count
extern int g_inventory_count;

// Inline function for ID validation
static inline int is_valid_id(int id) { return id > 0; }

// Add enum for sort key
typedef enum {
    SORT_BY_ID,
    SORT_BY_BRAND
} SortKey;

// Recursive merge sort and binary search for inventory
void merge_sort_inventory(InventoryItem *arr, int left, int right, SortKey key);
int recursive_binary_search(const InventoryItem *arr, int left, int right, int id);

// Use calloc/realloc for dynamic inventory loading
InventoryItem* load_inventory_with_calloc(int *inventory_count);

InventoryItem* load_inventory(int *inventory_count);
void save_inventory(const InventoryItem *inventory, int inventory_count);
void view_inventory(const char *filter);
void edit_inventory(int id);
void delete_inventory(int id);
void add_inventory();
void sort_inventory();
int Id_tracker();

int input_int(const char *prompt);
float input_float(const char *prompt);

int copy_file(const char *src, const char *dest);

#endif