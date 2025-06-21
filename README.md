# Canteen Management System 🍽️

Welcome to the **Canteen Management System**, a slick software solution by **Team Runtime Assassins** that makes canteen operations a breeze! Built with C++, ImGui, and SQLite3, it handles orders, billing, inventory, and more through a user-friendly graphical interface. With SHA-256 password hashing and TOTP two-factor authentication, it’s secure, efficient, and customer-focused. Want to see it shine? Check out the demo video link provided in the main branch!

## Features 🚀

Our system packs 14 powerful features:

- **User Authentication & Role Management**: Secure login with SHA-256 hashing and TOTP 2FA. Roles: Admin (full control), Manager (operations), Biller (billing).
- **Menu Management**: Add, edit, delete, or toggle menu items via ImGui UI.
- **Order Management**: Create, track, edit, or cancel orders with refund support.
- **Billing & Payment**: Generate itemized bills with taxes, supporting wallet payments or cash/card for guests.
- **User Wallet System**: Manage cashless payments with balance tracking and top-ups.
- **Loyalty Program**: Reward customers with points for discounts or free items.
- **Discount & Promotion Management**: Create percentage, fixed, or time-limited combo deals.
- **Inventory Management**: Track stock with real-time low-stock alerts.
- **Analytics Dashboard**: Visualize sales, top items, and user activity trends.
- **System Settings & Configuration**: Customize tax rates, loyalty rules, and backups.
- **Bill Saving as Files**: Save bills as text or PDF in `output/bills/`.
- **User Activity Log**: Audit actions (logins, orders, refunds) in `output/activity_log.txt`.
- **System Backup**: Manual or scheduled backups to protect data.
- **Refund Option**: Process refunds for canceled orders.

## Demo 🎥

Get a glimpse of the Canteen Management System’s features:

- **Video Demo**: Find the demo video link in `demo_link.txt` in the repository’s main branch to watch the ImGui UI, order processing, billing, and more.
- **Documentation**: `docs/Canteen-Management-System.pdf` and `docs/Project report.pdf` include screenshots and feature walkthroughs.

*Note*: Ensure the link in `demo_link.txt` is accessible (e.g., set to “Anyone with the link” for Google Drive). Contact us if you need help.

## Requirements 🛠️

To build and run the system, you’ll need:

### For macOS
- **OS**: macOS (e.g., Ventura, Sonoma).
- **Compiler**: Clang (via Xcode or `brew install llvm`).
- **Libraries**:
  - **ImGui**: Included in `lib/imgui/`.
  - **GLFW**: Window/input handling.
  - **OpenGL**: Version 3.2+ (included with macOS).
  - **SQLite3**: Database management.
  - **OpenSSL**: For SHA-256 and TOTP.
- **Tools**:
  - CMake: For building.
  - SQLite3 CLI: For database setup.
  - (Optional) LaTeX: For bill PDFs (`texlive-full`).
- **Hardware**: Mac with 4GB RAM, 1GB free storage.

### For Windows
- **OS**: Windows 10 or 11.
- **Compiler**: MSVC (Visual Studio) or MinGW (GCC).
- **Libraries**:
  - **ImGui**: Included in `lib/imgui/`.
  - **GLFW**: Window/input handling.
  - **OpenGL**: Version 3.2+.
  - **SQLite3**: Database management.
  - **OpenSSL**: For SHA-256 and TOTP.
- **Tools**:
  - CMake: For building.
  - SQLite3 CLI: For database setup.
  - (Optional) LaTeX: For bill PDFs (e.g., MiKTeX).
- **Hardware**: PC with 4GB RAM, 1GB free storage.

## Installation 📦

### macOS
1. **Clone the Repository**:
   ```bash
   git clone https://github.com/RUDRAKSHARMA18/Canteen-management-system.git
   cd Canteen-management-system
   ```
2. **Install Dependencies**:
   - Install Homebrew: `/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"`
   - Install dependencies:
     ```bash
     brew install cmake sqlite glfw openssl
     ```
   - (Optional) LaTeX for PDFs:
     ```bash
     brew install --cask mactex
     ```
3. **Build the Project**:
   ```bash
   mkdir build && cd build
   cmake -DOPENSSL_ROOT_DIR=$(brew --prefix openssl) ..
   make
   ```
4. **Initialize the Database**:
   ```bash
   sqlite3 data/users.db < data/populate_db.sql
   ```
5. **Run the Application**:
   ```bash
   ./CanteenManagementSystem
   ```
   - Or admin panel: `./AdminPanel`

### Windows
1. **Clone the Repository**:
   - Use Git Bash or download ZIP from GitHub and extract.
     ```bash
     git clone https://github.com/RUDRAKSHARMA18/Canteen-management-system.git
     cd Canteen-management-system
     ```
2. **Install Dependencies**:
   - Install Visual Studio (Community) with C++ Desktop Development.
   - Install CMake: Download from [cmake.org](https://cmake.org/download/), add to PATH.
   - Install SQLite3: Download DLL/tools from [sqlite.org](https://www.sqlite.org/download.html).
   - Install GLFW: Download binaries from [glfw.org](https://www.glfw.org/download.html) or build from source.
   - Install OpenSSL: Download Win64 OpenSSL from [slproweb.com](https://slproweb.com/products/Win32OpenSSL.html), add to PATH.
   - (Optional) Install MiKTeX: From [miktex.org](https://miktex.org/download).
3. **Build the Project**:
   - In Command Prompt or PowerShell:
     ```cmd
     mkdir build && cd build
     cmake -G "Visual Studio 16 2019" -A x64 -DOPENSSL_ROOT_DIR="C:\Program Files\OpenSSL-Win64" ..
     cmake --build . --config Release
     ```
   - Or MinGW:
     ```bash
     mkdir build && cd build
     cmake -G "MinGW Makefiles" -DOPENSSL_ROOT_DIR="C:\Program Files\OpenSSL-Win64" ..
     mingw32-make
     ```
4. **Initialize the Database**:
   ```bash
   sqlite3 data/users.db < data/populate_db.sql
   ```
5. **Run the Application**:
   ```bash
   .\Release\CanteenManagementSystem.exe
   ```
   - Or admin panel: `.\Release\AdminPanel.exe`

## Usage 📝

- **Admin**:
  - **Console UI** (`AdminPanel`): Manage users, reset TOTP, view logs (`src/admin.cpp`).
  - **ImGui UI** (`CanteenManagementSystem`): Access all features (menu, inventory, analytics, backups, settings).
  - Example: Log in with admin credentials from `data/populate_db.sql`, use TOTP, add a menu item.

- **Manager**:
  - **ImGui UI**: Manage menus, inventory, orders, discounts, loyalty, analytics.
  - Example: Create a combo deal or check stock alerts.

- **Biller**:
  - **ImGui UI**: Process orders, generate bills, apply discounts, top up wallets.
  - Example: Create an order, redeem loyalty points, save a bill as PDF (`output/bills/`).

- **Guide**: See `docs/Canteen-Management-System.pdf` or `docs/Project report.pdf` for details.

## License 📜

Licensed under the **Apache 2.0 License** (see [LICENSE](LICENSE)). ImGui in `lib/imgui/` is under the MIT License (see `lib/imgui/LICENSE.txt`).

## About the Developers 🌟

**Team Runtime Assassins** is a passionate developer crew:
- Priyanshi Rana
- Priyanshi Sharma
- Riya Fulambarkar
- Rudra Kumar Sharma

## Contact 📬

Questions or feedback? Open an issue or email `rudra_18@outlook.com`.

---

*Developed with 💻 and ☕ by Team Runtime Assassins, 2025.*