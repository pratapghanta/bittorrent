
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


Documentation:
-------
https://lucid.app/lucidchart/23658d17-f79b-4b29-8b20-4802f412a2c2/edit?beaconFlowId=6ED9F0E9CA307AE0&invitationId=inv_f96cff65-eee1-47be-a64a-926fd159d9b5&page=0_0#
