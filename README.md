# Online Judge

- Implemented FTP on top of TCP sockets to implement an online judge that takes C/C++ files from multiple clients    and evaluates them based on some test cases.
- Appropriate feedback is sent to the client - Compile Error / Runtime Error / TLE / Wrong Answer / Accepted.
- Implemented FTP Commands such as RETR, STOR, LIST, QUIT and DELE.

Running server.cpp:
./a.out port_number

Running client.cpp:
./a.out "localhost" port_number
