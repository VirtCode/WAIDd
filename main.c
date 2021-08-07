#include <stdio.h>
#include <X11/Xlib.h>
#include <time.h>
#include <unistd.h>

Display* display;

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

int readConfig(char* path){
    FILE* config = fopen(path, "r");
    if (!config) return 1;

    char buff[255];

    for(int i = 0; i < 1000; i++){
        printf("%dasdf", fgets(buff, 255, config) == buff);
        printf("%s", buff);
    }

    fclose(config);
}

int main(){

    readConfig("/home/joshua/.config/polybar/launch.sh");

    return 0;
}




