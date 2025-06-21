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

 
#define GL_SILENCE_DEPRECATION
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cerrno>
#include <ctime>
#include <algorithm>
#include <filesystem>
#include <ctime>
#include "sha256.h"

struct MenuItem {
    int id;
    std::string name;
    float price;
    bool available;
};

struct OrderItem {
    int item_id;
    std::string name;
    int quantity;
    float price;
};

struct Order {
    int order_id;
    std::string user_id;
    std::string status;
    float total;
    int created_at;
    std::vector<OrderItem> items;
};

struct Bill {
    int bill_id;
    int order_id;
    float tax;
    float total;
    std::string payment_method;
    int created_at;
    bool refunded;
};

struct Wallet {
    std::string user_id;
    float balance;
};

struct Discount {
    int discount_id;
    std::string name;
    std::string type;
    float value;
    int start_time;
    int end_time;
    std::string combo_items;
};

struct Inventory {
    int item_id;
    int quantity;
    int low_stock_threshold;
};

struct LoyaltyPoints {
    std::string user_id;
    int points;
};

struct LoyaltyTransaction {
    int transaction_id;
    std::string user_id;
    int points;
    std::string type;
    int timestamp;
};

struct ActivityLog {
    int log_id;
    std::string user_id;
    std::string action;
    int timestamp;
};

struct SalesData {
    float total_sales;
    int order_count;
};

struct TopItem {
    int item_id;
    std::string name;
    int total_quantity;
};

struct UserDetails {
    std::string username;
    std::string last_order;
    int loyalty_points;
    float wallet_balance;
};



void initDatabase(sqlite3* db) {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            username TEXT PRIMARY KEY,
            password TEXT,
            role TEXT,
            totp_secret TEXT
        );
        CREATE TABLE IF NOT EXISTS menu_items (
            item_id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            price REAL NOT NULL,
            available INTEGER NOT NULL DEFAULT 1
        );
        CREATE TABLE IF NOT EXISTS orders (
            order_id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id TEXT,
            status TEXT NOT NULL,
            total REAL NOT NULL,
            created_at INTEGER NOT NULL
        );
        CREATE TABLE IF NOT EXISTS order_items (
            order_item_id INTEGER PRIMARY KEY AUTOINCREMENT,
            order_id INTEGER NOT NULL,
            item_id INTEGER NOT NULL,
            quantity INTEGER NOT NULL,
            price REAL NOT NULL,
            FOREIGN KEY (order_id) REFERENCES orders(order_id),
            FOREIGN KEY (item_id) REFERENCES menu_items(item_id)
        );
        CREATE TABLE IF NOT EXISTS bills (
            bill_id INTEGER PRIMARY KEY AUTOINCREMENT,
            order_id INTEGER NOT NULL,
            tax REAL NOT NULL,
            total REAL NOT NULL,
            payment_method TEXT NOT NULL,
            created_at INTEGER NOT NULL,
            refunded INTEGER NOT NULL DEFAULT 0,
            FOREIGN KEY (order_id) REFERENCES orders(order_id)
        );
        CREATE TABLE IF NOT EXISTS wallets (
            user_id TEXT PRIMARY KEY,
            balance REAL NOT NULL DEFAULT 0,
            FOREIGN KEY (user_id) REFERENCES users(username)
        );
        CREATE TABLE IF NOT EXISTS discounts (
            discount_id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            type TEXT NOT NULL,
            value REAL NOT NULL,
            start_time INTEGER,
            end_time INTEGER,
            combo_items TEXT
        );
        CREATE TABLE IF NOT EXISTS inventory (
            item_id INTEGER PRIMARY KEY,
            quantity INTEGER NOT NULL DEFAULT 0,
            low_stock_threshold INTEGER NOT NULL DEFAULT 10,
            FOREIGN KEY (item_id) REFERENCES menu_items(item_id)
        );
        CREATE TABLE IF NOT EXISTS loyalty_points (
            user_id TEXT PRIMARY KEY,
            points INTEGER NOT NULL DEFAULT 0,
            FOREIGN KEY (user_id) REFERENCES users(username)
        );
        CREATE TABLE IF NOT EXISTS loyalty_transactions (
            transaction_id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id TEXT NOT NULL,
            points INTEGER NOT NULL,
            type TEXT NOT NULL,
            timestamp INTEGER NOT NULL,
            FOREIGN KEY (user_id) REFERENCES loyalty_points(user_id)
        );
        CREATE TABLE IF NOT EXISTS activity_log (
            log_id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id TEXT,
            action TEXT NOT NULL,
            timestamp INTEGER NOT NULL
        );
        CREATE TABLE IF NOT EXISTS settings (
            key TEXT PRIMARY KEY,
            value REAL NOT NULL
        );
    )";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Database init error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}




void logActivity(sqlite3* db, const std::string& user_id, const std::string& action) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO activity_log (user_id, action, timestamp) VALUES (?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (user_id.empty()) {
            sqlite3_bind_null(stmt, 1);
        } else {
            sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_STATIC);
        }
        sqlite3_bind_text(stmt, 2, action.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, std::time(nullptr));
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "SQL insert error (activity_log): " << sqlite3_errmsg(db) << std::endl;
        } else {
            std::cerr << "Logged to database: User: " << (user_id.empty() ? "None" : user_id) << ", Action: " << action << std::endl;
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "SQL prepare error (activity_log): " << sqlite3_errmsg(db) << std::endl;
    }

    const std::string log_path = "/Users/rudra/Library/Mobile Documents/com~apple~CloudDocs/RUDRA FILES/SEM-2/Hackathon/activity_log.txt";
    std::ofstream log_file(log_path, std::ios::app);
    if (log_file.is_open()) {
        time_t now = std::time(nullptr);
        log_file << "[" << std::ctime(&now) << "] User: " << (user_id.empty() ? "None" : user_id)
                 << ", Action: " << action << "\n";
        log_file.close();
        std::cerr << "Successfully logged to " << log_path << ": User: " << (user_id.empty() ? "None" : user_id) << ", Action: " << action << std::endl;
    } else {
        std::cerr << "Failed to open " << log_path << " for writing: " << strerror(errno) << std::endl;
    }
}

std::vector<ActivityLog> viewActivityLog(sqlite3* db) {
    std::vector<ActivityLog> logs;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT log_id, user_id, action, timestamp FROM activity_log ORDER BY timestamp DESC;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            ActivityLog log;
            log.log_id = sqlite3_column_int(stmt, 0);
            log.user_id = sqlite3_column_text(stmt, 1) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) : "";
            log.action = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            log.timestamp = sqlite3_column_int(stmt, 3);
            logs.push_back(log);
        }
        sqlite3_finalize(stmt);
    }
    return logs;
}

bool checkPassword(sqlite3* db, const std::string& username, const std::string& password, std::string& role, std::string& totp_secret) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT password, role, totp_secret FROM users WHERE username = ?";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    bool valid = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* storedPass = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* storedRole = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* storedTotpSecret = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string hashedInput = sha256(password);
        if (storedPass && storedPass == hashedInput) {
            role = storedRole ? storedRole : "unknown";
            totp_secret = storedTotpSecret ? storedTotpSecret : "";
            valid = true;
        }
    }
    sqlite3_finalize(stmt);
    return valid;
}

bool checkTotp(const std::string& totp_secret, const std::string& totp_code) {
    if (totp_secret.empty() || totp_code.empty()) {
        std::cerr << "TOTP verification failed: empty secret or code" << std::endl;
        return false;
    }
    return verify_totp(totp_secret, totp_code);
}

std::string formatTimestamp(int timestamp) {
    time_t time = timestamp;
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", localtime(&time));
    return std::string(buffer);
}

// std::string getTimestampForFilename() {
//     time_t now = std::time(nullptr);
//     char buffer[20];
//     strftime(buffer, sizeof(buffer), "%d%m%Y%H%M%S", localtime(&now));
//     return std::string(buffer);
// }

void drawDoodle(ImDrawList* draw_list, ImVec2 center, float radius) {
    draw_list->AddCircle(center, radius, IM_COL32(200, 200, 200, 50), 12, 1.0f);
    for (int i = 0; i < 8; i++) {
        float angle = i * 3.14159f / 4.0f;
        ImVec2 p1 = ImVec2(center.x + radius * cos(angle), center.y + radius * sin(angle));
        ImVec2 p2 = ImVec2(center.x + (radius + 20) * cos(angle), center.y + (radius + 20) * sin(angle));
        draw_list->AddLine(p1, p2, IM_COL32(200, 200, 200, 50), 1.0f);
    }
}

bool addMenuItem(sqlite3* db, const std::string& name, float price, bool available) {
    sqlite3_stmt* menu_stmt = nullptr;
    sqlite3_stmt* inv_stmt = nullptr;
    const char* sql = "INSERT INTO menu_items (name, price, available) VALUES (?, ?, ?);";
    bool success = false;

    if (sqlite3_prepare_v2(db, sql, -1, &menu_stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(menu_stmt, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(menu_stmt, 2, price);
        sqlite3_bind_int(menu_stmt, 3, available ? 1 : 0);
        if (sqlite3_step(menu_stmt) == SQLITE_DONE) {
            int item_id = sqlite3_last_insert_rowid(db);
            const char* inv_sql = "INSERT OR REPLACE INTO inventory (item_id, quantity, low_stock_threshold) VALUES (?, 0, 10);";
            if (sqlite3_prepare_v2(db, inv_sql, -1, &inv_stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(inv_stmt, 1, item_id);
                if (sqlite3_step(inv_stmt) == SQLITE_DONE) {
                    success = true;
                } else {
                    std::cerr << "SQL insert error (inventory): " << sqlite3_errmsg(db) << std::endl;
                }
                sqlite3_finalize(inv_stmt);
            } else {
                std::cerr << "SQL prepare error (inventory): " << sqlite3_errmsg(db) << std::endl;
            }
        } else {
            std::cerr << "SQL insert error (menu_items): " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(menu_stmt);
    } else {
        std::cerr << "SQL prepare error (menu_items): " << sqlite3_errmsg(db) << std::endl;
    }

    return success;
}

void editMenuItem(sqlite3* db, int item_id, const std::string& name, float price, bool available) {
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE menu_items SET name = ?, price = ?, available = ? WHERE item_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 2, price);
        sqlite3_bind_int(stmt, 3, available ? 1 : 0);
        sqlite3_bind_int(stmt, 4, item_id);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "SQL update error: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
    }
}

void deleteMenuItem(sqlite3* db, int item_id) {
    sqlite3_stmt* stmt;
    const char* sql = "DELETE FROM menu_items WHERE item_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, item_id);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "SQL delete error: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
    }
    const char* inv_sql = "DELETE FROM inventory WHERE item_id = ?;";
    if (sqlite3_prepare_v2(db, inv_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, item_id);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "SQL delete error (inventory): " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
    }
}

std::vector<MenuItem> viewMenuItems(sqlite3* db, bool available_only = false) {
    std::vector<MenuItem> items;
    sqlite3_stmt* stmt;
    std::string sql = available_only ?
        "SELECT mi.item_id, mi.name, mi.price, mi.available FROM menu_items mi "
        "JOIN inventory i ON mi.item_id = i.item_id WHERE mi.available = 1 AND i.quantity > 0;" :
        "SELECT item_id, name, price, available FROM menu_items;";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            MenuItem item;
            item.id = sqlite3_column_int(stmt, 0);
            item.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            item.price = sqlite3_column_double(stmt, 2);
            item.available = sqlite3_column_int(stmt, 3) == 1;
            items.push_back(item);
        }
        sqlite3_finalize(stmt);
    }
    return items;
}

void addInventory(sqlite3* db, int item_id, int quantity, int low_stock_threshold) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT OR REPLACE INTO inventory (item_id, quantity, low_stock_threshold) VALUES (?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, item_id);
        sqlite3_bind_int(stmt, 2, quantity);
        sqlite3_bind_int(stmt, 3, low_stock_threshold);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "SQL insert error (inventory): " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
    }
}

void updateInventory(sqlite3* db, int item_id, int quantity, int low_stock_threshold) {
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE inventory SET quantity = ?, low_stock_threshold = ? WHERE item_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, quantity);
        sqlite3_bind_int(stmt, 2, low_stock_threshold);
        sqlite3_bind_int(stmt, 3, item_id);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "SQL update error (inventory): " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
    }
}

std::vector<Inventory> viewInventory(sqlite3* db) {
    std::vector<Inventory> inventory;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT item_id, quantity, low_stock_threshold FROM inventory;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Inventory inv;
            inv.item_id = sqlite3_column_int(stmt, 0);
            inv.quantity = sqlite3_column_int(stmt, 1);
            inv.low_stock_threshold = sqlite3_column_int(stmt, 2);
            inventory.push_back(inv);
        }
        sqlite3_finalize(stmt);
    }
    return inventory;
}

void addLoyaltyPoints(sqlite3* db, const std::string& user_id, int points, const std::string& type) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT OR REPLACE INTO loyalty_points (user_id, points) VALUES (?, COALESCE((SELECT points FROM loyalty_points WHERE user_id = ?) + ?, ?));";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, user_id.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, points);
        sqlite3_bind_int(stmt, 4, points);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "SQL insert error (loyalty_points): " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
    }

    const char* trans_sql = "INSERT INTO loyalty_transactions (user_id, points, type, timestamp) VALUES (?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, trans_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, points);
        sqlite3_bind_text(stmt, 3, type.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, std::time(nullptr));
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "SQL insert error (loyalty_transactions): " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
    }
    logActivity(db, user_id, "Loyalty points " + type + ": " + std::to_string(points));
}

bool redeemLoyaltyPoints(sqlite3* db, const std::string& user_id, int points, float& discount) {
    if (points < 10) return false;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT points FROM loyalty_points WHERE user_id = ?;";
    int current_points = 0;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            current_points = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (current_points < points) return false;

    discount = points / 10.0f;
    addLoyaltyPoints(db, user_id, -points, "redeemed");
    return true;
}

std::vector<LoyaltyPoints> viewLoyaltyPoints(sqlite3* db) {
    std::vector<LoyaltyPoints> points;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT user_id, points FROM loyalty_points;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            LoyaltyPoints lp;
            lp.user_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            lp.points = sqlite3_column_int(stmt, 1);
            points.push_back(lp);
        }
        sqlite3_finalize(stmt);
    }
    return points;
}

std::vector<LoyaltyTransaction> viewLoyaltyTransactions(sqlite3* db) {
    std::vector<LoyaltyTransaction> transactions;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT transaction_id, user_id, points, type, timestamp FROM loyalty_transactions ORDER BY timestamp DESC;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            LoyaltyTransaction trans;
            trans.transaction_id = sqlite3_column_int(stmt, 0);
            trans.user_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            trans.points = sqlite3_column_int(stmt, 2);
            trans.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            trans.timestamp = sqlite3_column_int(stmt, 4);
            transactions.push_back(trans);
        }
        sqlite3_finalize(stmt);
    }
    return transactions;
}

void addDiscount(sqlite3* db, const std::string& name, const std::string& type, float value, int start_time, int end_time, const std::string& combo_items) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO discounts (name, type, value, start_time, end_time, combo_items) VALUES (?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, type.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 3, value);
        sqlite3_bind_int(stmt, 4, start_time);
        sqlite3_bind_int(stmt, 5, end_time);
        sqlite3_bind_text(stmt, 6, combo_items.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "SQL insert error: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
    }
}

void editDiscount(sqlite3* db, int discount_id, const std::string& name, const std::string& type, float value, int start_time, int end_time, const std::string& combo_items) {
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE discounts SET name = ?, type = ?, value = ?, start_time = ?, end_time = ?, combo_items = ? WHERE discount_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, type.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 3, value);
        sqlite3_bind_int(stmt, 4, start_time);
        sqlite3_bind_int(stmt, 5, end_time);
        sqlite3_bind_text(stmt, 6, combo_items.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 7, discount_id);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "SQL update error: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
    }
}

void deleteDiscount(sqlite3* db, int discount_id) {
    sqlite3_stmt* stmt;
    const char* sql = "DELETE FROM discounts WHERE discount_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, discount_id);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "SQL delete error: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
    }
}

std::vector<Discount> viewDiscounts(sqlite3* db, bool active_only = false) {
    std::vector<Discount> discounts;
    sqlite3_stmt* stmt;
    std::string sql = active_only ?
        "SELECT discount_id, name, type, value, start_time, end_time, combo_items FROM discounts WHERE ? BETWEEN start_time AND end_time;" :
        "SELECT discount_id, name, type, value, start_time, end_time, combo_items FROM discounts;";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (active_only) {
            sqlite3_bind_int(stmt, 1, std::time(nullptr));
        }
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Discount discount;
            discount.discount_id = sqlite3_column_int(stmt, 0);
            discount.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            discount.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            discount.value = sqlite3_column_double(stmt, 3);
            discount.start_time = sqlite3_column_int(stmt, 4);
            discount.end_time = sqlite3_column_int(stmt, 5);
            discount.combo_items = sqlite3_column_text(stmt, 6) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
            discounts.push_back(discount);
        }
        sqlite3_finalize(stmt);
    }
    return discounts;
}

float applyDiscount(sqlite3* db, int discount_id, float order_total, const std::vector<OrderItem>& items) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT type, value, combo_items FROM discounts WHERE discount_id = ? AND ? BETWEEN start_time AND end_time;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg(db) << std::endl;
        return order_total;
    }

    sqlite3_bind_int(stmt, 1, discount_id);
    sqlite3_bind_int(stmt, 2, std::time(nullptr));
    float new_total = order_total;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        float value = sqlite3_column_double(stmt, 1);
        std::string combo_items_str = sqlite3_column_text(stmt, 2) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) : "";

        if (type == "percentage") {
            new_total *= (1.0 - value / 100.0);
        } else if (type == "fixed") {
            new_total = std::max(0.0f, new_total - value);
        } else if (type == "combo" && !combo_items_str.empty()) {
            std::vector<int> required_items;
            std::stringstream ss(combo_items_str);
            std::string item_id;
            while (std::getline(ss, item_id, ',')) {
                required_items.push_back(std::stoi(item_id));
            }
            bool combo_valid = true;
            for (int req_id : required_items) {
                bool found = false;
                for (const auto& item : items) {
                    if (item.item_id == req_id) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    combo_valid = false;
                    break;
                }
            }
            if (combo_valid) {
                new_total = std::max(0.0f, new_total - value);
            }
        }
    }
    sqlite3_finalize(stmt);
    return new_total;
}

bool processRefund(sqlite3* db, int bill_id, const std::string& admin_user_id) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT b.order_id, b.total, b.payment_method, b.refunded, o.user_id, o.status "
                     "FROM bills b JOIN orders o ON b.order_id = o.order_id WHERE b.bill_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, bill_id);
    bool success = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int order_id = sqlite3_column_int(stmt, 0);
        float total = sqlite3_column_double(stmt, 1);
        std::string payment_method = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        bool refunded = sqlite3_column_int(stmt, 3) == 1;
        std::string user_id = sqlite3_column_text(stmt, 4) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) : "";
        std::string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        if (refunded || status != "canceled") {
            sqlite3_finalize(stmt);
            return false;
        }

        const char* update_sql = "UPDATE bills SET refunded = 1 WHERE bill_id = ?;";
        if (sqlite3_prepare_v2(db, update_sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, bill_id);
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                std::cerr << "SQL update error (bills): " << sqlite3_errmsg(db) << std::endl;
                sqlite3_finalize(stmt);
                return false;
            }
            sqlite3_finalize(stmt);
        }

        if (payment_method == "wallet" && !user_id.empty()) {
            const char* wallet_sql = "UPDATE wallets SET balance = balance + ? WHERE user_id = ?;";
            if (sqlite3_prepare_v2(db, wallet_sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_double(stmt, 1, total);
                sqlite3_bind_text(stmt, 2, user_id.c_str(), -1, SQLITE_STATIC);
                if (sqlite3_step(stmt) != SQLITE_DONE) {
                    std::cerr << "SQL update error (wallets): " << sqlite3_errmsg(db) << std::endl;
                    sqlite3_finalize(stmt);
                    return false;
                }
                sqlite3_finalize(stmt);
            }
        }

        logActivity(db, admin_user_id, "Refund processed for bill_id: " + std::to_string(bill_id));
        success = true;
    }
    sqlite3_finalize(stmt);
    return success;
}

bool userExists(sqlite3* db, const std::string& user_id) {
    if (user_id == "guest") return true;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT username FROM users WHERE username = ? UNION SELECT user_id FROM wallets WHERE user_id = ?;";
    bool exists = false;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, user_id.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            exists = true;
        }
        sqlite3_finalize(stmt);
    }
    return exists;
}

float getWalletBalance(sqlite3* db, const std::string& user_id) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT balance FROM wallets WHERE user_id = ?;";
    float balance = 0.0f;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            balance = sqlite3_column_double(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    return balance;
}

int getLoyaltyPoints(sqlite3* db, const std::string& user_id) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT points FROM loyalty_points WHERE user_id = ?;";
    int points = 0;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            points = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    return points;
}

int createOrder(sqlite3* db, const std::string& user_id, const std::vector<OrderItem>& items) {
    if (!userExists(db, user_id)) {
        std::cerr << "User ID does not exist: " << user_id << std::endl;
        return -1;
    }

    float total = 0;
    for (const auto& item : items) {
        total += item.quantity * item.price;
    }

    bool sufficient_stock = true;
    for (const auto& item : items) {
        sqlite3_stmt* stmt;
        const char* sql = "SELECT quantity FROM inventory WHERE item_id = ?;";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, item.item_id);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int current_quantity = sqlite3_column_int(stmt, 0);
                if (current_quantity < item.quantity) {
                    sufficient_stock = false;
                    std::cerr << "Insufficient stock for item_id: " << item.item_id << std::endl;
                }
            }
            sqlite3_finalize(stmt);
        }
    }

    if (!sufficient_stock) {
        return -1;
    }

    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO orders (user_id, status, total, created_at) VALUES (?, 'pending', ?, ?);";
    int order_id = -1;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (user_id.empty() || user_id == "guest") {
            sqlite3_bind_null(stmt, 1);
        } else {
            sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_STATIC);
        }
        sqlite3_bind_double(stmt, 2, total);
        sqlite3_bind_int(stmt, 3, std::time(nullptr));
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            order_id = sqlite3_last_insert_rowid(db);
        }
        sqlite3_finalize(stmt);
    }

    if (order_id != -1) {
        for (const auto& item : items) {
            const char* item_sql = "INSERT INTO order_items (order_id, item_id, quantity, price) VALUES (?, ?, ?, ?);";
            if (sqlite3_prepare_v2(db, item_sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, order_id);
                sqlite3_bind_int(stmt, 2, item.item_id);
                sqlite3_bind_int(stmt, 3, item.quantity);
                sqlite3_bind_double(stmt, 4, item.price);
                if (sqlite3_step(stmt) != SQLITE_DONE) {
                    std::cerr << "SQL insert error: " << sqlite3_errmsg(db) << std::endl;
                }
                sqlite3_finalize(stmt);
            }

            const char* update_sql = "UPDATE inventory SET quantity = quantity - ? WHERE item_id = ?;";
            if (sqlite3_prepare_v2(db, update_sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, item.quantity);
                sqlite3_bind_int(stmt, 2, item.item_id);
                if (sqlite3_step(stmt) != SQLITE_DONE) {
                    std::cerr << "SQL update error (inventory): " << sqlite3_errmsg(db) << std::endl;
                }
                sqlite3_finalize(stmt);
            }
        }
        logActivity(db, user_id, "Order created: order_id " + std::to_string(order_id));
    }
    return order_id;
}

void cancelOrder(sqlite3* db, int order_id) {
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE orders SET status = 'canceled' WHERE order_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, order_id);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "SQL update error: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
    }
    logActivity(db, "", "Order canceled: order_id " + std::to_string(order_id));
}

void completeOrder(sqlite3* db, int order_id) {
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE orders SET status = 'completed' WHERE order_id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, order_id);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "SQL update error: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
    }
    logActivity(db, "", "Order completed: order_id " + std::to_string(order_id));
}

std::vector<Order> viewOrders(sqlite3* db, bool completed_only = false) {
    std::vector<Order> orders;
    sqlite3_stmt* stmt;
    std::string sql = completed_only ?
        "SELECT order_id, user_id, status, total, created_at FROM orders WHERE status = 'completed';" :
        "SELECT order_id, user_id, status, total, created_at FROM orders;";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Order order;
            order.order_id = sqlite3_column_int(stmt, 0);
            order.user_id = sqlite3_column_text(stmt, 1) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) : "";
            order.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            order.total = sqlite3_column_double(stmt, 3);
            order.created_at = sqlite3_column_int(stmt, 4);
            
            sqlite3_stmt* item_stmt;
            const char* item_sql = "SELECT oi.item_id, mi.name, oi.quantity, oi.price FROM order_items oi JOIN menu_items mi ON oi.item_id = mi.item_id WHERE oi.order_id = ?;";
            if (sqlite3_prepare_v2(db, item_sql, -1, &item_stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(item_stmt, 1, order.order_id);
                while (sqlite3_step(item_stmt) == SQLITE_ROW) {
                    OrderItem item;
                    item.item_id = sqlite3_column_int(item_stmt, 0);
                    item.name = reinterpret_cast<const char*>(sqlite3_column_text(item_stmt, 1));
                    item.quantity = sqlite3_column_int(item_stmt, 2);
                    item.price = sqlite3_column_double(item_stmt, 3);
                    order.items.push_back(item);
                }
                sqlite3_finalize(item_stmt);
            }
            orders.push_back(order);
        }
        sqlite3_finalize(stmt);
    }
    return orders;
}

std::vector<Bill> viewBills(sqlite3* db) {
    std::vector<Bill> bills;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT bill_id, order_id, tax, total, payment_method, created_at, refunded FROM bills;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Bill bill;
            bill.bill_id = sqlite3_column_int(stmt, 0);
            bill.order_id = sqlite3_column_int(stmt, 1);
            bill.tax = sqlite3_column_double(stmt, 2);
            bill.total = sqlite3_column_double(stmt, 3);
            bill.payment_method = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            bill.created_at = sqlite3_column_int(stmt, 5);
            bill.refunded = sqlite3_column_int(stmt, 6) == 1;
            bills.push_back(bill);
        }
        sqlite3_finalize(stmt);
    }
    return bills;
}





bool saveBillAsPDF(sqlite3* db, int bill_id) {
    sqlite3_stmt* stmt;
    const char* bill_sql = "SELECT b.bill_id, b.order_id, b.tax, b.total, b.payment_method, b.created_at "
                           "FROM bills b WHERE b.bill_id = ?;";
    if (sqlite3_prepare_v2(db, bill_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, bill_id);
    bool success = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Bill bill;
        bill.bill_id = sqlite3_column_int(stmt, 0);
        bill.order_id = sqlite3_column_int(stmt, 1);
        bill.tax = sqlite3_column_double(stmt, 2);
        bill.total = sqlite3_column_double(stmt, 3);
        bill.payment_method = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        bill.created_at = sqlite3_column_int(stmt, 5);

        std::vector<OrderItem> items;
        const char* item_sql = "SELECT oi.item_id, mi.name, oi.quantity, oi.price "
                               "FROM order_items oi JOIN menu_items mi ON oi.item_id = mi.item_id "
                               "WHERE oi.order_id = ?;";
        sqlite3_stmt* item_stmt;
        if (sqlite3_prepare_v2(db, item_sql, -1, &item_stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(item_stmt, 1, bill.order_id);
            while (sqlite3_step(item_stmt) == SQLITE_ROW) {
                OrderItem item;
                item.item_id = sqlite3_column_int(item_stmt, 0);
                item.name = reinterpret_cast<const char*>(sqlite3_column_text(item_stmt, 1));
                item.quantity = sqlite3_column_int(item_stmt, 2);
                item.price = sqlite3_column_double(item_stmt, 3);
                items.push_back(item);
            }
            sqlite3_finalize(item_stmt);
        } else {
            std::cerr << "SQL prepare error for items: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        std::string bills_dir = "/Users/rudra/Library/Mobile Documents/com~apple~CloudDocs/RUDRA FILES/SEM-2/Hackathon/bills/";
        try {
            std::filesystem::create_directories(bills_dir);
        } catch (const std::exception& e) {
            std::cerr << "Failed to create bills directory: " << e.what() << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        std::string tex_filename = bills_dir + "bill" + std::to_string(bill.bill_id) + ".tex";
        std::string pdf_filename = bills_dir + "bill" + std::to_string(bill.bill_id) + ".pdf";

        std::ofstream file(tex_filename);
        if (file.is_open()) {
            file << "\\documentclass[a4paper,12pt]{article}\n"
                 << "\\usepackage{geometry}\n"
                 << "\\geometry{margin=1in}\n"
                 << "\\usepackage{booktabs}\n"
                 << "\\usepackage{siunitx}\n"
                 << "\\sisetup{group-separator={,},group-minimum-digits=4}\n"
                 << "\\usepackage{noto}\n"
                 << "\\begin{document}\n"
                 << "\\centering\n"
                 << "\\textbf{Canteen Management System - Bill}\\\\\n"
                 << "\\vspace{0.5cm}\n"
                 << "\\begin{tabular}{lr}\n"
                 << "Bill ID: & " << bill.bill_id << " \\\\\n"
                 << "Order ID: & " << bill.order_id << " \\\\\n"
                 << "Date: & " << formatTimestamp(bill.created_at) << " \\\\\n"
                 << "\\end{tabular}\n"
                 << "\\vspace{0.5cm}\n"
                 << "\\begin{tabular}{llrr}\n"
                 << "\\toprule\n"
                 << "Item & Quantity & Price (Rs) & Total (Rs) \\\\\n"
                 << "\\midrule\n";

            for (const auto& item : items) {
                std::string escaped_name = item.name;
                for (char& c : escaped_name) {
                    if (c == '&' || c == '%' || c == '$' || c == '#' || c == '_' || c == '{' || c == '}')
                        c = '\\' + c;
                }
                file << escaped_name << " & " << item.quantity << " & \\num{" << item.price << "} & \\num{" << item.quantity * item.price << "} \\\\\n";
            }

            file << "\\bottomrule\n"
                 << "\\end{tabular}\n"
                 << "\\vspace{0.5cm}\n"
                 << "\\begin{tabular}{lr}\n"
                 << "Tax: & Rs \\num{" << bill.tax << "} \\\\\n"
                 << "Total: & Rs \\num{" << bill.total << "} \\\\\n"
                 << "Payment Method: & " << bill.payment_method << " \\\\\n"
                 << "\\end{tabular}\n"
                 << "\\end{document}\n";
            file.close();

            // Updated latexmk command with -cd option
            std::string command = "latexmk -cd -pdf \"" + tex_filename + "\" 2>&1";
            int result = std::system(command.c_str());
            if (result == 0) {
                success = true;
                logActivity(db, "", "Bill saved as PDF: bill_id " + std::to_string(bill.bill_id) + " at " + pdf_filename);
            } else {
                std::cerr << "Failed to compile LaTeX to PDF for bill " << bill.bill_id << std::endl;
                success = false;
            }
        } else {
            std::cerr << "Failed to open file for bill " << bill.bill_id << std::endl;
            success = false;
        }
    }
    sqlite3_finalize(stmt);
    return success;
}






bool createWallet(sqlite3* db, const std::string& phone_number, float initial_balance) {
    if (phone_number.length() != 10 || !std::all_of(phone_number.begin(), phone_number.end(), ::isdigit)) {
        std::cerr << "Invalid phone number: Must be exactly 10 digits." << std::endl;
        return false;
    }

    sqlite3_stmt* stmt;
    const char* check_sql = "SELECT username FROM users WHERE username = ? UNION SELECT user_id FROM wallets WHERE user_id = ?;";
    bool unique = true;
    if (sqlite3_prepare_v2(db, check_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, phone_number.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, phone_number.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            unique = false;
        }
        sqlite3_finalize(stmt);
    }

    if (!unique) {
        std::cerr << "Phone number already exists in users or wallets." << std::endl;
        return false;
    }

    const char* insert_sql = "INSERT INTO wallets (user_id, balance) VALUES (?, ?);";
    bool success = false;
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, phone_number.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 2, initial_balance);
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            success = true;
            logActivity(db, phone_number, "Wallet created with phone number: " + phone_number);
        } else {
            std::cerr << "SQL insert error (wallets): " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
    }

    return success;
}

void topUpWallet(sqlite3* db, const std::string& user_id, float amount) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT OR REPLACE INTO wallets (user_id, balance) VALUES (?, COALESCE((SELECT balance FROM wallets WHERE user_id = ?) + ?, ?));";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, user_id.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 3, amount);
        sqlite3_bind_double(stmt, 4, amount);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "SQL insert error: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
    }
    logActivity(db, user_id, "Wallet topped up: " + std::to_string(amount));
}

void deleteWallet(sqlite3* db, const std::string& user_id) {
    sqlite3_stmt* stmt;
    const char* check_sql = "SELECT balance FROM wallets WHERE user_id = ?;";
    float balance = -1;
    if (sqlite3_prepare_v2(db, check_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            balance = sqlite3_column_double(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    if (balance == 0) {
        const char* sql = "DELETE FROM wallets WHERE user_id = ?;";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                std::cerr << "SQL delete error: " << sqlite3_errmsg(db) << std::endl;
            }
            sqlite3_finalize(stmt);
        }
        logActivity(db, user_id, "Wallet deleted for user: " + user_id);
    } else {
        std::cerr << "Cannot delete wallet: Balance is not zero (" << balance << ")\n";
    }
}

std::vector<Wallet> viewWallets(sqlite3* db) {
    std::vector<Wallet> wallets;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT user_id, balance FROM wallets;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Wallet wallet;
            wallet.user_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            wallet.balance = sqlite3_column_double(stmt, 1);
            wallets.push_back(wallet);
        }
        sqlite3_finalize(stmt);
    }
    return wallets;
}

float getSetting(sqlite3* db, const std::string& key, float default_value) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT value FROM settings WHERE key = ?;";
    float value = default_value;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            value = sqlite3_column_double(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    return value;
}

void setSetting(sqlite3* db, const std::string& key, float value) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT OR REPLACE INTO settings (key, value) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 2, value);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "SQL insert error (settings): " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
    }
}

bool generateBill(sqlite3* db, int order_id, const std::string& payment_method, int discount_id, int loyalty_points_to_redeem, const std::string& user_id, std::string& error_message) {
    float tax_rate = getSetting(db, "tax_rate", 0.08f);
    float loyalty_earn_rate = getSetting(db, "loyalty_earn_rate", 10.0f);
    sqlite3_stmt* stmt;
    const char* order_sql = "SELECT total, user_id FROM orders WHERE order_id = ? AND status = 'pending';";
    float order_total = 0;
    std::string order_user_id;
    bool order_exists = false;

    // Validate order
    if (sqlite3_prepare_v2(db, order_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, order_id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            order_total = sqlite3_column_double(stmt, 0);
            order_user_id = sqlite3_column_text(stmt, 1) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) : "";
            order_exists = true;
        }
        sqlite3_finalize(stmt);
    } else {
        error_message = "Database error: " + std::string(sqlite3_errmsg(db));
        return false;
    }

    if (!order_exists) {
        error_message = "Order not found or already processed.";
        return false;
    }

    // Fetch order items
    std::vector<OrderItem> items;
    const char* item_sql = "SELECT oi.item_id, mi.name, oi.quantity, oi.price FROM order_items oi JOIN menu_items mi ON oi.item_id = mi.item_id WHERE oi.order_id = ?;";
    if (sqlite3_prepare_v2(db, item_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, order_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            OrderItem item;
            item.item_id = sqlite3_column_int(stmt, 0);
            item.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            item.quantity = sqlite3_column_int(stmt, 2);
            item.price = sqlite3_column_double(stmt, 3);
            items.push_back(item);
        }
        sqlite3_finalize(stmt);
    } else {
        error_message = "Database error fetching items: " + std::string(sqlite3_errmsg(db));
        return false;
    }

    // Apply discount
    if (discount_id > 0) {
        order_total = applyDiscount(db, discount_id, order_total, items);
    }

    // Apply loyalty points
    float loyalty_discount = 0.0f;
    if (loyalty_points_to_redeem > 0 && !order_user_id.empty() && order_user_id != "guest") {
        if (!redeemLoyaltyPoints(db, order_user_id, loyalty_points_to_redeem, loyalty_discount)) {
            error_message = "Failed to redeem loyalty points.";
            return false;
        }
        order_total = std::max(0.0f, order_total - loyalty_discount);
    }

    // Calculate tax and total
    float tax = order_total * tax_rate;
    float total = order_total + tax;

    // Process wallet payment
    if (payment_method == "wallet" && !order_user_id.empty() && order_user_id != "guest") {
        float balance = getWalletBalance(db, order_user_id);
        if (balance < total) {
            error_message = "Insufficient wallet balance: Rs " + std::to_string(balance) + " < Rs " + std::to_string(total);
            return false;
        }
        const char* wallet_sql = "UPDATE wallets SET balance = balance - ? WHERE user_id = ?;";
        if (sqlite3_prepare_v2(db, wallet_sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_double(stmt, 1, total);
            sqlite3_bind_text(stmt, 2, order_user_id.c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                error_message = "Failed to deduct wallet balance: " + std::string(sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return false;
            }
            sqlite3_finalize(stmt);
        } else {
            error_message = "Database error updating wallet: " + std::string(sqlite3_errmsg(db));
            return false;
        }
    }

    // Insert bill
    const char* bill_sql = "INSERT INTO bills (order_id, tax, total, payment_method, created_at, refunded) VALUES (?, ?, ?, ?, ?, 0);";
    int bill_id = -1;
    if (sqlite3_prepare_v2(db, bill_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, order_id);
        sqlite3_bind_double(stmt, 2, tax);
        sqlite3_bind_double(stmt, 3, total);
        sqlite3_bind_text(stmt, 4, payment_method.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 5, std::time(nullptr));
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            bill_id = sqlite3_last_insert_rowid(db);
        } else {
            error_message = "Failed to insert bill: " + std::string(sqlite3_errmsg(db));
        }
        sqlite3_finalize(stmt);
    } else {
        error_message = "Database error preparing bill: " + std::string(sqlite3_errmsg(db));
        return false;
    }

    if (bill_id == -1) {
        return false;
    }

    // Update order status
    completeOrder(db, order_id);

    // Add loyalty points
    if (!order_user_id.empty() && order_user_id != "guest") {
        int points_earned = static_cast<int>(total / loyalty_earn_rate);
        if (points_earned > 0) {
            addLoyaltyPoints(db, order_user_id, points_earned, "earned");
        }
    }

    logActivity(db, user_id, "Bill generated: order_id " + std::to_string(order_id));
    return true;
}

SalesData getSalesData(sqlite3* db) {
    SalesData data = {0.0f, 0};
    sqlite3_stmt* stmt;
    const char* sql = "SELECT SUM(total), COUNT(*) FROM bills WHERE refunded = 0;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            data.total_sales = sqlite3_column_double(stmt, 0);
            data.order_count = sqlite3_column_int(stmt, 1);
        }
        sqlite3_finalize(stmt);
    }
    return data;
}

std::vector<TopItem> getTopItems(sqlite3* db) {
    std::vector<TopItem> items;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT oi.item_id, mi.name, SUM(oi.quantity) as total_quantity "
                     "FROM order_items oi "
                     "JOIN menu_items mi ON oi.item_id = mi.item_id "
                     "JOIN bills b ON oi.order_id = b.order_id "
                     "WHERE b.refunded = 0 "
                     "GROUP BY oi.item_id, mi.name "
                     "ORDER BY total_quantity DESC LIMIT 5;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            TopItem item;
            item.item_id = sqlite3_column_int(stmt, 0);
            item.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            item.total_quantity = sqlite3_column_int(stmt, 2);
            items.push_back(item);
        }
        sqlite3_finalize(stmt);
    }
    return items;
}

bool backupDatabase(sqlite3* db, const std::string& backup_path) {
    sqlite3* backup_db;
    if (sqlite3_open(backup_path.c_str(), &backup_db) != SQLITE_OK) {
        std::cerr << "Cannot open backup database: " << sqlite3_errmsg(backup_db) << std::endl;
        sqlite3_close(backup_db);
        return false;
    }

    sqlite3_backup* backup = sqlite3_backup_init(backup_db, "main", db, "main");
    if (backup) {
        sqlite3_backup_step(backup, -1);
        sqlite3_backup_finish(backup);
    } else {
        std::cerr << "Backup initialization failed: " << sqlite3_errmsg(backup_db) << std::endl;
        sqlite3_close(backup_db);
        return false;
    }

    sqlite3_close(backup_db);
    logActivity(db, "", "Database backed up to " + backup_path);
    return true;
}

bool restoreDatabase(sqlite3* db, const std::string& backup_path) {
    sqlite3* backup_db;
    if (sqlite3_open(backup_path.c_str(), &backup_db) != SQLITE_OK) {
        std::cerr << "Cannot open backup database: " << sqlite3_errmsg(backup_db) << std::endl;
        sqlite3_close(backup_db);
        return false;
    }

    sqlite3_backup* backup = sqlite3_backup_init(db, "main", backup_db, "main");
    if (backup) {
        sqlite3_backup_step(backup, -1);
        sqlite3_backup_finish(backup);
    } else {
        std::cerr << "Restore initialization failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(backup_db);
        return false;
    }

    sqlite3_close(backup_db);
    logActivity(db, "", "Database restored from " + backup_path);
    return true;
}

std::vector<UserDetails> viewUserDetails(sqlite3* db) {
    std::vector<UserDetails> users;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT u.username, COALESCE(MAX(o.created_at), 0) as last_order, "
                     "COALESCE(lp.points, 0) as points, COALESCE(w.balance, 0.0) as balance "
                     "FROM users u "
                     "LEFT JOIN orders o ON u.username = o.user_id "
                     "LEFT JOIN loyalty_points lp ON u.username = lp.user_id "
                     "LEFT JOIN wallets w ON u.username = w.user_id "
                     "GROUP BY u.username;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            UserDetails user;
            user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            int last_order_time = sqlite3_column_int(stmt, 1);
            user.last_order = last_order_time ? formatTimestamp(last_order_time) : "None";
            user.loyalty_points = sqlite3_column_int(stmt, 2);
            user.wallet_balance = sqlite3_column_double(stmt, 3);
            users.push_back(user);
        }
        sqlite3_finalize(stmt);
    }
    return users;
}

void renderDashboard(sqlite3* db, const std::string& username, const std::string& role) {
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.96f, 1.0f), "Welcome, %s!", username.c_str());
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, 20));
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 0.8f), "Role: %s", role.c_str());
    ImGui::Dummy(ImVec2(0, 10));
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 0.8f), "Canteen Management System");
}

void renderProfile(sqlite3* db, const std::string& username) {
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.96f, 1.0f), "User Profile");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, 20));
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 0.8f), "Username: %s", username.c_str());
}

void renderMenuManagement(sqlite3* db, const std::string& role) {
    static char name[128] = "";
    static float price = 0.0f;
    static bool available = true;
    static int edit_id = -1;
    static std::string error_message = "";

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.96f, 1.0f), "Menu Management");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, 20));

    if (role == "admin" || role == "manager") {
        ImGui::InputText("Name", name, sizeof(name));
        ImGui::InputFloat("Price (Rs)", &price, 1.0f, 1.0f, "%.2f");
        if (price < 0) price = 0;
        ImGui::Checkbox("Available", &available);

        if (ImGui::Button(edit_id == -1 ? "Add Item" : "Update Item")) {
            if (strlen(name) > 0 && price > 0) {
                bool success = false;
                if (edit_id == -1) {
                    success = addMenuItem(db, name, price, available);
                } else {
                    editMenuItem(db, edit_id, name, price, available);
                    success = true;
                }
                if (success) {
                    name[0] = '\0';
                    price = 0.0f;
                    available = true;
                    edit_id = -1;
                    error_message = "Operation successful!";
                } else {
                    error_message = "Failed to add item. Check database.";
                }
            } else {
                error_message = "Invalid name or price.";
            }
        }
        ImGui::SameLine();
        if (edit_id != -1 && ImGui::Button("Cancel Edit")) {
            edit_id = -1;
            name[0] = '\0';
            price = 0.0f;
            available = true;
            error_message = "";
        }

        if (!error_message.empty()) {
            ImGui::TextColored(ImVec4(0.94f, 0.33f, 0.31f, 1.0f), "%s", error_message.c_str());
        }
    }

    ImGui::Dummy(ImVec2(0, 10));
    auto items = viewMenuItems(db, role == "biller");
    if (ImGui::BeginTable("MenuItems", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Price (Rs)");
        ImGui::TableSetupColumn("Available");
        ImGui::TableHeadersRow();

        for (const auto& item : items) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", item.id);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", item.name.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("Rs %.2f", item.price);
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", item.available ? "Yes" : "No");

            if (role == "admin" || role == "manager") {
                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(item.id);
                if (ImGui::Button("Edit")) {
                    edit_id = item.id;
                    strncpy(name, item.name.c_str(), sizeof(name));
                    price = item.price;
                    available = item.available;
                    error_message = "";
                }
                ImGui::SameLine();
                if (ImGui::Button("Delete")) {
                    deleteMenuItem(db, item.id);
                }
                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }
}

void renderOrderManagement(sqlite3* db, const std::string& role) {
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.96f, 1.0f), "Order Management");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, 20));

    if (role == "biller") {
        static char customer_id[128] = "";
        static std::vector<OrderItem> new_order_items;
        static int selected_item_id = -1;
        static std::string error_message = "";

        ImGui::InputText("Customer ID (required, enter 'guest' for non-registered)", customer_id, sizeof(customer_id));
        auto items = viewMenuItems(db, true);
        std::vector<const char*> item_names;
        for (const auto& item : items) {
            item_names.push_back(item.name.c_str());
        }

        ImGui::Combo("Select Item", &selected_item_id, item_names.data(), item_names.size());
        static int quantity = 1;
        ImGui::InputInt("Quantity", &quantity);
        if (quantity < 1) quantity = 1;
        if (ImGui::Button("Add to Order") && selected_item_id >= 0 && selected_item_id < items.size() && quantity > 0) {
            OrderItem item;
            item.item_id = items[selected_item_id].id;
            item.name = items[selected_item_id].name;
            item.quantity = quantity;
            item.price = items[selected_item_id].price;
            new_order_items.push_back(item);
            quantity = 1;
        }

        if (ImGui::BeginTable("NewOrderItems", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Quantity");
            ImGui::TableSetupColumn("Price (Rs)");
            ImGui::TableHeadersRow();

            for (const auto& item : new_order_items) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", item.name.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%d", item.quantity);
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("Rs %.2f", item.price);
            }
            ImGui::EndTable();
        }

        if (ImGui::Button("Create Order") && !new_order_items.empty() && strlen(customer_id) > 0) {
            if (!userExists(db, customer_id)) {
                error_message = "Invalid Customer ID. Use 'guest' for non-registered.";
            } else {
                int order_id = createOrder(db, customer_id, new_order_items);
                if (order_id != -1) {
                    new_order_items.clear();
                    customer_id[0] = '\0';
                    selected_item_id = -1;
                    error_message = "Order created successfully!";
                } else {
                    error_message = "Order creation failed: Insufficient stock or invalid user.";
                }
            }
        }

        if (!error_message.empty()) {
            ImGui::TextColored(ImVec4(0.94f, 0.33f, 0.31f, 1.0f), "%s", error_message.c_str());
        }
    }

    ImGui::Dummy(ImVec2(0, 10));
    auto orders = viewOrders(db);
    if (ImGui::BeginTable("Orders", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Customer");
        ImGui::TableSetupColumn("Status");
        ImGui::TableSetupColumn("Total (Rs)");
        ImGui::TableSetupColumn("Created");
        ImGui::TableHeadersRow();

        for (const auto& order : orders) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", order.order_id);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", order.user_id.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", order.status.c_str());
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("Rs %.2f", order.total);
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", formatTimestamp(order.created_at).c_str());

            if (role == "admin" || role == "manager") {
                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(order.order_id);
                if (ImGui::Button("Cancel") && order.status == "pending") {
                    cancelOrder(db, order.order_id);
                }
                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }
}



void renderBilling(sqlite3* db, const std::string& role, const std::string& user_id) {
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.96f, 1.0f), "Billing and Payment");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, 20));

    static bool refresh_completed_orders = true;
    if (role == "biller") {
        static int order_id = -1;
        static char customer_id[128] = "";
        static int payment_method = 0;
        static int selected_discount_id = -1;
        static int loyalty_points_to_redeem = 0;
        static int available_points = 0;
        static bool show_points = false;
        static std::string error_message = "";
        const char* methods[] = { "Wallet", "Cash", "Card" };

        ImGui::InputInt("Order ID", &order_id);
        ImGui::InputText("Customer ID (required, enter 'guest' for non-registered)", customer_id, sizeof(customer_id));
        if (ImGui::Button("Check Customer")) {
            if (strlen(customer_id) == 0) {
                error_message = "Customer ID is required.";
                show_points = false;
            } else if (!userExists(db, customer_id)) {
                error_message = "Invalid Customer ID. Use 'guest' for non-registered.";
                show_points = false;
            } else {
                error_message = "";
                if (customer_id != std::string("guest")) {
                    available_points = getLoyaltyPoints(db, customer_id);
                    show_points = true;
                } else {
                    show_points = false;
                    payment_method = 1; // Default to Cash for guest
                }
            }
        }

        if (show_points) {
            ImGui::Text("Available Loyalty Points: %d", available_points);
            ImGui::InputInt("Loyalty Points to Redeem", &loyalty_points_to_redeem);
            if (loyalty_points_to_redeem < 0) loyalty_points_to_redeem = 0;
            if (loyalty_points_to_redeem > available_points) loyalty_points_to_redeem = available_points;
        }

        ImGui::Combo("Payment Method", &payment_method, methods, IM_ARRAYSIZE(methods));
        if (std::string(customer_id) == "guest") {
            payment_method = payment_method == 0 ? 1 : payment_method; // Force Cash/Card for guest
        }

        auto discounts = viewDiscounts(db, true);
        std::vector<const char*> discount_names = {"None"};
        std::vector<int> discount_ids = {0};
        for (const auto& discount : discounts) {
            discount_names.push_back(discount.name.c_str());
            discount_ids.push_back(discount.discount_id);
        }
        static int discount_index = 0;
        ImGui::Combo("Discount", &discount_index, discount_names.data(), discount_names.size());
        selected_discount_id = discount_ids[discount_index];

        if (ImGui::Button("Generate Bill")) {
            error_message = "";
            if (order_id <= 0) {
                error_message = "Invalid Order ID.";
            } else if (strlen(customer_id) == 0) {
                error_message = "Customer ID is required.";
            } else if (!userExists(db, customer_id)) {
                error_message = "Invalid Customer ID.";
            } else {
                if (generateBill(db, order_id, methods[payment_method], selected_discount_id, loyalty_points_to_redeem, user_id, error_message)) {
                    error_message = "Bill generated successfully!";
                    refresh_completed_orders = true;
                    order_id = -1;
                    customer_id[0] = '\0';
                    discount_index = 0;
                    selected_discount_id = -1;
                    loyalty_points_to_redeem = 0;
                    show_points = false;
                }
            }
        }

        if (!error_message.empty()) {
            ImGui::TextColored(error_message.find("successfully") != std::string::npos ? 
                               ImVec4(0.30f, 0.69f, 0.31f, 1.0f) : ImVec4(0.94f, 0.33f, 0.31f, 1.0f), 
                               "%s", error_message.c_str());
        }

        ImGui::Dummy(ImVec2(0, 20));
        ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.96f, 1.0f), "Completed Orders");
        ImGui::Dummy(ImVec2(0, 10));
        auto completed_orders = viewOrders(db, true);
        if (refresh_completed_orders) {
            completed_orders = viewOrders(db, true); // Force refresh
            refresh_completed_orders = false;
        }
        if (ImGui::BeginTable("CompletedOrders", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Order ID");
            ImGui::TableSetupColumn("Customer");
            ImGui::TableSetupColumn("Status");
            ImGui::TableSetupColumn("Total (Rs)");
            ImGui::TableSetupColumn("Created");
            ImGui::TableSetupColumn("Actions");
            ImGui::TableHeadersRow();

            for (const auto& order : completed_orders) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", order.order_id);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", order.user_id.c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", order.status.c_str());
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("Rs %.2f", order.total);
                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%s", formatTimestamp(order.created_at).c_str());
                ImGui::TableSetColumnIndex(5);
                ImGui::PushID(order.order_id + 2000);
                auto bills = viewBills(db);
                bool found = false;
                for (const auto& bill : bills) {
                    if (bill.order_id == order.order_id) {
                        if (ImGui::Button("Save as PDF")) {
                            if (saveBillAsPDF(db, bill.bill_id)) {
                                ImGui::TextColored(ImVec4(0.30f, 0.69f, 0.31f, 1.0f), "Saved!");
                            } else {
                                ImGui::TextColored(ImVec4(0.94f, 0.33f, 0.31f, 1.0f), "Failed!");
                            }
                        }
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    ImGui::Text("No bill found");
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    }

    ImGui::Dummy(ImVec2(0, 20));
    ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.96f, 1.0f), "All Bills");
    ImGui::Dummy(ImVec2(0, 10));
    auto bills = viewBills(db);
    if (ImGui::BeginTable("Bills", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Order ID");
        ImGui::TableSetupColumn("Tax (Rs)");
        ImGui::TableSetupColumn("Total (Rs)");
        ImGui::TableSetupColumn("Method");
        ImGui::TableSetupColumn("Actions");
        ImGui::TableHeadersRow();

        for (auto& bill : bills) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", bill.bill_id);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", bill.order_id);
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("Rs %.2f", bill.tax);
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("Rs %.2f", bill.total);
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", bill.payment_method.c_str());
            ImGui::TableSetColumnIndex(5);
            if (role == "admin" || role == "biller") {
                ImGui::PushID(bill.bill_id + 1000);
                if (ImGui::Button("Save as PDF")) {
                    if (saveBillAsPDF(db, bill.bill_id)) {
                        ImGui::TextColored(ImVec4(0.30f, 0.69f, 0.31f, 1.0f), "Saved!");
                    } else {
                        ImGui::TextColored(ImVec4(0.94f, 0.33f, 0.31f, 1.0f), "Failed!");
                    }
                }
                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }
}





void renderWallets(sqlite3* db, const std::string& role) {
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.96f, 1.0f), "Wallet Management");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, 20));

    if (role == "biller") {
        static char user_id[128] = "";
        static float amount = 0.0f;
        static std::string error_message = "";

        ImGui::InputText("User ID", user_id, sizeof(user_id));
        ImGui::InputFloat("Top-Up Amount (Rs)", &amount, 1.0f, 1.0f, "%.2f");
        if (amount < 0) amount = 0;
        if (ImGui::Button("Top Up") && strlen(user_id) > 0 && amount > 0) {
            if (userExists(db, user_id) && user_id != std::string("guest")) {
                topUpWallet(db, user_id, amount);
                error_message = "Wallet topped up successfully!";
                user_id[0] = '\0';
                amount = 0.0f;
            } else {
                error_message = "Invalid User ID.";
            }
        }

        if (!error_message.empty()) {
            ImGui::TextColored(ImVec4(0.94f, 0.33f, 0.31f, 1.0f), "%s", error_message.c_str());
        }
    }

    if (role == "manager") {
        static char phone_number[11] = "";
        static float initial_balance = 0.0f;
        static std::string error_message = "";

        ImGui::InputText("Phone Number (10 digits)", phone_number, sizeof(phone_number));
        ImGui::InputFloat("Initial Balance (Rs)", &initial_balance, 1.0f, 1.0f, "%.2f");
        if (initial_balance < 0) initial_balance = 0;
        if (ImGui::Button("Create Wallet") && strlen(phone_number) > 0) {
            if (createWallet(db, phone_number, initial_balance)) {
                error_message = "Wallet created successfully!";
                phone_number[0] = '\0';
                initial_balance = 0.0f;
            } else {
                error_message = "Failed to create wallet: Invalid or duplicate phone number.";
            }
        }

        if (!error_message.empty()) {
            ImGui::TextColored(ImVec4(0.94f, 0.33f, 0.31f, 1.0f), "%s", error_message.c_str());
        }
    }

    if (role == "admin") {
        ImGui::Dummy(ImVec2(0, 10));
        auto wallets = viewWallets(db);
        if (ImGui::BeginTable("Wallets", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("User ID");
            ImGui::TableSetupColumn("Balance (Rs)");
            ImGui::TableSetupColumn("Actions");
            ImGui::TableHeadersRow();

            for (const auto& wallet : wallets) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", wallet.user_id.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Rs %.2f", wallet.balance);
                ImGui::TableSetColumnIndex(2);
                ImGui::PushID(wallet.user_id.c_str());
                if (ImGui::Button("Delete") && wallet.balance == 0) {
                    deleteWallet(db, wallet.user_id);
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    }
}

void renderDiscounts(sqlite3* db, const std::string& role) {
    static char name[128] = "";
    static int type_index = 0;
    static float value = 0.0f;
    static char start_time[20] = "";
    static char end_time[20] = "";
    static char combo_items[128] = "";
    static int edit_id = -1;
    const char* types[] = {"percentage", "fixed", "combo"};
    static std::string error_message = "";

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.96f, 1.0f), "Discount Management");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, 20));

    if (role == "admin" || role == "manager") {
        ImGui::InputText("Name", name, sizeof(name));
        ImGui::Combo("Type", &type_index, types, IM_ARRAYSIZE(types));
        ImGui::InputFloat("Value", &value, 1.0f, 1.0f, "%.2f");
        if (value < 0) value = 0;
        ImGui::InputText("Start Time (DD-MM-YYYY HH:MM:SS)", start_time, sizeof(start_time));
        ImGui::InputText("End Time (DD-MM-YYYY HH:MM:SS)", end_time, sizeof(end_time));
        ImGui::InputText("Combo Items (IDs, e.g., 1,2)", combo_items, sizeof(combo_items));

        if (ImGui::Button(edit_id == -1 ? "Add Discount" : "Update Discount")) {
            if (strlen(name) > 0 && value >= 0) {
                struct tm tm_start = {}, tm_end = {};
                if (strptime(start_time, "%d-%m-%Y %H:%M:%S", &tm_start) && strptime(end_time, "%d-%m-%Y %H:%M:%S", &tm_end)) {
                    int start_ts = mktime(&tm_start);
                    int end_ts = mktime(&tm_end);
                    if (edit_id == -1) {
                        addDiscount(db, name, types[type_index], value, start_ts, end_ts, combo_items);
                    } else {
                        editDiscount(db, edit_id, name, types[type_index], value, start_ts, end_ts, combo_items);
                    }
                    name[0] = '\0';
                    type_index = 0;
                    value = 0.0f;
                    start_time[0] = '\0';
                    end_time[0] = '\0';
                    combo_items[0] = '\0';
                    edit_id = -1;
                    error_message = "Discount operation successful!";
                } else {
                    error_message = "Invalid date/time format.";
                }
            } else {
                error_message = "Invalid name or value.";
            }
        }
        ImGui::SameLine();
        if (edit_id != -1 && ImGui::Button("Cancel Edit")) {
            edit_id = -1;
            name[0] = '\0';
            type_index = 0;
            value = 0.0f;
            start_time[0] = '\0';
            end_time[0] = '\0';
            combo_items[0] = '\0';
            error_message = "";
        }

        if (!error_message.empty()) {
            ImGui::TextColored(ImVec4(0.94f, 0.33f, 0.31f, 1.0f), "%s", error_message.c_str());
        }
    }

    ImGui::Dummy(ImVec2(0, 10));
    auto discounts = viewDiscounts(db, role == "biller");
    if (ImGui::BeginTable("Discounts", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Value");
        ImGui::TableSetupColumn("Start Time");
        ImGui::TableSetupColumn("End Time");
        ImGui::TableSetupColumn("Combo Items");
        ImGui::TableHeadersRow();

        for (const auto& discount : discounts) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", discount.discount_id);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", discount.name.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", discount.type.c_str());
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%.2f", discount.value);
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", formatTimestamp(discount.start_time).c_str());
            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%s", formatTimestamp(discount.end_time).c_str());
            ImGui::TableSetColumnIndex(6);
            ImGui::Text("%s", discount.combo_items.c_str());

            if (role == "admin" || role == "manager") {
                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(discount.discount_id);
                if (ImGui::Button("Edit")) {
                    edit_id = discount.discount_id;
                    strncpy(name, discount.name.c_str(), sizeof(name));
                    for (int i = 0; i < IM_ARRAYSIZE(types); i++) {
                        if (discount.type == types[i]) {
                            type_index = i;
                            break;
                        }
                    }
                    value = discount.value;
                    strncpy(start_time, formatTimestamp(discount.start_time).c_str(), sizeof(start_time));
                    strncpy(end_time, formatTimestamp(discount.end_time).c_str(), sizeof(end_time));
                    strncpy(combo_items, discount.combo_items.c_str(), sizeof(combo_items));
                    error_message = "";
                }
                ImGui::SameLine();
                if (ImGui::Button("Delete")) {
                    deleteDiscount(db, discount.discount_id);
                }
                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }
}

void renderInventory(sqlite3* db, const std::string& role) {
    static int item_id = 0;
    static int quantity = 0;
    static int low_stock_threshold = 10;
    static int edit_id = -1;
    static std::string error_message = "";

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.96f, 1.0f), "Inventory Management");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, 20));

    if (role == "admin" || role == "manager") {
        ImGui::InputInt("Item ID", &item_id);
        ImGui::InputInt("Quantity", &quantity);
        ImGui::InputInt("Low Stock Threshold", &low_stock_threshold);
        if (quantity < 0) quantity = 0;
        if (low_stock_threshold < 0) low_stock_threshold = 0;

        if (ImGui::Button(edit_id == -1 ? "Add Stock" : "Update Stock")) {
            if (item_id > 0) {
                if (edit_id == -1) {
                    addInventory(db, item_id, quantity, low_stock_threshold);
                } else {
                    updateInventory(db, edit_id, quantity, low_stock_threshold);
                }
                item_id = 0;
                quantity = 0;
                low_stock_threshold = 10;
                edit_id = -1;
                error_message = "Stock operation successful!";
            } else {
                error_message = "Invalid Item ID.";
            }
        }
        ImGui::SameLine();
        if (edit_id != -1 && ImGui::Button("Cancel Edit")) {
            edit_id = -1;
            item_id = 0;
            quantity = 0;
            low_stock_threshold = 10;
            error_message = "";
        }

        if (!error_message.empty()) {
            ImGui::TextColored(ImVec4(0.94f, 0.33f, 0.31f, 1.0f), "%s", error_message.c_str());
        }
    }

    ImGui::Dummy(ImVec2(0, 10));
    auto inventory = viewInventory(db);
    if (ImGui::BeginTable("Inventory", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Item ID");
        ImGui::TableSetupColumn("Item Name");
        ImGui::TableSetupColumn("Quantity");
        ImGui::TableSetupColumn("Low Stock Threshold");
        ImGui::TableSetupColumn("Status");
        ImGui::TableHeadersRow();

        auto items = viewMenuItems(db);
        for (const auto& inv : inventory) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", inv.item_id);
            ImGui::TableSetColumnIndex(1);
            std::string item_name = "Unknown";
            for (const auto& item : items) {
                if (item.id == inv.item_id) {
                    item_name = item.name;
                    break;
                }
            }
            ImGui::Text("%s", item_name.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%d", inv.quantity);
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%d", inv.low_stock_threshold);
            ImGui::TableSetColumnIndex(4);
            if (inv.quantity <= inv.low_stock_threshold) {
                ImGui::TextColored(ImVec4(0.94f, 0.33f, 0.31f, 1.0f), "Low Stock!");
            } else {
                ImGui::Text("OK");
            }

            if (role == "admin" || role == "manager") {
                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(inv.item_id);
                if (ImGui::Button("Edit")) {
                    edit_id = inv.item_id;
                    item_id = inv.item_id;
                    quantity = inv.quantity;
                    low_stock_threshold = inv.low_stock_threshold;
                    error_message = "";
                }
                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }
}

void renderLoyalty(sqlite3* db, const std::string& role) {
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.96f, 1.0f), "Loyalty Program");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, 20));

    if (role == "admin" || role == "manager") {
        static char user_id[128] = "";
        static int points = 0;
        static std::string error_message = "";

        ImGui::InputText("User ID", user_id, sizeof(user_id));
        ImGui::InputInt("Points to Add", &points);
        if (points < 0) points = 0;
        if (ImGui::Button("Add Points") && strlen(user_id) > 0 && points > 0) {
            if (userExists(db, user_id) && user_id != std::string("guest")) {
                addLoyaltyPoints(db, user_id, points, "earned");
                error_message = "Points added successfully!";
                user_id[0] = '\0';
                points = 0;
            } else {
                error_message = "Invalid User ID.";
            }
        }

        if (!error_message.empty()) {
            ImGui::TextColored(ImVec4(0.94f, 0.33f, 0.31f, 1.0f), "%s", error_message.c_str());
        }
    }

    ImGui::Dummy(ImVec2(0, 10));
    auto points = viewLoyaltyPoints(db);
    if (ImGui::BeginTable("LoyaltyPoints", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("User ID");
        ImGui::TableSetupColumn("Points");
        ImGui::TableHeadersRow();

        for (const auto& lp : points) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", lp.user_id.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", lp.points);
        }
        ImGui::EndTable();
    }

    ImGui::Dummy(ImVec2(0, 10));
    auto transactions = viewLoyaltyTransactions(db);
    if (ImGui::BeginTable("LoyaltyTransactions", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Transaction ID");
        ImGui::TableSetupColumn("User ID");
        ImGui::TableSetupColumn("Points");
        ImGui::TableSetupColumn("Type");
        ImGui::TableHeadersRow();

        for (const auto& trans : transactions) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", trans.transaction_id);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", trans.user_id.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%d", trans.points);
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", trans.type.c_str());
        }
        ImGui::EndTable();
    }
}

void renderActivityLog(sqlite3* db, const std::string& role) {
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.96f, 1.0f), "Activity Log");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, 20));

    if (role == "admin" || role == "manager") {
        auto logs = viewActivityLog(db);
        if (ImGui::BeginTable("ActivityLog", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Log ID");
            ImGui::TableSetupColumn("User ID");
            ImGui::TableSetupColumn("Action");
            ImGui::TableSetupColumn("Timestamp");
            ImGui::TableHeadersRow();

            for (const auto& log : logs) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", log.log_id);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", log.user_id.c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", log.action.c_str());
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%s", formatTimestamp(log.timestamp).c_str());
            }
            ImGui::EndTable();
        }
    } else {
        ImGui::TextColored(ImVec4(0.94f, 0.33f, 0.31f, 1.0f), "Access restricted to Admin or Manager roles.");
    }
}

void renderAnalytics(sqlite3* db, const std::string& role) {
    if (role != "admin" && role != "manager") {
        ImGui::TextColored(ImVec4(0.94f, 0.33f, 0.31f, 1.0f), "Access restricted to Admin or Manager roles.");
        return;
    }

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.96f, 1.0f), "Analytics");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, 20));

    SalesData sales = getSalesData(db);
    ImGui::Text("Total Sales: Rs %.2f", sales.total_sales);
    ImGui::Text("Total Orders: %d", sales.order_count);
    ImGui::Dummy(ImVec2(0, 10));

    auto top_items = getTopItems(db);
    if (ImGui::BeginTable("TopItems", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Item ID");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Total Quantity");
        ImGui::TableHeadersRow();

        for (const auto& item : top_items) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", item.item_id);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", item.name.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%d", item.total_quantity);
        }
        ImGui::EndTable();
    }
}

void renderSettings(sqlite3* db, const std::string& role) {
    if (role != "admin") {
        ImGui::TextColored(ImVec4(0.94f, 0.33f, 0.31f, 1.0f), "Access restricted to Admin role.");
        return;
    }

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.96f, 1.0f), "Settings");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, 20));

    static float tax_rate = getSetting(db, "tax_rate", 0.08f);
    static float loyalty_earn_rate = getSetting(db, "loyalty_earn_rate", 10.0f);
    ImGui::InputFloat("Tax Rate", &tax_rate, 0.01f, 0.01f, "%.2f");
    ImGui::InputFloat("Loyalty Earn Rate (Rs per point)", &loyalty_earn_rate, 1.0f, 1.0f, "%.2f");
    if (tax_rate < 0) tax_rate = 0;
    if (loyalty_earn_rate < 0) loyalty_earn_rate = 0;

    if (ImGui::Button("Save Settings")) {
        setSetting(db, "tax_rate", tax_rate);
        setSetting(db, "loyalty_earn_rate", loyalty_earn_rate);
        ImGui::TextColored(ImVec4(0.30f, 0.69f, 0.31f, 1.0f), "Settings saved!");
    }
}

void renderBackup(sqlite3* db, const std::string& role) {
    if (role != "admin") {
        ImGui::TextColored(ImVec4(0.94f, 0.33f, 0.31f, 1.0f), "Access restricted to Admin role.");
        return;
    }

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.96f, 1.0f), "Backup Database");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, 20));

    static char backup_path[128] = "users_backup.db";
    ImGui::InputText("Backup Path", backup_path, sizeof(backup_path));
    if (ImGui::Button("Create Backup")) {
        if (backupDatabase(db, backup_path)) {
            ImGui::TextColored(ImVec4(0.30f, 0.69f, 0.31f, 1.0f), "Backup created successfully!");
        } else {
            ImGui::TextColored(ImVec4(0.94f, 0.33f, 0.31f, 1.0f), "Backup failed!");
        }
    }

    static char restore_path[128] = "users_backup.db";
    ImGui::InputText("Restore Path", restore_path, sizeof(restore_path));
    if (ImGui::Button("Restore Database")) {
        if (restoreDatabase(db, restore_path)) {
            ImGui::TextColored(ImVec4(0.30f, 0.69f, 0.31f, 1.0f), "Database restored successfully!");
        } else {
            ImGui::TextColored(ImVec4(0.94f, 0.33f, 0.31f, 1.0f), "Restore failed!");
        }
    }
}

void renderUsers(sqlite3* db, const std::string& role) {
    if (role != "admin") {
        ImGui::TextColored(ImVec4(0.94f, 0.33f, 0.31f, 1.0f), "Access restricted to Admin role.");
        return;
    }

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.96f, 1.0f), "User Details");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, 20));

    auto users = viewUserDetails(db);
    if (ImGui::BeginTable("Users", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Username");
        ImGui::TableSetupColumn("Last Order");
        ImGui::TableSetupColumn("Loyalty Points");
        ImGui::TableSetupColumn("Wallet Balance (Rs)");
        ImGui::TableHeadersRow();

        for (const auto& user : users) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", user.username.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", user.last_order.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%d", user.loyalty_points);
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("Rs %.2f", user.wallet_balance);
        }
        ImGui::EndTable();
    }
}

enum Page { DASHBOARD, PROFILE, MENU, ORDERS, BILLING, WALLETS, DISCOUNTS, INVENTORY, LOYALTY, ACTIVITY_LOG, ANALYTICS, SETTINGS, BACKUP, USERS };
enum LoginStage { LOGIN_CREDENTIALS, LOGIN_TOTP };

int main() {
    sqlite3* db;
    const char* db_path = "/Users/rudra/Library/Mobile Documents/com~apple~CloudDocs/RUDRA FILES/SEM-2/Hackathon/users.db";
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }
    initDatabase(db);

    if (!glfwInit()) {
        sqlite3_close(db);
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Canteen Management System", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        sqlite3_close(db);
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("/System/Library/Fonts/Helvetica.ttc", 24.0f);
    if (io.Fonts->Fonts.Size == 0) {
        io.Fonts->AddFontDefault();
    }

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 6.0f;
    style.FramePadding = ImVec2(10, 8);
    style.WindowPadding = ImVec2(15, 15);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.12f, 0.13f, 0.98f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.18f, 0.20f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.23f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.01f, 0.53f, 0.82f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.01f, 0.47f, 0.74f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.01f, 0.41f, 0.65f, 1.0f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.96f, 0.96f, 0.96f, 1.0f);

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true) || !ImGui_ImplOpenGL3_Init("#version 150")) {
        glfwDestroyWindow(window);
        glfwTerminate();
        sqlite3_close(db);
        return -1;
    }

    char username[128] = "";
    char password[128] = "";
    char totp_code[7] = "";
    std::string status = "";
    std::string logged_in_username = "";
    std::string user_role = "";
    std::string pending_totp_secret = "";
    float status_alpha = 0.0f;
    bool login_attempted = false;
    bool logged_in = false;
    float panel_alpha = 1.0f;
    Page current_page = DASHBOARD;
    LoginStage login_stage = LOGIN_CREDENTIALS;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
        drawDoodle(draw_list, ImVec2(100, 100), 30.0f);
        drawDoodle(draw_list, ImVec2(io.DisplaySize.x - 100, io.DisplaySize.y - 100), 30.0f);
        drawDoodle(draw_list, ImVec2(io.DisplaySize.x - 100, 100), 30.0f);
        drawDoodle(draw_list, ImVec2(100, io.DisplaySize.y - 100), 30.0f);

        if (!logged_in) {
            float panel_width = io.DisplaySize.x * 0.9f;
            ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(panel_width, io.DisplaySize.y * 0.9f), ImGuiCond_Always);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, panel_alpha);
            ImGui::Begin("Login", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

            ImGui::PushFont(io.Fonts->Fonts[0]);
            ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.96f, 1.0f), login_stage == LOGIN_CREDENTIALS ? "Sign In" : "Two-Factor Authentication");
            ImGui::PopFont();
            ImGui::Dummy(ImVec2(0, 25));
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 0.5f), login_stage == LOGIN_CREDENTIALS ? "~*~ Welcome ~*~" : "~*~ Verify Identity ~*~");
            ImGui::Dummy(ImVec2(0, 25));

            if (login_stage == LOGIN_CREDENTIALS) {
                ImGui::Text("Username");
                ImGui::PushItemWidth(-1);
                ImGui::InputTextWithHint("##username", "Enter username", username, sizeof(username));
                ImGui::PopItemWidth();
                ImGui::Dummy(ImVec2(0, 20));

                ImGui::Text("Password");
                ImGui::PushItemWidth(-1);
                ImGui::InputTextWithHint("##password", "Enter password", password, sizeof(password), ImGuiInputTextFlags_Password);
                ImGui::PopItemWidth();
                ImGui::Dummy(ImVec2(0, 30));

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 10));
                if (ImGui::Button("Next", ImVec2(panel_width * 0.5f, 50))) {
                    login_attempted = true;
                    status_alpha = 1.0f;
                    if (checkPassword(db, username, password, user_role, pending_totp_secret)) {
                        login_stage = LOGIN_TOTP;
                        status = "";
                        totp_code[0] = '\0';
                    } else {
                        status = "Invalid Username or Password";
                    }
                }
                ImGui::PopStyleVar();
            } else {
                ImGui::Text("TOTP Code");
                ImGui::PushItemWidth(-1);
                ImGui::InputTextWithHint("##totp", "Enter 6-digit code", totp_code, sizeof(totp_code));
                ImGui::PopItemWidth();
                ImGui::Dummy(ImVec2(0, 30));

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 10));
                if (ImGui::Button("Verify", ImVec2(panel_width * 0.5f, 50))) {
                    login_attempted = true;
                    status_alpha = 1.0f;
                    if (checkTotp(pending_totp_secret, totp_code)) {
                        status = "Login Successful!";
                        logged_in = true;
                        panel_alpha = 0.0f;
                        logged_in_username = username;
                        logActivity(db, logged_in_username, "User logged in");
                    } else {
                        status = "Invalid TOTP Code";
                    }
                }
                ImGui::PopStyleVar();

                ImGui::Dummy(ImVec2(0, 20));
                if (ImGui::Button("Back", ImVec2(panel_width * 0.5f, 40))) {
                    login_stage = LOGIN_CREDENTIALS;
                    status = "";
                    totp_code[0] = '\0';
                    pending_totp_secret = "";
                }
            }

            if (login_attempted && status != "") {
                ImGui::Dummy(ImVec2(0, 20));
                ImVec4 status_color = status == "Login Successful!" ? 
                    ImVec4(0.30f, 0.69f, 0.31f, status_alpha) : ImVec4(0.94f, 0.33f, 0.31f, status_alpha);
                ImGui::TextColored(status_color, "%s", status.c_str());
                status_alpha -= 0.01f;
                if (status_alpha < 0.0f) status_alpha = 0.0f;
            }

            ImGui::End();
            ImGui::PopStyleVar();
        } else {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Always);
            ImGui::Begin("Dashboard", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

            ImGui::BeginGroup();
            ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.96f, 1.0f), "Menu");
            ImGui::Dummy(ImVec2(0, 10));
            if (ImGui::Button("Dashboard", ImVec2(150, 40))) {
                current_page = DASHBOARD;
            }
            if (ImGui::Button("Profile", ImVec2(150, 40))) {
                current_page = PROFILE;
            }
            if (ImGui::Button("Menu", ImVec2(150, 40))) {
                current_page = MENU;
            }
            if (ImGui::Button("Orders", ImVec2(150, 40))) {
                current_page = ORDERS;
            }
            if (ImGui::Button("Billing", ImVec2(150, 40))) {
                current_page = BILLING;
            }
            if (user_role == "admin" || user_role == "biller" || user_role == "manager") {
                if (ImGui::Button("Wallets", ImVec2(150, 40))) {
                    current_page = WALLETS;
                }
            }
            if (user_role == "admin" || user_role == "manager" || user_role == "biller") {
                if (ImGui::Button("Discounts", ImVec2(150, 40))) {
                    current_page = DISCOUNTS;
                }
            }
            if (user_role == "admin" || user_role == "manager") {
                if (ImGui::Button("Inventory", ImVec2(150, 40))) {
                    current_page = INVENTORY;
                }
            }
            if (user_role == "admin" || user_role == "manager") {
                if (ImGui::Button("Loyalty", ImVec2(150, 40))) {
                    current_page = LOYALTY;
                }
            }
            if (user_role == "admin" || user_role == "manager") {
                if (ImGui::Button("Activity Log", ImVec2(150, 40))) {
                    current_page = ACTIVITY_LOG;
                }
            }
            if (user_role == "admin" || user_role == "manager") {
                if (ImGui::Button("Analytics", ImVec2(150, 40))) {
                    current_page = ANALYTICS;
                }
            }
            if (user_role == "admin") {
                if (ImGui::Button("Settings", ImVec2(150, 40))) {
                    current_page = SETTINGS;
                }
            }
            if (user_role == "admin") {
                if (ImGui::Button("Backup", ImVec2(150, 40))) {
                    current_page = BACKUP;
                }
            }
            if (user_role == "admin") {
                if (ImGui::Button("Users", ImVec2(150, 40))) {
                    current_page = USERS;
                }
            }
            ImGui::Dummy(ImVec2(0, 20));
            if (ImGui::Button("Logout", ImVec2(150, 40))) {
                logged_in = false;
                panel_alpha = 1.0f;
                login_attempted = false;
                status = "";
                username[0] = '\0';
                password[0] = '\0';
                totp_code[0] = '\0';
                pending_totp_secret = "";
                current_page = DASHBOARD;
                login_stage = LOGIN_CREDENTIALS;
                logActivity(db, logged_in_username, "User logged out");
            }
            ImGui::EndGroup();

            ImGui::SameLine();
            ImGui::BeginGroup();
            switch (current_page) {
                case DASHBOARD:
                    renderDashboard(db, logged_in_username, user_role);
                    break;
                case PROFILE:
                    renderProfile(db, logged_in_username);
                    break;
                case MENU:
                    renderMenuManagement(db, user_role);
                    break;
                case ORDERS:
                    renderOrderManagement(db, user_role);
                    break;
                case BILLING:
                    renderBilling(db, user_role, logged_in_username);
                    break;
                case WALLETS:
                    renderWallets(db, user_role);
                    break;
                case DISCOUNTS:
                    renderDiscounts(db, user_role);
                    break;
                case INVENTORY:
                    renderInventory(db, user_role);
                    break;
                case LOYALTY:
                    renderLoyalty(db, user_role);
                    break;
                case ACTIVITY_LOG:
                    renderActivityLog(db, user_role);
                    break;
                case ANALYTICS:
                    renderAnalytics(db, user_role);
                    break;
                case SETTINGS:
                    renderSettings(db, user_role);
                    break;
                case BACKUP:
                    renderBackup(db, user_role);
                    break;
                case USERS:
                    renderUsers(db, user_role);
                    break;
            }
            ImGui::EndGroup();

            ImGui::End();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.07f, 0.08f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    sqlite3_close(db);

    return 0;
}