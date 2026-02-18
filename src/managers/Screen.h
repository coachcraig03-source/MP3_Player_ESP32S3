// =====================================================================
//  Screen.h - Pure interface for all screens
// =====================================================================

#ifndef SCREEN_H
#define SCREEN_H

class Screen {
public:
    virtual void begin() = 0;
    virtual void update() = 0;
    virtual void handleTouch(int x, int y) = 0;
    virtual ~Screen() {}
};

#endif // SCREEN_H
