#!/bin/bash

echo "=========================================="
echo "  Hospital Patient Billing System Setup"
echo "=========================================="

echo "Building application..."
make clean
make

if [ -f "hospital_billing" ]; then
    echo ""
    echo "✅ Setup completed successfully!"
    echo ""
    echo "To run the system:"
    echo "  ./hospital_billing"
    echo ""
    echo "Available commands:"
    echo "  make clean     - Clean build files"
    echo "  make           - Rebuild application"
    echo "  make run       - Build and run"
else
    echo "❌ Build failed! Check for errors."
    exit 1
fi
