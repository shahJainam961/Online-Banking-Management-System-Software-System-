#include "../clientConstants/const.h"
#include "../client-serverStructs/ourStruct.h"
#include <sys/types.h>  
#include <sys/socket.h> 
#include <netinet/ip.h> 
#include <stdio.h>      
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

char rBuffer[2000], wBuffer[2000];
ssize_t rBytes, wBytes;   


void createNormalAccount(int fd, int type){
    struct customer c;
    char name[30];
    printf("\nEnter your Name: ");
    scanf("%s", name);
    getchar();

    char gender;
    printf("\nEnter your Gender - M(Male), F(Female), O(Other): ");
    gender = getchar();
    getchar();

    char dob[9];
    printf("\nEnter your DateOfBirth (DDMMYYYY): ");
    scanf("%s",dob);
    getchar();

    char password[30];
    printf("\nEnter a Password: ");
    scanf("%s", password);
    getchar();

    strcpy(c.name, name);
    c.gender = gender;
    strcpy(c.dob, dob);
    strcpy(c.password, password);

    write(fd, &c, sizeof(c));

    if(type==0){
        read(fd, &c, sizeof(c));
        printf("\n\x1b[42mAccount sucessfully created!! Your loginId is %d\n\x1b[30;47m",c.id);  
    }
    return;
}

void createJointAccount(int fd, int type){
    printf("\nPlease enter your 1st Account Holder details as and when asked!!\n\n");
    createNormalAccount(fd, type);
    printf("\nPlease enter your 2nd Account Holder details as and when asked!!\n\n");
    createNormalAccount(fd, type);
    struct customer c1,c2;
    read(fd, &c1, sizeof(c1));
    read(fd, &c2, sizeof(c2));
    printf("\n\x1b[42mAccount sucessfully created!! Your loginIds are %d and %d respectively!!\x1b[30;47m\n\n",c1.id, c2.id);
    return;
}

int createAccount(int fd, int admin){
    char accType;
    m:system("clear");
    printf(WELCOME_PROMPT);
    printf(ACCOUNT_TYPE_MENU);
    printf("Enter your choice:\t");
    accType = getchar();
    getchar();
    write(fd, &accType, sizeof(accType));
    if(accType=='P') return 0;
    else if(!(accType=='0') && !(accType=='1')){
        printf("\n\n\x1b[37;41mInvalid Input TryAgain !!\x1b[30;47m\n\n");
        sleep(1);
        system("clear");
        write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
        goto m;
    }
    if(accType=='0'){ // Normal Account
        if(admin==0){
            printf("\nPlease enter your personal details as and when asked!!\n\n");
        }
        else if(admin==1){
            printf("\nPlease enter the person's details for which you want to make an account!!\n\n");
        }
        createNormalAccount(fd, 0);
        return 1;
    }
    else if(accType=='1'){ // Joint Account
        createJointAccount(fd, 1);
        return 1;
    }

}

void printDetails(int fd, struct account a){
    printf("\n\n%s",DASHED_LINES);
    printf("\t\tAccount Details\n");
    printf("%s",DASHED_LINES);
    printf("\tAccount Number: %d\n",a.accNo);
    printf("\tAccount Type 0(Normal), 1(Joint): %d\n",a.accType);
    printf("\tCurrent Balance: %d\n",a.currBalance);
    printf("%s",DASHED_LINES);
    if(a.accType==0){
        struct customer c;
        read(fd, &c, sizeof(c));
        printf("\t\tAccount Holder Details\n");
        printf("%s",DASHED_LINES);
        printf("\tId: %d\n",c.id);
        printf("\tAccount Holder Name: %s\n",c.name);
        printf("\tGender-M(Male), F(Female), O(Other): %c\n",c.gender);
        printf("\tDate of Birth (DDMMYYYY): %s\n",c.dob);
        printf("%s",DASHED_LINES);
    }
    else if(a.accType==1){
        struct customer c1;
        read(fd, &c1, sizeof(c1));
        struct customer c2;
        read(fd, &c2, sizeof(c2));
        printf("\t\tFirst Account Holder Details\n");
        printf("%s",DASHED_LINES);
        printf("\tId: %d\n",c1.id);
        printf("\tAccount Holder Name: %s\n",c1.name);
        printf("\tGender-M(Male), F(Female), O(Other): %c\n",c1.gender);
        printf("\tDate of Birth (DDMMYYYY): %s\n",c1.dob);
        printf("%s",DASHED_LINES);
        printf("\t\tSecond Account Holder Details\n");
        printf("%s",DASHED_LINES);
        printf("\tId: %d\n",c2.id);
        printf("\tAccount Holder Name: %s\n",c2.name);
        printf("\tGender-M(Male), F(Female), O(Other): %c\n",c2.gender);
        printf("\tDate of Birth (DDMMYYYY): %s\n",c2.dob);
        printf("%s",DASHED_LINES);
    }
}

void passwordChangeService(int fd){
    struct customer c;
    printf("\n\n%s",DASHED_LINES);
    printf("\tEnter old Password:\t");
    scanf("%s",c.password);
    getchar();
    write(fd, &c, sizeof(c));
    char status;
    read(fd, &status, 1);
    if(status=='C'){ //old password matches
        printf("\tEnter New Password:\t");
        scanf("%s",c.password);
        getchar();
        write(fd, &c, sizeof(c));
        read(fd, &status, 1);
        if(status=='Y'){
            printf("\t\x1b[42mPassword Successfully Changed!!\x1b[30;47m\n");
            printf("%s",DASHED_LINES);
            return;
        }
        else{
            printf("\t\x1b[37;41mInternal Server Error, Please try again.\x1b[30;47m\n");
            printf("%s",DASHED_LINES);
            return;
        }
    }
    else if(status=='W'){
        printf("\t\x1b[37;41mOld Password did not matched!!\x1b[30;47m\n");
        printf("%s",DASHED_LINES);
        return;
    }
}

void withdrawService(int fd){
    struct transaction t1;
    printf("\n\n%s",DASHED_LINES);
    printf(ENTER_WITHDRAWAL_AMOUNT);
    scanf("%d",&t1.transactMoney);
    getchar();
    t1.typeOfTransaction = 1; //withdraw
    write(fd, &t1, sizeof(t1));
    char status;
    read(fd, &status, 1);
    if(status=='W'){
        printf(INSUFFICIENT_BALANCE);
        printf("%s\n",DASHED_LINES);
        return;
    }
    else if(status == 'C'){
        struct account a;
        read(fd, &a, sizeof(a));
        printf("\t%d Rs withdrawn from your account\n",t1.transactMoney);
        printf("\tAvailable Balance: %d\n",a.currBalance);
        printf("%s\n",DASHED_LINES);
        return;
    }
}

void depositService(int fd){
    struct transaction t1;
    printf("\n\n%s",DASHED_LINES);
    printf(ENTER_DEPOSIT_MONEY);
    scanf("%d",&t1.transactMoney);
    getchar();
    t1.typeOfTransaction = 0; // deposit
    write(fd, &t1, sizeof(t1));
    char status;
    read(fd, &status, 1);
    if(status == 'C'){
        struct account a;
        read(fd, &a, sizeof(a));
        printf("\t%d Rs deposited to your account\n",t1.transactMoney);
        printf("\tAvailable Balance: %d\n",a.currBalance);
        printf("%s\n",DASHED_LINES);
        return;
    }
}

void balanceEnqService(int fd){
    struct account a;
    read(fd, &a, sizeof(a));
    printf("\n\n%s",DASHED_LINES);
    printf("\tCurrent Available Balance: %d\n",a.currBalance);
    printf("%s",DASHED_LINES);
    return;
}

void loginServices(int fd){
    while(1){
        printf(LOGIN_SERVICES_MENU);
        printf("Enter your choice:\t");
        char choice;
        choice = getchar();
        getchar();
        write(fd, &choice, 1);
        if(choice == 'N'){ // Logout
            printf("\n\n\x1b[42mLogout Successful !!\x1b[30;47m\n\n");
            return;
        }
        else if(choice == '4'){ // Show Account Details
            struct account a;
            read(fd, &a, sizeof(a));
            printDetails(fd, a);
            printf("\nPress Enter to continue!!\n");
            getchar();
            system("clear");
            write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
            continue;
        }
        else if(choice == '3'){ // Change password
            passwordChangeService(fd);
            sleep(2);
            system("clear");
            write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
            continue;
        }
        else if(choice == '2'){ // Balance Enquiry
            balanceEnqService(fd);
            printf("\nPress Enter to continue!!\n");
            getchar();
            system("clear");
            write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
            continue;
        }
        else if(choice == '1'){ // Withdraw Money
            withdrawService(fd);
            printf("\nPress Enter to continue!!\n");
            getchar();
            system("clear");
            write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
            continue;
        }
        else if(choice == '0'){ // Deposit Money
            depositService(fd);
            printf("\nPress Enter to continue!!\n");
            getchar();
            system("clear");
            write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
            continue;
        }
        else{
            printf("\n\x1b[37;41mInvalid input, try again!!\x1b[30;47m\n");
            sleep(1);
            system("clear");
            write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
            continue;
        }
    }

    
}

void login(int fd){
    struct customer c; 
    printf("\n \tEnter your loginId: \t");
    scanf("%d", &c.id);
    getchar();

    printf("\n \tEnter your Password: \t");
    scanf("%s", c.password);
    getchar();

    write(fd, &c, sizeof(c));
    
    char status;
    read(fd, &status, 1);

    if(status == '!'){
        printf("\n\n\x1b[37;41mLoginId %d does not exist!!\x1b[30;47m\n\n", c.id);
        return;
    }
    else if(status == 'W'){
        printf("\n\n\x1b[37;41mPassword entered is incorrect!!\x1b[30;47m\n\n");
        return;
    }
    else if(status == 'I'){
        printf("\n\n\x1b[37;41mYour account is Deleted\x1b[30;47m\n\n");
        return;
    }
    else if(status == 'C'){
        printf("\n\n\x1b[42mLogin Successful !!\x1b[30;47m\n\n");
        sleep(2);
        system("clear");
        write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
        loginServices(fd);
        return;
    }
    else if(status == '$'){
        printf("\x1b[37;41m%s\x1b[30;47m\n\n",MAX_LOGIN_REACHED);
        return;
    }

}

void deleteAccountService(int fd){
    struct account a;
    printf("\n");
    printf(DASHED_LINES);
    printf("\tEnter Account number that you want to delete:\t");
    scanf("%d",&a.accNo);
    getchar();
    write(fd, &a, sizeof(a));
    char status;
    read(fd, &status, 1);
    if(status=='W'){
        printf(DASHED_LINES);
        printf("\n\x1b[37;41mAccount number %d does not exist!!\x1b[30;47m\n",a.accNo);
        return;
    }
    else if(status=='I'){
        printf(DASHED_LINES);
        printf("\n\x1b[37;41mAccount number %d is already deleted!!\x1b[30;47m\n",a.accNo);
        return;
    }
    else if(status=='C'){
        printf(DASHED_LINES);
        printf("\n\x1b[37;41mAccount number %d is deleted!!\x1b[30;47m\n",a.accNo);
        return;
    }
    else if(status=='A'){
        printf(DASHED_LINES);
        printf("\n\x1b[37;41mUser associated with account number %d is already logged in, cannot perform delete operation!!\x1b[30;47m\n",a.accNo);
        return;
    }
    
}

void searchAccountService(int fd){
    struct account a;
    printf(DASHED_LINES);
    printf("\tEnter Account number to search:\t");
    scanf("%d", &a.accNo);
    getchar();
    write(fd, &a, sizeof(a));
    char status;
    read(fd, &status, 1);
    if(status == 'W'){
        printf("\x1b[37;41mAccount number %d does not exist!\x1b[30;47m\n",a.accNo);
        printf(DASHED_LINES);
        return;
    }
    else if(status == 'I'){
        printf("\x1b[37;41mAccount number %d had been deleted!\x1b[30;47m\n",a.accNo);
        printf(DASHED_LINES);
        return;
    }
    else if(status == 'C'){
        struct account acc;
        read(fd, &acc, sizeof(acc));
        printDetails(fd, acc);
        return;
    }

}

void adminLoginServices(int fd){
    while(1){
        printf(ADMIN_LOGIN_SERVICES_MENU);
        printf("Enter your choice:\t");
        char choice;
        choice = getchar();
        getchar();
        write(fd, &choice, 1);
        if(choice == 'N'){ // Logout
            printf("\n\n\x1b[42mLogout Successful !!\x1b[30;47m\n\n");
            return;
        }
        else if(choice == '0'){ // Add Account
        
            int r = createAccount(fd, 1);
            if(r==0){
                system("clear");
                write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
                continue;
            }
            printf("\nPress Enter to continue!!\n");
            getchar();
            system("clear");
            write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
            continue;
        }
        else if(choice == '1'){ // Delete Account
            deleteAccountService(fd);
            printf("\nPress Enter to continue!!\n");
            getchar();
            system("clear");
            write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
            continue;
        }
        else if(choice == '2'){ // Modify Account Details
            printf("\nPress Enter to continue!!\n");
            getchar();
            system("clear");
            write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
            continue;
        }
        else if(choice == '3'){ // Search Account Details
            searchAccountService(fd);
            printf("\nPress Enter to continue!!\n");
            getchar();
            system("clear");
            write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
            continue;
        }
        else{
            printf("\n\x1b[37;41mInvalid input, try again!!\x1b[30;47m\n");
            sleep(1);
            system("clear");
            write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
            continue;
        }
    }
}

void adminLogin(int fd){
    struct adminCreds admin;
    printf("\n");
    printf(DASHED_LINES);
    printf("Enter adminId:\t");
    scanf("%d", &admin.id);
    getchar();

    printf("Enter adminPassword:\t");
    scanf("%s", admin.password);
    getchar();
    printf(DASHED_LINES);

    write(fd, &admin, sizeof(admin));

    char status;
    read(fd, &status, 1);
    if(status == 'W'){
        printf("\n\x1b[37;41mAdmin-Credentials are wrong!!\x1b[30;47m\n");
        return;
    }
    else if(status == 'I'){
        printf("\n\x1b[37;41mMax Admin-Login limit reached, logout from the other device for access!\x1b[30;47m\n");
        return;
    }
    else if(status == 'C'){
        printf("\n\n\x1b[42mAdmin-Login Successful !!\x1b[30;47m\n\n");
        sleep(2);
        system("clear");
        write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
        adminLoginServices(fd);
        return;
    }

}

void commHandler(int fd){ //Communication Handler
    
    write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
    
    while(1){
        printf("%s",MENU_PROMPT);

        char choice;
        choice = getchar();
        getchar();
        wBytes = write(fd, &choice, sizeof(choice));

        if(choice == 'N'){ // Exiting..
            printf("%s",EXIT_PROMPT);
            return;
        }
        else if(choice == '2'){ // Creating Account..
            int r = createAccount(fd, 0);
            if(r==0){
                system("clear");
                write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
                continue;
            }
            printf("\nPress Enter to continue!!\n");
            getchar();
            system("clear");
            write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
            continue;
        }
        else if(choice == '1'){ // Customer Login..
            login(fd);
            sleep(2);
            system("clear");
            write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
            continue;
        }
        else if(choice == '0'){
            adminLogin(fd);
            sleep(2);
            system("clear");
            write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
            continue;
        }
        else{
            printf("\n\x1b[37;41mInvalid input, try again!!\x1b[30;47m\n");
            sleep(1);
            system("clear");
            write(1,WELCOME_PROMPT,sizeof(WELCOME_PROMPT));
            continue;
        }
        
    }
}