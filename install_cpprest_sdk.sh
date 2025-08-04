#!/bin/bash

echo "Updating package list..."
sudo apt update

echo "Installing cpprestsdk (libcpprest-dev)..."
sudo apt install -y libcpprest-dev

echo "Installation complete."