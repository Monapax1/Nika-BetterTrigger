#include <iostream>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

#pragma once

struct MyDisplay {
private:
    std::unique_ptr<Display, decltype(&XCloseDisplay)> display{XOpenDisplay(nullptr), &XCloseDisplay};

public:
    MyDisplay() {
        if (!display)
            throw std::runtime_error("Could not open display");
    }

    void mouseClickLeft() {
        XTestFakeButtonEvent(display.get(), Button1, True, 0);
        XTestFakeButtonEvent(display.get(), Button1, False, 0);
        XFlush(display.get());
    }

    void mouseClick(int button) {
        XEvent event;
        memset(&event, 0x00, sizeof(event));
        event.type = ButtonPress;
        event.xbutton.button = button;
        event.xbutton.same_screen = True;
        XQueryPointer(display.get(), RootWindow(display.get(), DefaultScreen(display.get())), &event.xbutton.root, &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
        event.xbutton.subwindow = event.xbutton.window;
        while (event.xbutton.subwindow) {
            event.xbutton.window = event.xbutton.subwindow;
            XQueryPointer(display.get(), event.xbutton.window, &event.xbutton.root, &event.xbutton.subwindow, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
        }
        if (XSendEvent(display.get(), PointerWindow, True, 0xfff, &event) == 0)
            fprintf(stderr, "Error\n");
        XFlush(display.get());
        usleep(100000);
        event.type = ButtonRelease;
        event.xbutton.state = 0x100;
        if (XSendEvent(display.get(), PointerWindow, True, 0xfff, &event) == 0)
            fprintf(stderr, "Error\n");
        XFlush(display.get());
    }

    void moveMouseRelative(int pitchMovement, int yawMovement) {
        XTestFakeRelativeMotionEvent(display.get(), yawMovement, pitchMovement, CurrentTime);
        XFlush(display.get());
    }

    bool isLeftMouseButtonDown() {
        Window root, child;
        int root_x, root_y, win_x, win_y;
        unsigned int mask;
        if (XQueryPointer(display.get(), XRootWindow(display.get(), DefaultScreen(display.get())), &root, &child, &root_x, &root_y, &win_x, &win_y, &mask))
            return (mask & Button1Mask) != 0;
        return false;
    }

    bool keyDown(int keyCode) {
        char keys_return[32];
        XQueryKeymap(display.get(), keys_return);
        KeyCode kc2 = XKeysymToKeycode(display.get(), keyCode);
        bool buttonDown = !!(keys_return[kc2 >> 3] & (1 << (kc2 & 7)));
        return buttonDown;
    }

    bool keyDown(std::string XK_keyName) {
        KeySym keyCode = XStringToKeysym(trimXKPrefix(XK_keyName).c_str());
        return keyDown(keyCode);
    }

    std::string trimXKPrefix(const std::string& keyName) {
        if (keyName.compare(0, 3, "XK_") == 0)
            return keyName.substr(3);
        return keyName;
    }
};
