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
        void page_setup2_update();
        void page_setup3_normal();
        void page_normal1_crowd();
        void page_normal2_coin();
        bool on_delete_event(GdkEventAny *any_event) override;
    private:
        // Form
        Box box;
        Fixed fixed;
        ScrolledWindow scrolledWindow;
        Notebook tabControlSetup;
        Frame tabPageSetup1;
        Frame tabPageSetup2;
        Frame tabPageSetup3;
        Box box_exit;
        Label label_exit;
        
        // page_setup1_create
        Fixed fixedGridPageCreate;
        Grid grid_setup1;
        Label label_network;
        Label label_email;
        Entry entry_network;
        Entry entry_email;
        Button button_create;
        std::string network_s;
        std::string email_s;
        bool input_setup1_create_ok = false;

        // page_setup2_update
        Fixed fixedPageUpdate;
        Label label_update;

        // page_setup3_normal
        Fixed fixedTabPageNormal;
        Notebook tabControlNormal;
        Frame tabPageNormal1;
        Frame tabPageNormal2;

        // page_normal1_crowd
        Fixed fixedPageCrowd;
        Grid grid_normal1;
        Label label_crowd;

        // page_normal2_coin
        Fixed fixedPageCoin;

        // on_delete_event
        // This flag indicates if a first call to the "delete-event" signal
        // has been done. On a second call to this event, this should be
        // set to "true" to alter the handler's behaviour.
        bool flag = false;
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