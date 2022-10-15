struct adminCreds{
    int id;                     // admin Id
    char password[30];          // admin Password
};

struct activity{
    int id;                     // Customer id
    int login;                  // 0-notLoggedIn, 1-loggedIn
};

struct account{
    int accNo;                  // Unique Account Number
    int accType;                // 0-Normal Account, 1-Joint Account
    int id[2];                  // Customer ids
    int currBalance;            // Current Available Balance
    int active;                 // if the account is active or not. 0-Deleted, 1-Active
};


struct customer{
    int id;                     // Unique_id of the customer, also used as loginId
    char name[30];              // Name of the customer
    char gender;                // Male-M, Female-F, Other-O
    char dob[9];                // Date of Birth, format-ddmmyyyy
    char password[30];          // Password for logging in the service
    int accNo;                  // Account Number associated with the customer
};

struct transaction{
    int tid;                    // Unique transaction-id
    int accNo;                  // Account Number associated with the transaction
    int typeOfTransaction;      // 0-Deposit, 1-Withdraw
    int transactMoney;          // Deposited money if typeOfTransaction=0 else Withdrawn money if typeOfTransaction=1
    int beforeBalance;          // Balanace in the account before the transaction
    int afterBalance;           // Balanace in the account after the transaction
    char timeOfTransact[30];    // Time at which the transaction was made.
};