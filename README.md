# WAIDd
> WAIDd is an open source, lightweight daemon that keeps track of what you are doing on your Linux desktop. 
## About
This daemon does collect data about what you are doing on your X-Server. It does so by determining your currently focused window every so often. The data of *when* you have focused *which* window can then be used to state which applications you use how much and how often. 

This daemon does only collect the class (```WM_CLASS```) attribute of the currently selected window, so it has no idea what the application is being used for.

It is important to note that this program alone does only contain the WAID **daemon**, so the collected data cannot be visualized with just this program. This is what other programs are able to do, as noted below.

### How it Works
This program determines your activity as follows. It does fetch the window class of the currently selected window in a fixed interval, which is one minute per default. It then assumes that for the past interval only this program has been used. With that assumption, things like total usage can easily be calculated. <br>
This assumption could lead to some wrong values, but the longer this procedure is applied, the more accurate gets the data as a whole.

### Design Choices
The main design choice of this project was lightness and performance. This daemon has been developed with a minimal footprint in mind.

Because of this design choice has this daemon been written in C or does this program not use any external library to parse its config file. It does also not use more accurate methods of determining activity and most importantly does not include any other things like visualization of the data out of the box because of that. That's also why it does also store the data in its own file format as shown further down in this already too long readme.

## Installation
### 1. Installing Dependencies
For this program to fetch data from your X-Server, it uses the header ```X11/Xlib.h```. On most modern Linux systems, this header is not installed by default. So install the package for your distribution that adds this header.

On Debian based systems, the package ```libx11-dev``` adds the header. <br>
On Arch based systems, the package ```libx11``` presumably adds the header.

### 2. Obtaining the Binary
Next you should get your hands on a compiled version of this program. There are two ways to achieve this, compiling it yourself, or downloading a precompiled version (only available for ```amd64``` systems). 

#### Compiling
To compile this library you should first clone the git repository and make sure that you have ```make``` and ```gcc``` installed, since this is needed for the compile process. After that, navigate in to the repository directory and execute a command to build the binary.

To directly install WAIDd for the current user on your system, run:
```shell
make install
```

To directly install WAIDd globally on your system, run the following. Make sure that you are running this command as **root**.
```shell
make install-global
```

#### Download Binary
If you happen to run an ```amd64``` system and don't want to compile the binary manually, you can download a precompiled binary.

To do that, head to the release section of this repository and download the binary, attached to the latest version. After that, rename it to ```waidd``` and move it either to ```~/.local/bin/``` for an install for the current user, or to ```/bin/``` for a systemwide install.

### 3. Launch on Startup
The almost last step to installing this daemon is making sure, that it is always started in the background of your system.

In order to do that, you should add the following command to a file, that is executed **after** your X-Server has started.
```shell
waidd &
```
The file could be for example ```~/.xsession```, which is started right after the X-Server. Another good choice if you are running a custom desktop environment would be to add the command to the file which gets executed for your window manager like for example with bspwm to ```~/.config/bspwm/bspwmrc```. You could also try starting the daemon with your init system.

### 4. Configuration
The last and optional step is to configure the daemon to your needs. It is not necessary to configure the daemon since it already comes with default settings.

To configure WAIDd, you can create the config file in the following location: ```~/.config/waid/daemon.cfg``` <br>
There are following config options you can customize (the default options are entered below).

```ini
# The storage option lets you set where the collected data is saved
# Important to note is that the file has to be an absolute path
storage = /home/{your-username}/.waidfile

# The interval option lets you set how often data is collected
# The provided value is the amount of seconds between two collections
interval = 60
```
Great! You have (finally) installed and configured WAIDd correctly on your system.

## Viewing the collected Data
As mentioned above, this daemon does not include any way to visualize or to look at the collected data efficiently. That's why you could use these tools to do that:
* Coming Soon™

## Naming
The acronym "WAID" stands for "**W**hat **a**m **I** **d**oing?". The "d" suffix in "WAID**d**" states that the program is a daemon.

## Fileformat
WAID saves its recorded data in a custom format. It does not use any common file formats like Json or Xml nor does it use a text file to store the data. WAID uses a raw binary file, to store its data as defined below. 

While it is not easy to read the file, both with a program or as a human, this file format was chosen because it reduces the size of the file significantly and because it requires very less resources writing to or reading from it. Also the file is not intended do be read by, for example, a shell script, because there will be lightweight CLIs for that purpose. 

### Architecture
Generally, the waidfile format consists of multiple packages, that are chained behind each other. A packet begins with an id, which tells the reader which packet is following. Afterwards comes the body of the packet, which length depends on the type of packet defined by the packet id. There is no required order in which the packets should be ordered, other than that the first packet should be a version packet for consistency reasons and that packets depending on other packets should occur after its dependencies (like for example a record packet depending on a type define packet).

(The first row of the table determines the length of the data object in bytes. The second row states what the data stands for.)

| 1 | 1 ... x | 1 | 1 ... x | ... |
|:---|:---|:---|:---|:---|
| packet id | packet body of fist packet | packet id | packet body of second packet | ...|

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
 
The record packet is the most important packet. It holds the data for one snapshot taken by the daemon. When the daemon takes a snapshot, it stores the current program together with the current unix timestamp into the file. The program is stored over its id as defined by a preceding type define packet.

| 8 | 2 |
|:---|:---|
| unix timestamp of the time the record was taken | id of the recorded program, as previously defined by a type define packet | 

#### Length Statement Packet
> Packet ID: 0x04

The length statement packet defines the length of the following record packets. Every packet following a length statement packet says that the provided program has been used for the period of time, stated in the most recent length statement packet.

| 4 |
|:---|
| amount of seconds the following programs were used | 

## License
This program is licensed under the MIT license. Take a look at the LICENSE file to get more information.