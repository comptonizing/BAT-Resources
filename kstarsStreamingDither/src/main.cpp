#include "frame.h"

int main(int argc, char *argv[])
{
    Gtk::Main kit(argc, argv);
    KSD::FrmMain *frm = nullptr;
    Glib::RefPtr<Gtk::Builder> builder =
        Gtk::Builder::create_from_file("glade/kstarsStreamingDither.glade");
    builder->get_widget_derived("main", frm);
    kit.run(*frm);
}
