# Cloud Phonebook

This is a simple client-server program demonstrating socket programming in C. The program allows users to perform basic CRUD (Create, Read, Update, Delete) operations on a contact list stored on the server.

## Requirements
- C compiler (e.g. gcc)
- Basic knowledge of socket programming in C

## Usage
1. Clone the repository:
```bash
git clone https://github.com/example/socket-programming.git
```

2. Compile the server and client programs:
```bash
gcc -o server server.c
gcc -o client client.c
```

3. Run the server program:
```bash
./server
```

4. In a separate terminal, run the client program:
```bash
./client
```

5. Follow the prompts to perform CRUD operations on the contact list.

## How it Works
The server program listens for incoming client connections using a TCP socket. When a client connects, the server receives a request from the client, performs the appropriate operation on the contact list (stored in memory), and sends a response back to the client.

The client program prompts the user to select an operation to perform (add, get, update, or delete a contact), and sends the selected operation to the server along with any necessary data (e.g. contact information for adding a new contact). The client then waits for a response from the server and displays the result to the user.
