/**
 * SNIPER MOUSE
 * A macOS utility to adjust mouse sensitivity on-the-fly using side buttons.
 * 
 * License: MIT
 */

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hidsystem/IOHIDLib.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>

// --- Configuration ---

typedef struct {
    double default_speed;
    double precision_speed; // Button 4
    double tactical_speed;  // Button 5
} SniperConfig;

static const SniperConfig CONFIG = {
    .default_speed = 1.5,
    .precision_speed = 0.1,
    .tactical_speed = 0.5
};

// --- Prototypes ---

void set_mouse_speed(double speed);
void reset_speed(void);
void handle_signal(int sig);
void print_banner(void);
bool check_accessibility(void);
CGEventRef mouse_callback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon);

// --- Implementation ---

/**
 * Updates the system mouse acceleration setting.
 */
void set_mouse_speed(double speed) {
    io_connect_t connection;
    io_service_t service = IOServiceGetMatchingService(kIOMainPortDefault, IOServiceMatching(kIOHIDSystemClass));

    if (!service) return;

    kern_return_t kr = IOServiceOpen(service, mach_task_self(), kIOHIDParamConnectType, &connection);
    IOObjectRelease(service);

    if (kr == KERN_SUCCESS) {
        // Note: IOHIDSetAccelerationWithKey is deprecated but still functional for system-wide acceleration
        IOHIDSetAccelerationWithKey(connection, CFSTR(kIOHIDMouseAccelerationType), speed);
        IOServiceClose(connection);
    }
}

/**
 * Restores mouse speed to the configured default.
 */
void reset_speed(void) {
    set_mouse_speed(CONFIG.default_speed);
}

/**
 * Signal handler to ensure speed is restored on exit.
 */
void handle_signal(int sig) {
    exit(0);
}

/**
 * Event tap callback to monitor Mouse Button 4 and 5.
 */
CGEventRef mouse_callback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
    if (type != kCGEventOtherMouseDown && type != kCGEventOtherMouseUp) {
        return event;
    }

    int64_t button = CGEventGetIntegerValueField(event, kCGMouseEventButtonNumber);
    bool is_down = (type == kCGEventOtherMouseDown);

    switch (button) {
        case 3: // Button 4
            set_mouse_speed(is_down ? CONFIG.precision_speed : CONFIG.default_speed);
            break;
        case 4: // Button 5
            set_mouse_speed(is_down ? CONFIG.tactical_speed : CONFIG.default_speed);
            break;
    }

    return event;
}

/**
 * Checks if the application has Accessibility permissions.
 */
bool check_accessibility(void) {
    const void *keys[] = { kAXTrustedCheckOptionPrompt };
    const void *values[] = { kCFBooleanTrue };
    
    CFDictionaryRef options = CFDictionaryCreate(
        kCFAllocatorDefault, 
        keys, 
        values, 
        1, 
        &kCFTypeDictionaryKeyCallBacks, 
        &kCFTypeDictionaryValueCallBacks
    );

    bool trusted = AXIsProcessTrustedWithOptions(options);
    CFRelease(options);
    return trusted;
}

void print_banner(void) {
    printf("  ____  _   _ ___ ____  _____ ____      __  __  ___  _   _ ____  _____ \n");
    printf(" / ___|| \\ | |_ _|  _ \\| ____|  _ \\    |  \\/  |/ _ \\| | | / ___|| ____|\n");
    printf(" \\___ \\|  \\| || || |_) |  _| | |_) |   | |\\/| | | | | | | \\___ \\|  _|  \n");
    printf("  ___) | |\\  || ||  __/| |___|  _ <    | |  | | |_| | |_| |___) | |___ \n");
    printf(" |____/|_| \\_|___|_|   |_____|_| \\_\\   |_|  |_|\\___/ \\___/|____/|_____|\n");
    printf("\n");
    printf(" A utility to adjust mouse sensitivity on-the-fly:\n");
    printf(" - [Hold Button 4]  Precision Mode (%.1f speed)\n", CONFIG.precision_speed);
    printf(" - [Hold Button 5]  Tactical Mode  (%.1f speed)\n", CONFIG.tactical_speed);
    printf(" - [Release]        Default Speed  (%.1f speed)\n", CONFIG.default_speed);
    printf("\n Listening for mouse events... (Ctrl+C to stop)\n");
}

int main(int argc, char *argv[]) {
    if (atexit(reset_speed) != 0) {
        fprintf(stderr, "Fatal: Could not register exit handler.\n");
        return EXIT_FAILURE;
    }

    signal(SIGINT,  handle_signal);
    signal(SIGTERM, handle_signal);

    print_banner();

    if (!check_accessibility()) {
        fprintf(stderr, "\nERROR: Accessibility permissions not granted.\n");
        fprintf(stderr, "Please enable permissions for this app/terminal in System Settings.\n");
        return EXIT_FAILURE;
    }

    CGEventMask mask = CGEventMaskBit(kCGEventOtherMouseDown) | CGEventMaskBit(kCGEventOtherMouseUp);
    CFMachPortRef event_tap = CGEventTapCreate(
        kCGSessionEventTap, 
        kCGHeadInsertEventTap, 
        kCGEventTapOptionDefault, 
        mask, 
        mouse_callback, 
        NULL
    );

    if (!event_tap) {
        fprintf(stderr, "Fatal: Failed to create event tap.\n");
        return EXIT_FAILURE;
    }

    CFRunLoopSourceRef source = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, event_tap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
    CGEventTapEnable(event_tap, true);
    
    CFRunLoopRun();

    return EXIT_SUCCESS;
}