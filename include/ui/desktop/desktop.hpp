#pragma once

#include <iostream>
#include <string>
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
        void page_setup1_create();
        void on_button_create_clicked();
    private:
        // Form
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

        // page_setup1_create
        Fixed fixedTabPageCreate;
        Grid grid_setup1;
        Label label_network;
        Label label_email;
        Entry entry_network;
        Entry entry_email;
        Button button_create;
        std::string network_s;
        std::string email_s;
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