#Assignment
Write a simple TCP client and server.   
Both the client and server will run on the same machine.  
The client sends a succinct greeting to the server and server responds.    
The server will be invoked with the command   
\>\> server serverport  
and the client with the command  
\>\> client serverIP serverport    
For the server, choose a port number between greater than 4000.   
For the client determine the IP address of the server (which is also the IP address of the client!)   
and pass the argument serverIP as a character string in the dotted IP notation. (Due Feb 11)     
#Usage
##Installation
This assumes installation of cmake and make

1. cd to folder CSE4153, which contains CMakeLists.txt

2. cmake .

3. make

This should build both a client and server executable
##Server
server [-d] serverport

starts the server on the given port

-d uses the default port, 4349, 
and will override a port given on the commandline
##Client
client -adl serverIP severport

starts client with given server IP and port

-a uses the IP of my amazon EC2 instance, which has the server running on it

-d uses the default port, 4349

-l uses the IP for localhost, 127.0.0.1

these will also override the corresponding values given on the commandline 