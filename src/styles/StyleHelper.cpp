#include "StyleHelper.h"

QString StyleHelper::getApplicationStyleSheet() {
    return R"(
        QWidget {
            background-color: #181818;
            color: #E0E0E0;
            font-family: "Segoe UI", "Segoe UI Semibold", sans-serif;
            font-size: 13px;
        }
        
        QMainWindow {
            background-color: #181818;
        }

        /* QSplitter styling */
        QSplitter::handle {
            background-color: #303030;
        }
        QSplitter::handle:horizontal {
            width: 1px;
        }
        QSplitter::handle:vertical {
            height: 1px;
        }
        
        /* ScrollBar styling */
        QScrollBar:vertical {
            border: none;
            background: #1F1F1F;
            width: 10px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: #3E3E3E;
            min-height: 20px;
            border-radius: 5px;
            margin: 2px;
        }
        QScrollBar::handle:vertical:hover {
            background: #4FC3F7;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollBar:horizontal {
            border: none;
            background: #1F1F1F;
            height: 10px;
            margin: 0px;
        }
        QScrollBar::handle:horizontal {
            background: #3E3E3E;
            min-width: 20px;
            border-radius: 5px;
            margin: 2px;
        }
        QScrollBar::handle:horizontal:hover {
            background: #4FC3F7;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }
    )";
}

QString StyleHelper::getSidebarStyleSheet() {
    return R"(
        QFrame#SidebarFrame {
            background-color: #202020;
            border-right: 1px solid #303030;
        }
        
        QLabel#SidebarTitle {
            font-size: 16px;
            font-weight: bold;
            color: #4FC3F7;
            padding: 10px;
            border-bottom: 1px solid #303030;
        }

        /* Sidebar Item List Button */
        QPushButton {
            background-color: transparent;
            color: #AAAAAA;
            border: none;
            border-radius: 6px;
            text-align: left;
            padding: 10px 15px;
            font-weight: bold;
            font-size: 13px;
            margin: 2px 8px;
        }
        QPushButton:hover {
            background-color: #2D2D2D;
            color: #E0E0E0;
        }
        QPushButton:checked {
            background-color: #252525;
            color: #4FC3F7;
            border-left: 3px solid #4FC3F7;
            border-radius: 0px;
            border-top-right-radius: 6px;
            border-bottom-right-radius: 6px;
        }
    )";
}

QString StyleHelper::getTopToolbarStyleSheet() {
    return R"(
        QFrame#TopToolbarFrame {
            background-color: #202020;
            border-bottom: 1px solid #303030;
            min-height: 48px;
            max-height: 48px;
        }
        
        QToolButton {
            background-color: transparent;
            color: #E0E0E0;
            border: none;
            border-radius: 4px;
            padding: 6px;
            margin: 2px;
        }
        QToolButton:hover {
            background-color: #303030;
        }
        QToolButton:pressed {
            background-color: #252525;
            border: 1px solid #4FC3F7;
        }
        
        QLineEdit#SearchBox {
            background-color: #181818;
            color: #E0E0E0;
            border: 1px solid #303030;
            border-radius: 14px;
            padding: 4px 12px;
            min-width: 200px;
        }
        QLineEdit#SearchBox:focus {
            border: 1px solid #4FC3F7;
        }
    )";
}

QString StyleHelper::getEmulatorToolbarStyleSheet() {
    return R"(
        QFrame#EmulatorToolbarFrame {
            background-color: #202020;
            border-left: 1px solid #303030;
            max-width: 48px;
            min-width: 48px;
        }
        
        /* Circular Emulator Action Buttons */
        QToolButton {
            background-color: #252525;
            color: #E0E0E0;
            border: 1px solid #303030;
            border-radius: 18px; /* Circular for 36x36 size */
            min-width: 36px;
            max-width: 36px;
            min-height: 36px;
            max-height: 36px;
            margin: 4px;
        }
        QToolButton:hover {
            background-color: #303030;
            border-color: #4FC3F7;
        }
        QToolButton:pressed {
            background-color: #181818;
            border-color: #4FC3F7;
        }
    )";
}

QString StyleHelper::getStatusBarStyleSheet() {
    return R"(
        QFrame#StatusBarFrame {
            background-color: #202020;
            border-top: 1px solid #303030;
            min-height: 28px;
            max-height: 28px;
        }
        
        QLabel {
            color: #888888;
            font-size: 11px;
            padding: 2px 6px;
        }
        
        /* Indicator light colors */
        QLabel#StatusLight {
            font-weight: bold;
            font-size: 10px;
            border-radius: 4px;
            padding: 2px 6px;
        }
    )";
}
