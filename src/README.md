
Build dependencies:
___________________
1. OpenSSL
```
sudo apt-get install libssl-dev
```

Build instructions:
-------------------

1.	Cleaning the existing intermediate files
```
make clean
```

2.	Building the new executable
```
make all
```

3.	Building and running tests
```
make tests
```

Usage:
------
The binary is executed as follows.
```	
./bittorrent <torrent_file>
```
torrent_file is the path to the torrent_file.



Output:
-------

IF the .torrent file is valid torrent file
   
   Parsed data is printed to stdout
   
ELSE appropriate message is displayed.
