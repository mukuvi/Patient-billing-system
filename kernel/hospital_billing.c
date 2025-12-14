#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

// Database connection
sqlite3* db = NULL;

// Function prototypes
void init_database();
void close_database();
int authenticate();
void get_password(char *password, size_t size);
void clear_screen();
void display_main_menu();

// Patient functions
void add_patient();
void view_patients();
void search_patient();
void update_patient();
void delete_patient();

// Billing functions
void generate_bill();
void view_bills();
void search_bill();
void make_payment();
void view_payment_history();
void print_receipt();

// Report functions
void generate_report();
void view_statistics();

// System functions
void backup_database();
void restore_database();
void export_data();

// Utility functions
void print_header(const char *title);
int get_choice(int min, int max);
void get_string(const char *prompt, char *buffer, size_t size);
int get_integer(const char *prompt, int min, int max);
float get_float(const char *prompt, float min, float max);

int main() {
    printf("\n========================================\n");
    printf("   HOSPITAL PATIENT BILLING SYSTEM\n");
    printf("========================================\n");
    
    // Initialize database
    init_database();
    
    // Authenticate user
    if (!authenticate()) {
        printf("\nAccess denied! Exiting...\n");
        return 1;
    }
    
    // Main program loop
    int running = 1;
    while (running) {
        display_main_menu();
        int choice = get_choice(0, 16);
        
        switch(choice) {
            case 1: add_patient(); break;
            case 2: view_patients(); break;
            case 3: search_patient(); break;
            case 4: update_patient(); break;
            case 5: delete_patient(); break;
            case 6: generate_bill(); break;
            case 7: view_bills(); break;
            case 8: search_bill(); break;
            case 9: make_payment(); break;
            case 10: view_payment_history(); break;
            case 11: print_receipt(); break;
            case 12: generate_report(); break;
            case 13: view_statistics(); break;
            case 14: backup_database(); break;
            case 15: restore_database(); break;
            case 16: export_data(); break;
            case 0: 
                printf("\nThank you for using Hospital Billing System!\n");
                running = 0;
                break;
        }
    }
    
    close_database();
    return 0;
}

// ==================== DATABASE FUNCTIONS ====================

void init_database() {
    int rc = sqlite3_open("hospital.db", &db);
    if (rc != SQLITE_OK) {
        printf("Cannot open database: %s\n", sqlite3_errmsg(db));
        exit(1);
    }
    
    // Create tables
    const char *sql = 
        "CREATE TABLE IF NOT EXISTS users ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    username TEXT UNIQUE NOT NULL,"
        "    password TEXT NOT NULL,"
        "    role TEXT DEFAULT 'staff'"
        ");"
        
        "CREATE TABLE IF NOT EXISTS patients ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    name TEXT NOT NULL,"
        "    age INTEGER,"
        "    gender TEXT,"
        "    contact TEXT,"
        "    address TEXT,"
        "    disease TEXT,"
        "    admission_date DATE DEFAULT CURRENT_DATE,"
        "    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ");"
        
        "CREATE TABLE IF NOT EXISTS bills ("
        "    bill_no INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    patient_id INTEGER,"
        "    patient_name TEXT,"
        "    bill_date DATE DEFAULT CURRENT_DATE,"
        "    room_charges REAL DEFAULT 0,"
        "    doctor_fees REAL DEFAULT 0,"
        "    medicine_charges REAL DEFAULT 0,"
        "    lab_charges REAL DEFAULT 0,"
        "    other_charges REAL DEFAULT 0,"
        "    total_amount REAL DEFAULT 0,"
        "    amount_paid REAL DEFAULT 0,"
        "    balance_due REAL DEFAULT 0,"
        "    payment_status TEXT DEFAULT 'Pending',"
        "    payment_method TEXT,"
        "    FOREIGN KEY (patient_id) REFERENCES patients(id)"
        ");"
        
        "CREATE TABLE IF NOT EXISTS payments ("
        "    payment_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    bill_no INTEGER,"
        "    amount REAL,"
        "    payment_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "    payment_method TEXT,"
        "    FOREIGN KEY (bill_no) REFERENCES bills(bill_no)"
        ");";
    
    char *err_msg = 0;
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        printf("SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    
    // Insert default admin user if not exists
    sql = "INSERT OR IGNORE INTO users (username, password, role) VALUES "
          "('admin', 'admin123', 'admin'),"
          "('staff', 'staff123', 'staff');";
    sqlite3_exec(db, sql, 0, 0, 0);
    
    printf("Database initialized successfully!\n");
}

void close_database() {
    if (db) {
        sqlite3_close(db);
    }
}

// ==================== AUTHENTICATION ====================

int authenticate() {
    char username[50];
    char password[50];
    
    printf("\n=== LOGIN ===\n");
    printf("Default credentials:\n");
    printf("  Admin:  username=admin, password=admin123\n");
    printf("  Staff:  username=staff, password=staff123\n\n");
    
    printf("Username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;
    
    printf("Password: ");
    get_password(password, sizeof(password));
    
    // Query database for user
    char sql[200];
    snprintf(sql, sizeof(sql), 
             "SELECT role FROM users WHERE username = '%s' AND password = '%s'", 
             username, password);
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    if (rc != SQLITE_OK) {
        printf("Authentication error\n");
        return 0;
    }
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *role = (const char*)sqlite3_column_text(stmt, 0);
        printf("\nWelcome, %s! (Role: %s)\n", username, role);
        sqlite3_finalize(stmt);
        return 1;
    }
    
    sqlite3_finalize(stmt);
    printf("\nInvalid username or password!\n");
    return 0;
}

void get_password(char *password, size_t size) {
    struct termios oldt, newt;
    int i = 0;
    char ch;
    
    // Get current terminal settings
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    
    // Disable echo
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    // Read password
    while (i < size - 1) {
        ch = getchar();
        if (ch == '\n' || ch == '\r') {
            break;
        }
        password[i++] = ch;
        printf("*");
    }
    password[i] = '\0';
    
    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
}

// ==================== UTILITY FUNCTIONS ====================

void clear_screen() {
    printf("\033[2J\033[1;1H");
}

void print_header(const char *title) {
    printf("\n════════════════════════════════════════════════════\n");
    printf("                     %s\n", title);
    printf("════════════════════════════════════════════════════\n");
}

int get_choice(int min, int max) {
    int choice;
    char input[10];
    
    while (1) {
        printf("\nEnter choice (%d-%d, 0 to exit): ", min, max);
        if (fgets(input, sizeof(input), stdin) != NULL) {
            if (sscanf(input, "%d", &choice) == 1) {
                if (choice == 0 || (choice >= min && choice <= max)) {
                    return choice;
                }
            }
        }
        printf("Invalid choice! Please try again.\n");
    }
}

void get_string(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (fgets(buffer, size, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
    }
}

int get_integer(const char *prompt, int min, int max) {
    int value;
    char input[20];
    
    while (1) {
        printf("%s", prompt);
        if (fgets(input, sizeof(input), stdin) != NULL) {
            if (sscanf(input, "%d", &value) == 1) {
                if (value >= min && value <= max) {
                    return value;
                }
            }
        }
        printf("Please enter a number between %d and %d.\n", min, max);
    }
}

float get_float(const char *prompt, float min, float max) {
    float value;
    char input[20];
    
    while (1) {
        printf("%s", prompt);
        if (fgets(input, sizeof(input), stdin) != NULL) {
            if (sscanf(input, "%f", &value) == 1) {
                if (value >= min && value <= max) {
                    return value;
                }
            }
        }
        printf("Please enter a number between %.2f and %.2f.\n", min, max);
    }
}

// ==================== MAIN MENU ====================

void display_main_menu() {
    clear_screen();
    printf("\n════════════════════════════════════════════════════\n");
    printf("           HOSPITAL BILLING SYSTEM - MAIN MENU       \n");
    printf("════════════════════════════════════════════════════\n");
    printf("   1.  Add New Patient\n");
    printf("   2.  View All Patients\n");
    printf("   3.  Search Patient\n");
    printf("   4.  Update Patient Information\n");
    printf("   5.  Delete Patient Record\n");
    printf("   6.  Generate New Bill\n");
    printf("   7.  View All Bills\n");
    printf("   8.  Search Bill\n");
    printf("   9.  Make Payment\n");
    printf("   10. View Payment History\n");
    printf("   11. Print Receipt\n");
    printf("   12. Generate Financial Report\n");
    printf("   13. View Statistics\n");
    printf("   14. Backup Database\n");
    printf("   15. Restore Database\n");
    printf("   16. Export Data\n");
    printf("\n   0.  Exit\n");
}

// ==================== PATIENT MANAGEMENT ====================

void add_patient() {
    clear_screen();
    print_header("ADD NEW PATIENT");
    
    char name[100], gender[10], contact[20], address[200], disease[100];
    int age;
    char admission_date[11];
    
    get_string("Patient Name: ", name, sizeof(name));
    age = get_integer("Age: ", 1, 120);
    
    printf("Gender (M/F/O): ");
    fgets(gender, sizeof(gender), stdin);
    gender[strcspn(gender, "\n")] = 0;
    
    get_string("Contact Number: ", contact, sizeof(contact));
    get_string("Address: ", address, sizeof(address));
    get_string("Disease/Diagnosis: ", disease, sizeof(disease));
    
    printf("Admission Date (YYYY-MM-DD, enter for today): ");
    fgets(admission_date, sizeof(admission_date), stdin);
    admission_date[strcspn(admission_date, "\n")] = 0;
    
    if (strlen(admission_date) == 0) {
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        strftime(admission_date, sizeof(admission_date), "%Y-%m-%d", tm);
    }
    
    // Insert into database
    char sql[500];
    snprintf(sql, sizeof(sql),
        "INSERT INTO patients (name, age, gender, contact, address, disease, admission_date) "
        "VALUES ('%s', %d, '%s', '%s', '%s', '%s', '%s')",
        name, age, gender, contact, address, disease, admission_date);
    
    char *err_msg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK) {
        printf("\n❌ Error adding patient: %s\n", err_msg);
        sqlite3_free(err_msg);
    } else {
        printf("\n✅ Patient added successfully!\n");
        printf("   Patient ID: %lld\n", sqlite3_last_insert_rowid(db));
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

void view_patients() {
    clear_screen();
    print_header("ALL PATIENTS");
    
    const char *sql = "SELECT id, name, age, gender, contact, admission_date FROM patients ORDER BY name";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        printf("Error fetching patients\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    printf("ID   Name                          Age Gender Contact       Admission\n");
    printf("══════════════════════════════════════════════════════════════════════\n");
    
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
        int id = sqlite3_column_int(stmt, 0);
        const char *name = (const char*)sqlite3_column_text(stmt, 1);
        int age = sqlite3_column_int(stmt, 2);
        const char *gender = (const char*)sqlite3_column_text(stmt, 3);
        const char *contact = (const char*)sqlite3_column_text(stmt, 4);
        const char *admission_date = (const char*)sqlite3_column_text(stmt, 5);
        
        printf("%-4d %-30s %-3d %-6s %-12s %s\n", 
               id, name, age, gender, contact, admission_date);
    }
    
    sqlite3_finalize(stmt);
    
    if (count == 0) {
        printf("No patients found.\n");
    } else {
        printf("\nTotal patients: %d\n", count);
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

void search_patient() {
    clear_screen();
    print_header("SEARCH PATIENT");
    
    printf("Search by:\n");
    printf("1. Name\n");
    printf("2. Contact\n");
    printf("3. Patient ID\n");
    printf("Enter choice: ");
    
    int choice = get_choice(1, 3);
    char search_term[100];
    
    printf("Enter search term: ");
    fgets(search_term, sizeof(search_term), stdin);
    search_term[strcspn(search_term, "\n")] = 0;
    
    char sql[500];
    if (choice == 1) {
        snprintf(sql, sizeof(sql), 
                 "SELECT * FROM patients WHERE name LIKE '%%%s%%' ORDER BY name", search_term);
    } else if (choice == 2) {
        snprintf(sql, sizeof(sql), 
                 "SELECT * FROM patients WHERE contact LIKE '%%%s%%'", search_term);
    } else {
        snprintf(sql, sizeof(sql), 
                 "SELECT * FROM patients WHERE id = %s", search_term);
    }
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        printf("Search failed\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    printf("\nSearch Results:\n");
    printf("════════════════════════════════════════════════════\n");
    
    int found = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        found = 1;
        int id = sqlite3_column_int(stmt, 0);
        const char *name = (const char*)sqlite3_column_text(stmt, 1);
        int age = sqlite3_column_int(stmt, 2);
        const char *gender = (const char*)sqlite3_column_text(stmt, 3);
        const char *contact = (const char*)sqlite3_column_text(stmt, 4);
        const char *address = (const char*)sqlite3_column_text(stmt, 5);
        const char *disease = (const char*)sqlite3_column_text(stmt, 6);
        const char *admission_date = (const char*)sqlite3_column_text(stmt, 7);
        
        printf("\nPatient ID: %d\n", id);
        printf("Name: %s\n", name);
        printf("Age: %d | Gender: %s\n", age, gender);
        printf("Contact: %s\n", contact);
        printf("Address: %s\n", address);
        printf("Disease: %s\n", disease);
        printf("Admission Date: %s\n", admission_date);
        printf("────────────────────────────────────────────────\n");
    }
    
    sqlite3_finalize(stmt);
    
    if (!found) {
        printf("No patients found.\n");
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

void update_patient() {
    clear_screen();
    print_header("UPDATE PATIENT");
    
    int patient_id = get_integer("Enter Patient ID to update: ", 1, 99999);
    
    // First, get current patient info
    char sql[500];
    snprintf(sql, sizeof(sql), "SELECT * FROM patients WHERE id = %d", patient_id);
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK || 
        sqlite3_step(stmt) != SQLITE_ROW) {
        printf("Patient not found!\n");
        sqlite3_finalize(stmt);
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    const char *current_name = (const char*)sqlite3_column_text(stmt, 1);
    int current_age = sqlite3_column_int(stmt, 2);
    const char *current_gender = (const char*)sqlite3_column_text(stmt, 3);
    const char *current_contact = (const char*)sqlite3_column_text(stmt, 4);
    const char *current_address = (const char*)sqlite3_column_text(stmt, 5);
    const char *current_disease = (const char*)sqlite3_column_text(stmt, 6);
    
    sqlite3_finalize(stmt);
    
    printf("\nCurrent Information:\n");
    printf("Name: %s\n", current_name);
    printf("Age: %d\n", current_age);
    printf("Gender: %s\n", current_gender);
    printf("Contact: %s\n", current_contact);
    printf("Address: %s\n", current_address);
    printf("Disease: %s\n", current_disease);
    
    printf("\nEnter new information (press Enter to keep current):\n");
    
    char name[100], gender[10], contact[20], address[200], disease[100];
    int age;
    char input[100];
    
    printf("Name [%s]: ", current_name);
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    strcpy(name, strlen(input) > 0 ? input : current_name);
    
    printf("Age [%d]: ", current_age);
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    age = strlen(input) > 0 ? atoi(input) : current_age;
    
    printf("Gender [%s]: ", current_gender);
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    strcpy(gender, strlen(input) > 0 ? input : current_gender);
    
    printf("Contact [%s]: ", current_contact);
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    strcpy(contact, strlen(input) > 0 ? input : current_contact);
    
    printf("Address [%s]: ", current_address);
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    strcpy(address, strlen(input) > 0 ? input : current_address);
    
    printf("Disease [%s]: ", current_disease);
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    strcpy(disease, strlen(input) > 0 ? input : current_disease);
    
    // Update database
    snprintf(sql, sizeof(sql),
        "UPDATE patients SET name = '%s', age = %d, gender = '%s', "
        "contact = '%s', address = '%s', disease = '%s' "
        "WHERE id = %d",
        name, age, gender, contact, address, disease, patient_id);
    
    char *err_msg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK) {
        printf("\n❌ Error updating patient: %s\n", err_msg);
        sqlite3_free(err_msg);
    } else {
        printf("\n✅ Patient updated successfully!\n");
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

void delete_patient() {
    clear_screen();
    print_header("DELETE PATIENT");
    
    int patient_id = get_integer("Enter Patient ID to delete: ", 1, 99999);
    
    // Check if patient exists
    char sql[500];
    snprintf(sql, sizeof(sql), "SELECT name FROM patients WHERE id = %d", patient_id);
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK || 
        sqlite3_step(stmt) != SQLITE_ROW) {
        printf("Patient not found!\n");
        sqlite3_finalize(stmt);
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    const char *patient_name = (const char*)sqlite3_column_text(stmt, 0);
    sqlite3_finalize(stmt);
    
    printf("\nPatient: %s (ID: %d)\n", patient_name, patient_id);
    printf("WARNING: This will delete the patient and all associated bills!\n");
    printf("Are you sure? (y/n): ");
    
    char confirm;
    scanf(" %c", &confirm);
    getchar();
    
    if (confirm != 'y' && confirm != 'Y') {
        printf("Deletion cancelled.\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Delete patient (cascade will delete bills)
    snprintf(sql, sizeof(sql), "DELETE FROM patients WHERE id = %d", patient_id);
    
    char *err_msg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK) {
        printf("\n❌ Error deleting patient: %s\n", err_msg);
        sqlite3_free(err_msg);
    } else {
        printf("\n✅ Patient deleted successfully!\n");
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

// ==================== BILLING FUNCTIONS ====================

void generate_bill() {
    clear_screen();
    print_header("GENERATE BILL");
    
    // Show patients
    view_patients();
    
    int patient_id = get_integer("\nEnter Patient ID for billing: ", 1, 99999);
    
    // Get patient name
    char sql[500];
    snprintf(sql, sizeof(sql), "SELECT name FROM patients WHERE id = %d", patient_id);
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK || 
        sqlite3_step(stmt) != SQLITE_ROW) {
        printf("Patient not found!\n");
        sqlite3_finalize(stmt);
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    const char *patient_name = (const char*)sqlite3_column_text(stmt, 0);
    sqlite3_finalize(stmt);
    
    printf("\nGenerating bill for: %s (ID: %d)\n", patient_name, patient_id);
    printf("════════════════════════════════════════════════════\n");
    
    float room_charges = get_float("Room charges: $", 0, 10000);
    float doctor_fees = get_float("Doctor fees: $", 0, 10000);
    float medicine_charges = get_float("Medicine charges: $", 0, 10000);
    float lab_charges = get_float("Lab charges: $", 0, 10000);
    float other_charges = get_float("Other charges: $", 0, 10000);
    
    float total_amount = room_charges + doctor_fees + medicine_charges + lab_charges + other_charges;
    
    printf("\nTotal Amount: $%.2f\n", total_amount);
    
    printf("\nPayment Status:\n");
    printf("1. Paid\n");
    printf("2. Pending\n");
    printf("3. Partial\n");
    printf("Enter choice: ");
    
    int status_choice = get_choice(1, 3);
    float amount_paid = 0;
    char payment_status[20];
    char payment_method[20] = "Cash";
    
    if (status_choice == 1) {
        strcpy(payment_status, "Paid");
        amount_paid = total_amount;
    } else if (status_choice == 3) {
        strcpy(payment_status, "Partial");
        amount_paid = get_float("Amount paid now: $", 0, total_amount);
    } else {
        strcpy(payment_status, "Pending");
    }
    
    if (amount_paid > 0) {
        printf("\nPayment Method:\n");
        printf("1. Cash\n");
        printf("2. Credit Card\n");
        printf("3. Debit Card\n");
        printf("4. Online Transfer\n");
        printf("Enter choice: ");
        
        int method_choice = get_choice(1, 4);
        switch(method_choice) {
            case 1: strcpy(payment_method, "Cash"); break;
            case 2: strcpy(payment_method, "Credit Card"); break;
            case 3: strcpy(payment_method, "Debit Card"); break;
            case 4: strcpy(payment_method, "Online Transfer"); break;
        }
    }
    
    float balance_due = total_amount - amount_paid;
    
    // Insert bill
    snprintf(sql, sizeof(sql),
        "INSERT INTO bills (patient_id, patient_name, room_charges, doctor_fees, "
        "medicine_charges, lab_charges, other_charges, total_amount, amount_paid, "
        "balance_due, payment_status, payment_method) "
        "VALUES (%d, '%s', %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, '%s', '%s')",
        patient_id, patient_name, room_charges, doctor_fees, medicine_charges,
        lab_charges, other_charges, total_amount, amount_paid, balance_due,
        payment_status, payment_method);
    
    char *err_msg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK) {
        printf("\n❌ Error generating bill: %s\n", err_msg);
        sqlite3_free(err_msg);
    } else {
        long long bill_no = sqlite3_last_insert_rowid(db);
        printf("\n✅ Bill generated successfully!\n");
        printf("   Bill Number: %lld\n", bill_no);
        printf("   Patient: %s\n", patient_name);
        printf("   Total Amount: $%.2f\n", total_amount);
        printf("   Amount Paid: $%.2f\n", amount_paid);
        printf("   Balance Due: $%.2f\n", balance_due);
        printf("   Status: %s\n", payment_status);
        
        // Record payment if any
        if (amount_paid > 0) {
            snprintf(sql, sizeof(sql),
                "INSERT INTO payments (bill_no, amount, payment_method) "
                "VALUES (%lld, %.2f, '%s')",
                bill_no, amount_paid, payment_method);
            sqlite3_exec(db, sql, 0, 0, 0);
        }
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

void view_bills() {
    clear_screen();
    print_header("ALL BILLS");
    
    const char *sql = "SELECT bill_no, patient_name, total_amount, amount_paid, "
                     "balance_due, payment_status, bill_date FROM bills ORDER BY bill_no DESC";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        printf("Error fetching bills\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    printf("Bill No  Patient Name               Total      Paid       Balance    Status     Date\n");
    printf("════════════════════════════════════════════════════════════════════════════════════\n");
    
    int count = 0;
    float total_billed = 0, total_paid = 0, total_outstanding = 0;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
        int bill_no = sqlite3_column_int(stmt, 0);
        const char *patient_name = (const char*)sqlite3_column_text(stmt, 1);
        float total_amount = sqlite3_column_double(stmt, 2);
        float amount_paid = sqlite3_column_double(stmt, 3);
        float balance_due = sqlite3_column_double(stmt, 4);
        const char *payment_status = (const char*)sqlite3_column_text(stmt, 5);
        const char *bill_date = (const char*)sqlite3_column_text(stmt, 6);
        
        printf("%-8d %-25s $%-9.2f $%-9.2f $%-9.2f %-10s %s\n", 
               bill_no, patient_name, total_amount, amount_paid, balance_due, payment_status, bill_date);
        
        total_billed += total_amount;
        total_paid += amount_paid;
        total_outstanding += balance_due;
    }
    
    sqlite3_finalize(stmt);
    
    if (count == 0) {
        printf("No bills found.\n");
    } else {
        printf("\nSummary:\n");
        printf("  Total Bills:        %d\n", count);
        printf("  Total Billed:       $%.2f\n", total_billed);
        printf("  Total Paid:         $%.2f\n", total_paid);
        printf("  Total Outstanding:  $%.2f\n", total_outstanding);
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

void search_bill() {
    clear_screen();
    print_header("SEARCH BILL");
    
    int bill_no = get_integer("Enter Bill Number: ", 1, 999999);
    
    char sql[500];
    snprintf(sql, sizeof(sql), "SELECT * FROM bills WHERE bill_no = %d", bill_no);
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK || 
        sqlite3_step(stmt) != SQLITE_ROW) {
        printf("Bill not found!\n");
        sqlite3_finalize(stmt);
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    int patient_id = sqlite3_column_int(stmt, 1);
    const char *patient_name = (const char*)sqlite3_column_text(stmt, 2);
    const char *bill_date = (const char*)sqlite3_column_text(stmt, 3);
    float room_charges = sqlite3_column_double(stmt, 4);
    float doctor_fees = sqlite3_column_double(stmt, 5);
    float medicine_charges = sqlite3_column_double(stmt, 6);
    float lab_charges = sqlite3_column_double(stmt, 7);
    float other_charges = sqlite3_column_double(stmt, 8);
    float total_amount = sqlite3_column_double(stmt, 9);
    float amount_paid = sqlite3_column_double(stmt, 10);
    float balance_due = sqlite3_column_double(stmt, 11);
    const char *payment_status = (const char*)sqlite3_column_text(stmt, 12);
    const char *payment_method = (const char*)sqlite3_column_text(stmt, 13);
    
    sqlite3_finalize(stmt);
    
    printf("\nBill Details:\n");
    printf("════════════════════════════════════════════════════\n");
    printf("Bill No: %d | Date: %s\n", bill_no, bill_date);
    printf("Patient: %s (ID: %d)\n", patient_name, patient_id);
    printf("════════════════════════════════════════════════════\n");
    printf("Room Charges:        $%10.2f\n", room_charges);
    printf("Doctor Fees:         $%10.2f\n", doctor_fees);
    printf("Medicine Charges:    $%10.2f\n", medicine_charges);
    printf("Lab Charges:         $%10.2f\n", lab_charges);
    printf("Other Charges:       $%10.2f\n", other_charges);
    printf("════════════════════════════════════════════════════\n");
    printf("TOTAL AMOUNT:        $%10.2f\n", total_amount);
    printf("Amount Paid:         $%10.2f\n", amount_paid);
    printf("Balance Due:         $%10.2f\n", balance_due);
    printf("════════════════════════════════════════════════════\n");
    printf("Payment Status:      %s\n", payment_status);
    printf("Payment Method:      %s\n", payment_method);
    
    printf("\nPress Enter to continue...");
    getchar();
}

void make_payment() {
    clear_screen();
    print_header("MAKE PAYMENT");
    
    // Show pending bills
    const char *sql = "SELECT bill_no, patient_name, total_amount, amount_paid, "
                     "balance_due FROM bills WHERE balance_due > 0 ORDER BY bill_no";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        printf("Error fetching bills\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    printf("Pending Bills:\n");
    printf("Bill No  Patient Name               Total      Paid       Balance\n");
    printf("═════════════════════════════════════════════════════════════════\n");
    
    int bills[100];
    float balances[100];
    int bill_count = 0;
    
    while (sqlite3_step(stmt) == SQLITE_ROW && bill_count < 100) {
        int bill_no = sqlite3_column_int(stmt, 0);
        const char *patient_name = (const char*)sqlite3_column_text(stmt, 1);
        float total_amount = sqlite3_column_double(stmt, 2);
        float amount_paid = sqlite3_column_double(stmt, 3);
        float balance_due = sqlite3_column_double(stmt, 4);
        
        printf("%-8d %-25s $%-9.2f $%-9.2f $%-9.2f\n", 
               bill_no, patient_name, total_amount, amount_paid, balance_due);
        
        bills[bill_count] = bill_no;
        balances[bill_count] = balance_due;
        bill_count++;
    }
    
    sqlite3_finalize(stmt);
    
    if (bill_count == 0) {
        printf("No pending bills found.\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    int bill_no = get_integer("\nEnter Bill Number to pay: ", 1, 999999);
    
    // Find the bill
    int found = 0;
    float max_payment = 0;
    for (int i = 0; i < bill_count; i++) {
        if (bills[i] == bill_no) {
            found = 1;
            max_payment = balances[i];
            break;
        }
    }
    
    if (!found) {
        printf("Bill not found or already paid!\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    printf("Maximum payment allowed: $%.2f\n", max_payment);
    float payment_amount = get_float("Enter payment amount: $", 0.01, max_payment);
    
    printf("\nPayment Method:\n");
    printf("1. Cash\n");
    printf("2. Credit Card\n");
    printf("3. Debit Card\n");
    printf("4. Online Transfer\n");
    printf("Enter choice: ");
    
    int method_choice = get_choice(1, 4);
    char payment_method[20];
    
    switch(method_choice) {
        case 1: strcpy(payment_method, "Cash"); break;
        case 2: strcpy(payment_method, "Credit Card"); break;
        case 3: strcpy(payment_method, "Debit Card"); break;
        case 4: strcpy(payment_method, "Online Transfer"); break;
    }
    
    // Update bill
    char update_sql[500];
    snprintf(update_sql, sizeof(update_sql),
        "UPDATE bills SET amount_paid = amount_paid + %.2f, "
        "balance_due = balance_due - %.2f WHERE bill_no = %d",
        payment_amount, payment_amount, bill_no);
    
    char *err_msg = 0;
    int rc = sqlite3_exec(db, update_sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK) {
        printf("Payment failed: %s\n", err_msg);
        sqlite3_free(err_msg);
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Update status if fully paid
    snprintf(update_sql, sizeof(update_sql),
        "UPDATE bills SET payment_status = CASE "
        "WHEN balance_due <= 0 THEN 'Paid' "
        "ELSE 'Partial' END WHERE bill_no = %d",
        bill_no);
    
    sqlite3_exec(db, update_sql, 0, 0, 0);
    
    // Record payment
    snprintf(update_sql, sizeof(update_sql),
        "INSERT INTO payments (bill_no, amount, payment_method) "
        "VALUES (%d, %.2f, '%s')",
        bill_no, payment_amount, payment_method);
    
    sqlite3_exec(db, update_sql, 0, 0, 0);
    
    printf("\n✅ Payment of $%.2f recorded successfully!\n", payment_amount);
    
    printf("\nPress Enter to continue...");
    getchar();
}

void view_payment_history() {
    clear_screen();
    print_header("PAYMENT HISTORY");
    
    int bill_no = get_integer("Enter Bill Number (0 for all payments): ", 0, 999999);
    
    char sql[500];
    if (bill_no == 0) {
        strcpy(sql, "SELECT p.payment_id, p.bill_no, b.patient_name, p.amount, "
                    "p.payment_method, p.payment_date "
                    "FROM payments p JOIN bills b ON p.bill_no = b.bill_no "
                    "ORDER BY p.payment_date DESC");
    } else {
        snprintf(sql, sizeof(sql), 
                 "SELECT p.payment_id, p.bill_no, b.patient_name, p.amount, "
                 "p.payment_method, p.payment_date "
                 "FROM payments p JOIN bills b ON p.bill_no = b.bill_no "
                 "WHERE p.bill_no = %d ORDER BY p.payment_date DESC", bill_no);
    }
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        printf("Error fetching payment history\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    printf("Payment ID  Bill No  Patient Name               Amount     Method        Date\n");
    printf("══════════════════════════════════════════════════════════════════════════════\n");
    
    int count = 0;
    float total_amount = 0;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
        int payment_id = sqlite3_column_int(stmt, 0);
        int bill_no = sqlite3_column_int(stmt, 1);
        const char *patient_name = (const char*)sqlite3_column_text(stmt, 2);
        float amount = sqlite3_column_double(stmt, 3);
        const char *payment_method = (const char*)sqlite3_column_text(stmt, 4);
        const char *payment_date = (const char*)sqlite3_column_text(stmt, 5);
        
        printf("%-10d %-8d %-25s $%-9.2f %-12s %s\n", 
               payment_id, bill_no, patient_name, amount, payment_method, payment_date);
        
        total_amount += amount;
    }
    
    sqlite3_finalize(stmt);
    
    if (count == 0) {
        printf("No payment records found.\n");
    } else {
        printf("\nSummary:\n");
        printf("  Total Payments: %d\n", count);
        printf("  Total Amount:   $%.2f\n", total_amount);
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

void print_receipt() {
    clear_screen();
    print_header("PRINT RECEIPT");
    
    int bill_no = get_integer("Enter Bill Number: ", 1, 999999);
    
    char sql[500];
    snprintf(sql, sizeof(sql), "SELECT * FROM bills WHERE bill_no = %d", bill_no);
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK || 
        sqlite3_step(stmt) != SQLITE_ROW) {
        printf("Bill not found!\n");
        sqlite3_finalize(stmt);
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    int patient_id = sqlite3_column_int(stmt, 1);
    const char *patient_name = (const char*)sqlite3_column_text(stmt, 2);
    const char *bill_date = (const char*)sqlite3_column_text(stmt, 3);
    float room_charges = sqlite3_column_double(stmt, 4);
    float doctor_fees = sqlite3_column_double(stmt, 5);
    float medicine_charges = sqlite3_column_double(stmt, 6);
    float lab_charges = sqlite3_column_double(stmt, 7);
    float other_charges = sqlite3_column_double(stmt, 8);
    float total_amount = sqlite3_column_double(stmt, 9);
    float amount_paid = sqlite3_column_double(stmt, 10);
    float balance_due = sqlite3_column_double(stmt, 11);
    const char *payment_status = (const char*)sqlite3_column_text(stmt, 12);
    const char *payment_method = (const char*)sqlite3_column_text(stmt, 13);
    
    sqlite3_finalize(stmt);
    
    // Print receipt
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    OFFICIAL RECEIPT                          ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ City General Hospital                                        ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  Receipt No: %-45d ║\n", bill_no);
    printf("║  Date:       %-45s ║\n", bill_date);
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  Patient: %-50s ║\n", patient_name);
    printf("║  Patient ID: %-48d ║\n", patient_id);
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║                                                              ║\n");
    printf("║  Room Charges ................................ $%10.2f  ║\n", room_charges);
    printf("║  Doctor Fees ................................. $%10.2f  ║\n", doctor_fees);
    printf("║  Medicine Charges ........................... $%10.2f  ║\n", medicine_charges);
    printf("║  Lab Charges ................................ $%10.2f  ║\n", lab_charges);
    printf("║  Other Charges .............................. $%10.2f  ║\n", other_charges);
    printf("║                                                              ║\n");
    printf("║  TOTAL AMOUNT ............................... $%10.2f  ║\n", total_amount);
    printf("║  AMOUNT PAID ............................... $%10.2f  ║\n", amount_paid);
    printf("║  BALANCE DUE ............................... $%10.2f  ║\n", balance_due);
    printf("║                                                              ║\n");
    printf("║  Payment Status: %-10s                                 ║\n", payment_status);
    printf("║  Payment Method: %-10s                                 ║\n", payment_method);
    printf("║                                                              ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  Thank you for choosing our hospital!                        ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("\nOptions:\n");
    printf("1. Save to file\n");
    printf("2. Print another receipt\n");
    printf("3. Return to menu\n");
    printf("Enter choice: ");
    
    int choice = get_choice(1, 3);
    
    if (choice == 1) {
        char filename[100];
        snprintf(filename, sizeof(filename), "receipt_%d.txt", bill_no);
        FILE *file = fopen(filename, "w");
        if (file) {
            fprintf(file, "Receipt No: %d\n", bill_no);
            fprintf(file, "Date: %s\n", bill_date);
            fprintf(file, "Patient: %s (ID: %d)\n", patient_name, patient_id);
            fprintf(file, "Total Amount: $%.2f\n", total_amount);
            fprintf(file, "Amount Paid: $%.2f\n", amount_paid);
            fprintf(file, "Balance Due: $%.2f\n", balance_due);
            fprintf(file, "Status: %s\n", payment_status);
            fclose(file);
            printf("\n✅ Receipt saved to: %s\n", filename);
        } else {
            printf("\n❌ Error saving receipt!\n");
        }
        printf("\nPress Enter to continue...");
        getchar();
    } else if (choice == 2) {
        print_receipt();
    }
}

// ==================== REPORT FUNCTIONS ====================

void generate_report() {
    clear_screen();
    print_header("FINANCIAL REPORT");
    
    printf("Select Report Type:\n");
    printf("1. Summary Report\n");
    printf("2. Outstanding Payments\n");
    printf("Enter choice: ");
    
    int choice = get_choice(1, 2);
    
    if (choice == 2) {
        // Outstanding payments
        const char *sql = "SELECT bill_no, patient_name, total_amount, amount_paid, "
                         "balance_due, bill_date FROM bills WHERE balance_due > 0 "
                         "ORDER BY balance_due DESC";
        
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
            printf("Error generating report\n");
            printf("\nPress Enter to continue...");
            getchar();
            return;
        }
        
        printf("\nOUTSTANDING PAYMENTS REPORT\n");
        printf("════════════════════════════════════════════════════\n");
        printf("Bill No  Patient Name               Total      Paid       Balance    Date\n");
        printf("══════════════════════════════════════════════════════════════════════════\n");
        
        float total_outstanding = 0;
        int count = 0;
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            count++;
            int bill_no = sqlite3_column_int(stmt, 0);
            const char *patient_name = (const char*)sqlite3_column_text(stmt, 1);
            float total_amount = sqlite3_column_double(stmt, 2);
            float amount_paid = sqlite3_column_double(stmt, 3);
            float balance_due = sqlite3_column_double(stmt, 4);
            const char *bill_date = (const char*)sqlite3_column_text(stmt, 5);
            
            printf("%-8d %-25s $%-9.2f $%-9.2f $%-9.2f %s\n", 
                   bill_no, patient_name, total_amount, amount_paid, balance_due, bill_date);
            
            total_outstanding += balance_due;
        }
        
        sqlite3_finalize(stmt);
        
        printf("\nSummary:\n");
        printf("  Total Outstanding Bills: %d\n", count);
        printf("  Total Outstanding Amount: $%.2f\n", total_outstanding);
        
    } else {
        // Summary report
        char sql[500];
        strcpy(sql, "SELECT COUNT(*), SUM(total_amount), SUM(amount_paid), "
                    "SUM(balance_due) FROM bills");
        
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK || 
            sqlite3_step(stmt) != SQLITE_ROW) {
            printf("Error generating report\n");
            sqlite3_finalize(stmt);
            printf("\nPress Enter to continue...");
            getchar();
            return;
        }
        
        int total_bills = sqlite3_column_int(stmt, 0);
        float total_billed = sqlite3_column_double(stmt, 1);
        float total_paid = sqlite3_column_double(stmt, 2);
        float total_outstanding = sqlite3_column_double(stmt, 3);
        
        sqlite3_finalize(stmt);
        
        printf("\nFINANCIAL SUMMARY REPORT\n");
        printf("════════════════════════════════════════════════════\n");
        printf("\nSummary Statistics:\n");
        printf("  Total Bills Generated:      %d\n", total_bills);
        printf("  Total Amount Billed:        $%.2f\n", total_billed);
        printf("  Total Amount Collected:     $%.2f\n", total_paid);
        printf("  Total Outstanding:          $%.2f\n", total_outstanding);
        printf("  Collection Rate:            %.1f%%\n", 
               total_billed > 0 ? (total_paid / total_billed * 100) : 0);
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

void view_statistics() {
    clear_screen();
    print_header("SYSTEM STATISTICS");
    
    printf("\nOverall Statistics:\n");
    printf("════════════════════════════════════════════════════\n");
    
    // Patient statistics
    const char *sql = "SELECT COUNT(*), "
                     "COUNT(CASE WHEN gender = 'M' THEN 1 END), "
                     "COUNT(CASE WHEN gender = 'F' THEN 1 END), "
                     "AVG(age) FROM patients";
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK && 
        sqlite3_step(stmt) == SQLITE_ROW) {
        int total_patients = sqlite3_column_int(stmt, 0);
        int male_patients = sqlite3_column_int(stmt, 1);
        int female_patients = sqlite3_column_int(stmt, 2);
        float avg_age = sqlite3_column_double(stmt, 3);
        
        printf("PATIENTS:\n");
        printf("  Total Patients:        %d\n", total_patients);
        printf("  Male Patients:         %d\n", male_patients);
        printf("  Female Patients:       %d\n", female_patients);
        printf("  Average Age:           %.1f years\n", avg_age);
    }
    sqlite3_finalize(stmt);
    
    // Bill statistics
    sql = "SELECT COUNT(*), SUM(total_amount), SUM(amount_paid), "
          "SUM(balance_due), AVG(total_amount) FROM bills";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK && 
        sqlite3_step(stmt) == SQLITE_ROW) {
        int total_bills = sqlite3_column_int(stmt, 0);
        float total_billed = sqlite3_column_double(stmt, 1);
        float total_paid = sqlite3_column_double(stmt, 2);
        float total_outstanding = sqlite3_column_double(stmt, 3);
        float avg_bill = sqlite3_column_double(stmt, 4);
        
        printf("\nBILLING:\n");
        printf("  Total Bills:           %d\n", total_bills);
        printf("  Total Amount Billed:   $%.2f\n", total_billed);
        printf("  Total Amount Paid:     $%.2f\n", total_paid);
        printf("  Total Outstanding:     $%.2f\n", total_outstanding);
        printf("  Average Bill Amount:   $%.2f\n", avg_bill);
        printf("  Collection Rate:       %.1f%%\n", 
               total_billed > 0 ? (total_paid / total_billed * 100) : 0);
    }
    sqlite3_finalize(stmt);
    
    printf("\nPress Enter to continue...");
    getchar();
}

// ==================== SYSTEM FUNCTIONS ====================

void backup_database() {
    clear_screen();
    print_header("BACKUP DATABASE");
    
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char backup_name[100];
    strftime(backup_name, sizeof(backup_name), "backup_%Y%m%d_%H%M%S.db", tm);
    
    printf("Creating backup: %s\n", backup_name);
    
    sqlite3 *backup_db;
    sqlite3_backup *backup;
    
    int rc = sqlite3_open(backup_name, &backup_db);
    if (rc != SQLITE_OK) {
        printf("Cannot create backup file\n");
        sqlite3_close(backup_db);
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    backup = sqlite3_backup_init(backup_db, "main", db, "main");
    if (backup) {
        sqlite3_backup_step(backup, -1);
        sqlite3_backup_finish(backup);
    }
    
    rc = sqlite3_errcode(backup_db);
    sqlite3_close(backup_db);
    
    if (rc == SQLITE_OK) {
        printf("\n✅ Database backed up successfully!\n");
        
        // Create backups directory
        system("mkdir -p backups 2>/dev/null");
        
        // Move to backups
        char command[200];
        snprintf(command, sizeof(command), "mv %s backups/ 2>/dev/null", backup_name);
        system(command);
        
        printf("Backup saved in: backups/%s\n", backup_name);
        
        // List recent backups
        printf("\nRecent backups:\n");
        system("ls -la backups/*.db 2>/dev/null | tail -5");
    } else {
        printf("\n❌ Backup failed!\n");
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}

void restore_database() {
    clear_screen();
    print_header("RESTORE DATABASE");
    
    printf("WARNING: This will overwrite current database!\n");
    printf("Available backups:\n");
    system("ls -la backups/*.db 2>/dev/null || echo 'No backup files found'");
    
    char backup_name[100];
    printf("\nEnter backup filename (from backups/ directory): ");
    fgets(backup_name, sizeof(backup_name), stdin);
    backup_name[strcspn(backup_name, "\n")] = 0;
    
    if (strlen(backup_name) == 0) {
        printf("No backup file specified.\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Check if file exists
    char full_path[200];
    snprintf(full_path, sizeof(full_path), "backups/%s", backup_name);
    FILE *file = fopen(full_path, "rb");
    if (!file) {
        printf("Backup file '%s' not found!\n", backup_name);
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    fclose(file);
    
    printf("Are you sure you want to restore from '%s'? (y/n): ", backup_name);
    
    char confirm;
    scanf(" %c", &confirm);
    getchar();
    
    if (confirm != 'y' && confirm != 'Y') {
        printf("Restore cancelled.\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }
    
    // Close current database
    sqlite3_close(db);
    
    // Copy backup file
    char command[200];
    snprintf(command, sizeof(command), "cp \"backups/%s\" hospital.db", backup_name);
    int result = system(command);
    
    if (result != 0) {
        printf("Failed to restore database!\n");
        exit(1);
    }
    
    // Reopen database
    int rc = sqlite3_open("hospital.db", &db);
    if (rc != SQLITE_OK) {
        printf("Failed to restore database\n");
        exit(1);
    }
    
    printf("✅ Database restored successfully from: %s\n", backup_name);
    
    printf("\nPress Enter to continue...");
    getchar();
}

void export_data() {
    clear_screen();
    print_header("EXPORT DATA");
    
    printf("Select data to export:\n");
    printf("1. Patients (CSV)\n");
    printf("2. Bills (CSV)\n");
    printf("3. Payments (CSV)\n");
    printf("Enter choice: ");
    
    int choice = get_choice(1, 3);
    
    switch(choice) {
        case 1:
            system("sqlite3 hospital.db -csv 'SELECT * FROM patients;' > patients.csv 2>/dev/null");
            printf("✅ Patients exported to patients.csv\n");
            break;
        case 2:
            system("sqlite3 hospital.db -csv 'SELECT * FROM bills;' > bills.csv 2>/dev/null");
            printf("✅ Bills exported to bills.csv\n");
            break;
        case 3:
            system("sqlite3 hospital.db -csv 'SELECT * FROM payments;' > payments.csv 2>/dev/null");
            printf("✅ Payments exported to payments.csv\n");
            break;
    }
    
    printf("\nPress Enter to continue...");
    getchar();
}
