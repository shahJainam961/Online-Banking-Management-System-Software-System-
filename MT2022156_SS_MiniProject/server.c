//Server Side
#define BASE 1000

#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "./client-serverStructs/ourStruct.h"
#include "./serverConstants/admin-creds.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>  
#include <sys/socket.h> 
#include <netinet/ip.h> 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>


char rBuffer[2000], wBuffer[2000];
ssize_t rBytes, wBytes;   

// To acquire lock on file
void acquireLock(int ffd, int typeOfLock, int whence, int start, int len, int pid){
    struct flock lock;
    lock.l_type = typeOfLock;
    lock.l_whence = whence;
    lock.l_start = start;
    lock.l_len = len;
    lock.l_pid = pid;

    fcntl(ffd, F_SETLKW, &lock);
    return;
}

// To release lock on file
void releaseLock(int ffd, int typeOfLock, int whence, int start, int len, int pid){
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_whence = whence;
    lock.l_start = start;
    lock.l_len = len;
    lock.l_pid = pid;

    fcntl(ffd, F_SETLK, &lock);
    return;
}

// To generate loginIds for the users
int getId(int userIdfd, int type){
    char buff;
    int id = 0;
    while(1){
        int s = read(userIdfd, &buff, 1);
        if(s==0) break;
        id = (id*10) + (buff-'0');
    }
    if(type == 0) id++;
    else if(type == 1) id += 2;
    sprintf(wBuffer, "%d", id);
    lseek(userIdfd, 0, SEEK_SET);
    write(userIdfd, wBuffer, strlen(wBuffer));

    return id;
}

// To create normal account
void normalAccountCreationHandler(int fd){

    // Reading new customer from client side
    struct customer c;
    read(fd, &c, sizeof(c));

    //Using the userIds file to generate new id
    int userIdfd = open("./files/userIds",O_RDWR);
    acquireLock(userIdfd, F_WRLCK, SEEK_SET, 0, 0, getpid());

    // Assigning id and account number to the new customer
    c.id = getId(userIdfd, 0);
    c.accNo = BASE + c.id;
    
    // Writing the new customer data into the customers file
    int custfd = open("./files/customers.dat",O_WRONLY);
    acquireLock(custfd, F_WRLCK, SEEK_SET, 0, 0, getpid());
    lseek(custfd, 0, SEEK_END);
    write(custfd, &c, sizeof(c));
    // Releasing lock from customers file
    releaseLock(custfd, F_UNLCK, SEEK_SET, 0, 0, getpid());
    close(custfd);

    // Making account of new customer
    struct account a;
    a.accNo = c.accNo;                  
    a.accType = 0;          // Normal Account               
    a.id[0] = c.id;         // User id
    a.id[1] = -1;           // -1 as it is normal account
    a.currBalance = BASE;   // Joining Bonus    
    a.active = 1;           // Account is active

    // Writing new account data into the accounts file
    int accfd = open("./files/accounts.dat",O_WRONLY);
    acquireLock(accfd, F_WRLCK, SEEK_SET, 0, 0, getpid());
    lseek(accfd, 0, SEEK_END);
    write(accfd, &a, sizeof(a));
    //Releasing lock from accounts file
    releaseLock(accfd, F_UNLCK, SEEK_SET, 0, 0, getpid());
    close(accfd);
    // Sending loginIds to client
    write(fd, &c, sizeof(c));

    // Making activity of that customer
    struct activity act;
    act.id = c.id;          // User id
    act.login = 0;          // Initially not logged in

    // Writing created activity into the activities file
    int actfd = open("./files/activities.dat",O_WRONLY);
    acquireLock(actfd, F_WRLCK, SEEK_SET, 0, 0, getpid());
    lseek(actfd, 0, SEEK_END);
    write(actfd, &act, sizeof(act));

    // Releasing lock from activities file
    releaseLock(actfd, F_UNLCK, SEEK_SET, 0, 0, getpid());
    close(actfd);

    // Releasing lock from userIDs file
    releaseLock(userIdfd, F_UNLCK, SEEK_SET, 0, 0, getpid());
    close(userIdfd);
}

// To create joint account
void jointAccountCreationHandler(int fd){
    // Reading both customers new data from client side
    struct customer c1, c2;
    read(fd, &c1, sizeof(c1));
    read(fd, &c2, sizeof(c2));

    //Using the userIds file to generate new id
    int userIdfd = open("./files/userIds",O_RDWR);
    acquireLock(userIdfd, F_WRLCK, SEEK_SET, 0, 0, getpid());

    // Assigning id and account number to the new customers
    c1.id = getId(userIdfd, 1) - 1;
    c1.accNo = BASE + c1.id;
    c2.accNo = c1.accNo;
    c2.id = c1.id + 1;

    // Writing the new customers data into the customers file
    int custfd = open("./files/customers.dat",O_WRONLY);
    acquireLock(custfd, F_WRLCK, SEEK_SET, 0, 0, getpid());
    lseek(custfd, 0, SEEK_END);
    write(custfd, &c1, sizeof(c1));
    write(custfd, &c2, sizeof(c2));
    releaseLock(custfd, F_UNLCK, SEEK_SET, 0, 0, getpid());
    close(custfd);

    // Making a new joint account
    struct account a;
    a.accNo = c1.accNo;                  
    a.accType = 1;                
    a.id[0] = c1.id;
    a.id[1] = c2.id;
    a.currBalance = BASE;    
    a.active = 1;

    // Writing new joint account to accounts file
    int accfd = open("./files/accounts.dat",O_WRONLY);
    acquireLock(accfd, F_WRLCK, SEEK_SET, 0, 0, getpid());
    lseek(accfd, 0, SEEK_END);
    write(accfd, &a, sizeof(a));

    // Writing dummy account which will be of no use just to maintan sequencing
    struct account tmp;
    tmp.id[0] = -1;
    tmp.id[1] = -1;
    tmp.currBalance = -1;
    tmp.accType = -1;
    tmp.accNo = a.accNo+1;
    tmp.active = -1;
    write(accfd, &tmp, sizeof(tmp));
    
    // Releasing lock from accounts file
    releaseLock(accfd, F_UNLCK, SEEK_SET, 0, 0, getpid());
    close(accfd);

    // Sending LoginIds to the clients
    write(fd, &c1, sizeof(c1));
    write(fd, &c2, sizeof(c2));

    // ACtivity tracking
    struct activity act;
    act.id = c1.id;
    act.login = 0;
    int actfd = open("./files/activities.dat",O_WRONLY);
    acquireLock(actfd, F_WRLCK, SEEK_SET, 0, 0, getpid());
    lseek(actfd, 0, SEEK_END);
    write(actfd, &act, sizeof(act));
    act.id = c2.id;
    write(actfd, &act, sizeof(act));

    // Releasing lock from activities file
    releaseLock(actfd, F_UNLCK, SEEK_SET, 0, 0, getpid());
    close(actfd);

    // Releasing lock from userIds file
    releaseLock(userIdfd, F_UNLCK, SEEK_SET, 0, 0, getpid());
    close(userIdfd);
}

// To handle displaying account details
void accountDetailsHandler(int fd, int id, int accNo){
    struct account a;
    // Searching for the requested account in accounts file
    int accfd = open("./files/accounts.dat",O_RDONLY);
    acquireLock(accfd, F_RDLCK, SEEK_SET, (accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid());
    lseek(accfd, (accNo-BASE-1)*sizeof(struct account), SEEK_SET);
    read(accfd, &a, sizeof(a));
    releaseLock(accfd, F_RDLCK, SEEK_SET, (accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid());
    close(accfd);

    write(fd, &a, sizeof(a));
    if(a.accType==0){ // Normal Account
        struct customer c;
        int custfd = open("./files/customers.dat",O_RDONLY);
        acquireLock(custfd, F_RDLCK, SEEK_SET, (id-1)*sizeof(struct customer), sizeof(struct customer), getpid());
        lseek(custfd, (id-1)*sizeof(struct customer), SEEK_SET);
        read(custfd, &c, sizeof(c));
        write(fd, &c, sizeof(c));
        releaseLock(custfd, F_UNLCK, SEEK_SET, (id-1)*sizeof(struct customer), sizeof(struct customer), getpid());
        close(custfd);
    }
    else if(a.accType==1){ // Joint Account
        struct customer c1,c2;
        int custfd = open("./files/customers.dat",O_RDONLY);
        acquireLock(custfd, F_RDLCK, SEEK_SET, (a.id[0]-1)*sizeof(struct customer), sizeof(struct customer), getpid());
        lseek(custfd, (a.id[0]-1)*sizeof(struct customer), SEEK_SET);
        read(custfd, &c1, sizeof(c1));
        read(custfd, &c2, sizeof(c2));
        write(fd, &c1, sizeof(c1));
        write(fd, &c2, sizeof(c2));
        acquireLock(custfd, F_UNLCK, SEEK_SET, (a.id[0]-1)*sizeof(struct customer), sizeof(struct customer), getpid());
        close(custfd);
    }
}

// To change the password
void passwordChangeHandler(int fd, int id){
    struct customer actual;
    int custfd = open("./files/customers.dat",O_RDONLY);
    struct flock lock1;

    lock1.l_type = F_RDLCK;
    lock1.l_whence = SEEK_SET;
    lock1.l_start=(id-1)*sizeof(struct customer);
    lock1.l_len=sizeof(struct customer);
    lock1.l_pid = getpid();

    fcntl(custfd, F_SETLKW, &lock1);
    acquireLock(custfd, F_RDLCK, SEEK_SET, (id-1)*sizeof(struct customer), sizeof(struct customer), getpid()); 
    lseek(custfd, (id-1)*sizeof(struct customer), SEEK_SET);
    read(custfd, &actual, sizeof(actual));
    releaseLock(custfd, F_UNLCK, SEEK_SET, (id-1)*sizeof(struct customer), sizeof(struct customer), getpid()); 
    close(custfd);

    struct customer c;
    read(fd, &c, sizeof(c));
    char status;
    if(strcmp(c.password, actual.password)==0){
        status = 'C';
        write(fd, &status, 1);
        read(fd, &c, sizeof(c));
        strcpy(actual.password,c.password);
        struct customer c1;
        int custfd = open("./files/customers.dat",O_RDWR);
        acquireLock(custfd, F_WRLCK, SEEK_SET, (id-1)*sizeof(struct customer), sizeof(struct customer), getpid()); 
        lseek(custfd, (id-1)*sizeof(struct customer), SEEK_SET);
        write(custfd, &actual, sizeof(actual));
        
        status = 'Y';
        write(fd, &status, 1);

        acquireLock(custfd, F_UNLCK, SEEK_SET, (id-1)*sizeof(struct customer), sizeof(struct customer), getpid()); 
        close(custfd);
        return;
    }
    else{
        status ='W';
        write(fd, &status, 1);
        return;
    }

}

// To decrease activity
void decreaseActivity(int id){
    struct activity act;
    int actfd = open("./files/activities.dat",O_RDWR);
    acquireLock(actfd, F_WRLCK, SEEK_SET, (id-1)*sizeof(struct activity), sizeof(struct activity), getpid()); 
    lseek(actfd, (id-1)*sizeof(struct activity), SEEK_SET);
    read(actfd, &act, sizeof(act));
    act.login=0;
    lseek(actfd, (id-1)*sizeof(struct activity), SEEK_SET);
    write(actfd, &act, sizeof(act));
    acquireLock(actfd, F_UNLCK, SEEK_SET, (id-1)*sizeof(struct activity), sizeof(struct activity), getpid()); 
    close(actfd);
}

// For withdraw service
void withdrawHandler(int fd, int id, int accNo){
    struct transaction t1;
    read(fd, &t1, sizeof(t1));
    
    struct account a;
    int accfd = open("./files/accounts.dat",O_RDWR);
    acquireLock(accfd, F_WRLCK, SEEK_SET, (accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid()); 
    lseek(accfd, (accNo-BASE-1)*sizeof(struct account), SEEK_SET);
    read(accfd, &a, sizeof(a));
    char status;
    if(a.currBalance<t1.transactMoney){
        status = 'W';
        write(fd, &status, 1);
        releaseLock(accfd, F_UNLCK, SEEK_SET, (accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid()); 
        close(accfd);
        return;
    }
    int transfd = open("./files/transactIds",O_RDWR);
    acquireLock(transfd, F_WRLCK, SEEK_SET, 0, 0, getpid()); 

    t1.tid = getId(transfd, 0);
    t1.accNo = accNo;
    t1.beforeBalance = a.currBalance;
    t1.afterBalance = a.currBalance - t1.transactMoney;
    time_t _t;
    time(&_t);
    strcpy(t1.timeOfTransact,ctime(&_t));

    int transferfd = open("./files/transactions.dat",O_WRONLY);
    acquireLock(transferfd, F_WRLCK, SEEK_SET, 0, 0, getpid()); 
    lseek(transferfd, 0, SEEK_END);
    write(transferfd, &t1, sizeof(t1));
    
    a.currBalance = t1.afterBalance;

    lseek(accfd, (accNo-BASE-1)*sizeof(struct account), SEEK_SET);
    write(accfd, &a, sizeof(a));
    status = 'C';
    write(fd, &status, 1);
    write(fd, &a, sizeof(a));
    acquireLock(transferfd, F_UNLCK, SEEK_SET, 0, 0, getpid()); 
    close(transferfd);
    
    releaseLock(transfd, F_UNLCK, SEEK_SET, 0, 0, getpid()); 
    close(transfd);

    releaseLock(accfd, F_UNLCK, SEEK_SET, (accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid()); 
    close(accfd);
    
    return;
}

// For Deposit money
void depositHandler(int fd, int id, int accNo){
    struct transaction t1;
    read(fd, &t1, sizeof(t1));

    struct account a;
    int accfd = open("./files/accounts.dat",O_RDWR);
    acquireLock(accfd, F_WRLCK, SEEK_SET, (accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid()); 

    int transfd = open("./files/transactIds",O_RDWR);
    acquireLock(transfd, F_WRLCK, SEEK_SET, 0, 0, getpid()); 

    int transferfd = open("./files/transactions.dat",O_WRONLY);
    acquireLock(transferfd, F_WRLCK, SEEK_SET, 0, 0, getpid()); 
    ////
    char status;
    lseek(accfd, (accNo-BASE-1)*sizeof(struct account), SEEK_SET);
    read(accfd, &a, sizeof(a));

    t1.tid = getId(transfd, 0);
    t1.accNo = accNo;
    t1.beforeBalance = a.currBalance;
    t1.afterBalance = a.currBalance + t1.transactMoney;
    time_t _t;
    time(&_t);
    strcpy(t1.timeOfTransact,ctime(&_t));
    lseek(transferfd, 0, SEEK_END);
    write(transferfd, &t1, sizeof(t1));

    a.currBalance = t1.afterBalance;
    lseek(accfd, (accNo-BASE-1)*sizeof(struct account), SEEK_SET);
    write(accfd, &a, sizeof(a));

    status = 'C';
    write(fd, &status, 1);
    write(fd, &a, sizeof(a));
    ////

    releaseLock(transferfd, F_UNLCK, SEEK_SET, 0, 0, getpid()); 
    close(transferfd);
    
    releaseLock(transfd, F_UNLCK, SEEK_SET, 0, 0, getpid()); 
    close(transfd);

    releaseLock(accfd, F_UNLCK, SEEK_SET, (accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid()); 
    close(accfd);
}

// For Balance Enquiry
void balanceEnqHandler(int fd, int accNo){
    struct account a;
    int accfd = open("./files/accounts.dat",O_RDONLY);
    acquireLock(accfd, F_RDLCK, SEEK_SET, (accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid()); 
    lseek(accfd, (accNo-BASE-1)*sizeof(struct account), SEEK_SET);
    read(accfd, &a, sizeof(a));
    write(fd, &a, sizeof(a));

    releaseLock(accfd, F_UNLCK, SEEK_SET, (accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid()); 
    close(accfd);
    return;
}

// For Login Services
void loginServicesHandler(int fd, int id, int accNo){
    while(1){
        char choice;
        read(fd, &choice, 1);
        if(choice=='N'){ // Logout
            decreaseActivity(id);
            return;
        }
        else if(choice=='4'){ // Account Details
            accountDetailsHandler(fd, id, accNo);
            continue;
        }
        else if(choice=='3'){
            passwordChangeHandler(fd, id);
            continue;
        }
        else if(choice == '2'){ // Balance Enquiry
            balanceEnqHandler(fd, accNo);
            continue;
        }
        else if(choice == '1'){ // Withdraw Money
            withdrawHandler(fd, id, accNo);
            continue;
        }
        else if(choice == '0'){ // Deposit Money
            depositHandler(fd, id, accNo);
            continue;
        }
    }

}

// To handle login
void loginHandler(int fd){
    struct customer c, actual;
    read(fd, &c, sizeof(c));
    int custfd = open("./files/customers.dat", O_RDONLY);
    acquireLock(custfd, F_RDLCK, SEEK_SET, 0, 0, getpid());

    lseek(custfd, (c.id-1)*sizeof(struct customer), SEEK_SET);
    int s = read(custfd, &actual, sizeof(actual));

    releaseLock(custfd, F_UNLCK, SEEK_SET, 0, 0, getpid()); 
    close(custfd);

    char status;
    if(s==0){
        status = '!';
        write(fd, &status, 1);
        return;
    }
    else{
        if(strcmp(actual.password, c.password)==0){
            struct account acc;
            int accfd = open("./files/accounts.dat", O_RDONLY);
            acquireLock(accfd, F_RDLCK, SEEK_SET, 0, 0, getpid()); 
            lseek(accfd, (actual.accNo-BASE-1)*sizeof(struct account), SEEK_SET);
            int p = read(accfd, &acc, sizeof(acc));
            releaseLock(accfd, F_UNLCK, SEEK_SET, 0, 0, getpid()); 
            close(accfd);
            if(acc.active == 0){
                status = 'I';
                write(fd, &status, 1);
                return;
            }

            struct activity act;
            int actfd = open("./files/activities.dat",O_RDWR);
            acquireLock(actfd, F_WRLCK, SEEK_SET, (c.id-1)*sizeof(struct activity), sizeof(struct activity), getpid()); 
            lseek(actfd, (c.id-1)*sizeof(struct activity), SEEK_SET);
            read(actfd, &act, sizeof(act));
            if(act.login==1){
                status='$';
                write(fd, &status, 1);
                releaseLock(actfd, F_UNLCK, SEEK_SET, (c.id-1)*sizeof(struct activity), sizeof(struct activity), getpid()); 
                close(actfd);
                return;
            }
            else if(act.login==0){
                act.login=1;
                lseek(actfd, (c.id-1)*sizeof(struct activity), SEEK_SET);
                write(actfd, &act, sizeof(act));
                releaseLock(actfd, F_UNLCK, SEEK_SET, (c.id-1)*sizeof(struct activity), sizeof(struct activity), getpid()); 
                close(actfd);
                status = 'C';
                write(fd, &status, 1);
                loginServicesHandler(fd, actual.id, acc.accNo);
                return;
            } 
        }
        else{
            status = 'W';
            write(fd, &status, 1);
            return;
        }
    }

}

// To reset admin activity
void resetAdminStatus(void){
    char s='0';
    int adminfd = open("./files/adminStatus",O_RDWR);
    acquireLock(adminfd, F_WRLCK, SEEK_SET, 0, 0, getpid()); 
    lseek(adminfd, 0, SEEK_SET);
    write(adminfd, &s, 1);
    releaseLock(adminfd, F_UNLCK, SEEK_SET, 0, 0, getpid()); 
    close(adminfd);
    return;
}

// To delete account by admin
void deleteAccountHandler(int fd){
    char status;
    struct account a,actual;
    read(fd, &a, sizeof(a));
    if(a.accNo <= BASE){
        status = 'W';
        write(fd, &status, 1);
        return;
    }
    int accfd = open("./files/accounts.dat",O_RDWR);
    acquireLock(accfd, F_WRLCK, SEEK_SET, (a.accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid()); 
    lseek(accfd, (a.accNo-BASE-1)*sizeof(struct account), SEEK_SET);
    int s = read(accfd, &actual, sizeof(actual));
    if(s==0){
        releaseLock(accfd, F_UNLCK, SEEK_SET, (a.accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid()); 
        close(accfd);
        status = 'W';
        write(fd, &status, 1);
        return;
    }
    else{
        if(actual.active==-1){
            releaseLock(accfd, F_UNLCK, SEEK_SET, (a.accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid()); 
            close(accfd);
            status = 'W';
            write(fd, &status, 1);
            return;
        }
        if(actual.active==0){
            releaseLock(accfd, F_UNLCK, SEEK_SET, (a.accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid()); 
            close(accfd);
            status = 'I';
            write(fd, &status, 1);
            return;
        }
        int id1 = actual.id[0];
        int id2 = actual.id[1];
        struct activity act;
        int actfd = open("./files/activities.dat",O_RDONLY);
        struct flock lock3;

        lock3.l_type = F_RDLCK;
        lock3.l_whence = SEEK_SET;
        lock3.l_start=(id1-1)*sizeof(struct activity);
        if(id2!=-1) lock3.l_len=2*sizeof(struct activity);
        else lock3.l_len=sizeof(struct activity);
        lock3.l_pid = getpid();

        fcntl(actfd, F_SETLKW, &lock3);
        lseek(actfd, (id1-1)*sizeof(struct activity), SEEK_SET);
        read(actfd, &act, sizeof(act));
        if(act.login==1){
            lock3.l_type = F_UNLCK;
            fcntl(actfd, F_SETLK, &lock3);
            close(actfd);
            releaseLock(accfd, F_UNLCK, SEEK_SET, (a.accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid()); 
            close(accfd);
            status = 'A';
            write(fd, &status, 1);
            return;
        }
        if(id2!=-1){
            read(actfd, &act, sizeof(act));
                if(act.login==1){
                lock3.l_type = F_UNLCK;
                fcntl(actfd, F_SETLK, &lock3);
                close(actfd);
                releaseLock(accfd, F_UNLCK, SEEK_SET, (a.accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid()); 
                close(accfd);
                status = 'A';
                write(fd, &status, 1);
                return;
            }
        }
        actual.active=0;
        lseek(accfd, (a.accNo-BASE-1)*sizeof(struct account), SEEK_SET);
        write(accfd, &actual, sizeof(actual));
        lock3.l_type = F_UNLCK;
        fcntl(actfd, F_SETLK, &lock3);
        close(actfd);
        releaseLock(accfd, F_UNLCK, SEEK_SET, (a.accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid()); 
        close(accfd);
        status = 'C';
        write(fd, &status, 1);
        return;
    }
}

// For searching the account by admin
void searchAccountHandler(int fd){
    struct account a;
    read(fd, &a, sizeof(a));
    char status;
    if(a.accNo <= BASE){
        status = 'W';
        write(fd, &status, 1);
        return;
    }
    struct account acc;
    int accfd = open("./files/accounts.dat", O_RDONLY);
    acquireLock(accfd, F_RDLCK, SEEK_SET, 0, 0, getpid()); 
    lseek(accfd, (a.accNo-BASE-1)*sizeof(struct account), SEEK_SET);
    int p = read(accfd, &acc, sizeof(acc));
    releaseLock(accfd, F_UNLCK, SEEK_SET, 0, 0, getpid()); 
    close(accfd);
    if(p==0 || acc.active == -1){
        status = 'W';
        write(fd, &status, 1);
        return;
    }
    if(acc.active==0){
        status = 'I';
        write(fd, &status, 1);
        return;
    }
    else if(acc.active==1){
        status = 'C';
        write(fd, &status, 1);
        accountDetailsHandler(fd, acc.id[0], acc.accNo);
        return;
    }



}

// For modification of customer details
void modifyAccountHandler(int fd){
    char status;
    struct account a, actual;
    read(fd, &a, sizeof(a));
    if(a.accNo <= BASE){
        status = 'W';
        write(fd, &status, 1);
        return;
    }
    int accfd = open("./files/accounts.dat",O_RDONLY);
    acquireLock(accfd, F_WRLCK, SEEK_SET, (a.accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid()); 
    lseek(accfd, (a.accNo-BASE-1)*sizeof(struct account), SEEK_SET);
    int s = read(accfd, &actual, sizeof(actual));
    if(s==0){
        releaseLock(accfd, F_UNLCK, SEEK_SET, (a.accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid()); 
        close(accfd);
        status = 'W';
        write(fd, &status, 1);
        return;
    }
    else{
        if(actual.active==-1){
            releaseLock(accfd, F_UNLCK, SEEK_SET, (a.accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid()); 
            close(accfd);
            status = 'W';
            write(fd, &status, 1);
            return;
        }
        if(actual.active==0){
            releaseLock(accfd, F_UNLCK, SEEK_SET, (a.accNo-BASE-1)*sizeof(struct account), sizeof(struct account), getpid()); 
            close(accfd);
            status = 'I';
            write(fd, &status, 1);
            return;
        }
        if(actual.accType==0){ // Normal Account
            status = 'C';
            write(fd, &status, 1);
            struct customer c;
            int custfd = open("./files/customers.dat",O_RDWR);
            acquireLock(custfd, F_RDLCK, SEEK_SET, (actual.id[0]-1)*sizeof(struct customer), sizeof(struct customer), getpid());
            lseek(custfd, (actual.id[0]-1)*sizeof(struct customer), SEEK_SET);
            read(custfd, &c, sizeof(c));
            write(fd, &c, sizeof(c));
            releaseLock(custfd, F_UNLCK, SEEK_SET, (actual.id[0]-1)*sizeof(struct customer), sizeof(struct customer), getpid());

            read(fd, &c, sizeof(c));
            acquireLock(custfd, F_WRLCK, SEEK_SET, (actual.id[0]-1)*sizeof(struct customer), sizeof(struct customer), getpid());
            lseek(custfd, (actual.id[0]-1)*sizeof(struct customer), SEEK_SET);
            write(custfd, &c, sizeof(c));
            releaseLock(custfd, F_UNLCK, SEEK_SET, (actual.id[0]-1)*sizeof(struct customer), sizeof(struct customer), getpid());
            close(custfd);
            status ='T';
            write(fd, &status, 1);
            return;
        }
        else if(actual.accType==1){ // Joint Account
            status = 'J';
            write(fd, &status, 1);
            struct customer c1, c2;
            int custfd = open("./files/customers.dat",O_RDWR);
            acquireLock(custfd, F_RDLCK, SEEK_SET, (actual.id[0]-1)*sizeof(struct customer), 2*sizeof(struct customer), getpid());
            lseek(custfd, (actual.id[0]-1)*sizeof(struct customer), SEEK_SET);
            read(custfd, &c1, sizeof(c1));
            read(custfd, &c2, sizeof(c2));
            write(fd, &c1, sizeof(c1));
            write(fd, &c2, sizeof(c2));
            releaseLock(custfd, F_UNLCK, SEEK_SET, (actual.id[0]-1)*sizeof(struct customer), 2*sizeof(struct customer), getpid());

            read(fd, &c1, sizeof(c1));
            read(fd, &c2, sizeof(c2));
            acquireLock(custfd, F_WRLCK, SEEK_SET, (actual.id[0]-1)*sizeof(struct customer), 2*sizeof(struct customer), getpid());
            lseek(custfd, (actual.id[0]-1)*sizeof(struct customer), SEEK_SET);
            write(custfd, &c1, sizeof(c1));
            write(custfd, &c2, sizeof(c2));
            releaseLock(custfd, F_UNLCK, SEEK_SET, (actual.id[0]-1)*sizeof(struct customer), sizeof(struct customer), getpid());
            close(custfd);
            status ='T';
            write(fd, &status, 1);
            return;
        }

    }
}

// Admin Services handler
void adminLoginServicesHandler(int fd){
    while(1){
        char choice;
        read(fd, &choice, 1);
        if(choice=='N'){ // Logout
            resetAdminStatus();
            return;
        }
        else if(choice=='0'){ // Add Account
            while(1){
                char accType;
                read(fd, &accType, sizeof(accType));
                if(accType=='0'){ // Normal Account Creation
                    normalAccountCreationHandler(fd);
                    break;
                }
                else if(accType=='1'){ // Joint Account Creation
                    jointAccountCreationHandler(fd);
                    break;
                }
                else if(accType == 'P'){ //Parent Menu
                    break;
                }
                else{
                    continue;
                }
            }
        }
        else if(choice=='1'){ //Delete Account
            deleteAccountHandler(fd);
            continue;
        }
        else if(choice == '2'){ // Modify Account Details
            modifyAccountHandler(fd);
            continue;
        }
        else if(choice == '3'){ // Search Account Details
            searchAccountHandler(fd);
            continue;
        }
    }
}

// Admin login handler
void adminLoginHandler(int fd){
    struct adminCreds admin;
    read(fd, &admin, sizeof(admin));
    char status;

    if(admin.id == ADMIN_LOGINID && strcmp(admin.password,ADMIN_PASSWORD)==0){
        char s;
        int adminfd = open("./files/adminStatus",O_RDWR);
        acquireLock(adminfd, F_WRLCK, SEEK_SET, 0, 0, getpid());      
        lseek(adminfd, 0, SEEK_SET);
        read(adminfd, &s, 1);
        if(s=='1'){
            releaseLock(adminfd, F_UNLCK, SEEK_SET, 0, 0, getpid());      
            close(adminfd);
            status = 'I';
            write(fd, &status, 1);
            return;
        }
        if(s=='0'){
            s='1';
            lseek(adminfd, 0, SEEK_SET);
            write(adminfd, &s, 1);
            releaseLock(adminfd, F_UNLCK, SEEK_SET, 0, 0, getpid());      
            close(adminfd);
            status = 'C';
            write(fd, &status, 1);
            adminLoginServicesHandler(fd);
            return;
        }

    }
    else{
        status = 'W';
        write(fd, &status, 1);
        return;
    }
    
}

// Client handler
void clientHandler(int fd){

    while(1){
        char choice;
        rBytes = read(fd, &choice, sizeof(choice));
        if(choice=='N'){ //Exit
            close(fd);
            return;
        }
        else if(choice=='2'){ // Account Creation
            while(1){
                char accType;
                read(fd, &accType, sizeof(accType));
                if(accType=='0'){ // Normal Account Creation
                    normalAccountCreationHandler(fd);
                    break;
                }
                else if(accType=='1'){ // Joint Account Creation
                    jointAccountCreationHandler(fd);
                    break;
                }
                else if(accType == 'P'){ //Parent Menu
                    break;
                }
                else{
                    continue;
                }
            }
        }
        else if(choice=='1'){ // Customer Login
            loginHandler(fd);
            continue;
        }
        else if(choice=='0'){
            adminLoginHandler(fd);
            continue;
        }
        
    }

}

int main(){

    int socketfd, socketBS, socketLS, connectionfd;
    struct sockaddr_in serverAddr, clientAddr;

    socketfd = socket(AF_INET, SOCK_STREAM, 0);

    if (socketfd == -1){
        perror("Error while creating socket! ");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);

    socketBS = bind(socketfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    if (socketBS == -1){
        perror("Error while binding to server socket! ");
        exit(EXIT_FAILURE);
    }

    socketLS = listen(socketfd, 10);
    if (socketLS == -1){
        perror("Error while trying to listen for connections! ");
        exit(EXIT_FAILURE);
    }

    while(1){

        int clientAddrSize = sizeof(clientAddr);
        connectionfd = accept(socketfd, (struct sockaddr *) &clientAddr, &clientAddrSize);
        if(connectionfd == -1){
            perror("Error while connecting to client! ");
            close(socketfd);
        }

        pid_t childPid = fork();
        if(childPid == -1){
            perror("Error in creating child process: ");
            exit(EXIT_FAILURE);
        }

        if(childPid == 0){
            clientHandler(connectionfd);
            close(connectionfd);
            exit(EXIT_SUCCESS);
        }

    }

    close(socketfd);
    return 0;
}
