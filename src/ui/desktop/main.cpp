
// #include "main.hpp"

// #include <map>

// #include "auth.hpp"
// #include "p2p.hpp"
// #include "json.hpp"
// #include "p2p_network.hpp"
// #include "p2p_network_c.hpp"

// #include "print_or_log.hpp"
// #include "configdir.hpp"

// extern int USE_LOG;

// using namespace Crowd;
// using namespace Coin;

// int main(int argc, char *argv[])
// {
//     ConfigDir cd;
//     cd.CreateDirInConfigDir("log");
//     Common::Print_or_log pl;
//     pl.init();

//     Auth a;
//     std::map<std::string, std::string> cred = a.authentication();

//     if (cred["error"] == "true")
//     {
//         pl.handle_print_or_log({"Error with authenticating"});
                
//         return 1;
//     } else {
//         // start crowd
//         std::packaged_task<void()> task1([cred] {
//             P2p p;
//             p.start_crowd(cred);
//         });
//         // Run task on new thread.
//         std::thread t1(std::move(task1));

//         // start coin
//         std::packaged_task<void()> task2([cred] {
//             // P2pNetworkC pnc;
//             // pnc.start_coin();
//         });
//         // Run task on new thread.
//         std::thread t2(std::move(task2));

//         // start server
//         std::packaged_task<void()> task3([] {
//             P2pNetwork pn;
//             pn.p2p_server();
//         });
//         // Run task on new thread.
//         std::thread t3(std::move(task3));

//         t1.join();
//         t2.join();
//         t3.join();
       
//     }

//     // // example of how to enable upnp
//     // std::cout << "server_peer: " << ip_mother_peer << ", my_ip: " << my_public_ip << std::endl;
//     // if (ip_mother_peer == my_public_ip)
//     // {
//     //     Upnp upnp;
//     //     Udp udp;
//     //     if(upnp.Upnp_main() == 0) // upnp possible
//     //     {
//     //         udp.udp_server();
//     //     }
//     // }

//     return 0;
// }

#include <gtkmm.h>

using namespace Glib;
using namespace Gtk;

class Form : public Window {
public:
  Form() {
    add(scrolledWindow);
    scrolledWindow.add(fixed);
    
    label1.set_text("Hello, World!");
    label1.override_color(Gdk::RGBA("#008000"));
    Pango::FontDescription textFont;
    textFont.set_family("Arial");
    textFont.set_size(34 * PANGO_SCALE);
    textFont.set_style(Pango::Style::STYLE_ITALIC);
    textFont.set_weight(Pango::Weight::WEIGHT_BOLD);
    label1.override_font(textFont);
    fixed.add(label1);
    fixed.move(label1, 5, 100);
    
    set_title("My first application");
    resize(300, 300);
    show_all();
  }
  
private:
  Fixed fixed;
  ScrolledWindow scrolledWindow;
  Label label1;
};

int main(int argc, char* argv[]) {
  RefPtr<Application> application = Application::create(argc, argv);
  Form form;
  return application->run(form);
}