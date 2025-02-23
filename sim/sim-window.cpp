// ****************************************************************************
//  mainwindow.cpp                                                DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Main window for the DM42 simulator
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the terms outlined in LICENSE.txt
// ****************************************************************************
//   This file is part of DB48X.
//
//   DB48X is free software: you can redistribute it and/or modify
//   it under the terms outlined in the LICENSE.txt file
//
//   DB48X is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// ****************************************************************************

#include <QtGui>
#include <QtCore>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QKeyEvent>
#include <QStandardPaths>

#include <dmcp.h>
#include <target.h>

#include "sim-window.h"
#include "sim-rpl.h"
#include "ui_sim-window.h"
#include "recorder.h"
#include "tests.h"


RECORDER(sim_keys, 16, "Recorder keys from the simulator");

extern bool run_tests;
extern bool db48x_keyboard;
extern bool shiftHeld;
extern bool altHeld;
MainWindow *MainWindow::mainWindow = nullptr;

MainWindow::MainWindow(QWidget *parent)
// ----------------------------------------------------------------------------
//    The main window of the simulator
// ----------------------------------------------------------------------------
    : QMainWindow(parent), ui(), rpl(this), tests(this), highlight()
{
    mainWindow = this;

    QCoreApplication::setOrganizationName("DB48X");
    QCoreApplication::setApplicationName("DB48X");

    ui.setupUi(this);

    ui.keyboard->setAttribute(Qt::WA_AcceptTouchEvents);
    ui.keyboard->installEventFilter(this);
    ui.screen->setAttribute(Qt::WA_AcceptTouchEvents);
    ui.screen->installEventFilter(this);
    if (db48x_keyboard)
        ui.keyboard->setStyleSheet("border-image: url(:/bitmap/keyboard-db48x.png) 0 0 0 0 stretch stretch;");
    else
        ui.keyboard->setStyleSheet("border-image: url(:/bitmap/keyboard.png) 0 0 0 0 stretch stretch;");

    highlight = new Highlight(ui.keyboard);
    highlight->setGeometry(0,0,0,0);
    highlight->show();

    setWindowTitle("DB48X");

    QObject::connect(this, SIGNAL(keyResizeSignal(const QRect &)),
                     highlight, SLOT(keyResizeSlot(const QRect &)));

    qreal dpratio = qApp->primaryScreen()->devicePixelRatio();
    resize(210 * dpratio, 390 * dpratio);

    rpl.start();

    if (run_tests)
        tests.start();
}

MainWindow::~MainWindow()
// ----------------------------------------------------------------------------
//  Destroy the main window
// ----------------------------------------------------------------------------
{
    key_push(tests::EXIT_PGM);
}


void MainWindow::resizeEvent(QResizeEvent * event)
// ----------------------------------------------------------------------------
//   Resizing the window
// ----------------------------------------------------------------------------
{
    qreal dpratio = qApp->primaryScreen()->devicePixelRatio();
    int w = ui.screen->screen_width;
    int h = ui.screen->screen_height + 5;
    if(!h)
        h = LCD_H+5;
    if(!w)
        w = LCD_W;
    qreal dpwidth = event->size().width();
    qreal realwidth = dpwidth * dpratio;
    qreal scale = realwidth / w;
    if((int)scale < 1)
        scale = 1.0;
    else
        scale = (int)scale;
    if (event->size().height() * 0.38 * dpratio < scale * h)
    {
        scale = event->size().height() * 0.38 * dpratio / h;
        if ((int) scale < 1)
            scale = 1.0;
        else
            scale = (int) scale;
    }
    ui.screen->setScale(scale / dpratio);
}


const int keyMap[] =
// ----------------------------------------------------------------------------
//   Key map for the DM42
// ----------------------------------------------------------------------------
{
    // Actual key mappings are in the relevant platform's target.h
    Qt::Key_Tab,        KB_ALPHA,
    Qt::Key_SysReq,     KB_ON,
    Qt::Key_Escape,     KB_ESC,
    Qt::Key_Period,     KB_DOT,
    Qt::Key_Space,      KB_SPC,
    Qt::Key_Question,   KB_QUESTION,
    Qt::Key_Control,    KB_SHIFT,
    // Qt::Key_Alt,        KB_LSHIFT,
    Qt::Key_Meta,       KB_RSHIFT,

    Qt::Key_Plus,       KB_ADD,
    Qt::Key_Minus,      KB_SUB,
    Qt::Key_Asterisk,   KB_MUL,
    Qt::Key_Slash,      KB_DIV,

    Qt::Key_Enter,      KB_ENT,
    Qt::Key_Return,     KB_ENT,
    Qt::Key_Backspace,  KB_BKS,
    Qt::Key_Up,         KB_UP,
    Qt::Key_Down,       KB_DN,
    Qt::Key_Left,       KB_LF,
    Qt::Key_Right,      KB_RT,

    Qt::Key_F1,         KB_F1,
    Qt::Key_F2,         KB_F2,
    Qt::Key_F3,         KB_F3,
    Qt::Key_F4,         KB_F4,
    Qt::Key_F5,         KB_F5,
    Qt::Key_F6,         KB_F6,

    Qt::Key_0,          KB_0,
    Qt::Key_1,          KB_1,
    Qt::Key_2,          KB_2,
    Qt::Key_3,          KB_3,
    Qt::Key_4,          KB_4,
    Qt::Key_5,          KB_5,
    Qt::Key_6,          KB_6,
    Qt::Key_7,          KB_7,
    Qt::Key_8,          KB_8,
    Qt::Key_9,          KB_9,
    Qt::Key_A,          KB_A,
    Qt::Key_B,          KB_B,
    Qt::Key_C,          KB_C,
    Qt::Key_D,          KB_D,
    Qt::Key_E,          KB_E,
    Qt::Key_F,          KB_F,
    Qt::Key_G,          KB_G,
    Qt::Key_H,          KB_H,
    Qt::Key_I,          KB_I,
    Qt::Key_J,          KB_J,
    Qt::Key_K,          KB_K,
    Qt::Key_L,          KB_L,
    Qt::Key_M,          KB_M,
    Qt::Key_N,          KB_N,
    Qt::Key_O,          KB_O,
    Qt::Key_P,          KB_P,
    Qt::Key_Q,          KB_Q,
    Qt::Key_R,          KB_R,
    Qt::Key_S,          KB_S,
    Qt::Key_T,          KB_T,
    Qt::Key_U,          KB_U,
    Qt::Key_V,          KB_V,
    Qt::Key_W,          KB_W,
    Qt::Key_X,          KB_X,
    Qt::Key_Y,          KB_Y,
    Qt::Key_Z,          KB_Z,

#ifdef KB_HOME
    Qt::Key_Home,       KB_HOME,
#endif // KB_HOME

#ifdef KB_HELP
    Qt::Key_F11,        KB_HELP,
#endif // KB_HELP

    0,0
};


struct mousemap
{
    int key, keynum;
    qreal left, right, top, bot;
} mouseMap[] = {

    { Qt::Key_F1,        38, 0.03, 0.15, 0.03, 0.10 },
    { Qt::Key_F2,        39, 0.20, 0.32, 0.03, 0.10 },
    { Qt::Key_F3,        40, 0.345, 0.47, 0.03, 0.10 },
    { Qt::Key_F4,        41, 0.52, 0.63, 0.03, 0.10 },
    { Qt::Key_F5,        42, 0.68, 0.80, 0.03, 0.10 },
    { Qt::Key_F6,        43, 0.83, 0.95, 0.03, 0.10 },

    { Qt::Key_A,          1, 0.03, 0.15, 0.15, 0.22 },
    { Qt::Key_B,          2, 0.20, 0.32, 0.15, 0.22 },
    { Qt::Key_C,          3, 0.345, 0.47, 0.15, 0.22 },
    { Qt::Key_D,          4, 0.52, 0.63, 0.15, 0.22 },
    { Qt::Key_E,          5, 0.68, 0.80, 0.15, 0.22 },
    { Qt::Key_F,          6, 0.83, 0.95, 0.15, 0.22 },

    { Qt::Key_G,          7, 0.03, 0.15, 0.275, 0.345 },
    { Qt::Key_H,          8, 0.20, 0.32, 0.275, 0.345 },
    { Qt::Key_I,          9, 0.345, 0.47, 0.275, 0.345 },
    { Qt::Key_J,         10, 0.52, 0.63, 0.275, 0.345 },
    { Qt::Key_K,         11, 0.68, 0.80, 0.275, 0.345 },
    { Qt::Key_L,         12, 0.83, 0.95, 0.275, 0.345 },

    { Qt::Key_Return,    13, 0.03, 0.32, 0.40, 0.47 },
    { Qt::Key_M,         14, 0.345, 0.47, 0.40, 0.47 },
    { Qt::Key_N,         15, 0.51, 0.64, 0.40, 0.47 },
    { Qt::Key_O,         16, 0.68, 0.80, 0.40, 0.47 },
    { Qt::Key_Backspace, 17, 0.83, 0.95, 0.40, 0.47 },

    { Qt::Key_Up,        18, 0.03, 0.15, 0.52, 0.59 },
    { Qt::Key_7,         19, 0.23, 0.36, 0.52, 0.59 },
    { Qt::Key_8,         20, 0.42, 0.56, 0.52, 0.59 },
    { Qt::Key_9,         21, 0.62, 0.75, 0.52, 0.59 },
    { Qt::Key_Slash,     22, 0.81, 0.95, 0.52, 0.59 },

    { Qt::Key_Down,      23, 0.03, 0.15, 0.645, 0.715 },
    { Qt::Key_4,         24, 0.23, 0.36, 0.645, 0.715 },
    { Qt::Key_5,         25, 0.42, 0.56, 0.645, 0.715 },
    { Qt::Key_6,         26, 0.62, 0.75, 0.645, 0.715 },
    { Qt::Key_Asterisk,  27, 0.81, 0.95, 0.645, 0.715 },

    { Qt::Key_Control,   28, 0.028, 0.145, 0.77, 0.84 },
    { Qt::Key_1,         29, 0.23, 0.36, 0.77, 0.84 },
    { Qt::Key_2,         30, 0.42, 0.56, 0.77, 0.84 },
    { Qt::Key_3,         31, 0.62, 0.75, 0.77, 0.84 },
    { Qt::Key_Minus,     32, 0.81, 0.95, 0.77, 0.84 },

    { Qt::Key_Escape,    33, 0.03, 0.15, 0.89, 0.97 },
    { Qt::Key_0,         34, 0.23, 0.36, 0.89, 0.97 },
    { Qt::Key_Period,    35, 0.42, 0.55, 0.89, 0.97 },
    { Qt::Key_Question,  36, 0.62, 0.74, 0.89, 0.97 },
    { Qt::Key_Plus,      37, 0.81, 0.95, 0.89, 0.97 },

    {                0,  0,      0.0,      0.0,      0.0,      0.0}
};


void MainWindow::pushKey(int key)
// ----------------------------------------------------------------------------
//   When pushing a key, update the highlight rectangle
// ----------------------------------------------------------------------------
{
    QRect rect(0, 0, 0, 0);
    for (mousemap *ptr = mouseMap; ptr->key; ptr++)
    {
        if (ptr->keynum == key)
        {
            int w = ui.keyboard->width();
            int h = ui.keyboard->height();
            rect.setCoords(ptr->left * w, ptr->top * h,
                           ptr->right * w, ptr->bot * h);
            break;
        }
    }
    record(sim_keys,
           "Key %d coords (%d, %d, %d, %d)",
           key,
           rect.x(),
           rect.y(),
           rect.width(),
           rect.height());
    emit keyResizeSignal(rect);
}


void Highlight::keyResizeSlot(const QRect &rect)
// ----------------------------------------------------------------------------
//   Receive signal that the widget was resized
// ----------------------------------------------------------------------------
{
    setGeometry(rect);
}


void Highlight::paintEvent(QPaintEvent *)
// ----------------------------------------------------------------------------
//   Repaing, showing the highlight
// ----------------------------------------------------------------------------
{
    QRect geo = geometry();
    record(sim_keys, "Repainting %d %d %d %d",
           geo.x(), geo.y(), geo.width(), geo.height());
    QRect local(3, 3, geo.width()-6, geo.height()-6);
    QPainter p(this);
    QPainterPath path;
    path.addRoundedRect(local, 8, 8);
    QPen pen(Qt::yellow, 4);
    p.setPen(pen);
    p.drawPath(path);
}


void MainWindow::keyPressEvent(QKeyEvent * ev)
// ----------------------------------------------------------------------------
//   Got a key - Push it to the simulator
// ----------------------------------------------------------------------------
{
    if (ev->isAutoRepeat())
    {
        ev->accept();
        return;
    }

    int k = ev->key();
    record(sim_keys, "Key press %d", k);

    if (k == Qt::Key_F11 || k == Qt::Key_F12)
    {
        if (!tests.isRunning())
        {
            tests.onlyCurrent = k == Qt::Key_F11;
            tests.start();
        }
    }

    if (k == Qt::Key_F10)
    {
        db48x_keyboard = !db48x_keyboard;
        if (db48x_keyboard)
            ui.keyboard->setStyleSheet("border-image: url(:/bitmap/keyboard-db48x.png) 0 0 0 0 stretch stretch;");
        else
            ui.keyboard->setStyleSheet("border-image: url(:/bitmap/keyboard.png) 0 0 0 0 stretch stretch;");
    }

    if (k == Qt::Key_Shift)
    {
        shiftHeld = true;
    }
    else if (k == Qt::Key_Alt)
    {
        altHeld = true;
    }
    else if (k >= Qt::Key_A && k <= Qt::Key_Z)
    {
        if (shiftHeld)
            key_push(KEY_UP);
        else if (altHeld)
            key_push(KEY_DOWN);
    }

    for (int i = 0; keyMap[i] != 0; i += 2)
    {
        if (k == keyMap[i])
        {
            record(sim_keys, "Key %d found at %d, DM42 key is %d",
                   k, i, keyMap[i+1]);
            key_push(keyMap[i+1]);
            ev->accept();
            return;
        }
    }

    QMainWindow::keyPressEvent(ev);
}


void MainWindow::keyReleaseEvent(QKeyEvent * ev)
// ----------------------------------------------------------------------------
//   Released a key - Send a 0 to the simulator
// ----------------------------------------------------------------------------
{
    if(ev->isAutoRepeat()) {
        ev->accept();
        return;
    }

    int k = ev->key();
    record(sim_keys, "Key release %d", k);
    if (k == Qt::Key_Shift)
        shiftHeld = false;
    else if (k == Qt::Key_Alt)
        altHeld = false;

    for (int i = 0; keyMap[i] != 0; i += 2)
    {
        if (k == keyMap[i])
        {
            record(sim_keys, "Key %d found at %d, sending key up", k, i);
            key_push(0);
            ev->accept();
            return;
        }
    }

    QMainWindow::keyReleaseEvent(ev);
}


bool MainWindow::eventFilter(QObject * obj, QEvent * ev)
// ----------------------------------------------------------------------------
//  Filter mouse / keyboard events
// ----------------------------------------------------------------------------
{
    if(obj == ui.keyboard) {
        if ((ev->type() == QEvent::TouchBegin) ||
            (ev->type() == QEvent::TouchUpdate) ||
            (ev->type() == QEvent::TouchEnd) ||
            (ev->type() == QEvent::TouchCancel))
        {
            QTouchEvent *me = static_cast < QTouchEvent * >(ev);
#if QT_VERSION < 0x060000
            auto &touchPoints = me->touchPoints();
#else
            auto &touchPoints = me->points();
#endif // Qt version 6
            qsizetype npoints = touchPoints.count();

            record(sim_keys, "Touch event %d points", npoints);

            for(int k = 0; k < npoints; ++k) {
#if QT_VERSION < 0x060000
                QPointF coordinates = touchPoints.at(k).startPos();
#else
                QPointF coordinates = touchPoints.at(k).pressPosition();
#endif // Qt version 6
                qreal relx, rely;
                int   pressed;

                if(touchPoints.at(k).state() & Qt::TouchPointPressed)
                    pressed = 1;
                else if(touchPoints.at(k).
                        state() & Qt::TouchPointReleased)
                    pressed = 0;
                else
                    continue;   // NOT INTERESTED IN DRAGGING

                relx = coordinates.x() / (qreal) ui.keyboard->width();
                rely = coordinates.y() / (qreal) ui.keyboard->height();
                record(sim_keys, "  [%d] at (%f, %f) %+s",
                       k, relx, rely, pressed ? "pressed" : "released");

                if (!pressed)
                    key_push(0);
                else
                    for (mousemap *ptr = mouseMap; ptr->key; ptr++)
                        if ((relx >= ptr->left) && (relx <= ptr->right) &&
                            (rely >= ptr->top) && (rely <= ptr->bot))
                        {
                            record(sim_keys, "  [%d] found at %d as %d",
                                   k, ptr - mouseMap, ptr->keynum);
                            key_push(ptr->keynum);
                        }
            }

            return true;
        }

        if (ev->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *me = static_cast < QMouseEvent * >(ev);
#if QT_VERSION < 0x060000
            qreal relx = (qreal) me->x() / (qreal) ui.keyboard->width();
            qreal rely = (qreal) me->y() / (qreal) ui.keyboard->height();
#else
            qreal relx =
                (qreal) me->position().x() / (qreal) ui.keyboard->width();
            qreal rely =
                (qreal) me->position().y() / (qreal) ui.keyboard->height();
#endif // Qt vertsion 6

            record(sim_keys, "Mouse button press at (%f, %f)", relx, rely);
            for (mousemap *ptr = mouseMap; ptr->key; ptr++)
                if ((relx >= ptr->left) && (relx <= ptr->right) &&
                    (rely >= ptr->top) && (rely <= ptr->bot))
                {
                    record(sim_keys, "Mouse coordinates found at %d as %d",
                           ptr - mouseMap, ptr->keynum);

                    key_push(ptr->keynum);
                }

            return true;
        }

        if(ev->type() == QEvent::MouseButtonRelease)
        {
            record(sim_keys, "Mouse button released");
            key_push(0);
            return true;
        }

        return false;
    }

    return false;
}



bool tests::image_match(cstring file, int x, int y, int w, int h, bool force)
// ----------------------------------------------------------------------------
//   Check if the screen matches a given file
// ----------------------------------------------------------------------------
{
    QPixmap &screen = MainWindow::theScreen();
    QPixmap img = screen.copy(x, y, w, h);
    QPixmap data;
    QString name = force ? "images/bad/" : "images/";
    name += file;
    name += ".png";
    QFileInfo reference(name);
    if (force || !reference.exists() || !data.load(name, "PNG"))
    {
        img.save(name, "PNG");
        return true;
    }
    return data.toImage() == img.toImage();
}
