# WAIDd
WAIDd is a lightweight daemon that constantly records what you are doing. 

## Fileformat



### Architecture
Generally, the waidfile format consists of multiple packages, that are chained behind each other. A packet begins with an id, which tells the reader which packet is following. Afterwards comes the body of the packet, which length depends on the type of packet defined by the packet id.   

(the first row of the table determines the length of the data object in bytes. the second row states what the data stands for)

| 1 | 1 ... x | 1 | 1 ... x |
|:---|:---|:---|:---|
| packet id | packet body of fist packet | packet id | packet body of second packet |

#### Version Packet
> Packet ID: 0x01

The version packet is the fist packet that should occur in a waidfile. It indicates the version of the file that is being read. It contains one byte that represents the version number.

| 1 |
|:---|
| version of the file |

#### Type Define Packet
> Packet ID: 0x02

The type define packet is used to define a program in the file. It assigns a unique number to every window class that has been recorded. The assigned number is later referenced as a program id.

| 2 | 1 ... ∞ |
|:---|:---|
| assigned id of the program | WM_CLASS attribute of the program, null terminated string | 

#### Record Packet
> Packet ID: 0x03
 
The record packet is the most important packet. It holds the data for one snapshot taken by the daemon. When the daemon takes a snapshot, it stores the current program together with the current unix timestamp into a file. The program is stored over its id as defined by a preceding type define packet.

| 8 | 2 |
|:---|:---|
| unix timestamp of the time the record was taken | id of the recorded program, as previously defined by a type define packet | 

#### Length Statement Packet
> Packet ID: 0x04

The length statement packets defines the length of the following record packets. Every packet following a length statement packet says that the provided program has been used for the period of time, stated in the most recent length statement packet.

| 4 |
|:---|
| amount of seconds the following programs were used | 
