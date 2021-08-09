#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define WAID_VERSION "1.0"

#define CONFIG_FILE "/.config/waid/daemon.cfg"

#define KEY_STORAGE "storage"
#define KEY_INTERVAL "interval"

#define DEFAULT_STORAGE "/.waidfile"
#define DEFAULT_INTERVAL 60

#define FILE_IDENTIFIER_VERSION 0x01
#define FILE_IDENTIFIER_DEFINE 0x02
#define FILE_IDENTIFIER_RECORD 0x03

#define FILE_VERSION_CHAOTIC 0x01

Display* display;

char* storage;
int interval;
char running;

char* programList;
int programListSize;
unsigned short currentMaxId;

FILE* waidfile;

int fetchProperty(Display* display, unsigned long* window, char* name, unsigned char** property){
    Atom type;
    int format;
    unsigned long items, bytesAfter;

    int status = XGetWindowProperty(display, *window, XInternAtom(display, name, True), 0, 255, False, AnyPropertyType,
                                    &type, &format, &items, &bytesAfter, property);
    if (status != Success) status = 1;
    else status = 0;
    return status;
}

int fetchWindow(unsigned char** nameHolder){
    // Open connection to X Server
    if (display == NULL){
        printf("Reopening connection to X Server\n");
        display = XOpenDisplay(NULL);
        if (display == NULL) return 1;
    }

    // Fetch default windows
    int screen = XDefaultScreen(display);
    unsigned long window = RootWindow(display, screen);

    // Fetch active window
    unsigned char* prop;
    if (fetchProperty(display, &window, "_NET_ACTIVE_WINDOW", &prop) == 1) return 1;
    window = prop[0] + (prop[1] << 8) + (prop[2] << 16) + (prop[3] << 24);

    // Fetch the name of said window
    if (fetchProperty(display, &window, "WM_CLASS", nameHolder) == 1) return 1;

    return 0;
}

void processConfigEntry(char* key, char* value){
    if (!strcmp(key, KEY_STORAGE)) {

        storage = malloc(strlen(value) + 1); // Allocate enough memory
        strncpy(storage, value, strlen(value));
        printf("Picked up custom storage from config\n");

    }else if (!strcmp(key, KEY_INTERVAL)){

        interval = atoi(value);
        printf("Picked up custom interval from config\n");

    }else printf("Unrecognized config option: \"%s\"\n", key);
}

void processConfig(char* line, int equalIndex, int length){
    if (equalIndex == 0) return; // Ignore values without keys

    // Detect length from key without spaces
    int keyEnd = equalIndex;
    for (int i = equalIndex-1; i >= 0; --i) {
        if (isspace(line[i])) keyEnd = i;
        else break;
    }
    if (keyEnd == 0) return; // Ignore keys that are only spaces

    // End String on Key End
    line[keyEnd] = '\0';
    char* key = line;

    // Detect Value Start
    int valueStart = equalIndex + 1;
    for (int i = valueStart; i < length; ++i) {
        if (isspace(line[i])) valueStart = i + 1;
        else break;
    }
    if (valueStart == length) return; // No value provided

    // Detect Value end
    int valueEnd = length;
    for (int i = length - 1; i >= valueStart; --i) {
        if (isspace(line[i])) valueEnd = i;
        else break;
    }
    if (valueEnd == valueStart) return;

    // Shorten Value string
    line[valueEnd] = '\0';
    char* value = line + valueStart;

    processConfigEntry(key, value);
}

int readConfig(char* path){
    printf("Reading config file from \"%s\"\n", path);
    FILE* config = fopen(path, "r");
    if (!config) return 1;

    char buff[255];

    while (1){
        if (!fgets(buff, 255, config)) break;

        char begun = 0;
        unsigned char begunIndex = 0;
        for (int i = 0; i < 255; ++i) {
            if (buff[i] != ' ' && !begun) { // Ignore spaces at beginning
                begun = 1;
                begunIndex = i;
            }

            if (begun) {
                if (buff[i] == '#' && i == begunIndex) break; // Ignore comments when first character is a hash
                if (buff[i] == '=') processConfig(buff + begunIndex, i - begunIndex, strlen(buff + begunIndex)); // Has equal sign -> is config pair
            }
        }
    }

    fclose(config);
    return 0;
}

void writeVersion(FILE* file, char version){
    char identifier = FILE_IDENTIFIER_VERSION;
    fwrite(&identifier, sizeof(char), 1, file);
    fwrite(&version, sizeof(char), 1, file);
}

void writeDefine(FILE* file, unsigned short id, char* name){
    char identifier = FILE_IDENTIFIER_DEFINE;
    fwrite(&identifier, sizeof(char), 1, file);
    fwrite(&id, sizeof(short), 1, file);
    fwrite(name, sizeof(char), strlen(name) + 1, file);
}

void writeRecord(FILE* file, long time, unsigned short id){
    char identifier = FILE_IDENTIFIER_RECORD;
    fwrite(&identifier, sizeof(char), 1, file);
    fwrite(&time, sizeof(long), 1, file);
    fwrite(&id, sizeof(short), 1, file);
}

void readVersion(FILE* file, char* version){
    fread(version, sizeof(char), 1, file);
}

void readDefine(FILE* file, unsigned short* id, char* buffer, int bufferSize){
    fread(id, sizeof(short), 1, file);

    for (int i = 0; i < bufferSize; ++i) {
        fread(buffer + i, sizeof(char), 1, file);
        if (buffer[i] == '\0') break;
    }
}

void skipRecord(FILE* file){
    fseek(file, sizeof(long) + sizeof(short), SEEK_CUR);
}

void setProgramId(unsigned short id, char* string){
    if (id > currentMaxId) currentMaxId = id;

    int addIndex = programListSize;
    programListSize += sizeof(short) + strlen(string) + 1;

    if (programList == NULL) programList = malloc(programListSize);
    else programList = realloc(programList, programListSize);


    for (int i = 0; i < sizeof(short); ++i) {
        (programList + addIndex)[i] = ((char *) &id)[i];
    }
    strcpy(programList + addIndex + sizeof(short), string);
}

short addProgram(char* name){
    setProgramId(++currentMaxId, name);
    writeDefine(waidfile, currentMaxId, name);

    return currentMaxId;
}

unsigned short getProgramId(char* name){

    for (int i = 0; i < programListSize;) {

        if(!strcmp(programList + i + sizeof(short), name)) return *((unsigned short *) (programList + i));
        i += sizeof(short);
        i += strlen(programList + i) + 1;
    }

    return 0;
}

void initializeStorage(){
    FILE* file = fopen(storage, "wb");
    writeVersion(file, FILE_VERSION_CHAOTIC);
    fclose(file);
}

int readStorage(){
    printf("Reading previous snapshots from \"%s\"\n", storage);
    FILE* file = fopen(storage, "rb");
    if (file == NULL) {
        printf("No storage file found, creating one now\n");
        initializeStorage();
        return 0;
    }

    int recordCount = 0;
    int defineCount = 0;

    char currentIndicator = 0;
    while (fread(&currentIndicator, sizeof(char), 1, file)){

        switch (currentIndicator) {
            case FILE_IDENTIFIER_VERSION: ;

                char version = 0;
                readVersion(file, &version);
                if (version != FILE_VERSION_CHAOTIC) {
                    printf("Existing storage file has an unsupported version (%d, expected %d), cannot use the file\n", version, FILE_VERSION_CHAOTIC);
                    return 1;
                }

                break;
            case FILE_IDENTIFIER_DEFINE: ;

                char* buffer = malloc(255);
                unsigned short id = 0;
                readDefine(file, &id, buffer, 255);

                setProgramId(id, buffer);

                free(buffer);
                defineCount++;

                break;
            case FILE_IDENTIFIER_RECORD:

                skipRecord(file);
                recordCount++;

                break;
        }
    }

    printf("Finished reading the file. It contains %d different Applications and %d total records.\n", defineCount, recordCount);

    return 0;
}

int openFile(){
    printf("Opening current waidfile \"%s\" to write in\n", storage);
    waidfile = fopen(storage, "ab");
    return waidfile == 0;
}

void takeRecord(long time){
    printf("Taking record at %ld\n", time);

    unsigned char* currentName;
    fetchWindow(&currentName);

    unsigned short id = getProgramId((char*) currentName);
    if (id == 0) {
        printf("New Program \"%s\" detected, adding it\n", currentName);
        id = addProgram((char*) currentName);
    }

    writeRecord(waidfile, time, id);

    fflush(waidfile);
}

void scheduleRecords(){
    running = 1;

    long lastTime = time(NULL);
    long currentTime = 0L;

    printf("Entering Schedule\n\n\n");
    while (running){
        sleep(1);

        currentTime = time(NULL);
        if (currentTime - lastTime >= interval) {
            takeRecord(currentTime);
            lastTime = currentTime;
        }
    }
}

int main(){
    printf("Starting the WAID daemon version %s \nPlease be aware that this daemon does only track your program usage and offers no way to display this data in a readable way.\nIn order to do that, other tools may need to be installed.\nAlso, make sure that no two instances of this daemon run simultaneously.\n\n", WAID_VERSION);

    char* home = getenv("HOME");

    char* config = malloc(strlen(home) + strlen(CONFIG_FILE) + 1); config[0] = '\0';
    strcat(config, home); strcat(config, CONFIG_FILE);
    if(readConfig(config)) printf("Failed to read config\n");
    free(config);

    if(storage == NULL) {
        storage = malloc(strlen(home) + strlen(DEFAULT_STORAGE) + 1); storage[0] = '\0';
        strcat(storage, home); strcat(storage, DEFAULT_STORAGE);
    }
    if (interval == 0) interval = DEFAULT_INTERVAL;

    printf("\nGoing to store program usage in: \"%s\"\n", storage);
    printf("A usage snapshot will be taken every %d seconds\n", interval);

    printf("\nConnecting to X Server\n");
    display = XOpenDisplay(NULL);
    if (display == NULL) printf("Failed to connect to X Server\n\n");

    if(readStorage()) {
        printf("\nFailed to read the storage file, because it is not supported\nGoing to exit\n");
        return 1;
    }
    if(openFile()){
        printf("\nFailed to open the storage file for appending\nGoing to exit\n");
        return 1;
    }

    printf("\nFinished initializing\n");
    scheduleRecords();

    return 0;
}




