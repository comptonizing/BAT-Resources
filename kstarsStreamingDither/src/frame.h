#pragma once

#include <iostream>
#include <gtkmm.h>
#include <glib.h>

#include "guider.h"

#define INDI_ERROR "kstars/INDI Error"
#define DBUS_ERROR "DBUS Error"
#define PHD_ERROR "PHD2 Error"

namespace KSD {

class FrmMain : public Gtk::ApplicationWindow {
    public:
        ~FrmMain();
        FrmMain(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);

    private:
        class ModelColumns : public Gtk::TreeModel::ColumnRecord {
            public:
                ModelColumns() {
                    add(m_colName);
                }
            Gtk::TreeModelColumn<Glib::ustring> m_colName;
        };

        Glib::RefPtr<Gtk::Builder> builder;
        Gtk::Button *m_btnRun;
        Gtk::ToggleButton *m_tglPHDConnect;
        Gtk::Entry *m_entPHDHost;
        Gtk::SpinButton *m_spnPHDInstance;
        Gtk::Label *m_lblPHDStatus;
        Gtk::RadioButton *m_radPHD;
        Gtk::RadioButton *m_radInternal;
        Gtk::ToggleButton *m_tglINDIConnect;
        Gtk::Label *m_lblINDIStatus;
        Gtk::ComboBox *m_cmbINDICamera;
        Guider *m_phd = nullptr;
        Glib::ustring m_PHDHost = "";
        std::unique_ptr<Gtk::MessageDialog> m_dialog;
        unsigned int m_PHDInstance = -1;
        bool m_PHDConnected = false;
        bool m_PHDError = false;
        Glib::RefPtr<Gio::DBus::Connection> m_dbus;
        Glib::RefPtr<Gio::DBus::Proxy> m_proxy;
        Glib::RefPtr<Gtk::ListStore> m_INDICameraModel;
        ModelColumns m_Columns;
        Glib::ustring m_selectedCamera = "";

        void setGuidInternal();
        void setGuidPHD();
        void togglePHDConnect();
        void connectPHD();
        void disconnectPHD();
        bool updatePHDStatus();
        void showError(Glib::ustring title, Glib::ustring message,
                Glib::ustring secondaryMessage = "");
        void connectINDI();
        void disconnectINDI();
        void toggleINDIConnect();
        bool updateINDIStatus();
        bool updateINDICameraMenu();
        void setSelectedCamera();
};

}
