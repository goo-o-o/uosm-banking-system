#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <locale.h>
#include <math.h>

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
    ERR_INVALID_ID_LENGTH = -18,
    ERR_DELETE_FILE_FAILED = -19,
    ERR_CREATE_FILE_FAILED = -20,
    ERR_LOG_TRANSACTION_FAILED = -21
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
        case ERR_INVALID_ACCOUNT_NUMBER_LENGTH: printf("Account Number must be 7-9 digits long!\n");
            break;
        case ERR_INVALID_ACCOUNT_NUMBER_FORMAT: printf("Account Number may only contain numbers!\n");
            break;
        case ERR_INVALID_ACCOUNT_NAME_FORMAT: printf("Account Name may not contain numbers!\n");
            break;
        case ERR_MALFORMED_FILE: printf("Malformed file!\n");
            break;
        case ERR_INVALID_OPTION: printf("Invalid option!\n");
            break;
        case ERR_INVALID_ID_FORMAT: printf("ID may only contain numbers!\n");
            break;
        case ERR_INVALID_ID_LENGTH: printf("ID must be 10 digits long!\n");
            break;
        case ERR_DELETE_FILE_FAILED: printf("Failed to delete file!\n");
            break;
        case ERR_CREATE_FILE_FAILED: printf("Failed to create file!\n");
            break;
        case ERR_LOG_TRANSACTION_FAILED: printf("Failed to log transaction!\n");
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

    char account_number[100]; // 7-9 digits
    char id[100]; // 10 digits
    // Coursework didn't specify much for this, so I will make it similar to BankAccount->account_number (10-digit number)
    // I almost forgot that id =/= account_number, not sure why we need 2 different ID's but sure
    // Decided to store both as strings
    // Edit: Storing as long might be easier
    // Edit: Never mind, need to store as string in case the ID starts with 0... time to revert

    enum AccountType account_type; // 0 for Savings, 1 for Current
    char pin[5]; // 4-digit pin, 5 digit buffer for the null terminator
    time_t date_created; // The date created using time_t
    double balance;
};


/**
 * I am assuming we don't need to log creating and deleting of accounts
 */
enum TransactionType {
    DEPOSIT,
    WITHDRAWAL,
    REMITTANCE
};


ErrorCode log_transaction(const enum TransactionType type, const float amount, struct BankAccount *first,
                          struct BankAccount *second) {
    // Turns out append automatically creates the file if absent too, nice
    FILE *log = fopen("./database/transactions.txt", "a");
    if (!log) {
        return ERR_CREATE_FILE_FAILED;
    }

    time_t current_time;
    time(&current_time);

    switch (type) {
        case 0: {
            if (first != NULL) {
                fprintf(log, "[ %s (%s) <- ] %.2f | %s",
                        first->name, first->account_number,
                        amount, ctime(&current_time));
                fclose(log);
                return SUCCESS;
            }
        }
        case 1: {
            if (first != NULL) {
                fprintf(log, "[ %s (%s) -> ] %.2f | %s",
                        first->name, first->account_number,
                        amount, ctime(&current_time));
                fclose(log);
                return SUCCESS;
            }
        }
        case 2: {
            if (first != NULL && second != NULL) {
                fprintf(log, "[ %s (%s) -> %s (%s) ] %.2f | %s",
                        first->name, first->account_number,
                        second->name, second->account_number,
                        amount, ctime(&current_time));
                fclose(log);
                return SUCCESS;
            }
        }
        default: {
            fclose(log);
            return ERR_LOG_TRANSACTION_FAILED;
        }
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

int save_or_update_account(struct BankAccount *account);

ErrorCode delete_account(struct BankAccount *account);

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
ErrorCode is_valid_name(const char *name) {
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


/**
 * Stores the current account being logged into, NULL if not logged in
 */
static struct BankAccount *current_account = NULL;

/**
 * Prints login details if logged in
 */
void print_login_details() {
    if (current_account == NULL) {
        printf("You aren't logged in!\n");
    } else {
        printf("You are logged in to:\n");
        print_account(current_account);
    }
}

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
    if (amount > 0 && amount <= 50000) {
        acc->balance += amount;
        save_or_update_account(acc);
        return SUCCESS;
    }
    // Almost forgot it has to be <= 50000, added new ErrorCode

    log_transaction(DEPOSIT, amount, acc, NULL);

    return ERR_INPUT_OUT_OF_RANGE;
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
    float truncated_amount = roundf(amount * 100.0f) / 100.0f;

    if (truncated_amount > acc->balance) {
        return ERR_INSUFFICIENT;
    }
    if (truncated_amount <= 0) {
        // Coursework didn't specify the range for this, will just do more than equals to 0 opposed to just more than 0
        // To prevent softlock when the user's balance is 0, and they accidentally click withdraw
        return ERR_INVALID_AMOUNT;
    }
    acc->balance -= truncated_amount;

    log_transaction(WITHDRAWAL, truncated_amount, acc, NULL);

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

/**
 * Gets the tax percent based on the account types
 * @param sender The person sending the money
 * @param recipient The receiver
 * @return The tax percent as a decimal float
 */
float get_tax_percent(const struct BankAccount *sender, const struct BankAccount *recipient) {
    if (sender->account_type == SAVINGS && recipient->account_type == CURRENT) {
        return .02F;
    }
    if (sender->account_type == CURRENT && recipient->account_type == SAVINGS) {
        return .03F;
    }
    return 0;
}

/**
 * Convenience method to calculate the actual tax amount based on the amount given
 * @param sender The sender
 * @param recipient The receiver
 * @param amount
 * @return The actual amount of tax
 */
float get_tax(const struct BankAccount *sender, const struct BankAccount *recipient, const float amount) {
    return get_tax_percent(sender, recipient) * amount;
}

/**
 * Calculates the maximum transferable balance based on the sender and receivers account types
 * @param sender The sender
 * @param recipient The receiver
 * @return The max transferable balance of the sender
 */
float get_max_transferable(const struct BankAccount *sender, const struct BankAccount *recipient) {
    return (float) current_account->balance / (1.0f + get_tax_percent(sender, recipient));
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

    const float max_transferable = get_max_transferable(sender, recipient);

    // Shift 2 decimal places to the left then round it, then we have to divide back
    float truncated_amount = roundf(amount * 100.0f) / 100.0f;
    float truncated_max_transferable = roundf(max_transferable * 100.0f) / 100.0f;

    // printf("Transfer amount: %.8f (rounded to 2 decimals: %.2f)\n", amount, truncated_amount);
    // printf("Max transferable: %.8f (rounded to 2 decimals: %.2f)\n", max_transferable, truncated_max_transferable);

    if (truncated_amount > truncated_max_transferable) return ERR_INSUFFICIENT;
    // Tax goes to bank
    sender->balance -= truncated_amount + get_tax(sender, recipient, truncated_amount);
    recipient->balance += truncated_amount;

    log_transaction(REMITTANCE, amount, sender, recipient);

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
            char account_number[256];
            const size_t len = strlen(file_name);
            strncpy(account_number, entry->d_name, len - 4);
            account_number[len - 4] = '\0';

            if (is_valid_account_number(account_number) == SUCCESS) {
                struct BankAccount *account = get_account_from_account_number(account_number);
                // print_account(account);
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
        }
        print_divider_thick();
    }

    result.accounts = arr;
    result.count = count;
    return result;
}


/**
 * @brief Deletes the file associated with this account, does not log out
 * @param account The account to have its entry deleted
 * @return
 * @p SUCCESS If the file was deleted \n
 * @p ERR_DELETE_FILE_FAILED If the file could not be deleted
 */
ErrorCode delete_account(struct BankAccount *account) {
    char file_path[512];
    snprintf(file_path, sizeof(file_path), "%s/%s.txt", path_to_db, account->account_number);
    // printf("%s", file_path);
    if (remove(file_path) == 0) {
        return SUCCESS;
    }
    perror("Error deleting file: ");
    return ERR_DELETE_FILE_FAILED;
}

/**
 * Saves or updates the given BankAccount into the database as a file
 * @param account The BankAccount to save
 * @return 1 If the file was saved or updated successfully \n
 * 0 if failed
 */
int save_or_update_account(struct BankAccount *account) {
    char file_path[512];
    snprintf(file_path, sizeof(file_path), "%s/%s.txt", path_to_db, account->account_number);

    FILE *file = fopen(file_path, "w");
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
 * @brief Checks if a BankAccount number has already been generated before
 * @param account_number The account number to validate
 * @return 1 if the number is unique\n 0 if duplicate
 */
int is_distinct_account_number(const char *account_number) {
    const DatabaseResult result = load_or_create_database(0);
    int count = 0;
    for (int i = 0; i < result.count; i++) {
        const struct BankAccount account = result.accounts[i];
        if (strcmp(account.account_number, account_number) == 0) {
            count++;
        }
        if (count > 1) {
            free(result.accounts);
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
    int count = 0;
    for (int i = 0; i < result.count; i++) {
        const struct BankAccount account = result.accounts[i];
        if (strcmp(account.id, id) == 0) {
            count++;
        }
        if (count > 1) {
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
        if (count > 1) {
            free(result.accounts);
            return 0;
        }
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
 * Gets the main word, usually the first word from a Menu Item
 * @param menu_item The full menu entry
 * @param word_out The pointer to the output word
 * @param word_size Size of output word
 */
void extract_menu_word(const char *menu_item, char *word_out, size_t word_size) {
    const char *start = menu_item;

    // First omit the index and the dot and space so like '1. '
    while (isdigit((unsigned char) *start) || *start == '.' || *start == ' ') {
        start++; // Can use pointer index
    }

    // Find end of first word
    const char *end = start;
    while (*end && *end != ' ') {
        end++;
    }

    // Copy word (lowercase)
    const int len = end - start;
    for (int i = 0; i < len && i < (int) (word_size - 1); i++) {
        word_out[i] = tolower((unsigned char) start[i]);
    }
    word_out[len] = '\0';
}

/**
 * @brief Helper method to find how close an input is to a menu entry using 3 different methods
 * @param input The input of the user
 * @param input_len The length of said input
 * @param menu_word The menu word to compare with
 * @return a value from 0 to 1100 (higher is better)
 */
int calculate_match_score(const char *input, const int input_len, const char *menu_word) {
    const int menu_len = strlen(menu_word);

    // First we try to match the prefix, this has the highest weight
    if (input_len <= menu_len &&
        strncmp(input, menu_word, input_len) == 0) {
        return 1000 + (100 - abs(menu_len - input_len)); // This awards more points the closer the word is
    }

    // Try to find a substring
    if (strstr(menu_word, input)) {
        return 500 + (100 - abs(menu_len - input_len)); // Same here
    }

    // If all else fails just count the number of matching letters
    int matches = 0;
    for (int i = 0; i < input_len; i++) {
        for (int j = 0; j < menu_len; j++) {
            if (input[i] == menu_word[j]) {
                matches++;
                break; // So that only one match per input letter
            }
        }
    }

    return matches * 10;
}


/**
 * Finds best menu option matching user input
 * @param list Menu items (e.g. ["1. Deposit", "2. Withdrawal"])
 * @param length Number of menu items
 * @param input User input ("dep", "1", etc.)
 * @return Index of best match, or -1 if none
 */
int get_suitable_option_from_list(const char *const list[], const size_t length, const char *input) {
    // If the user enters numeric input, prioritize it first
    if (strlen(input) == 1 && isdigit(input[0])) {
        int option = input[0] - '0' - 1; // "1" → 0, "2" → 1
        if (option >= 0 && option < (int) length) {
            return option;
        }
    }

    // Prep the input by lowercasing (could use strcasecmp() but whatever)
    char input_lower[50] = {0};
    for (int i = 0; input[i] && i < 49; i++) {
        input_lower[i] = tolower((unsigned char) input[i]);
    }
    const int input_length = strlen(input_lower);

    int best_index = -1;
    int best_score = -1;

    // Check each menu item
    for (int i = 0; i < (int) length; i++) {
        char menu_word[64] = {0};
        extract_menu_word(list[i], menu_word, sizeof(menu_word));

        const int score = calculate_match_score(input_lower, input_length, menu_word);

        // Update best if this score is better
        // In the case that they are equal, let's just use the earlier menu item
        if (score > best_score || (score == best_score && i < best_index)) {
            best_score = score;
            best_index = i;
        }
    }

    return best_index;
}


/**
 * Helper method to pass a MenuList in directly
 * @param menu The MenuList to use
 * @param input String that is less than 50 chars
 * @return Index of the most suitable option of the MenuList
 */
int get_suitable_option_from_menu_list(const struct MenuList *menu, const char *input) {
    return get_suitable_option_from_list(menu->entries, menu->size, input);
}

/**
 * @brief Wrapper to handle delete flow, we need to ask for some information to ensure the person owns the account
 */
void delete_page() {
    printf("Are you sure you would like to delete your Account? This action cannot be undone!\n");
    // Should I make it so that they can cancel at any step in the process
    while (1) {
        printf("Enter your Account Number: \n");
        char *account_number = get_input();

        if (strcasecmp(account_number, "cancel") == 0) {
            if (account_number) free(account_number);
            main_menu();
            return;
        }
        if (strcmp(account_number, current_account->account_number) == 0) {
            if (account_number) free(account_number);
            break;
        }
        if (account_number) free(account_number);
        printf("Invalid Account Number! Try again, or type 'cancel' to return.\n");
    }

    while (1) {
        printf("Enter the last 4 digits of your ID: \n");
        char *input = get_input();
        const size_t len = strlen(current_account->id);
        const char *last_four = &current_account->id[len - 4];

        if (strcasecmp(input, "cancel") == 0) {
            if (input) free(input);
            main_menu();
            return;
        }
        // Don't think we need to check for non-digits or inputs that aren't within the specified length as we can just check with the current account,
        // and current account will always have valid fields
        if (strcmp(input, last_four) == 0) {
            if (input) free(input);
            break;
        }

        if (input) free(input);
        printf("Invalid ID! Try again, or type 'cancel' to return.\n");
    }

    while (1) {
        printf("Enter your 4-digit PIN:\n");
        char *pin = get_input();

        if (strcasecmp(pin, "cancel") == 0) {
            if (pin) free(pin);
            main_menu();
            return;
        }
        if (strcmp(pin, current_account->pin) == 0) {
            if (pin) free(pin);
            break;
        }

        if (pin) free(pin);
        printf("Invalid PIN! Try again, or type 'cancel' to return.\n");
    }

    const ErrorCode code = delete_account(current_account);
    if (code == SUCCESS) {
        printf("Successfully deleted your Account!\n");
        current_account = NULL;
    } else handle_error_message(code);

    main_menu();
}

/**
 * @brief Wrapper to handle deposit flow
 */
void deposit_page() {
    printf("Enter the amount you would like to Deposit (Must be more than 0 and less than or equal to 50,000): \n");
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
    printf("Current Balance: %.2f\n", current_account->balance);
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

    if (db_res.count == 1) {
        printf("There is only 1 account in the database, unable to proceed with Remittance.\n");
        main_menu();
        return;
    }

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


    printf("Transferable balance: %.2f out of %.2f\n",
           get_max_transferable(current_account, recipient), current_account->balance);
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
    char *id;
    char *name;

    while (1) {
        printf("Enter your Name:\n");
        name = get_input();
        if (name == NULL) {
            free(name);
            continue;
        }
        const ErrorCode code = is_valid_name(name);
        if (code == SUCCESS)
            break;
        handle_error_message(code);
    }
    strcpy(acc->name, name);

    int option;
    while (1) {
        printf("Enter your account type (Savings/Current):\n");
        char *account_type_string = get_input();
        if (account_type_string == NULL) {
            free(account_type_string);
            continue;
        }
        option = get_suitable_option_from_list(account_types, NUM_ACCOUNT_TYPES, account_type_string);

        if (option == -1) {
            printf("Please enter a valid account type (Savings/Current):\n");
        } else {
            break;
        }
    }

    while (1) {
        printf("Enter your 10-digit ID:\n");
        id = get_input();
        if (id == NULL) continue;;
        const ErrorCode code = is_valid_id(id);
        if (code == SUCCESS)
            break;
        handle_error_message(code);
    }
    strcpy(acc->id, id);

    const enum AccountType account_type = (enum AccountType) option;
    acc->account_type = account_type;

    while (1) {
        printf("Enter your 4-digit PIN:\n");
        pin = get_input();
        if (pin == NULL) {
            free(pin);
            continue;
        }
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
               acc->id, acc->account_number, acc->name, (int *) &acc->account_type, acc->pin,
               &acc->date_created, &acc->balance) != 7) {
        return ERR_MALFORMED_FILE;
    }
    return SUCCESS;
}

struct BankAccount *get_account_from_account_number(char *account_number) {
    if (!account_number || account_number[0] == '\0') return NULL;
    struct BankAccount *acc = malloc(sizeof *acc);
    if (!acc) return NULL;
    char path[256];
    const int len = snprintf(path, sizeof(path), "./database/%s.txt", account_number);
    if (len < 0 || len >= sizeof(path)) {
        printf("Error: Path too long for ID: %s\n", account_number);
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
        // printf("db_result.accounts[i].id = %s, id = %s\n", db_result.accounts[i].id, id);
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
    struct BankAccount *from_number = get_account_from_account_number(identifier);
    struct BankAccount *from_id = get_account_from_id(identifier);

    if (from_name != NULL) {
        return from_name;
    }
    if (from_number != NULL) {
        return from_number;
    }
    if (from_id != NULL) {
        return from_id;
    }

    return NULL;
}

/**
 * Performs the actual login process
 * @param identifier The Identifier, this could be an Account Number, Name or ID
 * @param pin PIN
 * @return
 * @p ERR_ACCOUNT_NOT_FOUND If there was no matching account \n
 * @p ERR_INVALID_PIN If the pin was invalid \n
 * @p SUCCESS If successful, @p current_account is updated
 */
ErrorCode actually_login(char *identifier, const char *pin) {
    if (pin == NULL) return ERR_INVALID_PIN_FORMAT;

    struct BankAccount *acc = get_account_from_identifier(identifier);

    if (acc == NULL) {
        return ERR_ACCOUNT_NOT_FOUND;
    }

    if (is_valid_pin(pin) != SUCCESS) {
        free(acc);
        return ERR_INVALID_PIN;
    }
    current_account = acc;
    return SUCCESS;
}

/**
 * @brief Prompts and validates for a correct identifier, usually to be passed into get_bank_account_from_identifier()
 * @return The validated identifier
 */
char *get_valid_identifier() {
    while (1) {
        char *input = get_input();
        if (!input) continue;

        const int valid_name = is_valid_name(input) == SUCCESS;
        const int valid_number = is_valid_account_number(input) == SUCCESS;
        const int valid_id = is_valid_id(input) == SUCCESS;

        if (valid_number) return input;
        if (valid_name && is_distinct_name(input)) return input;
        if (valid_id && is_distinct_id(input)) return input;

        if (valid_name) {
            printf("Multiple accounts with this name. Enter ID, Account Number, or different name: \n");
        } else if (valid_id) {
            printf("Multiple accounts with this ID. Enter your Account Number or Account Name: \n");
        } else {
            printf("Invalid input. Enter Account Number (7-9 digits), ID (10 digits), or name: \n");
        }

        free(input);
    }
}

/**
 * Wrapper for login flow
 */
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

    // Abstracted to get_valid_identifier();

    char *identifier = get_valid_identifier();

    printf("Enter your 4-Digit PIN:\n");
    char *pin = get_input();

    const ErrorCode code = actually_login(identifier, pin);
    if (code == SUCCESS) {
        printf("Successfully logged in into %s!\n", current_account->name);
    } else handle_error_message(code);

    main_menu();
    free(identifier);
    free(pin);
}

/**
 * @brief Main main-menu wrapper that handles input when both logged-in and logged-out
 */
void main_menu() {
    print_divider_thin();
    print_login_details();
    print_divider_thin();

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

/**
 * @brief Wrapper for logout logic
 * @remarks CLion says there are memory leaks, but I think they are false positives
 */
void logout_page() {
    printf("Are you sure you would like to Logout? (y/n)\n");
    char *input = get_input();

    if (strcasecmp(input, "yes\n") == 0 || strcasecmp(input, "y\n") == 0 ||
        strcmp(input, "yes") == 0 || strcmp(input, "y") == 0) {
        current_account = NULL;
        printf("Logged out successfully!\n");
        if (input) free(input);
        main_menu();
        // ReSharper disable once CppDFAMemoryLeak
        return;
    }
    if (strcasecmp(input, "no\n") == 0 || strcasecmp(input, "n\n") == 0 ||
        strcmp(input, "no") == 0 || strcmp(input, "n") == 0) {
        if (input) free(input);
        main_menu();
        // ReSharper disable once CppDFAMemoryLeak
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
