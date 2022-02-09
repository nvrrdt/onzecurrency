#include "desktop.hpp"

#include "auth.hpp"
#include "p2p.hpp"
#include "json.hpp"
#include "p2p_network.hpp"
#include "p2p_network_c.hpp"

#include "print_or_log.hpp"
#include "configdir.hpp"

#include "full_hash.hpp"

using namespace UI;

Form::Form()
{
    add(scrolledWindow);
    scrolledWindow.add(fixed);

    tabControlSetup.set_size_request(800, 600);
    tabControlSetup.set_show_tabs(false);
    fixed.add(tabControlSetup);
    fixed.move(tabControlSetup, 0, 0);

    page_setup1_create();
    page_setup2_update();
    page_setup3_normal();
    page_setup4_exit();

    tabPageSetup1.show();
    tabPageSetup2.show();
    tabPageSetup3.show();
    tabPageSetup4.show();

    tabControlSetup.insert_page(tabPageSetup1, "Create", 0);
    tabControlSetup.insert_page(tabPageSetup2, "Update", 1);
    tabControlSetup.insert_page(tabPageSetup3, "Normal", 2);
    tabControlSetup.insert_page(tabPageSetup4, "Exit", 3);

    FullHash fh;
    Normal n;
    std::string my_full_hash = fh.get_full_hash();
    if (my_full_hash == "" && !input_setup1_create_ok)
    {
        tabControlSetup.set_current_page(0);
    }
    else
    {
        tabControlSetup.set_current_page(1);

        if (n.get_goto_normal_mode())
        {
            tabControlSetup.set_current_page(2);
        }
    }

    set_title("onze-desktop");
    resize(800, 600);
    show_all();
}

void Form::page_setup1_create()
{
    tabPageSetup1.add(fixedTabPageCreate);

    fixedTabPageCreate.add(grid_setup1);
    fixedTabPageCreate.move(grid_setup1, 250, 250);

    label_network.set_text("Network:");
    label_network.set_width_chars(9);
    label_email.set_text("Email:");
    label_email.set_width_chars(9);

    grid_setup1.attach(label_network, 0, 0, 1, 1);
    grid_setup1.attach(entry_network, 1, 0, 1, 1);
    grid_setup1.attach(label_email, 0, 1, 1, 1);
    grid_setup1.attach(entry_email, 1, 1, 1, 1);

    entry_network.set_max_length(80);
    network_s = entry_network.get_text();
    entry_email.set_max_length(80);
    email_s = entry_email.get_text();

    button_create.add_label("Create user");

    fixedTabPageCreate.add(button_create);
    fixedTabPageCreate.move(button_create, 400, 330);

    button_create.signal_clicked().connect( sigc::mem_fun(*this,
              &Form::on_button_create_clicked) );
}

void Form::on_button_create_clicked()
{
    Common::Print_or_log pl;
    pl.init();

    Crowd::Auth a;
    auto cred = a.authentication(network_s, email_s);

    if (cred["error"] == "true")
    {
        pl.handle_print_or_log({"Error with authenticating"});
                
        return;
    } else {
        input_setup1_create_ok = true;

        // Create log directory
        ConfigDir cd;
        cd.CreateDirInConfigDir("log");

        // start crowd
        std::packaged_task<void()> task1([cred] {
            P2p p;
            p.start_crowd(cred);
        });
        // Run task on new thread.
        std::thread t1(std::move(task1));

        // start coin
        std::packaged_task<void()> task2([cred] {
            // P2pNetworkC pnc;
            // pnc.start_coin();
        });
        // Run task on new thread.
        std::thread t2(std::move(task2));

        // start server
        std::packaged_task<void()> task3([] {
            P2pNetwork pn;
            pn.p2p_server();
        });
        // Run task on new thread.
        std::thread t3(std::move(task3));

        t1.join();
        t2.join();
        t3.join();
    }
}

void Form::page_setup2_update()
{
    tabPageSetup2.add(fixedTabPageUpdate);

    label_update.set_text("Processing update ... Please wait!");

    fixedTabPageUpdate.add(label_update);
    fixedTabPageUpdate.move(label_update, 250, 250);
}

void Form::page_setup3_normal()
{
    tabPageSetup3.add(fixedTabPageNormal);

    tabControlNormal.set_size_request(780, 580);
    fixedTabPageNormal.add(tabControlNormal);
    fixedTabPageNormal.move(tabControlNormal, 10, 10);

    page_normal1_crowd();
    page_normal2_coin();

    tabPageNormal1.show();
    tabPageNormal2.show();

    tabControlNormal.insert_page(tabPageNormal1, "Crowd", 0);
    tabControlNormal.insert_page(tabPageNormal2, "Coin", 1);
}

void Form::page_normal1_crowd()
{
}

void Form::page_normal2_coin()
{
}

void Form::page_setup4_exit()
{
    tabPageSetup4.add(fixedTabPageExit);

    label_exit.set_text("Preparing clean exit ... Please wait!");

    fixedTabPageExit.add(label_exit);
    fixedTabPageExit.move(label_exit, 250, 250);
}