# Database Server and Client

This project involves the implementation of a database server (dbserver) and client (dbclient) applications using sockets. Both the server and client programs are written in C, and a Makefile is provided for easy compilation.

## Database Server (dbserver)

The dbserver is a multi-threaded server that handles requests from clients. The server listens for connection requests and creates a new thread (handler) to handle each client's requests. The server never terminates and waits for the next request after serving each client.

### Message Format and Records

The messages exchanged between the server and client, as well as the records stored in the database, are defined in the `msg.h` file. Make sure to refer to it for understanding the message fields and type definitions.

### Handler Thread

Each handler thread processes requests from a dbclient and sends an appropriate response back to the client. There are two types of request messages:

1. PUT: This message contains a data record that needs to be stored in the database. The handler appends the record to the database file. Only the client data record should be stored, not the entire message. If the write is successful, the handler sends a SUCCESS message to the client; otherwise, it sends a FAIL message.

2. GET: This request message contains the ID of the record that needs to be fetched. The handler searches the database for a matching record (a record with an ID field that matches the ID in the GET message). If a matching record is found, the handler sends a SUCCESS message with the matching record to the client; otherwise, it sends a FAIL message.

## Database Client (dbclient)

The dbclient is an interactive client application that connects to the server. It prompts the user to choose from three operations: put, get, or quit.

### PUT Operation

If the user chooses to put, the client prompts for the name and ID, sends a PUT message to the server (filling the name and ID fields of the record), and waits for the response. If the response is SUCCESS, it prints a "put success" message; otherwise, it prints a "put failed" message.

### GET Operation

If the user chooses to get, the client prompts for the ID, sends a GET message to the server (filling only the ID field of the record), and waits for the response. If the response is SUCCESS, it prints the name and ID of the record; otherwise, it prints a "get failed" message.

### QUIT Operation

If the user chooses to quit, the client closes the socket, performs cleanup, and terminates the program.

### Compilation

To compile the server and client programs, use the following commands:

        gcc dbserver.c -o dbserver -Wall -Werror -std=gnu99 -pthread
        gcc dbclient.c -o dbclient -Wall -Werror -std=gnu99 -pthread
        
### Usage

To run the server, use the following command:

        ./dbserver <port>
        

Replace `<port>` with a randomly chosen port number (e.g., 3648). Ensure that the port number is not already in use. The server will listen on this port for incoming connections.

To run the client, use the following command:

        ./dbclient <server_dns_name> <port>
        
Replace `<server_dns_name>` with the DNS name or IP address of the server, and `<port>` with the same port number used to run the server.

## Makefile

A Makefile is provided for easy compilation of the server and client programs. The Makefile includes the following targets:

- `all`: Compiles both the server and client programs.
- `clean`: Removes object files and executables.
- `dbclient`: Compiles only the client program.
- `dbserver`: Compiles only the server program.

## Disclaimer

Please note that the source code provided in this repository is available for viewing purposes only. It was originally developed as a project for a class, and while you are welcome to examine the code and learn from it, it should not be reused or submitted for class assignments or any other academic or commercial purposes. Using this code for such purposes would violate academic integrity policies and may have consequences.

## Viewing the Source Code

To view the source code, navigate to the appropriate directory in this repository. The code is provided in files with the ".c" extension. You can open these files using any text editor or an Integrated Development Environment (IDE) of your choice. Feel free to explore the code, understand the implementation, and learn from it.

However, remember that it is important to develop your own solutions for class assignments and projects. Reusing or submitting this code as your own would be a violation of academic integrity and can have serious consequences. Use this code responsibly and solely for educational purposes.

## License

**Note:** The code in this repository is provided for the sole purpose of demonstration to potential employers. The code is not licensed for general use or distribution.

By accessing or using this code, you agree to the following terms:

1. You may only view and run the code for evaluation purposes related to employment opportunities.
2. You may not reproduce, distribute, modify, or use the code for any other purpose without explicit written permission.
3. You may not submit this code or any derivative of it for academic or educational purposes, including as part of coursework or assignments.


