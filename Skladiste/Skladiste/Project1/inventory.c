#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "inventory.h"

#define INVENTORY_FILE "spremiste.txt"

int g_inventory_count = 0;

static void free_inventory(InventoryItem **inventory) {
    if (inventory && *inventory) {
        free(*inventory);
        *inventory = NULL;
    }
}

void to_lowercase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = (char)tolower((unsigned char)str[i]);
    }
}

int input_int(const char* prompt) {
    int value;
    char ch;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &value) == 1) {
            // Clear input buffer
            while ((ch = getchar()) != '\n' && ch != EOF);
            if (value < 0) {
                printf("Negativni brojevi nisu dozvoljeni. Pokusajte ponovno.\n");
                continue;
            }
            return value;
        }
        else {
            printf("Upisite broj.\n");
            // Clear input buffer
            while ((ch = getchar()) != '\n' && ch != EOF);
        }
    }
}

float input_float(const char* prompt) {
    float value;
    char ch;
    while (1) {
        printf("%s", prompt);
        if (scanf("%f", &value) == 1) {
            // Clear input buffer
            while ((ch = getchar()) != '\n' && ch != EOF);
            if (value < 0) {
                printf("Negativne vrijednosti nisu dozvoljene. Pokusajte ponovno.\n");
                continue;
            }
            return value;
        }
        else {
            printf("Upisite broj.\n");
            // Clear input buffer
            while ((ch = getchar()) != '\n' && ch != EOF);
        }
    }
}

int compare_inventory_by_id_bsearch(const void *a, const void *b) {
    const InventoryItem *itemA = (const InventoryItem *)a;
    const InventoryItem *itemB = (const InventoryItem *)b;
    return itemA->id - itemB->id;
}

int copy_file(const char *src, const char *dest) {
    FILE *fsrc = fopen(src, "rb");
    if (!fsrc) return -1;
    FILE *fdest = fopen(dest, "wb");
    if (!fdest) { fclose(fsrc); return -2; }
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fsrc)) > 0) {
        if (fwrite(buf, 1, n, fdest) != n) {
            fclose(fsrc); fclose(fdest); return -3;
        }
    }
    fclose(fsrc);
    fclose(fdest);
    return 0;
}

void find_item_by_id(int id) {
    int inventory_count = 0;
    InventoryItem *inventory = load_inventory(&inventory_count);
    if (!inventory || inventory_count == 0) {
        free_inventory(&inventory);
        printf("Skladiste nije ucitano.\n");
        return;
    }
    sort_inventory();
    InventoryItem key = { .id = id };
    InventoryItem *found = bsearch(&key, inventory, inventory_count, sizeof(InventoryItem), compare_inventory_by_id_bsearch);
    if (found) {
        printf("Pronađeno: ID %d, Brend %s, Okus %s\n", found->id, found->brand, found->flavour);
    } else {
        printf("Proizvod sa ID-em nije pronaden.\n", id);
    }
    free_inventory(&inventory);
}

InventoryItem* load_inventory(int *inventory_count) {
    if (!inventory_count) return NULL;
    FILE *fp = NULL;
    *inventory_count = 0;
    if (fopen_s(&fp, INVENTORY_FILE, "r") != 0 || !fp) {
        if (fopen_s(&fp, INVENTORY_FILE, "a+") != 0 || !fp) {
            printf("Error: Nije moguce stvoriti ili otvoriti %s.\n", INVENTORY_FILE);
            exit(1);
        }
        fclose(fp);
        return NULL;
    }

    InventoryItem *inventory = (InventoryItem*)malloc(MAX_ITEMS * sizeof(InventoryItem));
    if (!inventory) {
        printf("Alokacija memorije neuspjesna.\n");
        fclose(fp);
        exit(1);
    }

    while (fscanf_s(fp, "%d %31s %31s %d %f",
                  &inventory[*inventory_count].id,
                  inventory[*inventory_count].brand, (unsigned)_countof(inventory[*inventory_count].brand),
                  inventory[*inventory_count].flavour, (unsigned)_countof(inventory[*inventory_count].flavour),
                  &inventory[*inventory_count].quantity,
                  &inventory[*inventory_count].price) == 5) {
        (*inventory_count)++;
        if (*inventory_count >= MAX_ITEMS) break;
    }
    fclose(fp);
    g_inventory_count = *inventory_count;
    return inventory;
}

InventoryItem* load_inventory_with_calloc(int *inventory_count) {
    if (!inventory_count) return NULL;
    FILE *fp = fopen(INVENTORY_FILE, "r");
    if (!fp) return NULL;
    int cap = 10;
    InventoryItem *inventory = (InventoryItem*)calloc(cap, sizeof(InventoryItem));
    if (!inventory) { fclose(fp); return NULL; }
    *inventory_count = 0;
    while (fscanf(fp, "%d %31s %31s %d %f",
                  &inventory[*inventory_count].id,
                  inventory[*inventory_count].brand,
                  inventory[*inventory_count].flavour,
                  &inventory[*inventory_count].quantity,
                  &inventory[*inventory_count].price) == 5) {
        (*inventory_count)++;
        if (*inventory_count >= cap) {
            cap *= 2;
            InventoryItem *tmp = (InventoryItem*)realloc(inventory, cap * sizeof(InventoryItem));
            if (!tmp) { free(inventory); fclose(fp); return NULL; }
            inventory = tmp;
        }
    }
    fclose(fp);
    return inventory;
}

void save_inventory(const InventoryItem *inventory, int inventory_count) {
    if (!inventory || inventory_count < 0) return;
    FILE *fp = NULL;
    if (fopen_s(&fp, INVENTORY_FILE, "w") != 0 || !fp) {
        //printf("Error: Could not open %s for writing.\n", INVENTORY_FILE);
        exit(1);
    }
    for (int i = 0; i < inventory_count; ++i) {
        fprintf(fp, "%d %s %s %d %.2f\n",
                inventory[i].id,
                inventory[i].brand,
                inventory[i].flavour,
                inventory[i].quantity,
                inventory[i].price);
    }
    fclose(fp);
}

static int compare_inventory_by_id(const void *a, const void *b) {
    const InventoryItem *itemA = (const InventoryItem *)a;
    const InventoryItem *itemB = (const InventoryItem *)b;
    return itemA->id - itemB->id;
}

void sort_inventory() {
    int inventory_count = 0;
    InventoryItem *inventory = load_inventory(&inventory_count);
    if (!inventory || inventory_count <= 1) {
        free_inventory(&inventory);
        return;
    }
    qsort(inventory, inventory_count, sizeof(InventoryItem), compare_inventory_by_id);
    save_inventory(inventory, inventory_count);
    free_inventory(&inventory);
}

static void merge(InventoryItem *arr, int l, int m, int r, SortKey key) {
    int n1 = m - l + 1, n2 = r - m;
    InventoryItem *L = malloc(n1 * sizeof(InventoryItem));
    InventoryItem *R = malloc(n2 * sizeof(InventoryItem));
    for (int i = 0; i < n1; ++i) L[i] = arr[l + i];
    for (int j = 0; j < n2; ++j) R[j] = arr[m + 1 + j];
    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        int cmp;
        if (key == SORT_BY_ID)
            cmp = (L[i].id <= R[j].id);
        else
            cmp = (strcmp(L[i].brand, R[j].brand) <= 0);
        if (cmp)
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
    free(L); free(R);
}

void merge_sort_inventory(InventoryItem *arr, int l, int r, SortKey key) {
    if (l < r) {
        int m = l + (r - l) / 2;
        merge_sort_inventory(arr, l, m, key);
        merge_sort_inventory(arr, m + 1, r, key);
        merge(arr, l, m, r, key);
    }
}

int recursive_binary_search(const InventoryItem *arr, int l, int r, int id) {
    if (l > r) return -1;
    int m = l + (r - l) / 2;
    if (arr[m].id == id) return m;
    if (arr[m].id > id) return recursive_binary_search(arr, l, m - 1, id);
    return recursive_binary_search(arr, m + 1, r, id);
}

int Id_tracker() {
    int inventory_count = 0;
    InventoryItem *inventory = load_inventory(&inventory_count);
    int max_id = 0;
    for (int i = 0; i < inventory_count; ++i) {
        if (inventory[i].id > max_id) max_id = inventory[i].id;
    }
    free_inventory(&inventory);
    return max_id + 1;
}

void add_inventory() {
    int inventory_count = 0;
    InventoryItem* inventory = load_inventory_with_calloc(&inventory_count);

    if (inventory_count >= MAX_ITEMS) {
        printf("Skladiste puno, nije moguce dodati vise proizvoda.\n");
        free(inventory);
        return;
    }

    InventoryItem new_item;
    new_item.id = Id_tracker();

    printf("Upisi brend: ");
    scanf("%31s", new_item.brand);
    printf("Upisi okus: ");
    scanf("%31s", new_item.flavour);
    getchar(); // consume newline left by scanf

    // convert to lowercase immediately after input
    to_lowercase(new_item.brand);
    to_lowercase(new_item.flavour);

    new_item.quantity = input_int("Upisi kolicinu: ");
    new_item.price = input_float("Upisi cjenu: ");

    FILE* fp = fopen(INVENTORY_FILE, "a");
    if (!fp) {
        free(inventory);
        return;
    }
    fprintf(fp, "%d %s %s %d %.2f\n",
        new_item.id,
        new_item.brand,
        new_item.flavour,
        new_item.quantity,
        new_item.price);
    fclose(fp);

    printf("Novo pice dodano: ID: %d, %s %s, Kolicina: %d, Cijena: %.2f\n",
        new_item.id, new_item.brand, new_item.flavour,
        new_item.quantity, new_item.price);

    free(inventory);
    sort_inventory();
}

void view_inventory(const char* filter) {
    int inventory_count = 0;
    InventoryItem* inventory = load_inventory_with_calloc(&inventory_count);

    char sort_choice;
    printf("Sortiraj skladiste po ID-u (I) ili brendu (B)? ");
    sort_choice = getchar();
    int c; while ((c = getchar()) != '\n' && c != EOF);

    SortKey key = SORT_BY_ID;
    if (sort_choice == 'b' || sort_choice == 'B') key = SORT_BY_BRAND;

    merge_sort_inventory(inventory, 0, inventory_count - 1, key);

    // make a lowercase copy of filter
    char filter_lower[32];
    strncpy(filter_lower, filter, sizeof(filter_lower) - 1);
    filter_lower[sizeof(filter_lower) - 1] = '\0';
    to_lowercase(filter_lower);

    printf("\nLista:\n");
    for (int i = 0; i < inventory_count; ++i) {
        if (strcmp(filter_lower, "all") == 0 ||
            strstr(inventory[i].brand, filter_lower) ||
            strstr(inventory[i].flavour, filter_lower)) {
            printf("ID: %d | Brend: %s | Okus: %s | Kolicina: %d | Cijena: $%.2f\n",
                inventory[i].id, inventory[i].brand, inventory[i].flavour,
                inventory[i].quantity, inventory[i].price);
        }
    }
    free(inventory);
}

// In edit_inventory and delete_inventory, allow recursive_binary_search
void edit_inventory(int id) {
    int inventory_count = 0;
    InventoryItem* inventory = load_inventory_with_calloc(&inventory_count);

    merge_sort_inventory(inventory, 0, inventory_count - 1, SORT_BY_ID);
    int idx = recursive_binary_search(inventory, 0, inventory_count - 1, id);
    if (idx == -1) {
        printf("Item with ID %d not found.\n", id);
        free(inventory);
        return;
    }

    printf("Uredivanje (Brend: %s, Okus: %s, Kolicina: %d, Cijena: %.2f).\n",
        inventory[idx].brand, inventory[idx].flavour, inventory[idx].quantity, inventory[idx].price);

    printf("Upisi novi brend: ");
    scanf("%31s", inventory[idx].brand);
    printf("Upisi novi okus: ");
    scanf("%31s", inventory[idx].flavour);
    getchar(); // consume newline left by scanf

    to_lowercase(inventory[idx].brand);
    to_lowercase(inventory[idx].flavour);

    inventory[idx].quantity = input_int("Upisi novu kolicinu: ");
    inventory[idx].price = input_float("Upisi novu cijenu: ");

    printf("Izvrsena promjena.\n");
    save_inventory(inventory, inventory_count);
    sort_inventory();
    free(inventory);
}

void delete_inventory(int id) {
    int inventory_count = 0;
    InventoryItem *inventory = load_inventory_with_calloc(&inventory_count);

    merge_sort_inventory(inventory, 0, inventory_count - 1, SORT_BY_ID);
    int idx = recursive_binary_search(inventory, 0, inventory_count - 1, id);
    if (idx == -1) {
        printf("Item with ID %d not found.\n", id);
        free(inventory);
        return;
    }

    for (int j = idx; j < inventory_count - 1; ++j) {
        inventory[j] = inventory[j + 1];
    }
    --inventory_count;
    printf("Predment obrisan.\n");
    save_inventory(inventory, inventory_count);
    sort_inventory();
    free_inventory(&inventory);
}
