#pragma once
#include <QString>

class StyleHelper {
public:
    static QString getApplicationStyleSheet();
    static QString getSidebarStyleSheet();
    static QString getTopToolbarStyleSheet();
    static QString getEmulatorToolbarStyleSheet();
    static QString getStatusBarStyleSheet();
};
