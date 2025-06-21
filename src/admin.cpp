/*
 * Copyright 2025 Runtime Assassins
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
 
#include <sqlite3.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cerrno>
#include "sha256.h"
#include <openssl/hmac.h>
#include <openssl/sha.h>

void initDatabase(sqlite3* db) {
    const char* sql = "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password TEXT, role TEXT, totp_secret TEXT);";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Database init error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cerr << "Database initialized successfully" << std::endl;
    }
}

bool addUser(sqlite3* db, const std::string& username, const std::string& password, const std::string& role) {
    if (role != "admin" && role != "manager" && role != "biller") {
        std::cout << "Invalid role. Must be 'admin', 'manager', or 'biller'.\n";
        return false;
    }

    std::string totp_secret = generate_base32_secret();
    std::cout << "TOTP Secret Key: " << totp_secret << std::endl;

    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO users (username, password, role, totp_secret) VALUES (?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    std::string hashedPassword = sha256(password);
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, role.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, totp_secret.c_str(), -1, SQLITE_STATIC);

    int attempts = 3;
    bool totp_verified = false;
    while (attempts > 0 && !totp_verified) {
        std::string totp_code;
        std::cout << "Enter the 6-digit TOTP code (" << attempts << " attempts left): ";
        std::getline(std::cin, totp_code);

        if (verify_totp(totp_secret, totp_code)) {
            totp_verified = true;
        } else {
            std::cout << "Invalid TOTP code. Try again." << std::endl;
            attempts--;
        }
    }

    if (!totp_verified) {
        std::cout << "Too many failed attempts. User not added." << std::endl;
        std::cout << "Ensure system clock is synced (e.g., 'sudo sntp -s time.apple.com') and verify secret: " << totp_secret << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    if (!success) {
        std::cerr << "SQL insert error: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cerr << "User added successfully: " << username << std::endl;
    }
    sqlite3_finalize(stmt);
    return success;
}

bool viewUsers(sqlite3* db) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT username, password, role FROM users;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    std::cout << "\nUsers in the database:\n";
    std::cout << "Username\tPassword (SHA-256 Hash)\t\t\t\t\t\tRole\n";
    std::cout << "--------\t-----------------------------------------------\t----\n";
    bool hasUsers = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        hasUsers = true;
        const char* username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::cout << (username ? username : "null") << "\t"
                  << (password ? password : "null") << "\t"
                  << (role ? role : "null") << "\n";
    }
    if (!hasUsers) {
        std::cout << "No users found.\n";
    }
    sqlite3_finalize(stmt);
    return true;
}

bool deleteUser(sqlite3* db, const std::string& username) {
    sqlite3_stmt* stmt;
    const char* sql = "DELETE FROM users WHERE username = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    if (!success || sqlite3_changes(db) == 0) {
        std::cerr << "No user found with username: " << username << std::endl;
    } else {
        std::cerr << "User deleted successfully: " << username << std::endl;
    }
    sqlite3_finalize(stmt);
    return success;
}

bool resetTotpSecret(sqlite3* db, const std::string& username) {
    std::string new_totp_secret = generate_base32_secret();
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE users SET totp_secret = ? WHERE username = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, new_totp_secret.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    if (!success || sqlite3_changes(db) == 0) {
        std::cerr << "No user found with username: " << username << std::endl;
    } else {
        std::cerr << "TOTP secret reset successfully for: " << username << std::endl;
        std::cout << "New TOTP Secret Key: " << new_totp_secret << std::endl;
    }
    sqlite3_finalize(stmt);
    return success;
}

void viewActivityLogFile() {
    const std::string log_path = "/Users/rudra/Library/Mobile Documents/com~apple~CloudDocs/RUDRA FILES/SEM-2/Hackathon/activity_log.txt";
    std::ifstream log_file(log_path);
    if (!log_file.good()) {
        std::cerr << "File does not exist or is inaccessible: " << log_path << " (errno: " << strerror(errno) << ")\n";
        return;
    }
    if (log_file.is_open()) {
        std::cout << "\nActivity Log (" << log_path << "):\n";
        std::cout << "-------------\n";
        std::string line;
        bool has_logs = false;
        while (std::getline(log_file, line)) {
            has_logs = true;
            std::cout << line << "\n";
        }
        if (!has_logs) {
            std::cout << "No logs found in " << log_path << "\n";
        } else {
            std::cout << "Successfully read logs from " << log_path << "\n";
        }
        log_file.close();
    } else {
        std::cerr << "Failed to open " << log_path << " for reading: " << strerror(errno) << "\n";
    }
}

int main() {
    sqlite3* db;
    const char* db_path = "/Users/rudra/Library/Mobile Documents/com~apple~CloudDocs/RUDRA FILES/SEM-2/Hackathon/users.db";
    std::cerr << "Opening database at " << db_path << std::endl;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }
    initDatabase(db);

    std::string choice;
    while (true) {
        std::cout << "\nAdmin Panel Menu:\n";
        std::cout << "1. Add User\n";
        std::cout << "2. View Users\n";
        std::cout << "3. Delete User\n";
        std::cout << "4. Reset TOTP Secret\n";
        std::cout << "5. View Activity Log\n";
        std::cout << "6. Exit\n";
        std::cout << "Enter choice (1-6): ";
        std::getline(std::cin, choice);

        if (choice == "1") {
            std::string username, password, role;
            std::cout << "Enter username: ";
            std::getline(std::cin, username);
            std::cout << "Enter password: ";
            std::getline(std::cin, password);
            std::cout << "Enter role (admin/manager/biller): ";
            std::getline(std::cin, role);

            if (addUser(db, username, password, role)) {
                std::cout << "User added successfully with TOTP enabled!\n";
            } else {
                std::cout << "Failed to add user.\n";
            }
        } else if (choice == "2") {
            viewUsers(db);
        } else if (choice == "3") {
            std::string username;
            std::cout << "Enter username to delete: ";
            std::getline(std::cin, username);
            deleteUser(db, username);
        } else if (choice == "4") {
            std::string username;
            std::cout << "Enter username to reset TOTP secret: ";
            std::getline(std::cin, username);
            resetTotpSecret(db, username);
        } else if (choice == "5") {
            viewActivityLogFile();
        } else if (choice == "6") {
            std::cout << "Exiting Admin Panel.\n";
            break;
        } else {
            std::cout << "Invalid choice. Please enter 1-6.\n";
        }
    }

    sqlite3_close(db);
    return 0;
}
