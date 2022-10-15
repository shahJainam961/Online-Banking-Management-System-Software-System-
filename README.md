# Online-Banking-Management-System-Software-System-
CS-513 | Software Systems | Mini-Project | Report
Definition – Design and Development of Online Banking Management System
Aim:
To develop a Online banking Management System for banking functionalities, should be user-friendly. Project involves
functionalities mentioned in the project description. (Steps to execute the project at the end).
Concept Used :
System calls, Socket Programming, File Locking, File Management.
Files Description:
You can find different files in the “files” folder used as storage, which is meant to be accessible by server only.
1) accounts.dat : These file contains details of the account of customer.
2) activities.dat: It contains login activity of the users if they are logged in or not. For admin activity “adminStatus” file is there
which has only 1 character which changes according to the admin activity.
3) customers.dat : It contains customer details.
4) transactions.dat : These file contain all the transaction happened so far.
5) userIds | transactIds : Use for generating user ids and transaction ids respectively.
Features:
• Can handle multiple clients simultaneously.
• Used Proper File Locking to access file.
• Socket-Programming is used for connecting client-server.
• System calls related to socket, file locking, file manipulation,etc are used.
• Structures are stored in the file so it will be easy to access(read/write) into/from the file.
• Two types of User: Customer and Admin.
• Customer can access services like Deposit, Withdraw, Balance Enquiry, Password Change, Show Account details.
• Admin can change customers details, also can view customers details.
Important Functionalities:
• Customer can create new joint account or normal(regular) account.
• Account can be of regular type or joint account, in joint account two customer are owners of the account ,both will
have their own loginId and password, whereas for regular account, one individual is owner of that account.
• Only Valid User are able to access the Services as the system will authenticate the user based on loginId and password.
• A valid user can only be logged in one session/terminal at a time. If we try to login on other terminal it will show Max
Login Limit reached.
• While making a transaction in joint account holder, the other holder will get blocked until the current transaction has
been completed.
• Withdrawing more than current available balance will throw error of Insuffucient balance.
• Password if changed only if the old password is matched.
• A user won’t be able to login if his/her account is deleted by the admin. Also admin cannot delete a logged in account
and it will show an error message, a user is logged in, you cannot delete the account.
Steps to Execute:
• To start the Server in background, run below commands in the terminal:
o gcc server.c -o server.out
o ./server.out &
• To start the client, run below commands in the terminal:
o gcc client.c -o client.out
o ./client.out
• Can run multiple clients by running below command in different terminals:
o ./client.out
