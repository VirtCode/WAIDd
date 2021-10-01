# WAIDd
> WAIDd is an open source, lightweight daemon that keeps track of what you are doing on your Linux PC. 
## About


## Fileformat
WAID saves its recorded data in a custom format. It does not use any common file formats like Json or Xml nor does it use a text file to store the data. WAID uses a raw binary file, to store its data as defined below. 

While it is not easy to read the file, both with a program or as a human, this file format was chosen because it reduces the size of the file significantly and because it requires very less resources writing to or reading from it. Also the file is not intended do be read by, for example, a shell script, because there will be lightweight CLIs for that purpose. 

### Architecture
Generally, the waidfile format consists of multiple packages, that are chained behind each other. A packet begins with an id, which tells the reader which packet is following. Afterwards comes the body of the packet, which length depends on the type of packet defined by the packet id. There is no required order in which the packets should be ordered, other than that the first packet should be a version packet for consistency reasons and that packets depending on other packets should occur after its dependencies (like for example a record packet depending on a type define packet).

(The first row of the table determines the length of the data object in bytes. The second row states what the data stands for.)

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

## Naming
The acronym "WAID" stands for "<u>W</u>hat <u>a</u>m <u>I</u> <u>d</u>oing?". The "d" suffix in "WAIDd" states that the program is a daemon.

## License
This program is free software and is licensed under the MIT license. Take a look at the LICENSE file to read more.