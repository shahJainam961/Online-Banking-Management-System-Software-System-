#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../client-serverStructs/ourStruct.h"
#include "../serverConstants/admin-creds.h"
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

void normalAccountCreationHandler(int fd){
    struct customer c;
    read(fd, &c, sizeof(c));
    int userIdfd = open("./files/userIds",O_RDWR);
    struct flock lock;

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start=0;
    lock.l_len=0;
    lock.l_pid = getpid();

    fcntl(userIdfd, F_SETLKW, &lock);

    c.id = getId(userIdfd, 0);
    c.accNo = BASE + c.id;

    int custfd = open("./files/customers.dat",O_WRONLY);
    struct flock lock1;

    lock1.l_type = F_WRLCK;
    lock1.l_whence = SEEK_SET;
    lock1.l_start=0;
    lock1.l_len=0;
    lock1.l_pid = getpid();

    fcntl(custfd, F_SETLKW, &lock1);
    lseek(custfd, 0, SEEK_END);
    write(custfd, &c, sizeof(c));
    lock1.l_type = F_UNLCK;
    fcntl(custfd, F_SETLK, &lock1);
    close(custfd);

    struct account a;
    a.accNo = c.accNo;                  
    a.accType = 0;                
    a.id[0] = c.id;
    a.id[1] = -1;
    a.currBalance = BASE;    
    a.active = 1;

    int accfd = open("./files/accounts.dat",O_WRONLY);
    struct flock lock2;

    lock2.l_type = F_WRLCK;
    lock2.l_whence = SEEK_SET;
    lock2.l_start=0;
    lock2.l_len=0;
    lock2.l_pid = getpid();

    fcntl(accfd, F_SETLKW, &lock2);
    lseek(accfd, 0, SEEK_END);
    write(accfd, &a, sizeof(a));
    lock2.l_type = F_UNLCK;
    fcntl(accfd, F_SETLK, &lock2);
    close(accfd);
    
    write(fd, &c, sizeof(c));

    struct activity act;
    act.id = c.id;
    act.login = 0;
    int actfd = open("./files/activities.dat",O_WRONLY);
    struct flock lock3;

    lock3.l_type = F_WRLCK;
    lock3.l_whence = SEEK_SET;
    lock3.l_start=0;
    lock3.l_len=0;
    lock3.l_pid = getpid();

    fcntl(actfd, F_SETLKW, &lock3);
    lseek(actfd, 0, SEEK_END);
    write(actfd, &act, sizeof(act));
    lock3.l_type = F_UNLCK;
    fcntl(actfd, F_SETLK, &lock3);
    close(actfd);


    lock.l_type = F_UNLCK;
    fcntl(userIdfd, F_SETLK, &lock);
    close(userIdfd);
}

void jointAccountCreationHandler(int fd){
    struct customer c1, c2;
    read(fd, &c1, sizeof(c1));
    read(fd, &c2, sizeof(c2));
    int userIdfd = open("./files/userIds",O_RDWR);
    struct flock lock;

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start=0;
    lock.l_len=0;
    lock.l_pid = getpid();

    fcntl(userIdfd, F_SETLKW, &lock);

    c1.id = getId(userIdfd, 1) - 1;
    c1.accNo = BASE + c1.id;
    c2.accNo = c1.accNo;
    c2.id = c1.id + 1;

    int custfd = open("./files/customers.dat",O_WRONLY);
    struct flock lock1;

    lock1.l_type = F_WRLCK;
    lock1.l_whence = SEEK_SET;
    lock1.l_start=0;
    lock1.l_len=0;
    lock1.l_pid = getpid();

    fcntl(custfd, F_SETLKW, &lock1);
    lseek(custfd, 0, SEEK_END);
    write(custfd, &c1, sizeof(c1));
    write(custfd, &c2, sizeof(c2));
    lock1.l_type = F_UNLCK;
    fcntl(custfd, F_SETLK, &lock1);
    close(custfd);

    struct account a;
    a.accNo = c1.accNo;                  
    a.accType = 1;                
    a.id[0] = c1.id;
    a.id[1] = c2.id;
    a.currBalance = BASE;    
    a.active = 1;

    int accfd = open("./files/accounts.dat",O_WRONLY);
    struct flock lock2;

    lock2.l_type = F_WRLCK;
    lock2.l_whence = SEEK_SET;
    lock2.l_start=0;
    lock2.l_len=0;
    lock2.l_pid = getpid();

    fcntl(accfd, F_SETLKW, &lock2);
    lseek(accfd, 0, SEEK_END);
    write(accfd, &a, sizeof(a));
    //dummy account which will be of no use
    struct account tmp;
    tmp.id[0] = -1;
    tmp.id[1] = -1;
    tmp.currBalance = -1;
    tmp.accType = -1;
    tmp.accNo = a.accNo+1;
    tmp.active = -1;
    write(accfd, &tmp, sizeof(tmp));
    //
    lock2.l_type = F_UNLCK;
    fcntl(accfd, F_SETLK, &lock2);
    close(accfd);

    write(fd, &c1, sizeof(c1));
    write(fd, &c2, sizeof(c2));

    struct activity act;
    act.id = c1.id;
    act.login = 0;
    int actfd = open("./files/activities.dat",O_WRONLY);
    struct flock lock3;

    lock3.l_type = F_WRLCK;
    lock3.l_whence = SEEK_SET;
    lock3.l_start=0;
    lock3.l_len=0;
    lock3.l_pid = getpid();

    fcntl(actfd, F_SETLKW, &lock3);
    lseek(actfd, 0, SEEK_END);
    write(actfd, &act, sizeof(act));
    act.id = c2.id;
    write(actfd, &act, sizeof(act));
    lock3.l_type = F_UNLCK;
    fcntl(actfd, F_SETLK, &lock3);
    close(actfd);

    lock.l_type = F_UNLCK;
    fcntl(userIdfd, F_SETLK, &lock);
    close(userIdfd);
}

void accountDetailsHandler(int fd, int id, int accNo){
    struct account a;
    int accfd = open("./files/accounts.dat",O_RDONLY);
    struct flock lock2;

    lock2.l_type = F_RDLCK;
    lock2.l_whence = SEEK_SET;
    lock2.l_start=(accNo-BASE-1)*sizeof(struct account);
    lock2.l_len=sizeof(struct account);
    lock2.l_pid = getpid();

    fcntl(accfd, F_SETLKW, &lock2);
    lseek(accfd, (accNo-BASE-1)*sizeof(struct account), SEEK_SET);
    read(accfd, &a, sizeof(a));
    lock2.l_type = F_UNLCK;
    fcntl(accfd, F_SETLK, &lock2);
    close(accfd);
    write(fd, &a, sizeof(a));
    if(a.accType==0){ // Normal Account
        struct customer c;
        int custfd = open("./files/customers.dat",O_RDONLY);
        struct flock lock1;

        lock1.l_type = F_RDLCK;
        lock1.l_whence = SEEK_SET;
        lock1.l_start=(id-1)*sizeof(struct customer);
        lock1.l_len=sizeof(struct customer);
        lock1.l_pid = getpid();

        fcntl(custfd, F_SETLKW, &lock1);
        lseek(custfd, (id-1)*sizeof(struct customer), SEEK_SET);
        read(custfd, &c, sizeof(c));
        write(fd, &c, sizeof(c));
        lock1.l_type = F_UNLCK;
        fcntl(custfd, F_SETLK, &lock1);
        close(custfd);
    }
    else if(a.accType==1){ // Joint Account
        struct customer c1,c2;
        int custfd = open("./files/customers.dat",O_RDONLY);
        struct flock lock1;

        lock1.l_type = F_RDLCK;
        lock1.l_whence = SEEK_SET;
        lock1.l_start=(a.id[0]-1)*sizeof(struct customer);
        lock1.l_len=2*sizeof(struct customer);
        lock1.l_pid = getpid();

        fcntl(custfd, F_SETLKW, &lock1);
        lseek(custfd, (a.id[0]-1)*sizeof(struct customer), SEEK_SET);
        read(custfd, &c1, sizeof(c1));
        read(custfd, &c2, sizeof(c2));
        write(fd, &c1, sizeof(c1));
        write(fd, &c2, sizeof(c2));
        lock1.l_type = F_UNLCK;
        fcntl(custfd, F_SETLK, &lock1);
        close(custfd);
    }
}

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
    lseek(custfd, (id-1)*sizeof(struct customer), SEEK_SET);
    read(custfd, &actual, sizeof(actual));
    lock1.l_type = F_UNLCK;
    fcntl(custfd, F_SETLK, &lock1);
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
        struct flock lock1;

        lock1.l_type = F_WRLCK;
        lock1.l_whence = SEEK_SET;
        lock1.l_start=(id-1)*sizeof(struct customer);
        lock1.l_len=sizeof(struct customer);
        lock1.l_pid = getpid();

        fcntl(custfd, F_SETLKW, &lock1);
        lseek(custfd, (id-1)*sizeof(struct customer), SEEK_SET);
        write(custfd, &actual, sizeof(actual));
        
        status = 'Y';
        write(fd, &status, 1);

        lock1.l_type = F_UNLCK;
        fcntl(custfd, F_SETLK, &lock1);
        close(custfd);
        return;
    }
    else{
        status ='W';
        write(fd, &status, 1);
        return;
    }

}

void decreaseActivity(int id){
    struct activity act;
    int actfd = open("./files/activities.dat",O_RDWR);
    struct flock lock3;

    lock3.l_type = F_WRLCK;
    lock3.l_whence = SEEK_SET;
    lock3.l_start=(id-1)*sizeof(struct activity);
    lock3.l_len=sizeof(struct activity);
    lock3.l_pid = getpid();

    fcntl(actfd, F_SETLKW, &lock3);
    lseek(actfd, (id-1)*sizeof(struct activity), SEEK_SET);
    read(actfd, &act, sizeof(act));
    act.login=0;
    lseek(actfd, (id-1)*sizeof(struct activity), SEEK_SET);
    write(actfd, &act, sizeof(act));
    lock3.l_type = F_UNLCK;
    fcntl(actfd, F_SETLK, &lock3);
    close(actfd);
}

void withdrawHandler(int fd, int id, int accNo){
    struct transaction t1;
    read(fd, &t1, sizeof(t1));
    
    struct account a;
    int accfd = open("./files/accounts.dat",O_RDWR);
    struct flock lock2;

    lock2.l_type = F_WRLCK;
    lock2.l_whence = SEEK_SET;
    lock2.l_start=(accNo-BASE-1)*sizeof(struct account);
    lock2.l_len=sizeof(struct account);
    lock2.l_pid = getpid();

    fcntl(accfd, F_SETLKW, &lock2);
    lseek(accfd, (accNo-BASE-1)*sizeof(struct account), SEEK_SET);
    read(accfd, &a, sizeof(a));
    char status;
    if(a.currBalance<t1.transactMoney){
        status = 'W';
        write(fd, &status, 1);
        lock2.l_type = F_UNLCK;
        fcntl(accfd, F_SETLK, &lock2);
        close(accfd);
        return;
    }
    int transfd = open("./files/transactIds",O_RDWR);
    struct flock lock;

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start=0;
    lock.l_len=0;
    lock.l_pid = getpid();

    fcntl(transfd, F_SETLKW, &lock);

    t1.tid = getId(transfd, 0);
    t1.accNo = accNo;
    t1.beforeBalance = a.currBalance;
    t1.afterBalance = a.currBalance - t1.transactMoney;
    time_t _t;
    time(&_t);
    strcpy(t1.timeOfTransact,ctime(&_t));

    int transferfd = open("./files/transactions.dat",O_WRONLY);
    struct flock lock1;

    lock1.l_type = F_WRLCK;
    lock1.l_whence = SEEK_SET;
    lock1.l_start=0;
    lock1.l_len=0;
    lock1.l_pid = getpid();

    fcntl(transferfd, F_SETLKW, &lock1);
    lseek(transferfd, 0, SEEK_END);
    write(transferfd, &t1, sizeof(t1));
    
    a.currBalance = t1.afterBalance;

    lseek(accfd, (accNo-BASE-1)*sizeof(struct account), SEEK_SET);
    write(accfd, &a, sizeof(a));
    
    status = 'C';
    write(fd, &status, 1);
    write(fd, &a, sizeof(a));
    
    lock1.l_type = F_UNLCK;
    fcntl(transferfd, F_SETLK, &lock1);
    close(transferfd);
    
    lock.l_type = F_UNLCK;
    fcntl(transfd, F_SETLK, &lock);
    close(transfd);

    lock2.l_type = F_UNLCK;
    fcntl(accfd, F_SETLK, &lock2);
    close(accfd);
    
    return;
}

void depositHandler(int fd, int id, int accNo){
    struct transaction t1;
    read(fd, &t1, sizeof(t1));

    struct account a;
    int accfd = open("./files/accounts.dat",O_RDWR);
    struct flock lock2;

    lock2.l_type = F_WRLCK;
    lock2.l_whence = SEEK_SET;
    lock2.l_start=(accNo-BASE-1)*sizeof(struct account);
    lock2.l_len=sizeof(struct account);
    lock2.l_pid = getpid();

    fcntl(accfd, F_SETLKW, &lock2);

    int transfd = open("./files/transactIds",O_RDWR);
    struct flock lock;

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start=0;
    lock.l_len=0;
    lock.l_pid = getpid();

    fcntl(transfd, F_SETLKW, &lock);

    int transferfd = open("./files/transactions.dat",O_WRONLY);
    struct flock lock1;

    lock1.l_type = F_WRLCK;
    lock1.l_whence = SEEK_SET;
    lock1.l_start=0;
    lock1.l_len=0;
    lock1.l_pid = getpid();

    fcntl(transferfd, F_SETLKW, &lock1);

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

    lock1.l_type = F_UNLCK;
    fcntl(transferfd, F_SETLK, &lock1);
    close(transferfd);
    
    lock.l_type = F_UNLCK;
    fcntl(transfd, F_SETLK, &lock);
    close(transfd);

    lock2.l_type = F_UNLCK;
    fcntl(accfd, F_SETLK, &lock2);
    close(accfd);
}

void balanceEnqHandler(int fd, int accNo){
    struct account a;
    int accfd = open("./files/accounts.dat",O_RDONLY);
    struct flock lock2;

    lock2.l_type = F_RDLCK;
    lock2.l_whence = SEEK_SET;
    lock2.l_start=(accNo-BASE-1)*sizeof(struct account);
    lock2.l_len=sizeof(struct account);
    lock2.l_pid = getpid();

    fcntl(accfd, F_SETLKW, &lock2);
    lseek(accfd, (accNo-BASE-1)*sizeof(struct account), SEEK_SET);
    read(accfd, &a, sizeof(a));
    write(fd, &a, sizeof(a));

    lock2.l_type = F_UNLCK;
    fcntl(accfd, F_SETLK, &lock2);
    close(accfd);
    return;
}

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

void loginHandler(int fd){
    struct customer c, actual;
    read(fd, &c, sizeof(c));
    int custfd = open("./files/customers.dat", O_RDONLY);
    struct flock lock1;

    lock1.l_type = F_RDLCK;
    lock1.l_whence = SEEK_SET;
    lock1.l_start=0;
    lock1.l_len=0;
    lock1.l_pid = getpid();

    fcntl(custfd, F_SETLKW, &lock1);

    lseek(custfd, (c.id-1)*sizeof(struct customer), SEEK_SET);
    int s = read(custfd, &actual, sizeof(actual));
    lock1.l_type = F_UNLCK;
    fcntl(custfd, F_SETLK, &lock1);
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
            struct flock lock2;

            lock2.l_type = F_RDLCK;
            lock2.l_whence = SEEK_SET;
            lock2.l_start=0;
            lock2.l_len=0;
            lock2.l_pid = getpid();

            fcntl(accfd, F_SETLKW, &lock2);

            lseek(accfd, (actual.accNo-BASE-1)*sizeof(struct account), SEEK_SET);
            int p = read(accfd, &acc, sizeof(acc));
            lock1.l_type = F_UNLCK;
            fcntl(accfd, F_SETLK, &lock2);
            close(accfd);
            if(acc.active == 0){
                status = 'I';
                write(fd, &status, 1);
                return;
            }

            struct activity act;
            int actfd = open("./files/activities.dat",O_RDWR);
            struct flock lock3;

            lock3.l_type = F_WRLCK;
            lock3.l_whence = SEEK_SET;
            lock3.l_start=(c.id-1)*sizeof(struct activity);
            lock3.l_len=sizeof(struct activity);
            lock3.l_pid = getpid();

            fcntl(actfd, F_SETLKW, &lock3);
            lseek(actfd, (c.id-1)*sizeof(struct activity), SEEK_SET);
            read(actfd, &act, sizeof(act));
            if(act.login==1){
                status='$';
                write(fd, &status, 1);
                lock3.l_type = F_UNLCK;
                fcntl(actfd, F_SETLK, &lock3);
                close(actfd);
                return;
            }
            else if(act.login==0){
                act.login=1;
                lseek(actfd, (c.id-1)*sizeof(struct activity), SEEK_SET);
                write(actfd, &act, sizeof(act));
                lock3.l_type = F_UNLCK;
                fcntl(actfd, F_SETLK, &lock3);
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

void resetAdminStatus(void){
    char s='0';
    int adminfd = open("./files/adminStatus",O_RDWR);
    struct flock lock;

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start=0;
    lock.l_len=0;
    lock.l_pid = getpid();

    fcntl(adminfd, F_SETLKW, &lock);
    lseek(adminfd, 0, SEEK_SET);
    write(adminfd, &s, 1);
    lock.l_type = F_UNLCK;
    fcntl(adminfd, F_SETLK, &lock);
    close(adminfd);
    return;
}

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
    struct flock lock2;

    lock2.l_type = F_WRLCK;
    lock2.l_whence = SEEK_SET;
    lock2.l_start=(a.accNo-BASE-1)*sizeof(struct account);
    lock2.l_len=sizeof(struct account);
    lock2.l_pid = getpid();

    fcntl(accfd, F_SETLKW, &lock2);
    lseek(accfd, (a.accNo-BASE-1)*sizeof(struct account), SEEK_SET);
    int s = read(accfd, &actual, sizeof(actual));
    if(s==0){
        lock2.l_type = F_UNLCK;
        fcntl(accfd, F_SETLK, &lock2);
        close(accfd);
        status = 'W';
        write(fd, &status, 1);
        return;
    }
    else{
        if(actual.active==-1){
            lock2.l_type = F_UNLCK;
            fcntl(accfd, F_SETLK, &lock2);
            close(accfd);
            status = 'W';
            write(fd, &status, 1);
            return;
        }
        if(actual.active==0){
            lock2.l_type = F_UNLCK;
            fcntl(accfd, F_SETLK, &lock2);
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
            lock2.l_type = F_UNLCK;
            fcntl(accfd, F_SETLK, &lock2);
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
                lock2.l_type = F_UNLCK;
                fcntl(accfd, F_SETLK, &lock2);
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
        lock2.l_type = F_UNLCK;
        fcntl(accfd, F_SETLK, &lock2);
        close(accfd);
        status = 'C';
        write(fd, &status, 1);
        return;
    }
}

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
    struct flock lock2;

    lock2.l_type = F_RDLCK;
    lock2.l_whence = SEEK_SET;
    lock2.l_start=0;
    lock2.l_len=0;
    lock2.l_pid = getpid();

    fcntl(accfd, F_SETLKW, &lock2);

    lseek(accfd, (a.accNo-BASE-1)*sizeof(struct account), SEEK_SET);
    int p = read(accfd, &acc, sizeof(acc));
    lock2.l_type = F_UNLCK;
    fcntl(accfd, F_SETLK, &lock2);
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
        // else if(choice == '2'){ // Modify Account Details
        //     balanceEnqHandler(fd, accNo);
        //     continue;
        // }
        else if(choice == '3'){ // Search Account Details
            searchAccountHandler(fd);
            continue;
        }
    }
}

void adminLoginHandler(int fd){
    struct adminCreds admin;
    read(fd, &admin, sizeof(admin));
    char status;

    if(admin.id == ADMIN_LOGINID && strcmp(admin.password,ADMIN_PASSWORD)==0){
        char s;
        int adminfd = open("./files/adminStatus",O_RDWR);
        struct flock lock;

        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start=0;
        lock.l_len=0;
        lock.l_pid = getpid();

        fcntl(adminfd, F_SETLKW, &lock);
        lseek(adminfd, 0, SEEK_SET);
        read(adminfd, &s, 1);
        if(s=='1'){
            lock.l_type = F_UNLCK;
            fcntl(adminfd, F_SETLK, &lock);
            close(adminfd);
            status = 'I';
            write(fd, &status, 1);
            return;
        }
        if(s=='0'){
            s='1';
            lseek(adminfd, 0, SEEK_SET);
            write(adminfd, &s, 1);
            lock.l_type = F_UNLCK;
            fcntl(adminfd, F_SETLK, &lock);
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

void clientHandler(int fd){ // Client Handler

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