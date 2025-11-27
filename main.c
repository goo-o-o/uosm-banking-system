#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <locale.h>

#ifdef _WIN32
#include <windows.h>
#endif

/**
 * @brief Error codes, I was having trouble keeping up and handling all the different codes across all methods
 */
typedef enum {
    SUCCESS = 1,
    ERR_INVALID_FORMAT = -1,
    ERR_INSUFFICIENT = -2,
    ERR_INVALID_PIN = -3,
    ERR_ACCOUNT_NOT_FOUND = -4,
    ERR_SELF_TRANSFER = -5,
    ERR_INVALID_AMOUNT = -6,
    ERR_SAVE_FAILED = -7,
    ERR_MALLOC_FAILED = -8,
    ERR_INPUT_OUT_OF_RANGE = -9,
    ERR_INVALID_PIN_LENGTH = -10,
    ERR_INVALID_PIN_FORMAT = -11,
    ERR_INVALID_OPTION = -12,
    ERR_INVALID_ACCOUNT_NUMBER_LENGTH = -13,
    ERR_INVALID_ACCOUNT_NUMBER_FORMAT = -14,
    ERR_INVALID_ACCOUNT_NAME_FORMAT = -15,
    ERR_MALFORMED_FILE = -16,
    ERR_INVALID_ID_FORMAT = -17,
    ERR_INVALID_ID_LENGTH = -18
} ErrorCode;

void handle_error_message(const ErrorCode code) {
    switch (code) {
        case ERR_INVALID_FORMAT: printf("Invalid format!\n");
            break;
        case ERR_INPUT_OUT_OF_RANGE: printf("Amount must be more than 0 and less than or equal to 50,000!\n");
            break;
        case ERR_INVALID_AMOUNT: printf("Amount must be more than 0!\n");
            break;
        case ERR_INSUFFICIENT: printf("Insufficient balance!\n");
            break;
        case ERR_SELF_TRANSFER: printf("Cannot send money to yourself!\n");
            break;
        case ERR_SAVE_FAILED: printf("Failed to save changes!\n");
            break;
        case ERR_ACCOUNT_NOT_FOUND: printf("Account not found!\n");
            break;
        case ERR_INVALID_PIN_LENGTH: printf("PIN must be 4 digits long!\n");
            break;
        case ERR_INVALID_PIN_FORMAT: printf("PIN may only contain numbers!\n");
            break;
        case ERR_INVALID_ACCOUNT_NUMBER_LENGTH: printf("Account Number must be 10 digits long!\n");
            break;
        case ERR_INVALID_ACCOUNT_NUMBER_FORMAT: printf("Account Number may only contain numbers!\n");
            break;
        case ERR_INVALID_ACCOUNT_NAME_FORMAT: printf("Account Name may not contain numbers!\n");
            break;
        case ERR_MALFORMED_FILE: printf("Malformed file!\n");
            break;
        case ERR_INVALID_OPTION: printf("Invalid option!\n");
            break;
        case ERR_INVALID_ID_FORMAT: printf("Invalid Account ID format!\n");
            break;
        case ERR_INVALID_ID_LENGTH: printf("Invalid Account ID length!\n");
            break;
        default: printf("Operation failed (unknown error)\n");
            break;
    }
}

/**
 * @brief Enable UTF-8 in the terminal
 * @remark print_divider() characters were printing correctly on my desktop but not on my laptop, turns out this was the reason \n
 * Added setlocale() in hopes it would work on linux (I do not have a Linux client) \n
 * @note <a href="https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/setmode?view=msvc-170">Source</a>
 */
static void enable_utf8() {
    setlocale(LC_ALL, "C.UTF-8");
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    _setmode(_fileno(stdout), CP_UTF8);
    setvbuf(stdout, NULL, _IONBF, 0);
#endif
}

/**
 * @brief Quickly prints a horizontal line composed of "━" characters
 * @remark On windows, @link enable_utf8 @endlink needs to be called first
 */
void print_divider_thick() {
    for (int i = 0; i < 50; i++)
        printf("━");
    printf("\n");
}

/**
 * @brief Quickly prints a horizontal line composed of "─" characters
 * @remark On windows, @link enable_utf8 @endlink needs to be called first
 */
void print_divider_thin() {
    for (int i = 0; i < 50; i++)
        printf("─");
    printf("\n");
}

/**
 * @brief Checks if a file is a txt file
 * @param file_name The name of the file
 * @return 1 if the file is a txt file, else 0
 */
int is_txt_file(const char *file_name) {
    const size_t len = strlen(file_name);
    if (strcmp(file_name + len - 4, ".txt") == 0) {
        return 1;
    }
    return 0;
}

/**
 * Safely get an input of any length
 * @return The string of the input
 * @remark <a href="https://github.com/muhsinzara/unlimited-string-in-C/blob/master/usic.c">Source</a>
 */
char *get_input() {
    size_t size = 2;
    size_t length = 0;
    char *buffer = malloc(size);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate memory\n");
        return NULL;
    }

    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        if (length + 1 >= size) {
            size *= 2;
            char *newBuffer = realloc(buffer, size);
            if (!newBuffer) {
                fprintf(stderr, "Failed to reallocate memory\n");
                free(buffer);
                return NULL;
            }
            buffer = newBuffer;
        }
        buffer[length++] = (char) ch;
    }
    buffer[length] = '\0';

    return buffer;
}


/**
 * @brief Robust method to determine whether a string is a float input
 *
 * @param input The string
 *
 * @note Posted by Stephan Lechner \n Retrieved 2025-11-24 \n License - CC BY-SA 3.0 \n
 * <a href="https://stackoverflow.com/a/45554836">Source</a>
 *
 */
int is_string_float(const char *input) {
    char *foo;
    strtod(input, &foo);
    if (foo == input) {
        return 0;
    }
    if (foo[strspn(foo, " \t\r\n")] != '\0') {
        return 0;
    }
    return 1;
}

const char *path_to_db = "./database";
char const *account_types[] = {"Savings", "Current"};

enum AccountType {
    SAVINGS, CURRENT, NUM_ACCOUNT_TYPES
};

/**
 * Main struct for managing accounts
 */
struct BankAccount {
    char name[100]; // The Account/User's name

    char account_number[100];
    // Coursework didn't specify much for this, so I will make it similar to BankAccount->account_number (10-digit number)
    char id[100];
    // I almost forgot that id =/= account_number, not sure why we need 2 different ID's but sure
    // Decided to store both as strings
    // Edit: Storing as long might be easier
    // Edit: Never mind, need to store as string in case the ID starts with 0... time to revert

    enum AccountType account_type; // 0 for Savings, 1 for Current
    char pin[5]; // 4-digit pin, 5 digit buffer for the null terminator
    time_t date_created; // The date created using time_t
    double balance;
};

int save_or_update_account(struct BankAccount *account);

void main_menu(void);

void deposit_page(void);

void withdrawal_page(void);

void remittance_page(void);

void logout_page(void);

char *get_valid_identifier();

struct BankAccount *get_account_from_identifier(char *identifier);

/**
 * @brief Abstract method to get the account from @p BankAccount::id
 * @note This method is Nullable
 * @param id The queried ID in the form of a string
 * @return BankAccount with corresponding id if present
 */
struct BankAccount *get_account_from_id(char *id);

/**
 * @brief Abstract method to get the account from @p BankAccount::account_number
 * @note This method is Nullable
 * @param account_number The queried account number in the form of a string
 * @return BankAccount with corresponding id if present
 */
struct BankAccount *get_account_from_account_number(char *account_number);

/**
 * @brief Abstract method to get the account from @p BankAccount::name
 * @note This method is Nullable
 * @param name The queried name in the form of a string
 * @return BankAccount with corresponding name if present
 */
struct BankAccount *get_account_from_name(const char *name);

/**
 * Convenience struct to print out Menu Lists easier
 */
struct MenuList {
    size_t size;
    const char *entries[];
};


static const struct MenuList main_menu_logged_in = {
    .size = 5,
    .entries = {
        "Deposit",
        "Withdrawal",
        "Remittance",
        "Logout",
        "Delete"
    }
};

static const struct MenuList main_menu_logged_out = {
    .size = 3,
    .entries = {
        "Create a New Bank Account",
        "Login to an Existing Bank Account",
        "Exit"
    }
};

static const struct MenuList main_menu_logged_out_no_accounts = {
    .size = 2,
    .entries = {
        "Create a New Bank Account",
        "Exit"
    }
};


/**
 * @brief Validates a @p BankAccount::account_number
 * @param number The account number to be validated in the form of a string
 * @return
 * @p ERR_INVALID_ACCOUNT_NUMBER_LENGTH If the length is not between 7-9 \n
 * @p ERR_INVALID_ACCOUNT_NUMBER_FORMAT If the number contains a non-digit \n
 * @p SUCCESS If none of the above
 */
ErrorCode is_valid_account_number(const char *number) {
    const size_t len = strlen(number);
    if (len < 7 || len > 9) return ERR_INVALID_ACCOUNT_NUMBER_LENGTH;

    for (size_t i = 0; i < len; i++) {
        if (!isdigit((unsigned char) number[i])) return ERR_INVALID_ACCOUNT_NUMBER_FORMAT;
    }
    return SUCCESS;
}


/**
 * @brief Validates a @p BankAccount::id
 * @param id The ID to be validated in the form of a string
 * @return
 * @p ERR_INVALID_ID_LENGTH If the length is not 10 \n
 * @p ERR_INVALID_ID_FORMAT If the id contains a non-digit \n
 * @p SUCCESS If none of the above
 */
ErrorCode is_valid_id(const char *id) {
    const size_t len = strlen(id);
    if (len != 10) return ERR_INVALID_ID_LENGTH;

    for (size_t i = 0; i < len; i++) {
        if (!isdigit((unsigned char) id[i])) return ERR_INVALID_ID_FORMAT;
    }
    return SUCCESS;
}

/**
 * @brief Validates a @p BankAccount::name
 * @param name The name to be validated in the form of a string
 * @return
 * @p ERR_INVALID_ACCOUNT_NAME_FORMAT If the name contains a digit \n
 * @p SUCCESS If none of the above
 */
int is_valid_name(const char *name) {
    for (int i = 0; i < strlen(name); i++) {
        if (isdigit((unsigned char) name[i]))
            return ERR_INVALID_ACCOUNT_NAME_FORMAT;
    }
    return SUCCESS;
}

/**
 * @brief Validates a @p BankAccount::pin
 * @param pin The PIN to be validated in the form of a string
 * @return
 * @p ERR_INVALID_PIN_LENGTH If the length is not 7-9 \n
 * @p ERR_INVALID_PIN_FORMAT If the PIN contains a non-digit \n
 * @p SUCCESS If none of the above */
ErrorCode is_valid_pin(const char *pin) {
    const size_t len = strlen(pin);
    if (len != 4) {
        return ERR_INVALID_PIN_LENGTH;
    }

    for (size_t i = 0; i < len; i++) {
        if (!isdigit((unsigned char) pin[i])) {
            return ERR_INVALID_PIN_FORMAT;
        }
    }
    return SUCCESS;
}

static struct BankAccount *current_account = NULL;

/**
 * @brief Convenience method to check if two BankAccounts are equal
 * @param acc The first account to compare with
 * @param other The other BankAccount to check with
 * @return 1 if both BankAccount's are equal \n 0 if not
 */
static int equal(const struct BankAccount *acc, const struct BankAccount *other) {
    if (!other) return 0;
    if (acc->balance != acc->balance) return 0;
    if (strcmp(acc->pin, other->pin) != 0) return 0;
    if (strcmp(acc->account_number, other->account_number) != 0) return 0;
    if (acc->account_type != other->account_type) return 0;
    if (difftime(acc->date_created, other->date_created) != 0) return 0;
    if (strcmp(acc->name, other->name) != 0) return 0;
    return 1;
}

/**
 * @brief Convenience method to deposit into a BankAccount
 * @param acc The BankAccount to deposit into
 * @param amount The amount to deposit
 * @return
 * @p ERR_INPUT_OUT_OF_RANGE If the input is less than 0 or more than 50,000 \n
 * @p SUCCESS If none of the above
 */
static ErrorCode float_deposit(struct BankAccount *acc, const float amount) {
    if (amount <= 0 || amount > 50000) return ERR_INPUT_OUT_OF_RANGE;
    // Almost forgot it has to be <= 50000, added new ErrorCode

    acc->balance += amount;
    save_or_update_account(acc);
    return SUCCESS;
}

/**
 * @brief Convenience method to deposit into a BankAccount with built-in input validation
 * @param acc The BankAccount to deposit into
 * @param amount_str The amount to deposit
 * @return
 * @p ERR_INVALID_INPUT If the input is not a float \n
 * @p ERR_INVALID_AMOUNT If the input is less than 0 \n
 * @p SUCCESS If none of the above \n
 */
static ErrorCode deposit(struct BankAccount *acc, const char *amount_str) {
    if (!is_string_float(amount_str)) return ERR_INVALID_FORMAT;

    const float amount = strtof(amount_str, NULL);

    return float_deposit(acc, amount);
}

/**
 * @brief Convenience method to withdraw from a BankAccount with built-in value validation
 * @param acc The BankAccount to withdraw from
 * @param amount The amount to withdraw as a float
 * @returns
 * @p ERR_INSUFFICIENT If balance is insufficient \n
 * @p ERR_INVALID_INPUT If amount is less than 0 \n
 * @p ERR_SAVE_FAILED If the changes were not saved to disk \n
 * @p SUCCESS If none of the above
 */
static ErrorCode float_withdrawal(struct BankAccount *acc, const float amount) {
    if (amount > acc->balance) {
        return ERR_INSUFFICIENT;
    }
    if (amount <= 0) {
        // Coursework didn't specify the range for this, will just do more than equals to 0 opposed to just more than 0
        // To prevent softlock when the user's balance is 0, and they accidentally click withdraw
        return ERR_INVALID_AMOUNT;
    }
    acc->balance -= amount;
    if (!save_or_update_account(acc)) return ERR_SAVE_FAILED;
    return SUCCESS;
}

/**
 * @brief Convenience method to withdraw from a BankAccount with built-in value and input validation
 * @param acc The BankAccount to withdraw from
 * @param amount_str The amount to withdraw as a string
 * @return
 * @p ERR_INSUFFICIENT If balance is insufficient \n
 * @p ERR_INVALID_AMOUNT If amount is less than 0 \n
 * @p ERR_INVALID_INPUT If the input is not a float \n
 * @p ERR_SAVE_FAILED If the changes were not saved to disk \n
 * @p SUCCESS If none of the above */
static ErrorCode withdrawal(struct BankAccount *acc, const char *amount_str) {
    if (!is_string_float(amount_str)) return ERR_INVALID_FORMAT;

    const float amount = strtof(amount_str, NULL);

    return float_withdrawal(acc, amount);
}

float get_tax(const struct BankAccount *sender, const struct BankAccount *recipient, const float amount) {
    float amount_taxed = amount;
    if (sender->account_type == SAVINGS && recipient->account_type == CURRENT) {
        amount_taxed *= .02F;
    } else if (sender->account_type == CURRENT && recipient->account_type == SAVINGS) {
        amount_taxed *= .03F;
    } else {
        amount_taxed = 0;
    }
    return amount_taxed;
}

/**
 * @brief Transfer cash to another BankAccount with built-in value validation
 * @param sender The sender
 * @param recipient The receiver of the cash
 * @param amount The amount to transfer as a float
 * @return
 * @p ERR_INVALID_AMOUNT If the amount was less than 0 \n
 * @p ERR_INSUFFICIENT If amount exceeds the current balance \n
 * @p ERR_SELF_TRANSFER If the sender is the recipient, this shouldn't happen \n
 * @p ERR_SAVE_FAILED If the changes did not get saved in storage \n
 * @p SUCCESS If none of the above
 */
static ErrorCode float_remittance(struct BankAccount *sender, struct BankAccount *recipient, const float amount) {
    if (amount < 0) return ERR_INVALID_AMOUNT;
    if (equal(sender, recipient)) return ERR_SELF_TRANSFER;


    const float tax = get_tax(sender, recipient, amount);

    if (tax + amount > sender->balance) return ERR_INSUFFICIENT;
    // Tax goes to bank
    sender->balance -= tax + amount;
    recipient->balance += amount;

    if (!save_or_update_account(sender)) return ERR_SAVE_FAILED;
    if (!save_or_update_account(recipient)) return ERR_SAVE_FAILED;

    return SUCCESS;
}

/**
 * Transfer cash to another BankAccount with built-in value and input validation
 * @param sender The sender
 * @param recipient The receiver
 * @param amount_str The amount to transfer as a string
 * @return
 * @p ERR_INVALID_INPUT If the input is not a float \n
 * @p ERR_INVALID_AMOUNT If the amount was less than 0 \n
 * @p ERR_INSUFFICIENT If amount exceeds the current balance \n
 * @p ERR_SELF_TRANSFER If the sender is the recipient, this shouldn't happen \n
 * @p ERR_SAVE_FAILED If the changes did not get saved in storage \n
 * @p SUCCESS If none of the above
 */
static ErrorCode remittance(struct BankAccount *sender, struct BankAccount *recipient, const char *amount_str) {
    if (!is_string_float(amount_str)) return ERR_INVALID_FORMAT;

    const float amount = strtof(amount_str, NULL);

    return float_remittance(sender, recipient, amount);
}

/**
 * @brief Simple struct to get the list of BankAccounts as well as the size of the list from a method
 */
typedef struct {
    struct BankAccount *accounts;
    size_t count;
} DatabaseResult;

/**
 * @brief Create the database folder if absent
 * @param debug Whether to print debug messages
 */
void create_database_folder_if_absent(const int debug) {
    const DIR *dir_ptr = opendir(path_to_db);
    if (dir_ptr == NULL) {
        if (debug) printf("Database not found, creating Database folder...\n");
        mkdir(path_to_db);
        if (debug) printf("Database successfully created!\n");
    } else {
        if (debug) printf("Database found!\n");
    }
}

/**
 * @brief Retrieves or creates the database
 * @param debug Whether to print debug messages
 * @return a DatabaseResult containing the accounts
 */
DatabaseResult
load_or_create_database(const int debug) {
    DatabaseResult result = {NULL, 0};
    struct dirent *entry;

    DIR *dir_ptr = opendir(path_to_db);
    create_database_folder_if_absent(debug);

    if (debug) printf("Loading accounts...\n");

    // This reads each entry in the folder
    size_t capacity = 4;
    size_t count = 0;
    struct BankAccount *arr = malloc(capacity * sizeof(*arr));
    if (!dir_ptr && (dir_ptr = opendir(path_to_db)) == NULL) {
        perror("Failed to open Database Directory\n");
        return result;
    }
    while ((entry = readdir(dir_ptr)) != NULL) {
        const char *file_name = entry->d_name;
        if (strcmp(file_name, ".") == 0 || strcmp(file_name, "..") == 0)
            continue;
        if (is_txt_file(file_name)) {
            char id[256];
            const size_t len = strlen(file_name);
            strncpy(id, entry->d_name, len - 4);
            id[len - 4] = '\0';

            if (is_valid_account_number(id)) {
                struct BankAccount *account = get_account_from_account_number(id);
                if (!account) continue;

                if (count >= capacity) {
                    // Grow the array if count will exceed capacity
                    const size_t new_capacity = capacity * 2;
                    struct BankAccount *temp = realloc(arr, new_capacity * sizeof *arr);
                    if (!temp) {
                        perror("Malloc failed\n");
                        free(arr);
                        arr = NULL;
                        closedir(dir_ptr);
                        free(account);
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
            print_divider_thick();
        }
    }

    result.accounts = arr;
    result.count = count;
    return result;
}

/**
 * Saves or updates the given BankAccount into the database as a file
 * @param account The BankAccount to save
 * @return 1 if success\n 0 if failed
 */
int save_or_update_account(struct BankAccount *account) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s.txt", path_to_db, account->account_number);

    FILE *file = fopen(filepath, "w");
    if (!file) {
        perror("Failed to save account");
        return 0;
    }

    fprintf(file, "%s\n", account->id);
    fprintf(file, "%s\n", account->account_number);
    fprintf(file, "%s\n", account->name);
    fprintf(file, "%d\n", account->account_type);
    fprintf(file, "%s\n", account->pin);
    fprintf(file, "%ld\n", (long) account->date_created);
    fprintf(file, "%.2f\n", account->balance);

    fclose(file);

    return 1;
}

/**
 * Convenience method to print a MenuList
 * @param menu The MenuList to print
 */
static void print_list(const struct MenuList *menu) {
    for (size_t i = 0; i < menu->size; i++) {
        printf("%d. %s\n", (int) (i + 1), menu->entries[i]);
    }
}

/**
 * Convenience method to print basic info about a BankAccount
 * @param acc The BankAccount to print
 */
static void print_account_simple(const struct BankAccount *acc) {
    printf("Name: %s\n", acc->name);
    printf("Account Number: %s\n", acc->account_number);
    printf("ID: %s\n", acc->id);
    printf("Type: %s\n", account_types[acc->account_type]);
}

/**
 * Prints all information regarding a BankAccount, except its pin
 * @param acc The BankAccount to print
 */
static void print_account(const struct BankAccount *acc) {
    print_account_simple(acc);
    printf("Date Created: %s", ctime(&acc->date_created)); // pass address
    printf("Balance: %.2f\n", acc->balance);
}


/**
 * @brief Checks if a BankAccount number has already been generated before
 * @param account_number The account number to validate
 * @return 1 if the number is unique\n 0 if duplicate
 */
int is_distinct_account_number(const char *account_number) {
    const DatabaseResult result = load_or_create_database(0);
    for (int i = 0; i < result.count; i++) {
        const struct BankAccount account = result.accounts[i];
        if (strcmp(account.account_number, account_number) == 0) {
            return 0;
        }
    }
    free(result.accounts);
    return 1;
}


/**
 * @brief Checks if an ID has already been generated before
 * @param id The ID
 * @return 1 if the ID is unique\n 0 if duplicate
 */
int is_distinct_id(const char *id) {
    const DatabaseResult result = load_or_create_database(0);
    for (int i = 0; i < result.count; i++) {
        const struct BankAccount account = result.accounts[i];
        if (strcmp(account.id, id) == 0) {
            free(result.accounts);
            return 0;
        }
    }
    free(result.accounts);
    return 1;
}

/**
 * @brief Checks if a Name has  been used before
 * @param name The Name
 * @return 1 if the ID is unique\n 0 if duplicate
 */
int is_distinct_name(const char *name) {
    const DatabaseResult result = load_or_create_database(0);
    int count = 0;
    for (int i = 0; i < result.count; i++) {
        const struct BankAccount account = result.accounts[i];
        if (strcasecmp(account.name, name) == 0) {
            count++;
        }
        if (count > 1) return 0;
    }
    free(result.accounts);
    return 1;
}

/**
 * Generates an account number for a BankAccount
 * @return A unique number ranging from 7-9 digits
 */
char *generate_account_number() {
    unsigned char id_str[10];
    while (1) {
        // Seed it first
        srand(time(0));

        // rand() only goes up to 32767;
        // rand() % (max_number + 1 - minimum_number) + minimum_number

        const int digits = rand() % 3 + 7;

        // At first, I wanted to concat a bunch of digits into a string and return that, but this is cleaner
        // Basically shift left and then add 0-9 to the end, repeat until the ID is fully built
        // for (int i = 0; i < digits; i++) {
        //     id = id * 10 + rand() % 10;
        // }
        // Above implementation lacks the ability to have an ID start with 0


        id_str[0] = '\0'; // Clear the buffer first
        // Now let us build
        for (int i = 0; i < digits; i++) {
            id_str[i] = '0' + rand() % 10;
            // We take ASCII value of 0 which is 48, and then just add 0-9, which corresponds to 48-57 which is the chars '0' to '9'
        }
        id_str[digits] = '\0'; // Terminate the string


        // To ensure its distinct, placeholder method for now
        if (is_distinct_id(id_str)) {
            char *result = malloc(digits + 1);
            strcpy(result, id_str);
            // Can't return a local address since it gets deleted
            return result;
        }
    }
}

/**
 * Uses char matching to determine the most suitable option from a given list of strings
 * @param list A list of strings
 * @param length The length of the list
 * @param input The input (must be less than 50 chars)
 * @return Index of the most suitable option in the list
 */
int get_suitable_option_from_list(const char *const list[], const size_t length, const char *input) {
    if (isdigit(input[0])) {
        const int option = input[0] - '0' - 1; // Returns ASCII by itself, need to convert
        if (option < length)
            return option;
        return -1;
    }

    int best = -1, best_score = -1;

    char lower_in[50] = {0};
    for (int i = 0; input[i] && i < 49; ++i)
        lower_in[i] = tolower((unsigned char) input[i]); // Lowercase to normalize
    const int input_len = strlen(lower_in);

    for (int i = 0; i < length; ++i) {
        // Here we will loop through each menu entry, calculate the score and get the most suitable, so the highest score
        const char *full_menu_entry = list[i];
        const char *space = strchr(full_menu_entry, ' ');
        const int menu_entry_len = space ? space - full_menu_entry : strlen(full_menu_entry);
        // Taking advantage of pointer subtraction to get length of entry (first word)

        // Extract & lowercase first word to new variable
        char menu_entry_first[64] = {0};
        for (int j = 0; j < menu_entry_len && j < 63; ++j)
            menu_entry_first[j] = tolower((unsigned char) full_menu_entry[j]);

        int score = 0;

        // Match prefix if possible
        if (input_len <= menu_entry_len && strncmp(lower_in, menu_entry_first, input_len) == 0)
            // Match first n chars, if possible
            score = 1000 + (100 - abs(menu_entry_len - input_len)); // Shorter diff in word = higher score


        else if (strstr(menu_entry_first, lower_in)) // Check if input is present anywhere in menu entry
            score = 500 + (100 - abs(menu_entry_len - input_len)); // Shorter diff in word = higher score

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

/**
 * Convenience method to pass a MenuList in directly
 * @param menu The MenuList to use
 * @param input String that is less than 50 chars
 * @return Index of the most suitable option of the MenuList
 */
int get_suitable_option_from_menu_list(const struct MenuList *menu, const char *input) {
    return get_suitable_option_from_list(menu->entries, menu->size, input);
}

// TODO: to implement
void delete_page() {
}

/**
 * @brief Wrapper to handle deposit flow
 */
void deposit_page() {
    printf("Enter the amount you would like to Deposit: \n");
    char *input = get_input();

    const ErrorCode code = deposit(current_account, input);
    if (code == SUCCESS) {
        printf("Deposited %.2f successfully!\n", strtof(input, NULL));
        main_menu();
    } else {
        handle_error_message(code);
        deposit_page();
    }
    if (input)
        free(input);
}

/**
 * Wrapper to handle withdrawal flow with feedback based on input
 */
void withdrawal_page() {
    printf("Current Balance: %.2f", current_account->balance);
    printf("Enter the amount you would like to Withdraw: \n");
    char *input = get_input();

    const ErrorCode code = withdrawal(current_account, input);
    if (code == SUCCESS) {
        printf("Withdrew %.2f successfully!\n", strtof(input, NULL));
        main_menu();
    } else {
        handle_error_message(code);
        withdrawal_page();
    }
    if (input)
        free(input);
}

DatabaseResult print_loaded_accounts() {
    const DatabaseResult database_result = load_or_create_database(true);
    for (int i = 0; i < database_result.count; i++) {
        struct BankAccount bank_account = database_result.accounts[i];
        if (equal(&bank_account, current_account)) continue;
        print_account_simple(&database_result.accounts[i]);
        if (i == database_result.count - 1) {
            print_divider_thick();
            break;
        }
        print_divider_thin();
    }
    return database_result;
}

void remittance_page() {
    print_divider_thick();
    const DatabaseResult db_res = print_loaded_accounts();

    printf("Enter the recipients Account Number, ID or Name: \n");
    char *identifier = get_valid_identifier();

    struct BankAccount *recipient = get_account_from_identifier(identifier);

    if (!recipient) {
        handle_error_message(ERR_ACCOUNT_NOT_FOUND);
        free(identifier);
        free(db_res.accounts);
        main_menu();
        return;
    }
    if (equal(recipient, current_account)) {
        handle_error_message(ERR_SELF_TRANSFER);
        free(recipient);
        free(identifier);
        free(db_res.accounts);
        main_menu();
        return;
    }

    // IDE giving so many false positives...


    printf("Transferable balance: %f\n",
           current_account->balance - get_tax(current_account, recipient, current_account->balance));
    printf("Enter the amount you would like to transfer:\n");
    char *amount_str = get_input();

    const ErrorCode code = remittance(current_account, recipient, amount_str);
    if (code == SUCCESS) {
        printf("Transferred %.2f to %s successfully!\n", strtof(amount_str, NULL), recipient->name);
    } else handle_error_message(code);

    free(amount_str);
    free(recipient);
    free(identifier);
    free(db_res.accounts);
    main_menu();
}

void print_date_and_time() {
    time_t current_time;
    time(&current_time);
    printf("Current time: %s", ctime(&current_time));
}


void create_page() {
    struct BankAccount *acc = malloc(sizeof *acc);

    char *pin;
    char *account_number;
    char *name;

    while (1) {
        printf("Enter your Name:\n");
        name = get_input();
        const ErrorCode code = is_valid_name(name);
        if (code == SUCCESS)
            break;
        handle_error_message(code);
    }
    strcpy(acc->name, name);

    int option;
    while (1) {
        printf("Enter your account type (Savings/Current):\n");
        const char *account_type_string = get_input();
        option = get_suitable_option_from_list(account_types, NUM_ACCOUNT_TYPES, account_type_string);

        if (option == -1) {
            printf("Please enter a valid account type (Savings/Current):\n");
        } else {
            break;
        }
    }

    while (1) {
        printf("Enter your 10-digit account number:\n");
        account_number = get_input();
        const ErrorCode code = is_valid_id(account_number);
        if (code == SUCCESS)
            break;
        handle_error_message(code);
    }
    strcpy(acc->id, account_number);

    const enum AccountType account_type = (enum AccountType) option;
    acc->account_type = account_type;

    while (1) {
        printf("Enter your 4-digit PIN:\n");
        pin = get_input();
        const ErrorCode code = is_valid_pin(pin);
        if (code == SUCCESS)
            break;
        handle_error_message(code);
    }
    strcpy(acc->pin, pin);


    sprintf(acc->account_number, "%s", generate_account_number());
    acc->balance = 0;
    time_t current_time;
    time(&current_time);
    acc->date_created = current_time;

    printf("Successfully created a New Account!\n");

    // I think I'll make it automatically log in
    current_account = acc;
    save_or_update_account(acc);
    main_menu();
}

ErrorCode validate_file(FILE *file, struct BankAccount *acc) {
    if (fscanf(file,
               "%99[^\n]\n" // id
               "%99[^\n]\n" // account_number
               "%99[^\n]\n" // name
               "%d\n" // account_type (enum as int)
               "%4s\n" // pin (4 digits)
               "%lld\n" // date_created
               "%lf", // balance
               acc->account_number, acc->id, acc->name, (int *) &acc->account_type, acc->pin,
               &acc->date_created, &acc->balance) != 7) {
        return ERR_MALFORMED_FILE;
    }
    return SUCCESS;
}

struct BankAccount *get_account_from_account_number(char *id) {
    if (!id || id[0] == '\0') return NULL;
    struct BankAccount *acc = malloc(sizeof *acc);
    if (!acc) return NULL;
    char path[256];
    const int len = snprintf(path, sizeof(path), "./database/%s.txt", id);
    if (len < 0 || len >= sizeof(path)) {
        printf("Error: Path too long for ID: %s\n", id);
        free(acc);
        return NULL;
    }
    FILE *file = fopen(path, "r");
    if (!file) return NULL;
    const ErrorCode code = validate_file(file, acc);
    fclose(file);
    if (code == SUCCESS) {
        return acc;
    }
    handle_error_message(ERR_MALFORMED_FILE);
    free(acc);
    return NULL;
}


struct BankAccount *get_account_from_name(const char *name) {
    const DatabaseResult db_result = load_or_create_database(false);
    if (db_result.count == 0) {
        free(db_result.accounts);
        return NULL;
    }
    for (size_t i = 0; i < db_result.count; i++) {
        if (strcasecmp(db_result.accounts[i].name, name) == 0) {
            struct BankAccount *copy = malloc(sizeof(struct BankAccount));
            if (copy) {
                *copy = db_result.accounts[i];
            }

            free(db_result.accounts);
            return copy;
        }
    }
    return NULL;
}

struct BankAccount *get_account_from_id(char *id) {
    const DatabaseResult db_result = load_or_create_database(false);
    if (db_result.count == 0) {
        free(db_result.accounts);
        return NULL;
    }
    for (size_t i = 0; i < db_result.count; i++) {
        if (strcasecmp(db_result.accounts[i].id, id) == 0) {
            struct BankAccount *copy = malloc(sizeof(struct BankAccount));
            if (copy) {
                *copy = db_result.accounts[i];
            }
            free(db_result.accounts);
            return copy;
        }
    }
    return NULL;
}

/**
 * Tries to identify and return a BankAccount related to the identifier
 * @param identifier The identifier to test with
 * @return The BankAccount if present, NULL if absent
 * @remark Checks in order of name -> account number -> account ID
 */
struct BankAccount *get_account_from_identifier(char *identifier) {
    struct BankAccount *from_name = get_account_from_name(identifier);
    struct BankAccount *from_id = get_account_from_id(identifier);
    struct BankAccount *from_number = get_account_from_account_number(identifier);

    if (from_name != NULL) {
        return from_name;
    }
    if (from_id != NULL) {
        return from_id;
    }
    if (from_number != NULL) {
        return from_number;
    }

    return NULL;
}

int login(char *identifier, const char *pin) {
    struct BankAccount *acc = get_account_from_identifier(identifier);

    if (acc == NULL) {
        printf("Failed to Login: Account not found\n");
        return 0;
    }

    if (is_valid_pin(pin) != SUCCESS) {
        free(acc);
        return 0;
    }

    printf("Successfully logged in to %s\n", acc->name);
    current_account = acc;
    return 1;
}

/**
 * @brief Prompts and validates for a correct identifier, usually to be passed into get_bank_account_from_identifier()
 * @param identifier The identifier to validate
 * @return The validated identifier
 */
char *get_valid_identifier() {
    while (1) {
        char *input = get_input();
        if (!input) continue;

        int valid_name = is_valid_name(input) == SUCCESS;
        int valid_number = is_valid_account_number(input) == SUCCESS;
        int valid_id = is_valid_id(input) == SUCCESS;

        if (valid_number) return input;
        if (valid_name && is_distinct_name(input)) return input;
        if (valid_id && is_distinct_id(input)) return input;

        if (valid_name) {
            printf("Multiple accounts with this name. Enter ID, Account Number, or different name: \n");
        } else if (valid_id) {
            printf("Multiple accounts with this ID. Enter your Account Number or Account Name: \n");
        } else {
            printf("Invalid input. Enter ID (7-9 digits), Account Number (10 digits), or name: \n");
        }

        free(input);
    }
}

void login_page() {
    printf("Enter your Account Number, ID or Name: \n");

    // Let me think about this
    // User enters name -> multiple found -> prompts to enter Number or ID -> user still enters name -> but this time it's another name -> go thru
    // User enters name -> multiple found -> prompts to enter Number or ID -> user still enters the same name -> prompts to enter Number or ID ->
    // repeat if user is stubborn -> until user either enters a different distinct name or a Number or ID
    // Luckily we don't need to differentiate ID and Number since their lengths will never overlap (7-9) and 10
    // But we do need to check for duplicate Numbers just in case, I will probably implement duplicate checking when creating accounts but just in case
    // User enters name -> multiple found -> prompts to enter Number or ID -> user enters Number -> duplicate Number -> prompts to enter either ID or Name ->
    // enters Name -> repeat -> enters ID -> pass

    char *identifier = get_valid_identifier();

    printf("Enter your 4-Digit PIN:\n");
    char *pin = get_input();

    login(identifier, pin);
    main_menu();
    free(identifier);
    free(pin);
}

void print_login_details() {
    if (current_account == NULL) {
        printf("You aren't logged in!\n");
    } else {
        print_account(current_account);
    }
}

void main_menu() {
    print_login_details();
    const int loggedIn = current_account != NULL;
    const int account_count = load_or_create_database(0).count;
    const struct MenuList *list = loggedIn
                                      ? &main_menu_logged_in
                                      : account_count == 0
                                            ? &main_menu_logged_out_no_accounts
                                            : &main_menu_logged_out;

    print_list(list);
    const char *input = get_input();

    const int option = get_suitable_option_from_menu_list(list, input);

    if (option == -1) {
        main_menu();
    } else {
        printf("Selected option %d (%s)\n", option + 1, list->entries[option]);
        if (!loggedIn) {
            // If not logged in
            if (account_count == 0) {
                switch (option) {
                    case 0:
                        create_page();
                        break;
                    case 1:
                        exit(1);
                    default: {
                        handle_error_message(ERR_INVALID_OPTION);
                        main_menu();
                    }
                }
            } else
                switch (option) {
                    case 0: create_page();
                        break;
                    case 1: login_page();
                    case 2: exit(1);
                    default: {
                        handle_error_message(ERR_INVALID_OPTION);
                        main_menu();
                    }
                }
        } else {
            // Logged in
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
    printf("Are you sure you would like to Logout? (y/n)\n");
    char *input = get_input();

    if (strcasecmp(input, "yes\n") == 0 || strcasecmp(input, "y\n") == 0 ||
        strcmp(input, "yes") == 0 || strcmp(input, "y") == 0) {
        current_account = NULL;
        printf("Logged out successfully!\n");
        if (input) free(input);
        main_menu();
        return;
    }
    if (strcasecmp(input, "no\n") == 0 || strcasecmp(input, "n\n") == 0 ||
        strcmp(input, "no") == 0 || strcmp(input, "n") == 0) {
        if (input) free(input);
        main_menu();
        return;
    }
    printf("Please enter a valid option\n");
    if (input) free(input);
    logout_page();
}

int main() {
    enable_utf8();
    print_divider_thick();
    printf("Welcome to the UoSM Banking System!\n");
    print_date_and_time();
    print_divider_thick();
    print_loaded_accounts();
    printf("What would you like to do today?\n");
    main_menu();
}
