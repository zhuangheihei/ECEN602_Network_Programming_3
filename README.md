### ECEN 602 Network Programming Assignment #3
* This is the workspace for ECEN 602 Network Programming Assignment #3
* In this assignment, we need to implement a Trivial File Transfer Protocol(TFTP) server using User Datagram Protocol(UDP)
    * Hong Zhuang implemented main function of server.cpp, including timed out and i/o multiplexing.
    * Ying Wang implemented several helper functions, such as read_packet(), to_tftp(), conducted code test, and composed project report.

* The TFTP message types implemented: RRQ, DATA, ACK, ERROR.
* Type command below to run server:
    * make
    * ./server server_ip server_port
* Type command below to clean object code:
    * make clean

* Server first bind to user-input ip and port to listen to client.
* If RRQ received, then server will create child process using fork() to create different socket with ephemeral port number to transmit file with client.
* If there is multiple RRQ requests, then server use select() to do i/o multiplexing like what we did in assignment 2.
* Server is able to transmit both binary file and text, use netaskii and octet mode respectively.
* After successful transmission, the server will clean up and close child processes.

* Errata:
    * There will be error message if there is any error in socket(), bind(), recvfrom() and sendto().
    * If file can not be read, the server will have error alert and send a message to client.
    * If transmission was interrupted, the server will wait for client for 1 second, and try to resend packet.
    * If no ACK from server after 10 resending attempts, then server will think client timed out and terminate this tranmission.
