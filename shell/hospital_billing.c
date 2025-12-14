#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>

// Database connection
sqlite3* db = NULL;

// Function prototypes
void init_database();
void close_database();
void display_menu();
void add_patient();
void view_patients();
void search_patient();
void generate_bill();
void view_bills();
void make_payment();
void generate_report();
void backup_database();
void restore_database();

int main() {
    printf("\n========================================\n");
    printf("   HOSPITAL PATIENT BILLING SYSTEM\n");
    printf("========================================\n");
    
    // Initialize database
    init_database();
    
    int choice;
    do {
        display_menu();
        printf("\nEnter your choice: ");
        scanf("%d", &choice);
        getchar(); // Clear newline
        
        switch(choice) {
            case 1:
                add_patient();
                break;
            case 2:
                view_patients();
                break;
            case 3:
                search_patient();
                break;
            case 4:
                generate_bill();
                break;
            case 5:
                view_bills();
                break;
            case 6:
                make_payment();
                break;
            case 7:
                generate_report();
                break;
            case 8:
                backup_database();
                break;
            case 9:
                restore_database();
                break;
            case 0:
                printf("\nThank you for using Hospital Billing System!\n");
                break;
            default:
                printf("\nInvalid choice! Please try again.\n");
        }
    } while(choice != 0);
    
    close_database();
    return 0;
}

void init_database() {
    int rc = sqlite3_open("hospital.db", &db);
    if (rc != SQLITE_OK) {
        printf("Cannot open database: %s\n", sqlite3_errmsg(db));
        exit(1);
    }
    
    // Create tables if they don't exist
    const char *sql = 
        "CREATE TABLE IF NOT EXISTS patients ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    name TEXT NOT NULL,"
        "    age INTEGER,"
        "    gender TEXT,"
        "    contact TEXT UNIQUE,"
        "    address TEXT,"
        "    disease TEXT,"
        "    admission_date TEXT"
        ");"
        
        "CREATE TABLE IF NOT EXISTS bills ("
        "    bill_no INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    patient_id INTEGER,"
        "    patient_name TEXT,"
        "    bill_date TEXT DEFAULT CURRENT_DATE,"
        "    room_charges REAL DEFAULT 0,"
        "    doctor_fees REAL DEFAULT 0,"
        "    medicine_charges REAL DEFAULT 0,"
        "    lab_charges REAL DEFAULT 0,"
        "    other_charges REAL DEFAULT 0,"
        "    total_amount REAL DEFAULT 0,"
        "    amount_paid REAL DEFAULT 0,"
        "    balance_due REAL DEFAULT 0,"
        "    payment_status TEXT DEFAULT 'Pending',"
        "    FOREIGN KEY (patient_id) REFERENCES patients(id)"
        ");";
    
    char *err_msg = 0;
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        printf("SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    } else {
        printf("Database initialized successfully!\n");
    }
}

void close_database() {
    if (db) {
        sqlite3_close(db);
    }
}

void display_menu() {
    printf("\n════════════════════════════════════════════════════\n");
    printf("                    MAIN MENU\n");
    printf("════════════════════════════════════════════════════\n");
    printf("  1. Add New Patient\n");
    printf("  2. View All Patients\n");
    printf("  3. Search Patient\n");
    printf("  4. Generate Bill\n");
    printf("  5. View All Bills\n");
    printf("  6. Make Payment\n");
    printf("  7. Generate Financial Report\n");
    printf("  8. Backup Database\n");
    printf("  9. Restore Database\n");
    printf("  0. Exit\n");
    printf("════════════════════════════════════════════════════\n");
}

void add_patient() {
    printf("\n════════════════════════════════════════════════════\n");
    printf("                   ADD NEW PATIENT\n");
    printf("════════════════════════════════════════════════════\n");
    
    char name[100], gender[10], contact[20], address[200], disease[100];
    int age;
    char admission_date[11];
    
    printf("Enter patient name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;
    
    printf("Enter age: ");
    scanf("%d", &age);
    getchar();
    
    printf("Enter gender (M/F/O): ");
    fgets(gender, sizeof(gender), stdin);
    gender[strcspn(gender, "\n")] = 0;
    
    printf("Enter contact number: ");
    fgets(contact, sizeof(contact), stdin);
    contact[strcspn(contact, "\n")] = 0;
    
    printf("Enter address: ");
    fgets(address, sizeof(address), stdin);
    address[strcspn(address, "\n")] = 0;
    
    printf("Enter disease/diagnosis: ");
    fgets(disease, sizeof(disease), stdin);
    disease[strcspn(disease, "\n")] = 0;
    
    printf("Enter admission date (YYYY-MM-DD): ");
    fgets(admission_date, sizeof(admission_date), stdin);
    admission_date[strcspn(admission_date, "\n")] = 0;
    
    // Validate date format
    if (strlen(admission_date) != 10 || admission_date[4] != '-' || admission_date[7] != '-') {
        printf("Invalid date format! Using current date.\n");
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        strftime(admission_date, sizeof(admission_date), "%Y-%m-%d", tm);
    }
    
    char sql[1000];
    sprintf(sql, 
        "INSERT INTO patients (name, age, gender, contact, address, disease, admission_date) "
        "VALUES ('%s', %d, '%s', '%s', '%s', '%s', '%s');",
        name, age, gender, contact, address, disease, admission_date);
    
    char *err_msg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK) {
        printf("Error adding patient: %s\n", err_msg);
        sqlite3_free(err_msg);
    } else {
        printf("\n✅ Patient added successfully!\n");
        printf("   Patient ID: %lld\n", sqlite3_last_insert_rowid(db));
    }
}

void view_patients() {
    printf("\n════════════════════════════════════════════════════\n");
    printf("                   ALL PATIENTS\n");
    printf("════════════════════════════════════════════════════\n");
    
    const char *sql = "SELECT id, name, age, gender, contact, admission_date FROM patients ORDER BY name;";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("Failed to fetch patients: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    printf("ID\tName\t\t\tAge\tGender\tContact\t\tAdmission Date\n");
    printf("───────────────────────────────────────────────────────────────────────────────\n");
    
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
        int id = sqlite3_column_int(stmt, 0);
        const char *name = (const char*)sqlite3_column_text(stmt, 1);
        int age = sqlite3_column_int(stmt, 2);
        const char *gender = (const char*)sqlite3_column_text(stmt, 3);
        const char *contact = (const char*)sqlite3_column_text(stmt, 4);
        const char *admission_date = (const char*)sqlite3_column_text(stmt, 5);
        
        printf("%-4d\t%-20s\t%-3d\t%-6s\t%-12s\t%s\n", 
               id, name, age, gender, contact, admission_date);
    }
    
    sqlite3_finalize(stmt);
    
    if (count == 0) {
        printf("No patients found in the database.\n");
    } else {
        printf("\nTotal patients: %d\n", count);
    }
}

void search_patient() {
    printf("\n════════════════════════════════════════════════════\n");
    printf("                   SEARCH PATIENT\n");
    printf("════════════════════════════════════════════════════\n");
    
    int choice;
    printf("Search by:\n");
    printf("1. Name\n");
    printf("2. Contact Number\n");
    printf("3. Patient ID\n");
    printf("Enter choice: ");
    scanf("%d", &choice);
    getchar();
    
    sqlite3_stmt *stmt;
    char sql[500];
    
    if (choice == 1) {
        char name[100];
        printf("Enter patient name (or part of name): ");
        fgets(name, sizeof(name), stdin);
        name[strcspn(name, "\n")] = 0;
        
        sprintf(sql, "SELECT * FROM patients WHERE name LIKE '%%%s%%' ORDER BY name;", name);
    } else if (choice == 2) {
        char contact[20];
        printf("Enter contact number: ");
        fgets(contact, sizeof(contact), stdin);
        contact[strcspn(contact, "\n")] = 0;
        
        sprintf(sql, "SELECT * FROM patients WHERE contact = '%s';", contact);
    } else if (choice == 3) {
        int id;
        printf("Enter patient ID: ");
        scanf("%d", &id);
        getchar();
        
        sprintf(sql, "SELECT * FROM patients WHERE id = %d;", id);
    } else {
        printf("Invalid choice!\n");
        return;
    }
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("Search failed: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    printf("\nSearch Results:\n");
    printf("ID\tName\t\tAge\tGender\tContact\t\tAddress\t\tDisease\n");
    printf("───────────────────────────────────────────────────────────────────────────────\n");
    
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
        
        printf("%d\t%s\t%d\t%s\t%s\t%s\t%s\n", 
               id, name, age, gender, contact, address, disease);
        printf("Admission Date: %s\n", admission_date);
        printf("───────────────────────────────────────────────────────────────────────────────\n");
    }
    
    sqlite3_finalize(stmt);
    
    if (!found) {
        printf("No patients found matching your search criteria.\n");
    }
}

void generate_bill() {
    printf("\n════════════════════════════════════════════════════\n");
    printf("                   GENERATE BILL\n");
    printf("════════════════════════════════════════════════════\n");
    
    // Show patients
    view_patients();
    
    int patient_id;
    printf("\nEnter Patient ID for billing: ");
    scanf("%d", &patient_id);
    getchar();
    
    // Get patient details
    char sql[500];
    sprintf(sql, "SELECT name FROM patients WHERE id = %d;", patient_id);
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK || sqlite3_step(stmt) != SQLITE_ROW) {
        printf("Patient not found!\n");
        sqlite3_finalize(stmt);
        return;
    }
    
    const char *patient_name = (const char*)sqlite3_column_text(stmt, 0);
    sqlite3_finalize(stmt);
    
    printf("\nGenerating bill for: %s (ID: %d)\n", patient_name, patient_id);
    printf("─────────────────────────────────────────────────────\n");
    
    float room_charges, doctor_fees, medicine_charges, lab_charges, other_charges;
    
    printf("Enter room charges: $");
    scanf("%f", &room_charges);
    
    printf("Enter doctor fees: $");
    scanf("%f", &doctor_fees);
    
    printf("Enter medicine charges: $");
    scanf("%f", &medicine_charges);
    
    printf("Enter lab charges: $");
    scanf("%f", &lab_charges);
    
    printf("Enter other charges: $");
    scanf("%f", &other_charges);
    
    float total_amount = room_charges + doctor_fees + medicine_charges + lab_charges + other_charges;
    
    printf("\nTotal Amount: $%.2f\n", total_amount);
    
    printf("\nPayment Status:\n");
    printf("1. Paid\n");
    printf("2. Pending\n");
    printf("3. Partial\n");
    printf("Enter choice: ");
    int status_choice;
    scanf("%d", &status_choice);
    
    float amount_paid = 0;
    char payment_status[20];
    
    if (status_choice == 1) {
        strcpy(payment_status, "Paid");
        amount_paid = total_amount;
    } else if (status_choice == 3) {
        strcpy(payment_status, "Partial");
        printf("Enter amount paid now: $");
        scanf("%f", &amount_paid);
        if (amount_paid > total_amount) {
            printf("Amount paid cannot exceed total amount! Setting to total amount.\n");
            amount_paid = total_amount;
        }
    } else {
        strcpy(payment_status, "Pending");
    }
    
    float balance_due = total_amount - amount_paid;
    
    // Insert bill into database
    sprintf(sql, 
        "INSERT INTO bills (patient_id, patient_name, room_charges, doctor_fees, "
        "medicine_charges, lab_charges, other_charges, total_amount, amount_paid, "
        "balance_due, payment_status) "
        "VALUES (%d, '%s', %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, '%s');",
        patient_id, patient_name, room_charges, doctor_fees, medicine_charges,
        lab_charges, other_charges, total_amount, amount_paid, balance_due, payment_status);
    
    char *err_msg = 0;
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK) {
        printf("Error generating bill: %s\n", err_msg);
        sqlite3_free(err_msg);
    } else {
        printf("\n✅ Bill generated successfully!\n");
        printf("   Bill Number: %lld\n", sqlite3_last_insert_rowid(db));
        printf("   Patient: %s\n", patient_name);
        printf("   Total Amount: $%.2f\n", total_amount);
        printf("   Amount Paid: $%.2f\n", amount_paid);
        printf("   Balance Due: $%.2f\n", balance_due);
        printf("   Status: %s\n", payment_status);
    }
}

void view_bills() {
    printf("\n════════════════════════════════════════════════════\n");
    printf("                   ALL BILLS\n");
    printf("════════════════════════════════════════════════════\n");
    
    const char *sql = "SELECT bill_no, patient_id, patient_name, total_amount, "
                     "payment_status, balance_due FROM bills ORDER BY bill_no DESC;";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("Failed to fetch bills: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    printf("Bill No\tPatient ID\tPatient Name\t\tTotal\t\tStatus\t\tBalance\n");
    printf("───────────────────────────────────────────────────────────────────────────────\n");
    
    int count = 0;
    float total_billed = 0, total_pending = 0;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
        int bill_no = sqlite3_column_int(stmt, 0);
        int patient_id = sqlite3_column_int(stmt, 1);
        const char *patient_name = (const char*)sqlite3_column_text(stmt, 2);
        float total_amount = sqlite3_column_double(stmt, 3);
        const char *payment_status = (const char*)sqlite3_column_text(stmt, 4);
        float balance_due = sqlite3_column_double(stmt, 5);
        
        printf("%-7d\t%-10d\t%-20s\t$%-8.2f\t%-10s\t$%-8.2f\n", 
               bill_no, patient_id, patient_name, total_amount, payment_status, balance_due);
        
        total_billed += total_amount;
        if (strcmp(payment_status, "Pending") == 0 || strcmp(payment_status, "Partial") == 0) {
            total_pending += balance_due;
        }
    }
    
    sqlite3_finalize(stmt);
    
    if (count == 0) {
        printf("No bills found in the database.\n");
    } else {
        printf("\nSummary:\n");
        printf("  Total Bills: %d\n", count);
        printf("  Total Billed: $%.2f\n", total_billed);
        printf("  Total Pending: $%.2f\n", total_pending);
    }
}

void make_payment() {
    printf("\n════════════════════════════════════════════════════\n");
    printf("                   MAKE PAYMENT\n");
    printf("════════════════════════════════════════════════════\n");
    
    // Show pending bills
    const char *sql = "SELECT bill_no, patient_name, total_amount, amount_paid, "
                     "balance_due FROM bills WHERE balance_due > 0 ORDER BY bill_no;";
    sqlite3_stmt *stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("Failed to fetch bills: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    printf("Pending Bills:\n");
    printf("Bill No\tPatient Name\t\tTotal\t\tPaid\t\tBalance\n");
    printf("────────────────────────────────────────────────────────────────────\n");
    
    int bills[100];
    float balances[100];
    int bill_count = 0;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int bill_no = sqlite3_column_int(stmt, 0);
        const char *patient_name = (const char*)sqlite3_column_text(stmt, 1);
        float total_amount = sqlite3_column_double(stmt, 2);
        float amount_paid = sqlite3_column_double(stmt, 3);
        float balance_due = sqlite3_column_double(stmt, 4);
        
        printf("%-7d\t%-20s\t$%-8.2f\t$%-8.2f\t$%-8.2f\n", 
               bill_no, patient_name, total_amount, amount_paid, balance_due);
        
        bills[bill_count] = bill_no;
        balances[bill_count] = balance_due;
        bill_count++;
    }
    
    sqlite3_finalize(stmt);
    
    if (bill_count == 0) {
        printf("No pending bills found.\n");
        return;
    }
    
    int bill_no;
    float payment_amount;
    
    printf("\nEnter Bill No to pay: ");
    scanf("%d", &bill_no);
    
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
        return;
    }
    
    printf("Maximum payment allowed: $%.2f\n", max_payment);
    printf("Enter payment amount: $");
    scanf("%f", &payment_amount);
    getchar();
    
    if (payment_amount <= 0 || payment_amount > max_payment) {
        printf("Invalid payment amount!\n");
        return;
    }
    
    // Update bill
    char update_sql[500];
    sprintf(update_sql, 
        "UPDATE bills SET amount_paid = amount_paid + %.2f, "
        "balance_due = balance_due - %.2f WHERE bill_no = %d;",
        payment_amount, payment_amount, bill_no);
    
    char *err_msg = 0;
    rc = sqlite3_exec(db, update_sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK) {
        printf("Payment failed: %s\n", err_msg);
        sqlite3_free(err_msg);
        return;
    }
    
    // Update status if fully paid
    sprintf(update_sql, 
        "UPDATE bills SET payment_status = CASE "
        "WHEN balance_due <= 0 THEN 'Paid' "
        "ELSE payment_status END WHERE bill_no = %d;",
        bill_no);
    
    sqlite3_exec(db, update_sql, 0, 0, &err_msg);
    
    printf("\n✅ Payment of $%.2f recorded successfully for Bill No: %d\n", payment_amount, bill_no);
}

void generate_report() {
    printf("\n════════════════════════════════════════════════════\n");
    printf("               FINANCIAL REPORT\n");
    printf("════════════════════════════════════════════════════\n");
    
    sqlite3_stmt *stmt;
    
    // Total statistics
    const char *sql = 
        "SELECT "
        "(SELECT COUNT(*) FROM patients) as total_patients, "
        "(SELECT COUNT(*) FROM bills) as total_bills, "
        "(SELECT SUM(total_amount) FROM bills) as total_billed, "
        "(SELECT SUM(amount_paid) FROM bills) as total_paid, "
        "(SELECT SUM(balance_due) FROM bills) as total_outstanding;";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK || sqlite3_step(stmt) != SQLITE_ROW) {
        printf("Failed to generate report\n");
        return;
    }
    
    int total_patients = sqlite3_column_int(stmt, 0);
    int total_bills = sqlite3_column_int(stmt, 1);
    float total_billed = sqlite3_column_double(stmt, 2);
    float total_paid = sqlite3_column_double(stmt, 3);
    float total_outstanding = sqlite3_column_double(stmt, 4);
    
    sqlite3_finalize(stmt);
    
    printf("Overall Statistics:\n");
    printf("─────────────────────────────────────────────────────\n");
    printf("Total Patients:        %d\n", total_patients);
    printf("Total Bills Generated: %d\n", total_bills);
    printf("Total Amount Billed:   $%.2f\n", total_billed);
    printf("Total Amount Paid:     $%.2f\n", total_paid);
    printf("Total Outstanding:     $%.2f\n", total_outstanding);
    printf("Collection Rate:       %.1f%%\n", 
           total_billed > 0 ? (total_paid / total_billed * 100) : 0);
    
    // Bill status breakdown
    printf("\nBill Status Breakdown:\n");
    printf("─────────────────────────────────────────────────────\n");
    
    sql = "SELECT payment_status, COUNT(*), SUM(total_amount), SUM(amount_paid), "
          "SUM(balance_due) FROM bills GROUP BY payment_status;";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc == SQLITE_OK) {
        printf("Status\t\tCount\tTotal\t\tPaid\t\tOutstanding\n");
        printf("────────────────────────────────────────────────────────────────────\n");
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *status = (const char*)sqlite3_column_text(stmt, 0);
            int count = sqlite3_column_int(stmt, 1);
            float total = sqlite3_column_double(stmt, 2);
            float paid = sqlite3_column_double(stmt, 3);
            float outstanding = sqlite3_column_double(stmt, 4);
            
            printf("%-10s\t%-5d\t$%-10.2f\t$%-10.2f\t$%-10.2f\n", 
                   status, count, total, paid, outstanding);
        }
        sqlite3_finalize(stmt);
    }
    
    // Top 5 patients by billing
    printf("\nTop 5 Patients by Billing:\n");
    printf("─────────────────────────────────────────────────────\n");
    
    sql = "SELECT patient_name, COUNT(*) as bill_count, SUM(total_amount) as total_billed "
          "FROM bills GROUP BY patient_id ORDER BY total_billed DESC LIMIT 5;";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc == SQLITE_OK) {
        printf("Patient Name\t\tBill Count\tTotal Billed\n");
        printf("─────────────────────────────────────────────────────\n");
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *patient_name = (const char*)sqlite3_column_text(stmt, 0);
            int bill_count = sqlite3_column_int(stmt, 1);
            float total_billed = sqlite3_column_double(stmt, 2);
            
            printf("%-20s\t%-10d\t$%-10.2f\n", patient_name, bill_count, total_billed);
        }
        sqlite3_finalize(stmt);
    }
}

void backup_database() {
    printf("\n════════════════════════════════════════════════════\n");
    printf("               BACKUP DATABASE\n");
    printf("════════════════════════════════════════════════════\n");
    
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char backup_name[100];
    strftime(backup_name, sizeof(backup_name), "backup_%Y%m%d_%H%M%S.db", tm);
    
    sqlite3 *backup_db;
    sqlite3_backup *backup;
    
    int rc = sqlite3_open(backup_name, &backup_db);
    if (rc != SQLITE_OK) {
        printf("Cannot create backup file: %s\n", sqlite3_errmsg(backup_db));
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
        printf("✅ Database backed up successfully to: %s\n", backup_name);
    } else {
        printf("❌ Backup failed!\n");
    }
}

void restore_database() {
    printf("\n════════════════════════════════════════════════════\n");
    printf("               RESTORE DATABASE\n");
    printf("════════════════════════════════════════════════════\n");
    
    printf("WARNING: This will overwrite current database!\n");
    printf("Enter backup filename (or press Enter to list backups): ");
    
    char backup_name[100];
    fgets(backup_name, sizeof(backup_name), stdin);
    backup_name[strcspn(backup_name, "\n")] = 0;
    
    if (strlen(backup_name) == 0) {
        system("ls *.db 2>/dev/null | grep -v '^hospital.db$'");
        printf("\nEnter backup filename to restore: ");
        fgets(backup_name, sizeof(backup_name), stdin);
        backup_name[strcspn(backup_name, "\n")] = 0;
    }
    
    if (strlen(backup_name) == 0) {
        printf("No backup file specified.\n");
        return;
    }
    
    // Check if file exists
    FILE *file = fopen(backup_name, "rb");
    if (!file) {
        printf("Backup file '%s' not found!\n", backup_name);
        return;
    }
    fclose(file);
    
    printf("Are you sure you want to restore from '%s'? (y/n): ", backup_name);
    char confirm;
    scanf(" %c", &confirm);
    getchar();
    
    if (confirm != 'y' && confirm != 'Y') {
        printf("Restore cancelled.\n");
        return;
    }
    
    // Close current database
    sqlite3_close(db);
    
    // Copy backup file to hospital.db
    char command[200];
    sprintf(command, "cp \"%s\" hospital.db", backup_name);
    system(command);
    
    // Reopen database - FIX: Declare rc here
    int rc = sqlite3_open("hospital.db", &db);
    if (rc != SQLITE_OK) {
        printf("Failed to restore database: %s\n", sqlite3_errmsg(db));
        exit(1);
    }
    
    printf("✅ Database restored successfully from: %s\n", backup_name);
}
