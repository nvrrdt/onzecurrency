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
    add(box);

    tabControlSetup.set_size_request(800, 600);
    tabControlSetup.set_show_tabs(false);
    box.pack_start(tabControlSetup, Gtk::PACK_SHRINK);

    page_setup1_create();
    page_setup2_update();
    page_setup3_normal();
    //page_setup4_exit();

    tabPageSetup1.show();
    tabPageSetup2.show();
    tabPageSetup3.show();
    //tabPageSetup4.show();

    label_exit.set_text("Preparing clean exit ... Please wait!");
    box_exit.pack_start(label_exit);
    box_exit.show();

    tabControlSetup.insert_page(tabPageSetup1, "Create", 0);
    tabControlSetup.insert_page(tabPageSetup2, "Update", 1);
    tabControlSetup.insert_page(tabPageSetup3, "Normal", 2);
    tabControlSetup.insert_page(box_exit, "Exit", 3);

    tabControlSetup.set_current_page(0);

    signal_delete_event().connect(sigc::mem_fun(*this, &Form::is_deleted));

    // FullHash fh;
    // Normal n;
    // std::string my_full_hash = fh.get_full_hash();
    // if (my_full_hash.empty() && !input_setup1_create_ok)
    // {
    //     tabControlSetup.set_current_page(0);
    // }
    // else if (!my_full_hash.empty())
    // {
    //     tabControlSetup.set_current_page(1);

    //     if (n.get_goto_normal_mode())
    //     {
    //         tabControlSetup.set_current_page(2);
    //     }
    // }
    
    set_title("onze-desktop");
    resize(800, 600);
    show_all();
}

void Form::page_setup1_create()
{
    tabPageSetup1.add(fixedGridPageCreate);

    fixedGridPageCreate.add(grid_setup1);
    fixedGridPageCreate.move(grid_setup1, 250, 250);

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

    fixedGridPageCreate.add(button_create);
    fixedGridPageCreate.move(button_create, 400, 330);

    button_create.signal_clicked().connect( sigc::mem_fun(*this,
              &Form::on_button_create_clicked) );
}

void Form::on_button_create_clicked()
{
    Common::Print_or_log pl;
    pl.init();

    Crowd::Auth a;
    auto cred = a.authentication("desktop");

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
    tabPageSetup2.add(fixedPageUpdate);

    label_update.set_text("Processing update ... Please wait!");

    fixedPageUpdate.add(label_update);
    fixedPageUpdate.move(label_update, 250, 250);
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
    tabPageNormal1.add(fixedPageCrowd);

    fixedPageCrowd.add(grid_normal1);
    fixedPageCrowd.move(grid_normal1, 250, 250);

    FullHash fh;
    std::string my_full_hash = fh.get_full_hash();
    label_crowd.set_text("Your user id is " + my_full_hash);

    grid_normal1.add(label_crowd);
}

void Form::page_normal2_coin()
{
    tabPageNormal2.add(fixedPageCoin);
}

// bool Form::on_delete_event(GdkEventAny *any_event)
// {
//     page_setup4_exit();
//     auto x = tabControlSetup.get_nth_page(3);
//     x->show_all();
//     tabControlSetup.set_current_page(3);
//     std::cout << "er " << tabControlSetup.get_current_page() << std::endl;

//     // MessageDialog dialog(*this, "Preparing clean exit ... Please wait!", true, MESSAGE_OTHER, BUTTONS_CLOSE, true);
//     // dialog.set_modal();
//     // dialog.set_position(WindowPosition::WIN_POS_CENTER);
//     // dialog.run();

//     std::this_thread::sleep_for(2000ms);
//     return Window::on_delete_event(any_event);

//     // if (dialog.run() == RESPONSE_YES)
//     //     return Window::on_delete_event(any_event);
//     // return true;

//     // page_setup4_exit();

//     // return Window::on_delete_event(any_event);
// }

void Form::page_setup4_exit()
{
    label_exit.set_text("Preparing clean exit ... Please wait!");

    box_exit.add(label_exit);
}

bool Form::is_deleted(GdkEventAny *any_event)
{
    tabControlSetup.set_current_page(3);
    std::cout << "current_page " << tabControlSetup.get_current_page() << std::endl;

    // Wait until intro_offline or new_offline is received
    std::this_thread::sleep_for(2000ms);
    return Window::on_delete_event(any_event);
}