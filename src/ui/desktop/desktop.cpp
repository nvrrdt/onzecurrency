#include "desktop.hpp"

Form::Form()
{
    add(scrolledWindow);
    scrolledWindow.add(fixed);

    tabControlSetup.set_size_request(780, 580);
    tabControlSetup.set_show_tabs(false);
    fixed.add(tabControlSetup);
    fixed.move(tabControlSetup, 10, 10);

    tabPageSetup1.show();
    tabPageSetup2.show();

    tabControlSetup.insert_page(tabPageSetup1, "Setup", 0);
    tabControlSetup.insert_page(tabPageSetup2, "Normal", 1);

    tabControlSetup.set_current_page(0);

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