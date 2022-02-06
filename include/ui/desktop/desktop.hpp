#pragma once

#include <iostream>
#include <gtkmm.h>

using namespace Glib;
using namespace Gtk;

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
    Notebook tabControlNormal;
    Frame tabPageNormal1;
    Frame tabPageNormal2;
    Fixed fixedTabPageNormal;
};