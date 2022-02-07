#include "desktop.hpp"

using namespace UI;

Form::Form()
{
    add(scrolledWindow);
    scrolledWindow.add(fixed);

    tabControlSetup.set_size_request(800, 600);
    tabControlSetup.set_show_tabs(false);
    fixed.add(tabControlSetup);
    fixed.move(tabControlSetup, 0, 0);

    tabPageSetup1.show();
    tabPageSetup2.show();
    tabPageSetup3.show();
    tabPageSetup4.show();

    tabControlSetup.insert_page(tabPageSetup1, "Create", 0);
    tabControlSetup.insert_page(tabPageSetup2, "Update", 1);
    tabControlSetup.insert_page(tabPageSetup3, "Normal", 2);
    tabControlSetup.insert_page(tabPageSetup4, "Exit", 3);

    Normal n;
    if (n.get_goto_normal_mode())
    {
        tabControlSetup.set_current_page(2);
    }

    tabPageSetup2.add(fixedTabPageNormal);

    tabControlNormal.set_size_request(780, 580);
    fixedTabPageNormal.add(tabControlNormal);
    fixedTabPageNormal.move(tabControlNormal, 10, 10);

    tabPageNormal1.show();
    tabPageNormal2.show();

    tabControlNormal.insert_page(tabPageNormal1, "Crowd", 0);
    tabControlNormal.insert_page(tabPageNormal2, "Coin", 1);

    set_title("onze-desktop");
    resize(800, 600);
    show_all();
}