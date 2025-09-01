#define _CRT_SECURE_NO_WARNINGS
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "inventory.h"

static MenuOption get_menu_option(const char* input) {
    if (strcmp(input, "view") == 0) return MENU_VIEW;
    if (strcmp(input, "edit") == 0) return MENU_EDIT;
    if (strcmp(input, "delete") == 0) return MENU_DELETE;
    if (strcmp(input, "add") == 0) return MENU_ADD;
    if (strcmp(input, "exit") == 0) return MENU_EXIT;
    return MENU_INVALID;
}

int main() {
    const char* menu_options[] = { "view", "edit", "delete", "add", "exit" };
    const char* menu_desc[] = {
        "za pregled skladista",
        "za uredjivanje proizvoda",
        "za brisanje proizvoda",
        "za dodavanje proizvoda",
        "za izlaz iz programa"
    };

    int running = 1;
    char input[32];

    FILE* fp = fopen("spremiste.txt", "a+");
    if (!fp) return 1;
    fclose(fp);

    while (running) {
        printf("\nMain Menu:\n");
        for (int i = 0; i < 5; ++i) {
            printf("  Upisi '%s' %s\n", menu_options[i], menu_desc[i]);
        }
        printf("  Upisi 'backup' za spremanje skladista\n");

        printf("Choose an option: ");
        if (scanf("%31s", input) != 1) {
            int c; while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        int c; while ((c = getchar()) != '\n' && c != EOF);

        to_lowercase(input);

        if (strcmp(input, "backup") == 0) {
            int res = copy_file("spremiste.txt", "spremiste_backup.txt");
            if (res == 0)
                printf("Spremanje uspjesno.\n");
            else
                printf("Spremanje neuspjesno.\n");
            continue;
        }

        MenuOption option = get_menu_option(input);
        switch (option) {
        case MENU_VIEW: {
            char filter[32];
            printf("Unesi filter (brend/okus ili 'all' za sve): ");
            if (scanf("%31s", filter) != 1) {
                printf("Pokusaj ponovo.\n");
                while ((c = getchar()) != '\n' && c != EOF);
                continue;
            }
            while ((c = getchar()) != '\n' && c != EOF);
            view_inventory(filter);
            break;
        }
        case MENU_EDIT: {
            int id = input_int("Upisi ID za promjenu: ");
            if (!is_valid_id(id)) {
                printf("Krivi ID.\n");
                break;
            }
            edit_inventory(id);
            break;
        }
        case MENU_DELETE: {
            int id = input_int("Upisi ID za brisanje: ");
            if (!is_valid_id(id)) {
                printf("Krivi ID.\n");
                break;
            }
            delete_inventory(id);
            break;
        }
        case MENU_ADD:
            add_inventory();
            break;
        case MENU_EXIT:
            running = 0;
            break;
        default:
            printf("Kriva opcija, pokusaj ponovo.\n");
        }
    }

    return 0;
}