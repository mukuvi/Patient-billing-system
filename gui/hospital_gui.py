import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import sqlite3
import csv
from datetime import datetime
import os
from PIL import Image, ImageTk  # For icons if needed

class HospitalBillingSystem:
    def __init__(self, root):
        self.root = root
        self.root.title("Hospital Patient Billing System")
        self.root.geometry("1200x700")
        self.root.configure(bg='#f0f0f0')
        
        # Initialize database
        self.init_database()
        
        # Create styles
        self.setup_styles()
        
        # Create main container
        self.create_main_container()
        
        # Show login screen initially
        self.show_login_screen()
    
    def setup_styles(self):
        """Configure ttk styles"""
        style = ttk.Style()
        style.theme_use('clam')
        
        # Configure colors
        style.configure('Title.TLabel', font=('Arial', 24, 'bold'), foreground='#2c3e50')
        style.configure('Header.TLabel', font=('Arial', 12, 'bold'), foreground='#34495e')
        style.configure('Success.TLabel', foreground='#27ae60')
        style.configure('Error.TLabel', foreground='#e74c3c')
        
        # Configure buttons
        style.configure('Primary.TButton', font=('Arial', 10, 'bold'), 
                       foreground='white', background='#3498db', borderwidth=0)
        style.map('Primary.TButton', background=[('active', '#2980b9')])
        
        style.configure('Success.TButton', font=('Arial', 10, 'bold'), 
                       foreground='white', background='#2ecc71', borderwidth=0)
        style.map('Success.TButton', background=[('active', '#27ae60')])
        
        style.configure('Danger.TButton', font=('Arial', 10, 'bold'), 
                       foreground='white', background='#e74c3c', borderwidth=0)
        style.map('Danger.TButton', background=[('active', '#c0392b')])
    
    def create_main_container(self):
        """Create main container frame"""
        self.main_container = tk.Frame(self.root, bg='#f0f0f0')
        self.main_container.pack(fill='both', expand=True, padx=20, pady=20)
    
    def clear_container(self):
        """Clear the main container"""
        for widget in self.main_container.winfo_children():
            widget.destroy()
    
    # ==================== DATABASE FUNCTIONS ====================
    
    def init_database(self):
        """Initialize SQLite database"""
        self.conn = sqlite3.connect('hospital.db', check_same_thread=False)
        self.conn.execute("PRAGMA foreign_keys = ON")
        self.conn.execute("PRAGMA encoding = 'UTF-8'")
        self.cursor = self.conn.cursor()
        
        # Create tables
        self.cursor.execute('''
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT UNIQUE NOT NULL,
                password TEXT NOT NULL,
                role TEXT DEFAULT 'staff',
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        self.cursor.execute('''
            CREATE TABLE IF NOT EXISTS patients (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                age INTEGER,
                gender TEXT,
                contact TEXT,
                address TEXT,
                disease TEXT,
                admission_date DATE DEFAULT CURRENT_DATE,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        self.cursor.execute('''
            CREATE TABLE IF NOT EXISTS bills (
                bill_no INTEGER PRIMARY KEY AUTOINCREMENT,
                patient_id INTEGER,
                patient_name TEXT,
                bill_date DATE DEFAULT CURRENT_DATE,
                room_charges REAL DEFAULT 0,
                doctor_fees REAL DEFAULT 0,
                medicine_charges REAL DEFAULT 0,
                lab_charges REAL DEFAULT 0,
                other_charges REAL DEFAULT 0,
                total_amount REAL DEFAULT 0,
                amount_paid REAL DEFAULT 0,
                balance_due REAL DEFAULT 0,
                payment_status TEXT DEFAULT 'Pending',
                payment_method TEXT,
                FOREIGN KEY (patient_id) REFERENCES patients(id) ON DELETE CASCADE
            )
        ''')
        
        self.cursor.execute('''
            CREATE TABLE IF NOT EXISTS payments (
                payment_id INTEGER PRIMARY KEY AUTOINCREMENT,
                bill_no INTEGER,
                amount REAL,
                payment_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                payment_method TEXT,
                FOREIGN KEY (bill_no) REFERENCES bills(bill_no) ON DELETE CASCADE
            )
        ''')
        
        # Insert default users if not exists
        self.cursor.execute('''
            INSERT OR IGNORE INTO users (username, password, role) 
            VALUES ('admin', 'admin123', 'admin'),
                   ('staff', 'staff123', 'staff')
        ''')
        
        self.conn.commit()
    
    def execute_query(self, query, params=()):
        """Execute SQL query safely"""
        try:
            self.cursor.execute(query, params)
            self.conn.commit()
            return True
        except sqlite3.Error as e:
            messagebox.showerror("Database Error", str(e))
            return False
    
    def fetch_all(self, query, params=()):
        """Fetch all results from query"""
        try:
            self.cursor.execute(query, params)
            return self.cursor.fetchall()
        except sqlite3.Error as e:
            messagebox.showerror("Database Error", str(e))
            return []
    
    def fetch_one(self, query, params=()):
        """Fetch single result from query"""
        try:
            self.cursor.execute(query, params)
            return self.cursor.fetchone()
        except sqlite3.Error as e:
            messagebox.showerror("Database Error", str(e))
            return None
    
    # ==================== AUTHENTICATION ====================
    
    def show_login_screen(self):
        """Display login screen"""
        self.clear_container()
        
        # Title
        title = ttk.Label(self.main_container, text="Hospital Billing System", 
                         style='Title.TLabel')
        title.pack(pady=(0, 30))
        
        # Login frame
        login_frame = tk.Frame(self.main_container, bg='white', 
                              relief='ridge', borderwidth=2)
        login_frame.pack(pady=20, ipadx=50, ipady=30)
        
        ttk.Label(login_frame, text="LOGIN", font=('Arial', 16, 'bold'),
                 background='white').grid(row=0, column=0, columnspan=2, pady=(0, 20))
        
        # Username
        ttk.Label(login_frame, text="Username:", background='white',
                 font=('Arial', 10)).grid(row=1, column=0, sticky='e', padx=5, pady=10)
        self.username_var = tk.StringVar()
        username_entry = ttk.Entry(login_frame, textvariable=self.username_var, 
                                  width=30, font=('Arial', 10))
        username_entry.grid(row=1, column=1, padx=5, pady=10)
        
        # Password
        ttk.Label(login_frame, text="Password:", background='white',
                 font=('Arial', 10)).grid(row=2, column=0, sticky='e', padx=5, pady=10)
        self.password_var = tk.StringVar()
        password_entry = ttk.Entry(login_frame, textvariable=self.password_var,
                                  show="*", width=30, font=('Arial', 10))
        password_entry.grid(row=2, column=1, padx=5, pady=10)
        
        # Login button
        login_btn = ttk.Button(login_frame, text="Login", 
                              command=self.login, style='Primary.TButton')
        login_btn.grid(row=3, column=0, columnspan=2, pady=20)
        
        # Default credentials note
        note_frame = tk.Frame(self.main_container, bg='#f0f0f0')
        note_frame.pack(pady=10)
        ttk.Label(note_frame, text="Default Credentials:", 
                 font=('Arial', 9, 'italic')).pack()
        ttk.Label(note_frame, text="Admin: admin / admin123", 
                 font=('Arial', 9)).pack()
        ttk.Label(note_frame, text="Staff: staff / staff123", 
                 font=('Arial', 9)).pack()
        
        # Bind Enter key to login
        username_entry.bind('<Return>', lambda e: self.login())
        password_entry.bind('<Return>', lambda e: self.login())
    
    def login(self):
        """Authenticate user"""
        username = self.username_var.get().strip()
        password = self.password_var.get().strip()
        
        if not username or not password:
            messagebox.showwarning("Input Error", "Please enter both username and password")
            return
        
        result = self.fetch_one(
            "SELECT role FROM users WHERE username = ? AND password = ?",
            (username, password)
        )
        
        if result:
            self.user_role = result[0]
            self.current_user = username
            messagebox.showinfo("Login Successful", 
                              f"Welcome, {username}! (Role: {self.user_role})")
            self.show_main_menu()
        else:
            messagebox.showerror("Login Failed", "Invalid username or password")
    
    def logout(self):
        """Logout current user"""
        self.user_role = None
        self.current_user = None
        self.show_login_screen()
    
    # ==================== MAIN MENU ====================
    
    def show_main_menu(self):
        """Display main menu"""
        self.clear_container()
        
        # Header
        header_frame = tk.Frame(self.main_container, bg='#2c3e50')
        header_frame.pack(fill='x', pady=(0, 20))
        
        ttk.Label(header_frame, text="Hospital Billing System", 
                 style='Title.TLabel', background='#2c3e50', 
                 foreground='white').pack(side='left', padx=20, pady=10)
        
        user_info = tk.Label(header_frame, 
                           text=f"User: {self.current_user} ({self.user_role})",
                           font=('Arial', 10), bg='#2c3e50', fg='white')
        user_info.pack(side='right', padx=20)
        
        logout_btn = ttk.Button(header_frame, text="Logout", 
                               command=self.logout, style='Danger.TButton')
        logout_btn.pack(side='right', padx=10)
        
        # Menu buttons grid
        menu_frame = tk.Frame(self.main_container, bg='#f0f0f0')
        menu_frame.pack(expand=True)
        
        menu_items = [
            ("👤 Add Patient", self.show_add_patient),
            ("📋 View Patients", self.show_view_patients),
            ("🔍 Search Patient", self.show_search_patient),
            ("✏️ Update Patient", self.show_update_patient),
            ("🗑️ Delete Patient", self.show_delete_patient),
            ("💰 Generate Bill", self.show_generate_bill),
            ("📊 View Bills", self.show_view_bills),
            ("💳 Make Payment", self.show_make_payment),
            ("📈 Reports", self.show_reports),
            ("💾 Backup", self.backup_database),
            ("📤 Export", self.show_export_menu),
            ("ℹ️ Statistics", self.show_statistics),
        ]
        
        row, col = 0, 0
        for text, command in menu_items:
            btn = tk.Button(menu_frame, text=text, font=('Arial', 11),
                          bg='#3498db', fg='white', padx=20, pady=15,
                          relief='raised', borderwidth=2,
                          command=command)
            btn.grid(row=row, column=col, padx=10, pady=10, sticky='nsew')
            btn.bind("<Enter>", lambda e, b=btn: b.config(bg='#2980b9'))
            btn.bind("<Leave>", lambda e, b=btn: b.config(bg='#3498db'))
            
            col += 1
            if col > 2:
                col = 0
                row += 1
        
        # Configure grid weights
        for i in range(3):
            menu_frame.columnconfigure(i, weight=1)
        for i in range(4):
            menu_frame.rowconfigure(i, weight=1)
    
    # ==================== PATIENT MANAGEMENT ====================
    
    def show_add_patient(self):
        """Show add patient form"""
        self.clear_container()
        self.create_back_button()
        
        ttk.Label(self.main_container, text="Add New Patient", 
                 style='Title.TLabel').pack(pady=(0, 20))
        
        # Create form
        form_frame = tk.Frame(self.main_container, bg='white', 
                             relief='ridge', borderwidth=1)
        form_frame.pack(padx=50, pady=20, fill='both', expand=True)
        
        # Form fields
        fields = [
            ("Name:", "name"),
            ("Age:", "age"),
            ("Gender (M/F/O):", "gender"),
            ("Contact:", "contact"),
            ("Address:", "address"),
            ("Disease:", "disease"),
            ("Admission Date (YYYY-MM-DD):", "admission_date")
        ]
        
        self.patient_vars = {}
        for i, (label, field) in enumerate(fields):
            ttk.Label(form_frame, text=label, background='white',
                     font=('Arial', 10)).grid(row=i, column=0, sticky='e', 
                                             padx=10, pady=10)
            
            if field == 'address':
                entry = tk.Text(form_frame, height=3, width=40, 
                               font=('Arial', 10))
                entry.grid(row=i, column=1, padx=10, pady=10, sticky='w')
                self.patient_vars[field] = entry
            else:
                var = tk.StringVar()
                entry = ttk.Entry(form_frame, textvariable=var, 
                                 width=40, font=('Arial', 10))
                entry.grid(row=i, column=1, padx=10, pady=10, sticky='w')
                self.patient_vars[field] = var
        
        # Set default date
        self.patient_vars['admission_date'].set(datetime.now().strftime('%Y-%m-%d'))
        
        # Buttons
        btn_frame = tk.Frame(form_frame, bg='white')
        btn_frame.grid(row=len(fields), column=0, columnspan=2, pady=20)
        
        ttk.Button(btn_frame, text="Save Patient", 
                  command=self.save_patient, style='Success.TButton').pack(side='left', padx=10)
        ttk.Button(btn_frame, text="Clear Form", 
                  command=self.clear_patient_form).pack(side='left', padx=10)
    
    def save_patient(self):
        """Save patient to database"""
        try:
            # Get values
            name = self.patient_vars['name'].get().strip()
            age = self.patient_vars['age'].get().strip()
            gender = self.patient_vars['gender'].get().strip()
            contact = self.patient_vars['contact'].get().strip()
            
            if isinstance(self.patient_vars['address'], tk.Text):
                address = self.patient_vars['address'].get("1.0", tk.END).strip()
            else:
                address = self.patient_vars['address'].get().strip()
                
            disease = self.patient_vars['disease'].get().strip()
            admission_date = self.patient_vars['admission_date'].get().strip()
            
            # Validate
            if not name:
                messagebox.showwarning("Validation Error", "Patient name is required")
                return
            
            if age and not age.isdigit():
                messagebox.showwarning("Validation Error", "Age must be a number")
                return
            
            # Insert into database
            query = '''
                INSERT INTO patients (name, age, gender, contact, address, disease, admission_date)
                VALUES (?, ?, ?, ?, ?, ?, ?)
            '''
            
            if self.execute_query(query, (name, age or None, gender, contact, 
                                         address, disease, admission_date or datetime.now().date())):
                messagebox.showinfo("Success", "Patient added successfully!")
                self.clear_patient_form()
        except Exception as e:
            messagebox.showerror("Error", f"Failed to save patient: {str(e)}")
    
    def clear_patient_form(self):
        """Clear patient form"""
        for field, widget in self.patient_vars.items():
            if isinstance(widget, tk.Text):
                widget.delete("1.0", tk.END)
            else:
                widget.set("")
        self.patient_vars['admission_date'].set(datetime.now().strftime('%Y-%m-%d'))
    
    def show_view_patients(self):
        """Show all patients in a table"""
        self.clear_container()
        self.create_back_button()
        
        ttk.Label(self.main_container, text="All Patients", 
                 style='Title.TLabel').pack(pady=(0, 20))
        
        # Search frame
        search_frame = tk.Frame(self.main_container, bg='#f0f0f0')
        search_frame.pack(fill='x', pady=(0, 10))
        
        ttk.Label(search_frame, text="Search:", background='#f0f0f0').pack(side='left', padx=5)
        self.search_var = tk.StringVar()
        search_entry = ttk.Entry(search_frame, textvariable=self.search_var, width=40)
        search_entry.pack(side='left', padx=5)
        
        ttk.Button(search_frame, text="Search", 
                  command=self.search_patients_table).pack(side='left', padx=5)
        ttk.Button(search_frame, text="Refresh", 
                  command=self.load_patients_table).pack(side='left', padx=5)
        
        # Create treeview
        tree_frame = tk.Frame(self.main_container)
        tree_frame.pack(fill='both', expand=True)
        
        # Scrollbars
        vsb = ttk.Scrollbar(tree_frame, orient="vertical")
        hsb = ttk.Scrollbar(tree_frame, orient="horizontal")
        
        # Treeview
        self.patients_tree = ttk.Treeview(tree_frame, 
                                         columns=('ID', 'Name', 'Age', 'Gender', 
                                                 'Contact', 'Admission Date'),
                                         show='headings',
                                         yscrollcommand=vsb.set,
                                         xscrollcommand=hsb.set)
        
        # Configure columns
        columns = [('ID', 50), ('Name', 200), ('Age', 50), 
                  ('Gender', 80), ('Contact', 150), ('Admission Date', 120)]
        
        for col, width in columns:
            self.patients_tree.heading(col, text=col)
            self.patients_tree.column(col, width=width, minwidth=50)
        
        vsb.config(command=self.patients_tree.yview)
        hsb.config(command=self.patients_tree.xview)
        
        # Grid layout
        self.patients_tree.grid(row=0, column=0, sticky='nsew')
        vsb.grid(row=0, column=1, sticky='ns')
        hsb.grid(row=1, column=0, sticky='ew')
        
        tree_frame.grid_rowconfigure(0, weight=1)
        tree_frame.grid_columnconfigure(0, weight=1)
        
        # Load data
        self.load_patients_table()
        
        # Bind double-click event
        self.patients_tree.bind('<Double-Button-1>', self.show_patient_details)
    
    def load_patients_table(self, search_term=""):
        """Load patients into treeview"""
        # Clear existing items
        for item in self.patients_tree.get_children():
            self.patients_tree.delete(item)
        
        # Fetch data
        if search_term:
            query = '''
                SELECT id, name, age, gender, contact, admission_date 
                FROM patients 
                WHERE name LIKE ? OR contact LIKE ?
                ORDER BY name
            '''
            params = (f'%{search_term}%', f'%{search_term}%')
        else:
            query = '''
                SELECT id, name, age, gender, contact, admission_date 
                FROM patients 
                ORDER BY name
            '''
            params = ()
        
        patients = self.fetch_all(query, params)
        
        # Insert data
        for patient in patients:
            self.patients_tree.insert('', 'end', values=patient)
    
    def search_patients_table(self):
        """Search patients"""
        search_term = self.search_var.get().strip()
        self.load_patients_table(search_term)
    
    def show_patient_details(self, event):
        """Show detailed view of selected patient"""
        selection = self.patients_tree.selection()
        if not selection:
            return
        
        item = self.patients_tree.item(selection[0])
        patient_id = item['values'][0]
        
        # Fetch detailed info
        query = '''
            SELECT name, age, gender, contact, address, disease, admission_date
            FROM patients WHERE id = ?
        '''
        patient = self.fetch_one(query, (patient_id,))
        
        if patient:
            details_window = tk.Toplevel(self.root)
            details_window.title("Patient Details")
            details_window.geometry("500x400")
            
            # Create form to show details
            fields = ["Name", "Age", "Gender", "Contact", "Address", "Disease", "Admission Date"]
            
            for i, (field, value) in enumerate(zip(fields, patient)):
                ttk.Label(details_window, text=f"{field}:", 
                         font=('Arial', 10, 'bold')).grid(row=i, column=0, 
                                                         sticky='e', padx=10, pady=10)
                if field == "Address":
                    text_widget = tk.Text(details_window, height=4, width=40,
                                         font=('Arial', 10))
                    text_widget.insert('1.0', value or '')
                    text_widget.config(state='disabled')
                    text_widget.grid(row=i, column=1, padx=10, pady=10, sticky='w')
                else:
                    ttk.Label(details_window, text=value or '', 
                             font=('Arial', 10)).grid(row=i, column=1, 
                                                     sticky='w', padx=10, pady=10)
    
    def show_search_patient(self):
        """Show patient search interface"""
        self.clear_container()
        self.create_back_button()
        
        ttk.Label(self.main_container, text="Search Patient", 
                 style='Title.TLabel').pack(pady=(0, 20))
        
        # Search options
        options_frame = tk.Frame(self.main_container, bg='#f0f0f0')
        options_frame.pack(pady=10)
        
        self.search_by = tk.StringVar(value="name")
        ttk.Radiobutton(options_frame, text="By Name", variable=self.search_by, 
                       value="name").pack(side='left', padx=20)
        ttk.Radiobutton(options_frame, text="By Contact", variable=self.search_by, 
                       value="contact").pack(side='left', padx=20)
        ttk.Radiobutton(options_frame, text="By ID", variable=self.search_by, 
                       value="id").pack(side='left', padx=20)
        
        # Search input
        input_frame = tk.Frame(self.main_container, bg='#f0f0f0')
        input_frame.pack(pady=20)
        
        ttk.Label(input_frame, text="Search Term:", background='#f0f0f0').pack(side='left', padx=5)
        self.search_term_var = tk.StringVar()
        search_entry = ttk.Entry(input_frame, textvariable=self.search_term_var, width=40)
        search_entry.pack(side='left', padx=5)
        
        ttk.Button(input_frame, text="Search", 
                  command=self.perform_patient_search).pack(side='left', padx=5)
        
        # Results treeview
        tree_frame = tk.Frame(self.main_container)
        tree_frame.pack(fill='both', expand=True, pady=10)
        
        # Create treeview for results
        columns = [('ID', 50), ('Name', 200), ('Age', 50), 
                  ('Gender', 80), ('Contact', 150), ('Admission Date', 120)]
        
        self.search_tree = ttk.Treeview(tree_frame, 
                                       columns=[col[0] for col in columns],
                                       show='headings',
                                       height=10)
        
        for col, width in columns:
            self.search_tree.heading(col, text=col)
            self.search_tree.column(col, width=width)
        
        scrollbar = ttk.Scrollbar(tree_frame, orient="vertical", 
                                 command=self.search_tree.yview)
        self.search_tree.configure(yscrollcommand=scrollbar.set)
        
        self.search_tree.pack(side='left', fill='both', expand=True)
        scrollbar.pack(side='right', fill='y')
    
    def perform_patient_search(self):
        """Execute patient search"""
        search_by = self.search_by.get()
        search_term = self.search_term_var.get().strip()
        
        if not search_term:
            messagebox.showwarning("Input Error", "Please enter a search term")
            return
        
        # Clear tree
        for item in self.search_tree.get_children():
            self.search_tree.delete(item)
        
        # Build query based on search type
        if search_by == "id" and search_term.isdigit():
            query = '''
                SELECT id, name, age, gender, contact, admission_date 
                FROM patients WHERE id = ?
            '''
            params = (int(search_term),)
        else:
            query = f'''
                SELECT id, name, age, gender, contact, admission_date 
                FROM patients WHERE {search_by} LIKE ?
            '''
            params = (f'%{search_term}%',)
        
        results = self.fetch_all(query, params)
        
        if not results:
            messagebox.showinfo("No Results", "No patients found matching your search")
            return
        
        # Add results to tree
        for result in results:
            self.search_tree.insert('', 'end', values=result)
    
    def show_update_patient(self):
        """Show update patient interface"""
        self.clear_container()
        self.create_back_button()
        
        ttk.Label(self.main_container, text="Update Patient", 
                 style='Title.TLabel').pack(pady=(0, 20))
        
        # Patient ID input
        id_frame = tk.Frame(self.main_container, bg='#f0f0f0')
        id_frame.pack(pady=10)
        
        ttk.Label(id_frame, text="Patient ID:", background='#f0f0f0').pack(side='left', padx=5)
        self.update_id_var = tk.StringVar()
        id_entry = ttk.Entry(id_frame, textvariable=self.update_id_var, width=20)
        id_entry.pack(side='left', padx=5)
        
        ttk.Button(id_frame, text="Load Patient", 
                  command=self.load_patient_for_update).pack(side='left', padx=10)
        
        # Form frame (initially hidden)
        self.update_form_frame = tk.Frame(self.main_container, bg='white', 
                                         relief='ridge', borderwidth=1)
        
        # Will be populated by load_patient_for_update()
    
    def load_patient_for_update(self):
        """Load patient data for updating"""
        patient_id = self.update_id_var.get().strip()
        
        if not patient_id or not patient_id.isdigit():
            messagebox.showwarning("Input Error", "Please enter a valid Patient ID")
            return
        
        # Fetch patient data
        query = "SELECT * FROM patients WHERE id = ?"
        patient = self.fetch_one(query, (patient_id,))
        
        if not patient:
            messagebox.showerror("Not Found", "Patient not found!")
            return
        
        # Create/Show update form
        self.update_form_frame.pack(padx=50, pady=20, fill='both', expand=True)
        
        # Clear previous widgets
        for widget in self.update_form_frame.winfo_children():
            widget.destroy()
        
        # Current info label
        ttk.Label(self.update_form_frame, text="Current Information (leave blank to keep):",
                 background='white', font=('Arial', 10, 'bold')).grid(row=0, column=0, 
                                                                     columnspan=2, pady=10)
        
        # Form fields
        fields = [
            ("Name:", "name", patient[1]),
            ("Age:", "age", patient[2]),
            ("Gender:", "gender", patient[3]),
            ("Contact:", "contact", patient[4]),
            ("Address:", "address", patient[5]),
            ("Disease:", "disease", patient[6]),
        ]
        
        self.update_vars = {}
        for i, (label, field, current_value) in enumerate(fields, start=1):
            ttk.Label(self.update_form_frame, text=label, background='white',
                     font=('Arial', 10)).grid(row=i, column=0, sticky='e', 
                                             padx=10, pady=10)
            
            if field == 'address':
                text_widget = tk.Text(self.update_form_frame, height=3, width=40,
                                     font=('Arial', 10))
                text_widget.insert('1.0', current_value or '')
                text_widget.grid(row=i, column=1, padx=10, pady=10, sticky='w')
                self.update_vars[field] = text_widget
            else:
                var = tk.StringVar(value=current_value or '')
                entry = ttk.Entry(self.update_form_frame, textvariable=var, 
                                 width=40, font=('Arial', 10))
                entry.grid(row=i, column=1, padx=10, pady=10, sticky='w')
                self.update_vars[field] = var
        
        # Update button
        ttk.Button(self.update_form_frame, text="Update Patient", 
                  command=lambda: self.update_patient_data(patient_id),
                  style='Success.TButton').grid(row=len(fields)+1, column=0, 
                                               columnspan=2, pady=20)
    
    def update_patient_data(self, patient_id):
        """Update patient in database"""
        try:
            # Get updated values
            name = self.update_vars['name'].get() if isinstance(self.update_vars['name'], tk.StringVar) else self.update_vars['name'].get("1.0", tk.END).strip()
            
            if isinstance(self.update_vars['address'], tk.Text):
                address = self.update_vars['address'].get("1.0", tk.END).strip()
            else:
                address = self.update_vars['address'].get().strip()
                
            age = self.update_vars['age'].get().strip()
            gender = self.update_vars['gender'].get().strip()
            contact = self.update_vars['contact'].get().strip()
            disease = self.update_vars['disease'].get().strip()
            
            # Build update query
            query = '''
                UPDATE patients 
                SET name = COALESCE(NULLIF(?, ''), name),
                    age = COALESCE(NULLIF(?, ''), age),
                    gender = COALESCE(NULLIF(?, ''), gender),
                    contact = COALESCE(NULLIF(?, ''), contact),
                    address = COALESCE(NULLIF(?, ''), address),
                    disease = COALESCE(NULLIF(?, ''), disease)
                WHERE id = ?
            '''
            
            params = (
                name if name else None,
                int(age) if age and age.isdigit() else None,
                gender if gender else None,
                contact if contact else None,
                address if address else None,
                disease if disease else None,
                patient_id
            )
            
            if self.execute_query(query, params):
                messagebox.showinfo("Success", "Patient updated successfully!")
                self.update_form_frame.pack_forget()
                self.update_id_var.set("")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to update patient: {str(e)}")
    
    def show_delete_patient(self):
        """Show delete patient interface"""
        self.clear_container()
        self.create_back_button()
        
        ttk.Label(self.main_container, text="Delete Patient", 
                 style='Title.TLabel').pack(pady=(0, 20))
        
        # Patient ID input
        id_frame = tk.Frame(self.main_container, bg='#f0f0f0')
        id_frame.pack(pady=10)
        
        ttk.Label(id_frame, text="Patient ID:", background='#f0f0f0').pack(side='left', padx=5)
        self.delete_id_var = tk.StringVar()
        id_entry = ttk.Entry(id_frame, textvariable=self.delete_id_var, width=20)
        id_entry.pack(side='left', padx=5)
        
        ttk.Button(id_frame, text="Load Patient", 
                  command=self.load_patient_for_delete).pack(side='left', padx=10)
        
        # Patient info frame
        self.delete_info_frame = tk.Frame(self.main_container, bg='white', 
                                         relief='ridge', borderwidth=1)
    
    def load_patient_for_delete(self):
        """Load patient data for deletion confirmation"""
        patient_id = self.delete_id_var.get().strip()
        
        if not patient_id or not patient_id.isdigit():
            messagebox.showwarning("Input Error", "Please enter a valid Patient ID")
            return
        
        # Fetch patient data
        query = "SELECT name, age, gender, contact FROM patients WHERE id = ?"
        patient = self.fetch_one(query, (patient_id,))
        
        if not patient:
            messagebox.showerror("Not Found", "Patient not found!")
            return
        
        # Show patient info and delete button
        self.delete_info_frame.pack(padx=50, pady=20, fill='both', expand=True)
        
        # Clear previous widgets
        for widget in self.delete_info_frame.winfo_children():
            widget.destroy()
        
        # Warning label
        warning = tk.Label(self.delete_info_frame, 
                          text="⚠️ WARNING: This will delete the patient and all associated bills!",
                          font=('Arial', 10, 'bold'), fg='red', bg='white')
        warning.pack(pady=10)
        
        # Patient info
        info_text = f"Patient: {patient[0]} (ID: {patient_id})\n"
        info_text += f"Age: {patient[1] if patient[1] else 'N/A'} | Gender: {patient[2] if patient[2] else 'N/A'}\n"
        info_text += f"Contact: {patient[3] if patient[3] else 'N/A'}"
        
        info_label = tk.Label(self.delete_info_frame, text=info_text,
                             font=('Arial', 10), bg='white', justify='left')
        info_label.pack(pady=20)
        
        # Delete button
        ttk.Button(self.delete_info_frame, text="CONFIRM DELETE", 
                  command=lambda: self.delete_patient_confirmed(patient_id),
                  style='Danger.TButton').pack(pady=20)
    
    def delete_patient_confirmed(self, patient_id):
        """Delete patient after confirmation"""
        if messagebox.askyesno("Confirm Delete", 
                              "Are you sure you want to delete this patient and all associated bills?"):
            query = "DELETE FROM patients WHERE id = ?"
            if self.execute_query(query, (patient_id,)):
                messagebox.showinfo("Success", "Patient deleted successfully!")
                self.delete_info_frame.pack_forget()
                self.delete_id_var.set("")
    
    # ==================== BILLING FUNCTIONS ====================
    
    def show_generate_bill(self):
        """Show generate bill form"""
        self.clear_container()
        self.create_back_button()
        
        ttk.Label(self.main_container, text="Generate New Bill", 
                 style='Title.TLabel').pack(pady=(0, 20))
        
        # Patient selection
        selection_frame = tk.Frame(self.main_container, bg='#f0f0f0')
        selection_frame.pack(pady=10, fill='x')
        
        ttk.Label(selection_frame, text="Select Patient:", 
                 background='#f0f0f0').pack(side='left', padx=5)
        
        # Fetch patients for dropdown
        patients = self.fetch_all("SELECT id, name FROM patients ORDER BY name")
        patient_names = [f"{pid}: {name}" for pid, name in patients]
        
        self.bill_patient_var = tk.StringVar()
        patient_combo = ttk.Combobox(selection_frame, 
                                    textvariable=self.bill_patient_var,
                                    values=patient_names, width=40)
        patient_combo.pack(side='left', padx=5)
        
        ttk.Button(selection_frame, text="Load Patient", 
                  command=self.load_patient_for_bill).pack(side='left', padx=10)
        
        # Bill form (initially hidden)
        self.bill_form_frame = tk.Frame(self.main_container, bg='white', 
                                       relief='ridge', borderwidth=1)
    
    def load_patient_for_bill(self):
        """Load patient and show bill form"""
        patient_str = self.bill_patient_var.get()
        if not patient_str:
            messagebox.showwarning("Input Error", "Please select a patient")
            return
        
        try:
            patient_id = int(patient_str.split(":")[0])
        except:
            messagebox.showerror("Error", "Invalid patient selection")
            return
        
        # Store patient ID
        self.current_bill_patient_id = patient_id
        
        # Show bill form
        self.bill_form_frame.pack(padx=50, pady=20, fill='both', expand=True)
        
        # Clear previous widgets
        for widget in self.bill_form_frame.winfo_children():
            widget.destroy()
        
        # Bill charges section
        charges_frame = tk.LabelFrame(self.bill_form_frame, text="Bill Charges", 
                                     bg='white', font=('Arial', 11, 'bold'))
        charges_frame.pack(fill='x', padx=20, pady=10)
        
        charges = [
            ("Room Charges ($):", "room_charges"),
            ("Doctor Fees ($):", "doctor_fees"),
            ("Medicine Charges ($):", "medicine_charges"),
            ("Lab Charges ($):", "lab_charges"),
            ("Other Charges ($):", "other_charges")
        ]
        
        self.bill_vars = {}
        for i, (label, field) in enumerate(charges):
            ttk.Label(charges_frame, text=label, background='white').grid(
                row=i, column=0, sticky='e', padx=10, pady=5)
            
            var = tk.StringVar(value="0.00")
            entry = ttk.Entry(charges_frame, textvariable=var, width=15)
            entry.grid(row=i, column=1, padx=10, pady=5, sticky='w')
            
            # Bind to recalculate total
            entry.bind('<KeyRelease>', self.calculate_bill_total)
            self.bill_vars[field] = var
        
        # Total amount
        ttk.Label(charges_frame, text="Total Amount ($):", 
                 background='white', font=('Arial', 10, 'bold')).grid(
                 row=len(charges), column=0, sticky='e', padx=10, pady=10)
        
        self.total_amount_var = tk.StringVar(value="0.00")
        ttk.Label(charges_frame, textvariable=self.total_amount_var,
                 background='white', font=('Arial', 10, 'bold'),
                 foreground='blue').grid(row=len(charges), column=1, 
                                       sticky='w', padx=10, pady=10)
        
        # Payment section
        payment_frame = tk.LabelFrame(self.bill_form_frame, text="Payment Details",
                                     bg='white', font=('Arial', 11, 'bold'))
        payment_frame.pack(fill='x', padx=20, pady=10)
        
        # Payment status
        ttk.Label(payment_frame, text="Payment Status:", 
                 background='white').grid(row=0, column=0, sticky='e', padx=10, pady=5)
        
        self.payment_status_var = tk.StringVar(value="Pending")
        status_combo = ttk.Combobox(payment_frame, 
                                   textvariable=self.payment_status_var,
                                   values=["Pending", "Partial", "Paid"], 
                                   state="readonly", width=15)
        status_combo.grid(row=0, column=1, padx=10, pady=5, sticky='w')
        status_combo.bind('<<ComboboxSelected>>', self.on_payment_status_change)
        
        # Amount paid
        ttk.Label(payment_frame, text="Amount Paid ($):", 
                 background='white').grid(row=1, column=0, sticky='e', padx=10, pady=5)
        
        self.amount_paid_var = tk.StringVar(value="0.00")
        amount_paid_entry = ttk.Entry(payment_frame, 
                                     textvariable=self.amount_paid_var,
                                     width=15, state='disabled')
        amount_paid_entry.grid(row=1, column=1, padx=10, pady=5, sticky='w')
        
        # Payment method
        ttk.Label(payment_frame, text="Payment Method:", 
                 background='white').grid(row=2, column=0, sticky='e', padx=10, pady=5)
        
        self.payment_method_var = tk.StringVar(value="Cash")
        method_combo = ttk.Combobox(payment_frame, 
                                   textvariable=self.payment_method_var,
                                   values=["Cash", "Credit Card", "Debit Card", 
                                          "Online Transfer", "Insurance"],
                                   state="readonly", width=15)
        method_combo.grid(row=2, column=1, padx=10, pady=5, sticky='w')
        
        # Generate bill button
        ttk.Button(self.bill_form_frame, text="Generate Bill", 
                  command=self.generate_bill, style='Success.TButton').pack(pady=20)
    
    def calculate_bill_total(self, event=None):
        """Calculate total bill amount"""
        try:
            total = 0.0
            for field in ['room_charges', 'doctor_fees', 'medicine_charges', 
                         'lab_charges', 'other_charges']:
                value = self.bill_vars[field].get().strip()
                if value:
                    total += float(value)
            self.total_amount_var.set(f"{total:.2f}")
        except:
            self.total_amount_var.set("0.00")
    
    def on_payment_status_change(self, event=None):
        """Handle payment status change"""
        status = self.payment_status_var.get()
        if status == "Paid":
            self.amount_paid_var.set(self.total_amount_var.get())
        elif status == "Pending":
            self.amount_paid_var.set("0.00")
    
    def generate_bill(self):
        """Generate and save bill"""
        try:
            # Calculate totals
            total_amount = float(self.total_amount_var.get())
            amount_paid = float(self.amount_paid_var.get())
            balance_due = total_amount - amount_paid
            
            # Get patient name
            query = "SELECT name FROM patients WHERE id = ?"
            patient = self.fetch_one(query, (self.current_bill_patient_id,))
            
            if not patient:
                messagebox.showerror("Error", "Patient not found!")
                return
            
            patient_name = patient[0]
            
            # Insert bill
            bill_query = '''
                INSERT INTO bills (
                    patient_id, patient_name, room_charges, doctor_fees,
                    medicine_charges, lab_charges, other_charges, total_amount,
                    amount_paid, balance_due, payment_status, payment_method
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            '''
            
            bill_params = (
                self.current_bill_patient_id,
                patient_name,
                float(self.bill_vars['room_charges'].get() or 0),
                float(self.bill_vars['doctor_fees'].get() or 0),
                float(self.bill_vars['medicine_charges'].get() or 0),
                float(self.bill_vars['lab_charges'].get() or 0),
                float(self.bill_vars['other_charges'].get() or 0),
                total_amount,
                amount_paid,
                balance_due,
                self.payment_status_var.get(),
                self.payment_method_var.get()
            )
            
            if self.execute_query(bill_query, bill_params):
                # Get the new bill number
                bill_no = self.cursor.lastrowid
                
                # Record payment if any
                if amount_paid > 0:
                    payment_query = '''
                        INSERT INTO payments (bill_no, amount, payment_method)
                        VALUES (?, ?, ?)
                    '''
                    self.execute_query(payment_query, (bill_no, amount_paid, 
                                                      self.payment_method_var.get()))
                
                messagebox.showinfo("Success", 
                                  f"Bill generated successfully!\nBill Number: {bill_no}")
                
                # Ask to print receipt
                if messagebox.askyesno("Print Receipt", "Would you like to print a receipt?"):
                    self.print_receipt(bill_no)
                
                # Clear form
                self.bill_form_frame.pack_forget()
                self.bill_patient_var.set("")
                
        except Exception as e:
            messagebox.showerror("Error", f"Failed to generate bill: {str(e)}")
    
    def show_view_bills(self):
        """Show all bills"""
        self.clear_container()
        self.create_back_button()
        
        ttk.Label(self.main_container, text="All Bills", 
                 style='Title.TLabel').pack(pady=(0, 20))
        
        # Create treeview
        tree_frame = tk.Frame(self.main_container)
        tree_frame.pack(fill='both', expand=True)
        
        # Columns
        columns = [
            ('Bill No', 80),
            ('Patient', 150),
            ('Total ($)', 100),
            ('Paid ($)', 100),
            ('Balance ($)', 100),
            ('Status', 80),
            ('Date', 100)
        ]
        
        self.bills_tree = ttk.Treeview(tree_frame, 
                                      columns=[col[0] for col in columns],
                                      show='headings')
        
        for col, width in columns:
            self.bills_tree.heading(col, text=col)
            self.bills_tree.column(col, width=width)
        
        # Scrollbar
        scrollbar = ttk.Scrollbar(tree_frame, orient="vertical", 
                                 command=self.bills_tree.yview)
        self.bills_tree.configure(yscrollcommand=scrollbar.set)
        
        self.bills_tree.pack(side='left', fill='both', expand=True)
        scrollbar.pack(side='right', fill='y')
        
        # Load bills
        self.load_bills()
        
        # Double-click to view details
        self.bills_tree.bind('<Double-Button-1>', self.show_bill_details)
    
    def load_bills(self):
        """Load bills into treeview"""
        # Clear existing items
        for item in self.bills_tree.get_children():
            self.bills_tree.delete(item)
        
        # Fetch bills
        query = '''
            SELECT bill_no, patient_name, total_amount, amount_paid,
                   balance_due, payment_status, bill_date
            FROM bills
            ORDER BY bill_no DESC
        '''
        
        bills = self.fetch_all(query)
        
        # Add to treeview
        for bill in bills:
            self.bills_tree.insert('', 'end', values=bill)
    
    def show_bill_details(self, event):
        """Show detailed bill view"""
        selection = self.bills_tree.selection()
        if not selection:
            return
        
        item = self.bills_tree.item(selection[0])
        bill_no = item['values'][0]
        
        # Fetch bill details
        query = "SELECT * FROM bills WHERE bill_no = ?"
        bill = self.fetch_one(query, (bill_no,))
        
        if bill:
            # Create details window
            details_window = tk.Toplevel(self.root)
            details_window.title(f"Bill Details - #{bill_no}")
            details_window.geometry("600x500")
            
            # Create notebook for tabs
            notebook = ttk.Notebook(details_window)
            notebook.pack(fill='both', expand=True, padx=10, pady=10)
            
            # Bill details tab
            details_tab = tk.Frame(notebook)
            notebook.add(details_tab, text="Bill Details")
            
            # Display bill information
            fields = [
                ("Bill Number:", bill[0]),
                ("Patient ID:", bill[1]),
                ("Patient Name:", bill[2]),
                ("Bill Date:", bill[3]),
                ("Room Charges:", f"${bill[4]:.2f}"),
                ("Doctor Fees:", f"${bill[5]:.2f}"),
                ("Medicine Charges:", f"${bill[6]:.2f}"),
                ("Lab Charges:", f"${bill[7]:.2f}"),
                ("Other Charges:", f"${bill[8]:.2f}"),
                ("Total Amount:", f"${bill[9]:.2f}"),
                ("Amount Paid:", f"${bill[10]:.2f}"),
                ("Balance Due:", f"${bill[11]:.2f}"),
                ("Payment Status:", bill[12]),
                ("Payment Method:", bill[13] or "N/A")
            ]
            
            for i, (label, value) in enumerate(fields):
                ttk.Label(details_tab, text=label, font=('Arial', 10, 'bold')).grid(
                    row=i, column=0, sticky='e', padx=10, pady=5)
                ttk.Label(details_tab, text=value, font=('Arial', 10)).grid(
                    row=i, column=1, sticky='w', padx=10, pady=5)
            
            # Payment history tab
            payments_tab = tk.Frame(notebook)
            notebook.add(payments_tab, text="Payment History")
            
            # Fetch payment history
            payment_query = '''
                SELECT payment_id, amount, payment_method, payment_date
                FROM payments WHERE bill_no = ?
                ORDER BY payment_date DESC
            '''
            payments = self.fetch_all(payment_query, (bill_no,))
            
            if payments:
                # Create treeview for payments
                payment_tree = ttk.Treeview(payments_tab, 
                                          columns=('ID', 'Amount', 'Method', 'Date'),
                                          show='headings')
                
                payment_tree.heading('ID', text='Payment ID')
                payment_tree.heading('Amount', text='Amount ($)')
                payment_tree.heading('Method', text='Method')
                payment_tree.heading('Date', text='Date')
                
                for payment in payments:
                    payment_tree.insert('', 'end', values=payment)
                
                payment_tree.pack(fill='both', expand=True, padx=10, pady=10)
            else:
                ttk.Label(payments_tab, text="No payment history found",
                         font=('Arial', 10)).pack(pady=20)
    
    def show_make_payment(self):
        """Show make payment interface"""
        self.clear_container()
        self.create_back_button()
        
        ttk.Label(self.main_container, text="Make Payment", 
                 style='Title.TLabel').pack(pady=(0, 20))
        
        # Bill selection
        selection_frame = tk.Frame(self.main_container, bg='#f0f0f0')
        selection_frame.pack(pady=10, fill='x')
        
        ttk.Label(selection_frame, text="Select Bill:", 
                 background='#f0f0f0').pack(side='left', padx=5)
        
        # Fetch pending bills
        bills = self.fetch_all('''
            SELECT bill_no, patient_name, balance_due 
            FROM bills 
            WHERE balance_due > 0 
            ORDER BY bill_no
        ''')
        
        bill_options = [f"{bill[0]}: {bill[1]} (Balance: ${bill[2]:.2f})" 
                       for bill in bills]
        
        self.payment_bill_var = tk.StringVar()
        bill_combo = ttk.Combobox(selection_frame, 
                                 textvariable=self.payment_bill_var,
                                 values=bill_options, width=50)
        bill_combo.pack(side='left', padx=5)
        
        ttk.Button(selection_frame, text="Load Bill", 
                  command=self.load_bill_for_payment).pack(side='left', padx=10)
        
        # Payment form (initially hidden)
        self.payment_form_frame = tk.Frame(self.main_container, bg='white', 
                                          relief='ridge', borderwidth=1)
    
    def load_bill_for_payment(self):
        """Load bill for payment"""
        bill_str = self.payment_bill_var.get()
        if not bill_str:
            messagebox.showwarning("Input Error", "Please select a bill")
            return
        
        try:
            bill_no = int(bill_str.split(":")[0])
        except:
            messagebox.showerror("Error", "Invalid bill selection")
            return
        
        # Store bill number
        self.current_payment_bill_no = bill_no
        
        # Get balance due from string
        import re
        balance_match = re.search(r'Balance: \$([\d.]+)', bill_str)
        if balance_match:
            self.current_bill_balance = float(balance_match.group(1))
        else:
            # Fallback: query database
            query = "SELECT balance_due FROM bills WHERE bill_no = ?"
            result = self.fetch_one(query, (bill_no,))
            if result:
                self.current_bill_balance = result[0]
            else:
                messagebox.showerror("Error", "Could not retrieve bill balance")
                return
        
        # Show payment form
        self.payment_form_frame.pack(padx=50, pady=20, fill='both', expand=True)
        
        # Clear previous widgets
        for widget in self.payment_form_frame.winfo_children():
            widget.destroy()
        
        # Payment amount
        ttk.Label(self.payment_form_frame, text=f"Balance Due: ${self.current_bill_balance:.2f}",
                 background='white', font=('Arial', 11, 'bold')).pack(pady=10)
        
        ttk.Label(self.payment_form_frame, text="Payment Amount ($):",
                 background='white').pack(pady=5)
        
        self.payment_amount_var = tk.StringVar()
        amount_entry = ttk.Entry(self.payment_form_frame, 
                                textvariable=self.payment_amount_var,
                                width=20, font=('Arial', 10))
        amount_entry.pack(pady=5)
        
        # Payment method
        ttk.Label(self.payment_form_frame, text="Payment Method:",
                 background='white').pack(pady=5)
        
        self.new_payment_method_var = tk.StringVar(value="Cash")
        method_combo = ttk.Combobox(self.payment_form_frame, 
                                   textvariable=self.new_payment_method_var,
                                   values=["Cash", "Credit Card", "Debit Card", 
                                          "Online Transfer", "Insurance"],
                                   state="readonly", width=20)
        method_combo.pack(pady=5)
        
        # Process payment button
        ttk.Button(self.payment_form_frame, text="Process Payment", 
                  command=self.process_payment, style='Success.TButton').pack(pady=20)
    
    def process_payment(self):
        """Process payment for selected bill"""
        try:
            payment_amount = float(self.payment_amount_var.get())
            
            # Validate payment amount
            if payment_amount <= 0:
                messagebox.showwarning("Validation Error", "Payment amount must be greater than 0")
                return
            
            if payment_amount > self.current_bill_balance:
                messagebox.showwarning("Validation Error", 
                                     f"Payment cannot exceed balance due (${self.current_bill_balance:.2f})")
                return
            
            # Update bill
            update_query = '''
                UPDATE bills 
                SET amount_paid = amount_paid + ?,
                    balance_due = balance_due - ?,
                    payment_status = CASE 
                        WHEN (balance_due - ?) <= 0 THEN 'Paid'
                        ELSE 'Partial'
                    END
                WHERE bill_no = ?
            '''
            
            if self.execute_query(update_query, (payment_amount, payment_amount, 
                                               payment_amount, self.current_payment_bill_no)):
                # Record payment
                payment_query = '''
                    INSERT INTO payments (bill_no, amount, payment_method)
                    VALUES (?, ?, ?)
                '''
                self.execute_query(payment_query, (self.current_payment_bill_no, 
                                                 payment_amount, 
                                                 self.new_payment_method_var.get()))
                
                messagebox.showinfo("Success", 
                                  f"Payment of ${payment_amount:.2f} recorded successfully!")
                
                # Clear form
                self.payment_form_frame.pack_forget()
                self.payment_bill_var.set("")
                
                # Refresh if on bills view
                if hasattr(self, 'bills_tree'):
                    self.load_bills()
                    
        except ValueError:
            messagebox.showerror("Error", "Please enter a valid payment amount")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to process payment: {str(e)}")
    
    # ==================== REPORTS & STATISTICS ====================
    
    def show_reports(self):
        """Show reports menu"""
        self.clear_container()
        self.create_back_button()
        
        ttk.Label(self.main_container, text="Reports", 
                 style='Title.TLabel').pack(pady=(0, 20))
        
        # Report options frame
        options_frame = tk.Frame(self.main_container, bg='#f0f0f0')
        options_frame.pack(pady=20)
        
        report_types = [
            ("Financial Summary", self.show_financial_summary),
            ("Outstanding Payments", self.show_outstanding_payments),
            ("Patient Statistics", self.show_patient_statistics),
            ("Revenue Report", self.show_revenue_report)
        ]
        
        for i, (text, command) in enumerate(report_types):
            btn = tk.Button(options_frame, text=text, font=('Arial', 11),
                          bg='#2ecc71', fg='white', padx=20, pady=15,
                          relief='raised', borderwidth=2,
                          command=command)
            btn.grid(row=i//2, column=i%2, padx=10, pady=10, sticky='nsew')
            btn.bind("<Enter>", lambda e, b=btn: b.config(bg='#27ae60'))
            btn.bind("<Leave>", lambda e, b=btn: b.config(bg='#2ecc71'))
        
        # Configure grid
        options_frame.columnconfigure(0, weight=1)
        options_frame.columnconfigure(1, weight=1)
    
    def show_financial_summary(self):
        """Display financial summary report"""
        report_window = tk.Toplevel(self.root)
        report_window.title("Financial Summary Report")
        report_window.geometry("600x400")
        
        # Fetch data
        query = '''
            SELECT 
                COUNT(*) as total_bills,
                SUM(total_amount) as total_billed,
                SUM(amount_paid) as total_paid,
                SUM(balance_due) as total_outstanding,
                AVG(total_amount) as avg_bill
            FROM bills
        '''
        
        result = self.fetch_one(query)
        
        if not result:
            ttk.Label(report_window, text="No billing data available",
                     font=('Arial', 12)).pack(pady=50)
            return
        
        total_bills, total_billed, total_paid, total_outstanding, avg_bill = result
        
        # Calculate collection rate
        collection_rate = (total_paid / total_billed * 100) if total_billed > 0 else 0
        
        # Create report text
        report_text = f"""
        FINANCIAL SUMMARY REPORT
        {'='*40}
        
        Total Bills Generated:      {total_bills or 0}
        Total Amount Billed:       ${total_billed or 0:,.2f}
        Total Amount Collected:    ${total_paid or 0:,.2f}
        Total Outstanding:         ${total_outstanding or 0:,.2f}
        Average Bill Amount:       ${avg_bill or 0:,.2f}
        Collection Rate:           {collection_rate:.1f}%
        
        {'='*40}
        
        Payment Status Distribution:
        """
        
        # Add payment status breakdown
        status_query = '''
            SELECT payment_status, COUNT(*), SUM(total_amount)
            FROM bills
            GROUP BY payment_status
        '''
        
        status_results = self.fetch_all(status_query)
        
        for status, count, amount in status_results:
            report_text += f"\n  {status}: {count} bills (${amount or 0:,.2f})"
        
        # Display report
        text_widget = tk.Text(report_window, font=('Courier', 10))
        text_widget.insert('1.0', report_text)
        text_widget.config(state='disabled')
        text_widget.pack(fill='both', expand=True, padx=10, pady=10)
        
        # Add export button
        export_btn = ttk.Button(report_window, text="Export to Text File",
                               command=lambda: self.export_report_to_file(report_text, 
                                                                         "financial_summary.txt"))
        export_btn.pack(pady=10)
    
    def show_outstanding_payments(self):
        """Display outstanding payments report"""
        report_window = tk.Toplevel(self.root)
        report_window.title("Outstanding Payments Report")
        report_window.geometry("800x500")
        
        # Fetch outstanding bills
        query = '''
            SELECT bill_no, patient_name, total_amount, amount_paid, 
                   balance_due, bill_date, payment_status
            FROM bills 
            WHERE balance_due > 0
            ORDER BY balance_due DESC
        '''
        
        bills = self.fetch_all(query)
        
        if not bills:
            ttk.Label(report_window, text="No outstanding payments",
                     font=('Arial', 12)).pack(pady=50)
            return
        
        # Create treeview
        tree_frame = tk.Frame(report_window)
        tree_frame.pack(fill='both', expand=True, padx=10, pady=10)
        
        # Create treeview
        columns = [
            ('Bill No', 80),
            ('Patient', 150),
            ('Total ($)', 100),
            ('Paid ($)', 100),
            ('Balance ($)', 100),
            ('Status', 80),
            ('Date', 100)
        ]
        
        tree = ttk.Treeview(tree_frame, columns=[col[0] for col in columns],
                           show='headings')
        
        for col, width in columns:
            tree.heading(col, text=col)
            tree.column(col, width=width)
        
        # Add data
        total_outstanding = 0
        for bill in bills:
            tree.insert('', 'end', values=bill)
            total_outstanding += bill[4]  # balance_due
        
        # Scrollbar
        scrollbar = ttk.Scrollbar(tree_frame, orient="vertical", 
                                 command=tree.yview)
        tree.configure(yscrollcommand=scrollbar.set)
        
        tree.pack(side='left', fill='both', expand=True)
        scrollbar.pack(side='right', fill='y')
        
        # Summary
        summary_frame = tk.Frame(report_window)
        summary_frame.pack(fill='x', padx=10, pady=10)
        
        ttk.Label(summary_frame, text=f"Total Outstanding Bills: {len(bills)}",
                 font=('Arial', 10, 'bold')).pack(side='left', padx=20)
        ttk.Label(summary_frame, text=f"Total Outstanding Amount: ${total_outstanding:,.2f}",
                 font=('Arial', 10, 'bold'), foreground='red').pack(side='left', padx=20)
    
    def show_patient_statistics(self):
        """Display patient statistics"""
        report_window = tk.Toplevel(self.root)
        report_window.title("Patient Statistics")
        report_window.geometry("500x400")
        
        # Fetch statistics
        query = '''
            SELECT 
                COUNT(*) as total,
                COUNT(CASE WHEN gender = 'M' THEN 1 END) as male,
                COUNT(CASE WHEN gender = 'F' THEN 1 END) as female,
                AVG(age) as avg_age,
                MIN(age) as min_age,
                MAX(age) as max_age
            FROM patients
        '''
        
        result = self.fetch_one(query)
        
        if not result:
            ttk.Label(report_window, text="No patient data available",
                     font=('Arial', 12)).pack(pady=50)
            return
        
        total, male, female, avg_age, min_age, max_age = result
        
        # Disease distribution
        disease_query = '''
            SELECT disease, COUNT(*) as count
            FROM patients
            WHERE disease IS NOT NULL AND disease != ''
            GROUP BY disease
            ORDER BY count DESC
            LIMIT 10
        '''
        
        disease_results = self.fetch_all(disease_query)
        
        # Create report
        report_text = f"""
        PATIENT STATISTICS
        {'='*40}
        
        Total Patients:       {total or 0}
        Male Patients:        {male or 0}
        Female Patients:      {female or 0}
        Other/Unknown:        {total - (male + female)}
        
        Age Statistics:
          Average Age:        {avg_age or 0:.1f} years
          Minimum Age:        {min_age or 0} years
          Maximum Age:        {max_age or 0} years
        
        {'='*40}
        
        Top 10 Diseases:
        """
        
        for disease, count in disease_results:
            report_text += f"\n  {disease}: {count} patients"
        
        if not disease_results:
            report_text += "\n  No disease data available"
        
        # Display report
        text_widget = tk.Text(report_window, font=('Courier', 10))
        text_widget.insert('1.0', report_text)
        text_widget.config(state='disabled')
        text_widget.pack(fill='both', expand=True, padx=10, pady=10)
    
    def show_revenue_report(self):
        """Display revenue report by month"""
        report_window = tk.Toplevel(self.root)
        report_window.title("Revenue Report")
        report_window.geometry("600x500")
        
        # Fetch monthly revenue
        query = '''
            SELECT 
                strftime('%Y-%m', bill_date) as month,
                COUNT(*) as bill_count,
                SUM(total_amount) as total_revenue,
                SUM(amount_paid) as collected,
                SUM(balance_due) as outstanding
            FROM bills
            GROUP BY strftime('%Y-%m', bill_date)
            ORDER BY month DESC
        '''
        
        results = self.fetch_all(query)
        
        if not results:
            ttk.Label(report_window, text="No revenue data available",
                     font=('Arial', 12)).pack(pady=50)
            return
        
        # Create treeview
        tree_frame = tk.Frame(report_window)
        tree_frame.pack(fill='both', expand=True, padx=10, pady=10)
        
        columns = [
            ('Month', 100),
            ('Bills', 80),
            ('Revenue ($)', 120),
            ('Collected ($)', 120),
            ('Outstanding ($)', 120)
        ]
        
        tree = ttk.Treeview(tree_frame, columns=[col[0] for col in columns],
                           show='headings')
        
        for col, width in columns:
            tree.heading(col, text=col)
            tree.column(col, width=width)
        
        # Add data
        for row in results:
            tree.insert('', 'end', values=row)
        
        # Scrollbar
        scrollbar = ttk.Scrollbar(tree_frame, orient="vertical", 
                                 command=tree.yview)
        tree.configure(yscrollcommand=scrollbar.set)
        
        tree.pack(side='left', fill='both', expand=True)
        scrollbar.pack(side='right', fill='y')
        
        # Summary
        total_revenue = sum(row[2] or 0 for row in results)
        total_collected = sum(row[3] or 0 for row in results)
        total_outstanding = sum(row[4] or 0 for row in results)
        
        summary_frame = tk.Frame(report_window)
        summary_frame.pack(fill='x', padx=10, pady=10)
        
        ttk.Label(summary_frame, text=f"Total Revenue: ${total_revenue:,.2f}",
                 font=('Arial', 10, 'bold')).pack(side='left', padx=20)
        ttk.Label(summary_frame, text=f"Total Collected: ${total_collected:,.2f}",
                 font=('Arial', 10, 'bold')).pack(side='left', padx=20)
        ttk.Label(summary_frame, text=f"Collection Rate: {(total_collected/total_revenue*100 if total_revenue>0 else 0):.1f}%",
                 font=('Arial', 10, 'bold')).pack(side='left', padx=20)
    
    def export_report_to_file(self, report_text, filename):
        """Export report to text file"""
        filepath = filedialog.asksaveasfilename(
            defaultextension=".txt",
            initialfile=filename,
            filetypes=[("Text files", "*.txt"), ("All files", "*.*")]
        )
        
        if filepath:
            try:
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(report_text)
                messagebox.showinfo("Success", f"Report exported to:\n{filepath}")
            except Exception as e:
                messagebox.showerror("Error", f"Failed to export report: {str(e)}")
    
    def show_statistics(self):
        """Show system statistics dashboard"""
        self.clear_container()
        self.create_back_button()
        
        ttk.Label(self.main_container, text="System Statistics", 
                 style='Title.TLabel').pack(pady=(0, 20))
        
        # Create dashboard
        dashboard_frame = tk.Frame(self.main_container, bg='#f0f0f0')
        dashboard_frame.pack(fill='both', expand=True, padx=20, pady=20)
        
        # Fetch statistics
        patient_stats = self.fetch_one('''
            SELECT COUNT(*), AVG(age) FROM patients
        ''') or (0, 0)
        
        billing_stats = self.fetch_one('''
            SELECT COUNT(*), SUM(total_amount), SUM(amount_paid), 
                   SUM(balance_due), AVG(total_amount)
            FROM bills
        ''') or (0, 0, 0, 0, 0)
        
        recent_patients = self.fetch_all('''
            SELECT name, admission_date 
            FROM patients 
            ORDER BY admission_date DESC 
            LIMIT 5
        ''') or []
        
        recent_bills = self.fetch_all('''
            SELECT bill_no, patient_name, total_amount 
            FROM bills 
            ORDER BY bill_date DESC 
            LIMIT 5
        ''') or []
        
        # Create statistics cards
        stats_grid = tk.Frame(dashboard_frame, bg='#f0f0f0')
        stats_grid.pack(fill='x', pady=(0, 20))
        
        stats_data = [
            ("Total Patients", patient_stats[0], "#3498db"),
            ("Average Age", f"{patient_stats[1]:.1f} years", "#2ecc71"),
            ("Total Bills", billing_stats[0], "#e74c3c"),
            ("Total Revenue", f"${billing_stats[1]:,.2f}", "#9b59b6"),
            ("Amount Collected", f"${billing_stats[2]:,.2f}", "#1abc9c"),
            ("Outstanding", f"${billing_stats[3]:,.2f}", "#f39c12"),
            ("Avg Bill Amount", f"${billing_stats[4]:,.2f}", "#34495e"),
        ]
        
        for i, (title, value, color) in enumerate(stats_data):
            card = tk.Frame(stats_grid, bg='white', relief='ridge', borderwidth=1)
            card.grid(row=i//3, column=i%3, padx=10, pady=10, sticky='nsew')
            
            tk.Label(card, text=title, font=('Arial', 10), 
                    bg='white', fg='#7f8c8d').pack(pady=(10, 5))
            tk.Label(card, text=str(value), font=('Arial', 16, 'bold'), 
                    bg='white', fg=color).pack(pady=(5, 10))
        
        # Configure grid
        for i in range(3):
            stats_grid.columnconfigure(i, weight=1)
        
        # Recent activity frames
        activity_frame = tk.Frame(dashboard_frame, bg='#f0f0f0')
        activity_frame.pack(fill='both', expand=True)
        
        # Recent patients
        patients_frame = tk.LabelFrame(activity_frame, text="Recent Patients", 
                                      bg='white', font=('Arial', 11, 'bold'))
        patients_frame.pack(side='left', fill='both', expand=True, padx=5)
        
        for patient in recent_patients:
            tk.Label(patients_frame, text=f"• {patient[0]} ({patient[1]})",
                    bg='white', font=('Arial', 9), anchor='w').pack(fill='x', padx=10, pady=2)
        
        if not recent_patients:
            tk.Label(patients_frame, text="No recent patients",
                    bg='white', font=('Arial', 9), fg='gray').pack(pady=10)
        
        # Recent bills
        bills_frame = tk.LabelFrame(activity_frame, text="Recent Bills", 
                                   bg='white', font=('Arial', 11, 'bold'))
        bills_frame.pack(side='right', fill='both', expand=True, padx=5)
        
        for bill in recent_bills:
            tk.Label(bills_frame, text=f"Bill #{bill[0]}: {bill[1]} (${bill[2]:.2f})",
                    bg='white', font=('Arial', 9), anchor='w').pack(fill='x', padx=10, pady=2)
        
        if not recent_bills:
            tk.Label(bills_frame, text="No recent bills",
                    bg='white', font=('Arial', 9), fg='gray').pack(pady=10)
    
    # ==================== SYSTEM FUNCTIONS ====================
    
    def backup_database(self):
        """Create database backup"""
        try:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            backup_filename = f"hospital_backup_{timestamp}.db"
            
            # Create backups directory if not exists
            if not os.path.exists("backups"):
                os.makedirs("backups")
            
            backup_path = os.path.join("backups", backup_filename)
            
            # Close current connection
            self.conn.close()
            
            # Copy database file
            import shutil
            shutil.copy2("hospital.db", backup_path)
            
            # Reopen connection
            self.conn = sqlite3.connect('hospital.db', check_same_thread=False)
            self.cursor = self.conn.cursor()
            
            messagebox.showinfo("Backup Successful", 
                              f"Database backed up to:\n{backup_path}")
            
        except Exception as e:
            messagebox.showerror("Backup Failed", f"Failed to create backup: {str(e)}")
            # Try to reopen connection
            try:
                self.conn = sqlite3.connect('hospital.db', check_same_thread=False)
                self.cursor = self.conn.cursor()
            except:
                pass
    
    def show_export_menu(self):
        """Show export data menu"""
        self.clear_container()
        self.create_back_button()
        
        ttk.Label(self.main_container, text="Export Data", 
                 style='Title.TLabel').pack(pady=(0, 20))
        
        # Export options
        options_frame = tk.Frame(self.main_container, bg='#f0f0f0')
        options_frame.pack(pady=20)
        
        export_options = [
            ("Export Patients to CSV", "patients"),
            ("Export Bills to CSV", "bills"),
            ("Export Payments to CSV", "payments"),
            ("Export All Data", "all")
        ]
        
        for i, (text, data_type) in enumerate(export_options):
            btn = tk.Button(options_frame, text=text, font=('Arial', 11),
                          bg='#3498db', fg='white', padx=20, pady=15,
                          relief='raised', borderwidth=2,
                          command=lambda dt=data_type: self.export_data(dt))
            btn.grid(row=i//2, column=i%2, padx=10, pady=10, sticky='nsew')
            btn.bind("<Enter>", lambda e, b=btn: b.config(bg='#2980b9'))
            btn.bind("<Leave>", lambda e, b=btn: b.config(bg='#3498db'))
        
        # Configure grid
        options_frame.columnconfigure(0, weight=1)
        options_frame.columnconfigure(1, weight=1)
    
    def export_data(self, data_type):
        """Export data to CSV"""
        try:
            if data_type == "patients":
                filename = "patients.csv"
                query = "SELECT * FROM patients"
            elif data_type == "bills":
                filename = "bills.csv"
                query = "SELECT * FROM bills"
            elif data_type == "payments":
                filename = "payments.csv"
                query = "SELECT * FROM payments"
            elif data_type == "all":
                # Export all three
                self.export_data("patients")
                self.export_data("bills")
                self.export_data("payments")
                return
            
            # Ask for save location
            filepath = filedialog.asksaveasfilename(
                defaultextension=".csv",
                initialfile=filename,
                filetypes=[("CSV files", "*.csv"), ("All files", "*.*")]
            )
            
            if not filepath:
                return  # User cancelled
            
            # Fetch data
            data = self.fetch_all(query)
            
            # Get column names
            self.cursor.execute(query)
            column_names = [description[0] for description in self.cursor.description]
            
            # Write to CSV
            with open(filepath, 'w', newline='', encoding='utf-8') as f:
                writer = csv.writer(f)
                writer.writerow(column_names)  # Header
                writer.writerows(data)         # Data
            
            messagebox.showinfo("Export Successful", 
                              f"Data exported to:\n{filepath}\n\n{len(data)} records exported.")
            
        except Exception as e:
            messagebox.showerror("Export Failed", f"Failed to export data: {str(e)}")
    
    def print_receipt(self, bill_no):
        """Generate printable receipt"""
        try:
            # Fetch bill details
            query = "SELECT * FROM bills WHERE bill_no = ?"
            bill = self.fetch_one(query, (bill_no,))
            
            if not bill:
                messagebox.showerror("Error", "Bill not found!")
                return
            
            # Create receipt text
            receipt_text = f"""
            {'='*50}
                         CITY GENERAL HOSPITAL
            {'='*50}
            
            Receipt No: {bill[0]}
            Date:       {bill[3]}
            
            Patient:    {bill[2]}
            Patient ID: {bill[1]}
            
            {'-'*50}
            Room Charges:       ${bill[4]:10.2f}
            Doctor Fees:        ${bill[5]:10.2f}
            Medicine Charges:   ${bill[6]:10.2f}
            Lab Charges:        ${bill[7]:10.2f}
            Other Charges:      ${bill[8]:10.2f}
            {'-'*50}
            TOTAL AMOUNT:       ${bill[9]:10.2f}
            AMOUNT PAID:        ${bill[10]:10.2f}
            BALANCE DUE:        ${bill[11]:10.2f}
            {'-'*50}
            
            Payment Status: {bill[12]}
            Payment Method: {bill[13] or 'N/A'}
            
            {'='*50}
            Thank you for choosing our hospital!
            {'='*50}
            """
            
            # Ask for save location
            filepath = filedialog.asksaveasfilename(
                defaultextension=".txt",
                initialfile=f"receipt_{bill_no}.txt",
                filetypes=[("Text files", "*.txt"), ("All files", "*.*")]
            )
            
            if filepath:
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(receipt_text)
                
                messagebox.showinfo("Receipt Saved", 
                                  f"Receipt saved to:\n{filepath}")
            
        except Exception as e:
            messagebox.showerror("Error", f"Failed to generate receipt: {str(e)}")
    
    def create_back_button(self):
        """Create back button in container"""
        back_frame = tk.Frame(self.main_container, bg='#f0f0f0')
        back_frame.pack(anchor='nw', pady=(0, 10))
        
        back_btn = ttk.Button(back_frame, text="← Back to Menu", 
                             command=self.show_main_menu,
                             style='Primary.TButton')
        back_btn.pack()
    
    def on_closing(self):
        """Handle window closing"""
        if messagebox.askokcancel("Quit", "Do you want to quit?"):
            self.conn.close()
            self.root.destroy()

# ==================== MAIN ENTRY POINT ====================

if __name__ == "__main__":
    root = tk.Tk()
    app = HospitalBillingSystem(root)
    
    # Set window icon (optional)
    try:
        root.iconbitmap('hospital_icon.ico')  # Windows
    except:
        pass
    
    # Handle window close
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    
    # Start main loop
    root.mainloop()