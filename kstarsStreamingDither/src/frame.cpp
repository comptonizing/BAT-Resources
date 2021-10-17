#include "frame.h"

namespace KSD {

FrmMain::FrmMain(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade) :
    Gtk::ApplicationWindow(cobject), builder(refGlade){
    builder->get_widget("button_start", m_btnRun);
    builder->get_widget("toggle_dither_phd_connect", m_tglPHDConnect);
    builder->get_widget("entry_dither_phd_host", m_entPHDHost);
    builder->get_widget("spin_dither_phd_instance", m_spnPHDInstance);
    builder->get_widget("label_dither_phd_status_indicator", m_lblPHDStatus);
    builder->get_widget("radio_dither_phd", m_radPHD);
    builder->get_widget("radio_dither_internal", m_radInternal);

    m_tglPHDConnect->signal_toggled().connect(sigc::mem_fun(*this, &FrmMain::togglePHDConnect));
    m_radPHD->signal_clicked().connect(sigc::mem_fun(*this, &FrmMain::setGuidPHD));
    m_radInternal->signal_clicked().connect(sigc::mem_fun(*this, &FrmMain::setGuidInternal));
}

FrmMain::~FrmMain() {
    delete m_phd;
}

void FrmMain::togglePHDConnect() {
    g_debug("togglePHDConnect");
    if ( m_tglPHDConnect->get_active() ) {
        connectPHD();
    } else {
        disconnectPHD();
    }
}

void FrmMain::connectPHD() {
    Glib::ustring newHost = m_entPHDHost->get_buffer()->get_text();
    unsigned int newInstance = m_spnPHDInstance->get_adjustment()->get_value();
    g_debug("newHost: %s ; newInstance: %u", newHost.c_str(), newInstance);
    if ( newHost == Glib::ustring("") ) {
        showError("PHD2 Settings", "Hostname is empty", "Please provide a valid hostname");
        m_tglPHDConnect->set_active(false);
        return;
    }
    if ( newHost != m_PHDHost || newInstance != m_PHDInstance ) {
        delete m_phd;
        m_phd = new Guider(newHost.c_str(), newInstance);
    }
    if ( ! m_phd->Connect() ) {
        showError("PHD2 Error", m_phd->LastError());
        m_tglPHDConnect->set_active(false);
        return;
    }
    m_lblPHDStatus->set_text("Connected");
    m_spnPHDInstance->set_sensitive(false);
    m_entPHDHost->set_sensitive(false);
    m_radPHD->set_sensitive(false);
    m_radInternal->set_sensitive(false);
}

void FrmMain::disconnectPHD() {
    if ( m_phd == nullptr ) {
        return;
    }
    m_phd->Disconnect();
    m_lblPHDStatus->set_text("Not connected");
    m_spnPHDInstance->set_sensitive(true);
    m_entPHDHost->set_sensitive(true);
    m_radPHD->set_sensitive(true);
    m_radInternal->set_sensitive(true);
    m_tglPHDConnect->set_active(false);
}

void FrmMain::showError(Glib::ustring title, Glib::ustring message,
        Glib::ustring secondaryMessage) {
        m_dialog.reset(new Gtk::MessageDialog(*this, message, false,
                Gtk::MessageType::MESSAGE_ERROR, Gtk::ButtonsType::BUTTONS_OK, true));
        m_dialog->set_title(title);
        if ( secondaryMessage != Glib::ustring("") ) {
            m_dialog->set_secondary_text(secondaryMessage);
        }
        m_dialog->set_modal(true);
        m_dialog->signal_response().connect(sigc::hide(sigc::mem_fun(*m_dialog, &Gtk::Widget::hide)));
        m_dialog->show();
}

void FrmMain::setGuidInternal() {
    disconnectPHD();
    m_spnPHDInstance->set_sensitive(false);
    m_entPHDHost->set_sensitive(false);
    m_tglPHDConnect->set_sensitive(false);
}

void FrmMain::setGuidPHD() {
    m_spnPHDInstance->set_sensitive(true);
    m_entPHDHost->set_sensitive(true);
    m_tglPHDConnect->set_sensitive(true);
}

}
