# DNS Responder

## What is this?
This C program was created to reply DNS lookup requests on UDP port 53. It opens a socket on the transport layer (layer 4) and listens for incoming network packets. When one is captured, it determinates whether a valid or an invalid DNS query was received. Then as a response, the program sends a valid packet containing a choosen IP address.
 
Without additional implementation - like a custom web server where this program points to and redirects HTTPS to HTTP - the browser can only handle HTTP domain queries which are resolved as a HTTP address.

## Supported systems
The program should be compiled on the designated operating system. It was tested and compiled with Microsoft Visual C++ 2010 Express on Windows 7 and GCC on Linux Mint. It has also worked with Cygwin on Windows.

### Microsoft Visual C++ on Windows
Create a new project and insert the source code as a C file and then compile.

### Cygwin on Windows
In Windows command line:
```
> gcc dns_responder.c -o dns_responder.exe -D __linux__
```

### GCC Linux
Compiling in Linux terminal:
```
$ gcc dns_responder.c -o dns_responder
```

Run:
```
$ sudo ./dns_responder
```

If there is an error like this,
```
Bind error: Address already in use
```
it means that something else is already listening on port 53.

To find out what is listening on port 53:
```
$ sudo netstat -npl | grep LISTEN | grep 53
```

Usually the process dnsmasq will show up. To disable it, need to edit '/etc/NetworkManager/NetworkManager.conf' and comment out 'dns=dnsmasq' line in it.
```
$ sudo nano /etc/NetworkManager/NetworkManager.conf 
```
```
#dns=dnsmasq
```

Now the network services need to be restarted and after that kill the dnsmasq process.
```
$ sudo service network-manager restart
$ sudo service networking restart
$ sudo killall dnsmasq
```

Check if the process stopped listening and run dns_responder again. 

Exit the program with CTRL + C