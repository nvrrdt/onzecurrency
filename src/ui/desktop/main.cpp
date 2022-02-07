// #include <iostream>
// #include <gtkmm.h>

// using namespace Glib;
// using namespace Gtk;

// class Form : public Window
// {
// public:
//     Form()
//     {
//         add(scrolledWindow);
//         scrolledWindow.add(fixed);

//         tabControl1.set_size_request(370, 250);
//         fixed.add(tabControl1);
//         fixed.move(tabControl1, 10, 10);

//         tabControl1.insert_page(tabPage1, "tabPage1", 0);
//         tabControl1.insert_page(tabPage2, "tabPage2", 1);
//         tabControl1.insert_page(tabPage3, "tabPage3", 2);

//         labelPage1.set_label("labelTabPage1");

//         tabControl1.set_tab_label(tabPage1, labelPage1);

//         tabPage1.add(fixedTabPage1);

//         fixedTabPage1.add(radioTop);
//         fixedTabPage1.move(radioTop, 10, 10);
//         radioTop.set_label("Top");
//         radioTop.set_group(radioButtonGroup);
//         radioTop.signal_toggled().connect([this]()
//                                           { tabControl1.set_tab_pos(PositionType::POS_TOP); });

//         fixedTabPage1.add(radioLeft);
//         fixedTabPage1.move(radioLeft, 10, 40);
//         radioLeft.set_label("Left");
//         radioLeft.set_group(radioButtonGroup);
//         radioLeft.signal_toggled().connect([this]()
//                                            { tabControl1.set_tab_pos(PositionType::POS_LEFT); });

//         fixedTabPage1.add(radioRight);
//         fixedTabPage1.move(radioRight, 10, 70);
//         radioRight.set_label("Right");
//         radioRight.set_group(radioButtonGroup);
//         radioRight.signal_toggled().connect([this]()
//                                             { tabControl1.set_tab_pos(PositionType::POS_RIGHT); });

//         fixedTabPage1.add(radioBottom);
//         fixedTabPage1.move(radioBottom, 10, 100);
//         radioBottom.set_label("Bottom");
//         radioBottom.set_group(radioButtonGroup);
//         radioBottom.signal_toggled().connect([this]()
//                                              { tabControl1.set_tab_pos(PositionType::POS_BOTTOM); });

//         set_title("TabControl example");
//         resize(390, 270);
//         show_all();
//     }

// private:
//     Fixed fixed;
//     ScrolledWindow scrolledWindow;
//     Notebook tabControl1;
//     Label labelPage1;
//     Frame tabPage1;
//     Frame tabPage2;
//     Frame tabPage3;
//     RadioButtonGroup radioButtonGroup;
//     RadioButton radioTop;
//     RadioButton radioLeft;
//     RadioButton radioRight;
//     RadioButton radioBottom;
//     Fixed fixedTabPage1;
// };

// class Form : public Window
// {
// public:
//     Form()
//     {
//         scrolledWindow.add(fixed);

//         button.set_label("Close");
//         button.signal_button_release_event().connect([&](GdkEventButton *event)
//         {
//             close();
//             return true;
//         });
//         fixed.add(button);
//         fixed.move(button, 10, 10);

//         set_title("Form Example");
//         move(320, 200);
//         resize(640, 480);
//         add(scrolledWindow);
//         show_all();
//     }

//     bool on_delete_event(GdkEventAny *any_event) override
//     {
//         MessageDialog dialog(*this, "Are you sure you want exit?", true, MESSAGE_QUESTION, BUTTONS_YES_NO, true);
//         dialog.set_title("Close Form");
//         dialog.set_modal();
//         dialog.set_position(WindowPosition::WIN_POS_CENTER);
//         if (dialog.run() == RESPONSE_YES)
//             return Window::on_delete_event(any_event);
//         return true;
//     }

// private:
//     ScrolledWindow scrolledWindow;
//     Fixed fixed;
//     Button button;
// };

#include "desktop.hpp"
#include "bridge.hpp"

using namespace UI;

int main(int argc, char *argv[])
{
    Bridge bridge;
    bridge.do_bridge();
    
    RefPtr<Application> application = Application::create(argc, argv);
    Form form;

    return application->run(form);
}