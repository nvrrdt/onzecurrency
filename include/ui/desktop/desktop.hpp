#pragma once

#include <iostream>
#include <gtkmm.h>

using namespace Glib;
using namespace Gtk;

namespace UI
{
    class Form : public Window
    {
    public:
        Form();
    private:
        Fixed fixed;
        ScrolledWindow scrolledWindow;
        Notebook tabControlSetup;
        Frame tabPageSetup1;
        Frame tabPageSetup2;
        Frame tabPageSetup3;
        Frame tabPageSetup4;
        Notebook tabControlNormal;
        Frame tabPageNormal1;
        Frame tabPageNormal2;
        Fixed fixedTabPageNormal;
    };

    class Normal
    {
    public:
        static void set_goto_normal_mode(bool goto_normal_mode)
        {
            goto_normal_mode_ = goto_normal_mode;
        }
        static bool get_goto_normal_mode()
        {
            return goto_normal_mode_;
        }
    private:
        static bool goto_normal_mode_;
    };
}