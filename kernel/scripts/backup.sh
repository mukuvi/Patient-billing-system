#!/bin/bash

BACKUP_DIR="backups"
mkdir -p "$BACKUP_DIR"

TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
BACKUP_FILE="$BACKUP_DIR/hospital_backup_$TIMESTAMP.db"

echo "Creating backup: $BACKUP_FILE"

if [ -f "hospital.db" ]; then
    cp hospital.db "$BACKUP_FILE"
    echo "✅ Backup created successfully!"
    
    # Keep only last 10 backups
    echo "Cleaning old backups..."
    ls -t "$BACKUP_DIR"/hospital_backup_*.db 2>/dev/null | tail -n +11 | xargs -r rm -f
    
    echo ""
    echo "Current backups:"
    ls -la "$BACKUP_DIR"/hospital_backup_*.db 2>/dev/null || echo "No backup files"
else
    echo "❌ Error: hospital.db not found!"
    exit 1
fi
