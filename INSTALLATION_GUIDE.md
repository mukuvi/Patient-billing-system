# Hospital Patient Billing System - Installation Guide

## 📋 System Overview

This project provides two implementations of a Hospital Patient Billing System:
1. **C Console Version** - Terminal-based application (kernel/)
2. **Python GUI Version** - Graphical user interface (gui/)

## 🖥️ System Requirements

### For C Version:
- GCC compiler (MinGW on Windows)
- SQLite3 library
- Linux/macOS/Windows with terminal

### For Python GUI Version:
- Python 3.6 or higher
- Tkinter (usually included with Python)
- SQLite3 (included with Python)

## ⚡ Quick Installation

### Option 1: Install Both Versions

```bash
# Clone the repository
git clone <repository-url>
cd Patient-billing-system

# Install C version
cd kernel
make  # or gcc hospital_billing.c -o hospital_billing -lsqlite3

# Install Python version
cd ../gui
pip install -r requirements.txt  # or pip3