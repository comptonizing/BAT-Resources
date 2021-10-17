#pragma once

#include <iostream>
#include <gtkmm.h>
#include <glib.h>

#include "guider.h"

namespace KSD {

class FrmMain : public Gtk::ApplicationWindow {
    public:
        ~FrmMain();
        FrmMain(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);

    private:
        Glib::RefPtr<Gtk::Builder> builder;
        Gtk::Button *m_btnRun;
        Gtk::ToggleButton *m_tglPHDConnect;
        Gtk::Entry *m_entPHDHost;
        Gtk::SpinButton *m_spnPHDInstance;
        Gtk::Label *m_lblPHDStatus;
        Gtk::RadioButton *m_radPHD;
        Gtk::RadioButton *m_radInternal;
        Guider *m_phd = nullptr;
        Glib::ustring m_PHDHost = "";
        std::unique_ptr<Gtk::MessageDialog> m_dialog;
        unsigned int m_PHDInstance = -1;

        void setGuidInternal();
        void setGuidPHD();
        void togglePHDConnect();
        void connectPHD();
        void disconnectPHD();
        void showError(Glib::ustring title, Glib::ustring message,
                Glib::ustring secondaryMessage = "");
};

}
