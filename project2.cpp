#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <iomanip>
#include <chrono>
#include <ctime>

// Class: Transaction
class Transaction {
private:
    std::string timestamp;
    std::string type; 
    double amount;
    double balanceAfter;
    std::string referenceAccountId;

    static std::string getCurrentTime() {
        auto now = std::chrono::system_clock::now();
        std::time_t timeNow = std::chrono::system_clock::to_time_t(now);
          char buffer[26];
        std::strftime(
          buffer,
            sizeof(buffer),
            "%Y-%m-%d %H:%M:%S",
            std::localtime(&timeNow)
        );
        std::string str(buffer);
        if (!str.empty() && str.back() == '\n') {
            str.pop_back(); 
        }
        return str;
    }

public:
    Transaction(std::string type, double amount, double balanceAfter, std::string refAcc = "")
        : timestamp(getCurrentTime()), type(type), amount(amount),
          balanceAfter(balanceAfter), referenceAccountId(refAcc) {}

    void display() const {
        std::cout << "[" << timestamp << "] "
                  << std::left << std::setw(14) << type
                  << " Amount: $" << std::right << std::setw(8) << std::fixed << std::setprecision(2) << amount
                  << " | Balance: $" << std::setw(9) << balanceAfter;
        if (!referenceAccountId.empty()) {
            std::cout << " [Ref: " << referenceAccountId << "]";
        }
        std::cout << "\n";
    }
};
// Class: Account
class Account {
private:
    std::string accountId;
    std::string accountType; 
    double balance;
    std::vector<Transaction> transactions;

    void logTransaction(const std::string& type, double amount, const std::string& refAcc = "") {
        transactions.emplace_back(type, amount, balance, refAcc);
    }

public:
    Account(std::string id, std::string type, double initialDeposit = 0.0)
        : accountId(id), accountType(type), balance(initialDeposit) {
        if (initialDeposit > 0.0) {
            logTransaction("INITIAL_DEP", initialDeposit);
        }
    }

    std::string getId() const { return accountId; }
    std::string getType() const { return accountType; }
    double getBalance() const { return balance; }

    bool deposit(double amount) {
        if (amount <= 0.0) {
            std::cout << "[Error] Deposit amount must be greater than zero.\n";
            return false;
        }
        balance += amount;
        logTransaction("DEPOSIT", amount);
        return true;
    }

    bool withdraw(double amount) {
        if (amount <= 0.0) {
            std::cout << "[Error] Withdrawal amount must be greater than zero.\n";
            return false;
        }
        if (amount > balance) {
            std::cout << "[Error] Insufficient funds in account " << accountId 
                      << ". Available balance: $" << std::fixed << std::setprecision(2) << balance << "\n";
            return false;
        }
        balance -= amount;
        logTransaction("WITHDRAWAL", amount);
        return true;
    }

    bool recordTransferOut(double amount, const std::string& targetAccId) {
        if (amount <= 0.0 || amount > balance) return false;
        balance -= amount;
        logTransaction("TRANSFER_OUT", amount, targetAccId);
        return true;
    }

    void recordTransferIn(double amount, const std::string& sourceAccId) {
        balance += amount;
        logTransaction("TRANSFER_IN", amount, sourceAccId);
    }

    void viewTransactions(int limit = 0) const {
        std::cout << "\n--- Transactions for Account: " << accountId << " ---\n";
        if (transactions.empty()) {
            std::cout << "No transactions recorded yet.\n";
            return;
        }

        size_t startIdx = 0;
        if (limit > 0 && static_cast<size_t>(limit) < transactions.size()) {
            startIdx = transactions.size() - limit;
        }

        for (size_t i = startIdx; i < transactions.size(); ++i) {
            transactions[i].display();
        }
    }
};

// Class: Customer
class Customer {
private:
    std::string customerId;
    std::string name;
    std::string email;
    std::unordered_map<std::string, std::shared_ptr<Account>> accounts;

public:
    Customer(std::string id, std::string name, std::string email)
        : customerId(id), name(name), email(email) {}

    std::string getId() const { return customerId; }
    std::string getName() const { return name; }

    void addAccount(std::shared_ptr<Account> account) {
        accounts[account->getId()] = account;
    }

    std::shared_ptr<Account> getAccount(const std::string& accountId) {
        auto it = accounts.find(accountId);
        if (it != accounts.end()) {
            return it->second;
        }
        return nullptr;
    }

    void displayProfile() const {
        std::cout << "\n====\n";
        std::cout << "Customer ID : " << customerId << "\n";
        std::cout << "Name        : " << name << "\n";
        std::cout << "Email       : " << email << "\n";
        std::cout << "Accounts    : " << accounts.size() << "\n";
        std::cout << "-----\n";
        
        for (const auto& entry : accounts) {
    const auto& id = entry.first;
    const auto& acc = entry.second;
    
            std::cout << "  * Acc ID: " << std::left << std::setw(10) << acc->getId()
                      << " | Type: " << std::setw(8) << acc->getType()
                      << " | Balance: $" << std::right << std::setw(8) << std::fixed << std::setprecision(2) << acc->getBalance() << "\n";
        }
        std::cout << "=====\n";
    }
};

// Class: Bank
class Bank {
private:
    std::string bankName;
    std::unordered_map<std::string, std::shared_ptr<Customer>> customers;
    std::unordered_map<std::string, std::shared_ptr<Account>> accounts;
    int nextCustomerId = 101;
    int nextAccountId = 1001;

public:
    explicit Bank(std::string name) : bankName(name) {}

    std::shared_ptr<Customer> registerCustomer(const std::string& name, const std::string& email) {
        std::string custId = "CUST" + std::to_string(nextCustomerId++);
        auto customer = std::make_shared<Customer>(custId, name, email);
        customers[custId] = customer;
        return customer;
    }

    std::shared_ptr<Account> openAccount(const std::string& customerId, const std::string& type, double initialDeposit = 0.0) {
        auto it = customers.find(customerId);
        if (it == customers.end()) {
            std::cout << "[Error] Customer ID " << customerId << " not found.\n";
            return nullptr;
        }

        std::string accId = "ACC" + std::to_string(nextAccountId++);
        auto account = std::make_shared<Account>(accId, type, initialDeposit);

        it->second->addAccount(account);
        accounts[accId] = account;
        return account;
    }

    bool transfer(const std::string& fromAccId, const std::string& toAccId, double amount) {
        if (fromAccId == toAccId) {
            std::cout << "[Error] Sender and receiver accounts cannot be the same.\n";
            return false;
        }
        if (amount <= 0.0) {
            std::cout << "[Error] Transfer amount must be positive.\n";
            return false;
        }

        auto srcIt = accounts.find(fromAccId);
        auto destIt = accounts.find(toAccId);

        if (srcIt == accounts.end()) {
            std::cout << "[Error] Source account " << fromAccId << " not found.\n";
            return false;
        }
        if (destIt == accounts.end()) {
            std::cout << "[Error] Destination account " << toAccId << " not found.\n";
            return false;
        }

        auto& srcAcc = srcIt->second;
        auto& destAcc = destIt->second;

        if (srcAcc->getBalance() < amount) {
            std::cout << "[Error] Transfer failed: Insufficient funds in " << fromAccId << ".\n";
            return false;
        }

        srcAcc->recordTransferOut(amount, toAccId);
        destAcc->recordTransferIn(amount, fromAccId);

        std::cout << "[Success] Transferred $" << std::fixed << std::setprecision(2) << amount
                  << " from " << fromAccId << " to " << toAccId << ".\n";
        return true;
    }
};

int main() {
    Bank bank("Global Apex Bank");


    auto alice = bank.registerCustomer("Alice Cooper", "alice@example.com");
    auto bob = bank.registerCustomer("Bob Vance", "bob@example.com");

   
    auto aliceSavings = bank.openAccount(alice->getId(), "SAVINGS", 1500.00);
    auto aliceChecking = bank.openAccount(alice->getId(), "CHECKING", 250.00);
    auto bobChecking = bank.openAccount(bob->getId(), "CHECKING", 400.00);

    aliceSavings->deposit(350.00);
    aliceSavings->withdraw(100.00);

    bank.transfer(aliceSavings->getId(), bobChecking->getId(), 500.00);

    alice->displayProfile();
    bob->displayProfile();

    aliceSavings->viewTransactions(5); 
    bobChecking->viewTransactions();
}
