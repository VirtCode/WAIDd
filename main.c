#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define KEY_STORAGE "storage"
#define KEY_INTERVAL "interval"

#define CONFIG_FILE strcat(getenv("HOME"), "/.config/waid/daemon.cfg"))
#define DEFAULT_STORAGE strcat(getenv("HOME"), "/.waidfile"))
#define DEFAULT_INTERVAL 60

Display* display;

char* storage;
int interval;



int fetchProperty(Display* display, unsigned long* window, char* name, unsigned char** property){
    Atom type;
    int format;
    unsigned long items, bytesAfter;

    int status = XGetWindowProperty(display, *window, XInternAtom(display, name, True), 0, 1000, False, AnyPropertyType,
                                    &type, &format, &items, &bytesAfter, property);
    if (status != Success) status = 1;
    else status = 0;
    return status;
}

int fetchWindow(unsigned char** nameHolder){
    // Open connection to X Server
    if (display == NULL){
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

    }else if (!strcmp(key, KEY_INTERVAL)){

        interval = atoi(value);

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

int main(){
    if(readConfig(CONFIG_FILE) printf("Failed to read config");
    printf("%d", interval);
    return 0;
}




