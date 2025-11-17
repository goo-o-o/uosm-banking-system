#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

struct MenuList {
    size_t size;
    char **entries;
};

void main_menu(void); // So im guessing this is like an abstract method in Java (C prototype)
void logout_page(void);

static void print_list(const struct MenuList *menu) {
    for (size_t i = 0; i < menu->size; i++) {
        printf("%zu. %s\n", i + 1, menu->entries[i]);
    }
}

// Redundant for now as strcasecmp exists
// static void string_to_lower_safe(const char *src, char *dst, size_t dst_size) {
//     if (!src) return;
//     size_t i = 0;
//     while (src[i] && i < dst_size - 1) {
//         dst[i] = (char) tolower(src[i]);
//         i++;
//     }
//     dst[i] = '\0';
// }

static char *main_menu_entries_logged_out[] = {
    "Create a New Bank Account", "Login to an Existing Bank Account"
};

static char *main_menu_entries_logged_in[] = {
    "Deposit", "Withdrawal", "Remittance", "Logout", "Delete"
};

#define MAIN_MENU_LOGGED_IN_COUNT (sizeof(main_menu_entries_logged_in) / sizeof(main_menu_entries_logged_in[0]))
#define MAIN_MENU_LOGGED_OUT_COUNT (sizeof(main_menu_entries_logged_out) / sizeof(main_menu_entries_logged_out[0]))

struct MenuList main_menu_list_logged_in = {
    MAIN_MENU_LOGGED_IN_COUNT,
    main_menu_entries_logged_in,
};

struct MenuList main_menu_list_logged_out = {
    MAIN_MENU_LOGGED_OUT_COUNT,
    main_menu_entries_logged_out,
};

enum AccountType {
    SAVINGS, CURRENT
};

char const *account_types[] = {"Savings", "Current"};

struct BankAccount {
    char name[100];
    char id[100];
    enum AccountType account_type;
    char pin[5];
    time_t date_created;
    double balance;

    int (*deposit)(void *self, float amount);

    int (*withdrawal)(void *self, float amount);

    int (*remittance)(void *self, char recipient[], float amount);

    void (*delete)(void *self);
};

int deposit(void *self_ptr, float amount) {
    struct BankAccount *acc = self_ptr;
    acc->balance += amount;
    return 1;
}

int withdrawal(void *self_ptr, float amount) {
    struct BankAccount *acc = self_ptr;
    if ((acc->balance - amount) > 0) {
        acc->balance -= amount;
        return 1;
    }
    return 0;
}

struct BankAccount *get_account_from_id(char *id);

struct BankAccount *get_account_from_name(char *name);

int is_valid_id(char *id);

int remittance(void *self_ptr, char recipient[], float amount) {
    struct BankAccount *acc = self_ptr;
    struct BankAccount *receiver;

    if (is_valid_id(recipient)) {
        // printf("get_account_from_id\n");
        receiver = get_account_from_id(recipient);
    } else {
        // printf("get_account_from_name\n");
        receiver = get_account_from_name(recipient);
        // printf("%s\n", acc->name);
    }

    if ((acc->balance - amount) > 0) {
        if (receiver != NULL) {
            acc->balance -= amount;
            receiver->balance += amount;
            printf("Successfully transferred %f to %s", amount, receiver->name);
            free(receiver);
            return 1;
        }
        printf("Cannot find recipient account\n");
        return 0;
    }
    printf("Not enough money brokie\n");
    free(receiver);
    return 0;
}


static void print_account_simple(const struct BankAccount *acc) {
    printf("Name: %s\n", acc->name);
    printf("ID: %s\n", acc->id);
}

static void print_account(const struct BankAccount *acc) {
    print_account_simple(acc);
    printf("Type: %s\n", account_types[acc->account_type]);
    printf("Date Created: %s", ctime(&acc->date_created)); // pass address
    printf("Balance: %.2f\n", acc->balance);
}


static struct BankAccount *current_account = NULL;

int duplicate_id(long long id) {
    return 0; // For now, haven't set up database
}

long long generate_account_id() {
    long long min_val = 1000000; // Smallest 7 digit
    long long max_val = 999999999; // Largest 9 digit

    long long id;
    while (true) {
        srand(time(NULL)); // Pass in a random seed based on the time for more randomness

        // Basically this is how other languages does intInRange or similar
        id = rand() % (max_val - min_val + 1) + min_val;
        if (!duplicate_id(id)) break; // To ensure its distinct, placeholder method for now
    }
    return id;
}


int min(const int a, const int b) {
    return a < b ? a : b;
}

int max(const int a, const int b) {
    return a > b ? a : b;
}

// https://www.geeksforgeeks.org/dsa/damerau-levenshtein-distance/
// Converted from Java to C since I am most comfortable with Java
// DLD isn't that good on its own, well in this case its just Levenshtein distance, for example 'dep' and 'del' both give the same distance from Delete and Deposit
// int optimalStringAlignmentDistance(char s1[], char s2[]) {
//     int size_s1 = strlen(s1);
//     int size_s2 = strlen(s2);
//
//     int dp[size_s1 + 1][size_s2 + 1];
//
//     for (int i = 0; i <= size_s1; i++) {
//         dp[i][0] = i;
//     }
//     for (int j = 0; j <= size_s2; j++) {
//         dp[0][j] = j;
//     }
//
//     for (int i = 1; i <= size_s1; i++) {
//         for (int j = 1; j <= size_s2; j++) {
//             if (s1[i - 1] == s2[j - 1]) {
//                 dp[i][j] = dp[i - 1][j - 1];
//             } else {
//                 dp[i][j] = 1 + min(min(dp[i - 1][j], dp[i][j - 1]), dp[i - 1][j - 1]);
//             }
//         }
//     }
//
//     return dp[size_s1][size_s2];
// }
//
//
// int get_suitable_option(const struct MenuList *menu, char input[50]) {
//     int best_idx = -1;
//     int min_dist = INT_MAX;
//
//     char lower_in[50] = {0};
//     for (int j = 0; input[j] && j < 49; ++j) {
//         lower_in[j] = tolower((unsigned char) input[j]);
//     }
//
//     for (int i = 0; i < menu->size; i++) {
//         char *entry = menu->entries[i];
//         char *space = strchr(entry, ' ');
//
//         int len = space ? space - entry : strlen(entry);
//
//         char first[64] = {0};
//         for (int j = 0; j < len && j < 63; ++j) {
//             first[j] = tolower((unsigned char) entry[j]);
//         }
//
//         int dist = optimalStringAlignmentDistance(first, lower_in);
//
//         if (dist < min_dist) {
//             min_dist = dist;
//             best_idx = i;
//         }
//     }
//     return best_idx;
// }


int get_suitable_option(const struct MenuList *menu, char input[50]) {
    if (isdigit(input[0])) {
        int option = input[0] - '0' - 1; // Returns ASCII by itself, need to convert
        if (option < menu->size)
            return option;
        return -1;
    }


    int best = -1, best_score = -1;

    char lower_in[50] = {0};
    for (int i = 0; input[i] && i < 49; ++i)
        lower_in[i] = tolower((unsigned char) input[i]); // Lowercase to normalize
    int input_len = strlen(lower_in);

    for (int i = 0; i < menu->size; ++i) {
        // Here we will loop through each menu entry, calculate the score and get the most suitable (higher score)
        char *full_menu_entry = menu->entries[i];
        char *space = strchr(full_menu_entry, ' ');
        int menu_entry_len = space ? space - full_menu_entry : strlen(full_menu_entry);
        // Taking advantage of pointer subtraction to get length of entry (first word)

        // Extract & lowercase first word to new variable
        char menu_entry_first[64] = {0};
        for (int j = 0; j < menu_entry_len && j < 63; ++j)
            menu_entry_first[j] = tolower((unsigned char) full_menu_entry[j]);

        int score = 0;

        // Match prefix if possible
        if (input_len <= menu_entry_len && strncmp(lower_in, menu_entry_first, input_len) == 0)
            // Match first n chars, if possible
            score = 1000 + (100 - abs(menu_entry_len - input_len)); // shorter diff in word = higher score


        else if (strstr(menu_entry_first, lower_in)) // Check if input is present anywhere in menu entry
            score = 500 + (100 - abs(menu_entry_len - input_len)); // shorter diff in word = higher score

        // Count matching chars
        else {
            int matches = 0;
            for (int a = 0; a < input_len; ++a)
                for (int b = 0; b < menu_entry_len; ++b)
                    if (lower_in[a] == menu_entry_first[b]) {
                        matches++;
                        break;
                    }
            // Break to make sure each match is distinct
            score = matches * 10;
        }

        // Update
        if (score > best_score || (score == best_score && i < best)) {
            best_score = score;
            best = i;
        }
    }

    return best;
}

void print_divider() {
    for (int i = 0; i < 50; i++) {
        printf("-");
    }
    printf("\n");
}

void create_page() {
}

void delete_page() {
}

void deposit_page() {
    printf("Enter the amount you would like to Deposit: ");
    fflush(stdout);

    float amount;
    if (scanf("%f", &amount) != 1 || amount <= 0) {
        while (getchar() != '\n') {}
        // clear buffer
        printf("Invalid amount. Enter a positive number: ");
        deposit_page();
        return;
    }
    current_account->deposit(current_account, amount);
    printf("Deposited %.2f successfully!\n", amount);

    main_menu();
}

void withdrawal_page() {
}

void remittance_page() {
}


void print_session_info() {
}

// https://www.sololearn.com/en/compiler-playground/c5AkrEzE6i02/?ref=app
void get_string(char *prompt, char **p_strp) {
    printf("%s", prompt);
    for (int i = 0; 1; i++) {
        if (i) {
            *p_strp = (char *) realloc(*p_strp, i * 2);
        } else {
            *p_strp = (char *) malloc(i + 1);
        }

        (*p_strp)[i] = getchar();
        if ((*p_strp)[i] == '\n' || (*p_strp)[i] == EOF) {
            (*p_strp)[i] = '\0';
            break;
        }
    }
}


void print_date_and_time() {
    time_t current_time;
    time(&current_time);

    printf("Current time: %s\n", ctime(&current_time));
}


int is_txt_file(const char *filename) {
    size_t len = strlen(filename);
    if (strcmp(filename + len - 4, ".txt") == 0) {
        // Compare last 4 chars
        return 1;
    }
    return 0;
}

int is_valid_id(char *id) {
    size_t len = strlen(id);
    if (len < 7 || len > 9) return 0;

    for (size_t i = 0; i < len; i++) {
        if (!isdigit((unsigned char) id[i])) return 0;
    }
    return 1;
}

// I forgot to initialize function pointers... this wasted so much time
// TODO: Call in create_page(), get_account_from_id() and get_account_from_name();
void init_account_vtable(struct BankAccount *acc) {
    acc->deposit = deposit;
    acc->withdrawal = withdrawal;
    acc->remittance = remittance;
    acc->delete = NULL; // TODO : implement later
}

struct BankAccount *get_account_from_id(char *id) {
    if (!id || id[0] == '\0') return NULL;

    // Unfortunately C has no built-in JSON parser, would make it more robust
    struct BankAccount *acc = malloc(sizeof *acc);

    if (!acc) return NULL;

    init_account_vtable(acc);

    char path[256];
    int len = snprintf(path, sizeof(path), "./database/%s.txt", id);
    if (len < 0 || len >= sizeof(path)) {
        printf("Error: Path too long for ID: %s\n", id);
        free(acc);
        return NULL;
    }

    FILE *p = fopen(path, "r");

    if (!p) return NULL;

    int n = fscanf(p,
                   "%99[^\n]\n" // id
                   "%99[^\n]\n" // name
                   "%d\n" // account_type (enum as int)
                   "%4s\n" // pin (4 digits)
                   "%lld\n" // date_created
                   "%lf", // balance
                   acc->id, acc->name, (int *) &acc->account_type, acc->pin,
                   &acc->date_created, &acc->balance);

    fclose(p);
    if (n != 6) {
        // Malformed
        printf("Malformed file: %s\n", path);
        free(acc);
        return NULL;
    }

    return acc; // Must remember to free in caller method
}


// Quick dirty struct to get the count as well
typedef struct {
    struct BankAccount *accounts;
    size_t count;
} DatabaseResult;

DatabaseResult
load_or_create_database(int debug) {
    DatabaseResult result = {NULL, 0};
    struct dirent *entry;

    const char *path_to_db = "./database";
    DIR *dir_ptr = opendir(path_to_db);


    if (debug) printf("Searching for Database...\n");
    if (dir_ptr == NULL) {
        if (debug) printf("Database not found, creating Database folder...\n");
        mkdir(path_to_db);
        if (debug) printf("Database successfully created!\n");
    } else {
        if (debug) printf("Database found!\n");
    }

    // if (debug) printf("Loading accounts...\n");

    // This reads each entry in the folder
    size_t capacity = 4;
    size_t count = 0;
    struct BankAccount *arr = malloc(capacity * sizeof(*arr));

    if (!dir_ptr && (dir_ptr = opendir(path_to_db)) == NULL) {
        perror("Failed to open Database Directory\n");
        return result;
    }
    while ((entry = readdir(dir_ptr)) != NULL) {
        char *file_name = entry->d_name;
        if (strcmp(file_name, ".") == 0 || strcmp(file_name, "..") == 0)
            continue;
        if (is_txt_file(file_name)) {
            char id[256];
            size_t len = strlen(file_name);
            strncpy(id, entry->d_name, len - 4);
            id[len - 4] = '\0';

            if (is_valid_id(id)) {
                // This is a little redundant since we already have the file, but it would be nice to be able to pass in ID and get the account object from anywhere
                // Edit: nvm it only gets the file name and doesn't open the file in memory yet
                struct BankAccount *account = get_account_from_id(id);
                if (!account) continue;

                // if (debug) printf("Count: %llu | Capacity: %llu", count, capacity);

                if (count >= capacity) {
                    // Grow the array if count will exceed capacity
                    size_t new_capacity = capacity * 2;
                    struct BankAccount *temp = realloc(arr, new_capacity * sizeof *arr);
                    if (!temp) {
                        perror("Malloc failed\n");
                        free(arr);
                        arr = NULL;
                        closedir(dir_ptr);
                        return result;
                    }
                    arr = temp;
                    capacity = new_capacity;
                }
                arr[count++] = *account;
            }
        }
    }

    closedir(dir_ptr);
    if (debug) {
        if (count == 0) {
            printf("No accounts found!\n");
        } else {
            printf("Loaded %llu account%s!\n", count, count == 1 ? "" : "s");
            print_divider();
        }
    }

    result.accounts = arr;
    result.count = count;
    return result;
}


struct BankAccount *get_account_from_name(char *name) {
    DatabaseResult db_result = load_or_create_database(false);
    if (db_result.count == 0) {
        free(db_result.accounts);
        // printf("db_result.count == 0\n");
        return NULL;
    }
    // char name_lower[64];
    // string_to_lower_safe(name, name_lower, sizeof(name_lower));
    for (size_t i = 0; i < db_result.count; i++) {
        // char acc_name_lower[32];
        // string_to_lower_safe(db_result.accounts[i].name, acc_name_lower, sizeof(acc_name_lower));

        // WHY DIDNT I KNOW strcasecmp EXISTED???????
        if (strcasecmp(db_result.accounts[i].name, name) == 0) {
            struct BankAccount *copy = malloc(sizeof(struct BankAccount));
            if (copy) {
                *copy = db_result.accounts[i];
            }

            init_account_vtable(copy);


            free(db_result.accounts);
            // printf("Name found %s\n", db_result.accounts[i].name);
            return copy;
            // The address of the local variable 'account' may escape the function, I'm guessing I need to make a copy which the caller has to free
        }
    }
    return NULL;
}

int login(char *first, char *pin) {
    struct BankAccount *acc;
    if (is_valid_id(first)) {
        // printf("get_account_from_id\n");
        acc = get_account_from_id(first);
    } else {
        // printf("get_account_from_name\n");
        acc = get_account_from_name(first);
        // printf("%s\n", acc->name);
    }
    if (acc == NULL) {
        printf("Failed to Login: Account not found\n");
        return 0;
    }
    if (strcmp(acc->pin, pin) != 0) {
        printf("Failed to Login: Incorrect PIN\n");
        free(acc);
        return 1;
    }
    printf("Successfully logged in to %s\n", acc->name);
    current_account = acc;
    return 1;
}

void login_page() {
    char *first = NULL;
    get_string("Enter your Account Number or Name: \n", &first);

    char *pin = NULL;
    get_string("Enter your 4-Digit PIN\n", &pin);

    // printf("%s\n", first);
    login(first, pin);
    main_menu();
    free(first);
    free(pin);
}

void print_login_details() {
    if (current_account == NULL) {
        printf("Not currently logged in\n");
    } else {
        print_account(current_account);
    }
}

void print_logo() {
    printf("  _   _  ___  ___ __  __   ___            _   _             ___         _               \n");
    printf(" | | | |/ _ \\/ __|  \\/  | | _ ) __ _ _ _ | |_(_)_ _  __ _  / __|_  _ __| |_ ___ _ __  \n");
    printf(" | |_| | (_) \\__ \\ |\\/| | | _ \\/ _` | ' \\| / / | ' \\/ _` | \\__ \\ || (_-<  _/ -_) '  \\   \n");
    printf("  \\___/ \\___/|___/_|  |_| |___/\\__,_|_||_|_\\_\\_|_||_\\__, | |___/\\_, /__/\\__\\___|_|_|_|  \n");
    printf("                                                    |___/       |__/                    \n");
    print_divider();
}

void main_menu() {
    print_login_details();
    fflush(stdin); // important...

    char input[50];
    int loggedIn = current_account != NULL;

    if (loggedIn) {
        print_list(&main_menu_list_logged_in);
    } else {
        print_list(&main_menu_list_logged_out);
    }
    fgets(input, sizeof(input), stdin);

    int option = loggedIn
                     ? get_suitable_option(&main_menu_list_logged_in, input)
                     : get_suitable_option(&main_menu_list_logged_out, input);

    if (option == -1) {
        // If invalid, call the function again
        main_menu();
    } else {
        printf("Selected option %d\n", option + 1);
        if (!loggedIn) {
            switch (option) {
                case 0: create_page();
                    break;
                case 1: login_page();
                default: main_menu();
            }
        } else {
            switch (option) {
                case 0:
                    deposit_page();
                    break;
                case 1:
                    withdrawal_page();
                    break;
                case 2:
                    remittance_page();
                    break;
                case 3:
                    logout_page();
                    break;
                case 4:
                    delete_page();
                    break;
                default: main_menu();
            }
        }
    }
}

void logout_page() {
    char buffer[64];
    printf("Are you sure you would like to Logout? (y/n)\n");
    fgets(buffer, sizeof(buffer), stdin);

    if (strcasecmp(buffer, "yes\n") == 0 || strcasecmp(buffer, "y\n") == 0 ||
        strcmp(buffer, "yes") == 0 || strcmp(buffer, "y") == 0) {
        current_account = NULL;
        printf("Logged out successfully!\n");
        main_menu();
        return;
    }
    if (strcasecmp(buffer, "no\n") == 0 || strcasecmp(buffer, "n\n") == 0 ||
        // WHY DIDNT I KNOW strcasecmp EXISTED???????
        strcmp(buffer, "no") == 0 || strcmp(buffer, "n") == 0) {
        main_menu();
        return;
    }
    printf("Please enter a valid option\n");
    logout_page();
}

int main() {
    print_logo();
    print_session_info();

    DatabaseResult res = load_or_create_database(true);
    if (res.accounts && res.count > 0) {
        for (size_t i = 0; i < res.count; i++) {
            print_account_simple(&res.accounts[i]);
            print_divider();
        }
    }
    free(res.accounts);

    printf("What would you like to do today?\n");
    main_menu();
}
