#include "frame.h"

namespace KSD {

FrmMain::FrmMain(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade) :
    Gtk::ApplicationWindow(cobject), builder(refGlade){
    set_title("kstars Streaming Dither");
    builder->get_widget("button_start", m_btnRun);
    builder->get_widget("toggle_dither_phd_connect", m_tglPHDConnect);
    builder->get_widget("entry_dither_phd_host", m_entPHDHost);
    builder->get_widget("spin_dither_phd_instance", m_spnPHDInstance);
    builder->get_widget("label_dither_phd_status_indicator", m_lblPHDStatus);
    builder->get_widget("radio_dither_phd", m_radPHD);
    builder->get_widget("radio_dither_internal", m_radInternal);
    builder->get_widget("toggle_indi_connect", m_tglINDIConnect);
    builder->get_widget("label_indi_status_indicator", m_lblINDIStatus);
    builder->get_widget("combo_indi_camera", m_cmbINDICamera);

    m_INDICameraModel = Gtk::ListStore::create(m_Columns);
    m_cmbINDICamera->set_model(m_INDICameraModel);
    m_cmbINDICamera->pack_start(m_Columns.m_colName);

    m_tglPHDConnect->signal_toggled().connect(sigc::mem_fun(*this, &FrmMain::togglePHDConnect));
    m_radPHD->signal_clicked().connect(sigc::mem_fun(*this, &FrmMain::setGuidPHD));
    m_radInternal->signal_clicked().connect(sigc::mem_fun(*this, &FrmMain::setGuidInternal));
    m_tglINDIConnect->signal_toggled().connect(sigc::mem_fun(*this, &FrmMain::toggleINDIConnect));

    m_cmbINDICamera->signal_changed().connect(sigc::mem_fun(*this, &FrmMain::setSelectedCamera));

    Glib::signal_timeout().connect(sigc::mem_fun(*this, &FrmMain::updatePHDStatus), 200);
    Glib::signal_timeout().connect(sigc::mem_fun(*this, &FrmMain::updateINDIStatus), 200);

    m_dbus = Gio::DBus::Connection::get_sync(Gio::DBus::BUS_TYPE_SESSION);
    if ( ! m_dbus ) {
        showError(DBUS_ERROR, "Error connecting to DBUS");
        return;
    }
}

FrmMain::~FrmMain() {
    delete m_phd;
}

void FrmMain::togglePHDConnect() {
    if ( m_tglPHDConnect->get_active() ) {
        connectPHD();
    } else {
        disconnectPHD();
    }
}

void FrmMain::connectPHD() {
    Glib::ustring newHost = m_entPHDHost->get_buffer()->get_text();
    unsigned int newInstance = m_spnPHDInstance->get_adjustment()->get_value();
    if ( newHost == Glib::ustring("") ) {
        showError(PHD_ERROR, "Hostname is empty", "Please provide a valid hostname");
        m_tglPHDConnect->set_active(false);
        return;
    }
    if ( newHost != m_PHDHost || newInstance != m_PHDInstance ) {
        delete m_phd;
        m_phd = new Guider(newHost.c_str(), newInstance);
    }
    if ( ! m_phd->Connect() ) {
        showError(PHD_ERROR, m_phd->LastError());
        m_tglPHDConnect->set_active(false);
        return;
    }
    m_lblPHDStatus->set_text("Connected");
    m_PHDConnected = true;
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
    m_PHDConnected = false;
    m_PHDError = false;
}

bool FrmMain::updatePHDStatus() {
    g_debug("Updating PHD status");
    if ( m_phd == nullptr || m_PHDError ) {
        return true;
    }
    if ( not m_PHDConnected ) {
        m_lblPHDStatus->set_text("Not connected");
        return true;
    }
    double avgDist;
    std::string appState;
    if ( ! m_phd->GetStatus(&appState, &avgDist) ) {
        showError(PHD_ERROR, m_phd->LastError());
        m_PHDError = true;
        m_lblPHDStatus->set_text("Error");
        return true;
    }
    m_lblPHDStatus->set_text(appState);
    return true;
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

void FrmMain::connectINDI() {
    g_debug("Connecting to INDI via dbus");
    m_proxy = Gio::DBus::Proxy::create_sync(m_dbus, "org.kde.kstars",
            "/KStars/INDI", "org.kde.kstars.INDI");
    if ( ! m_proxy ) {
        showError(DBUS_ERROR, "Error connecting to kstars/INDI");
        m_tglINDIConnect->set_active(false);
        return;
    }
    try {
        Glib::VariantContainerBase result = m_proxy->call_sync("getDevices");
    } catch(const Glib::Error& e) {
        showError(DBUS_ERROR, "Error connecting to kstars/INDI: " + e.what());
        m_tglINDIConnect->set_active(false);
        return;
    }
    if ( ! updateINDICameraMenu() ) {
        m_tglINDIConnect->set_active(false);
        return;
    }
}

void FrmMain::disconnectINDI() {
    g_debug("Disconnecting from INDI via dbus");
    m_INDICameraModel->clear();
    m_proxy.clear();
}

void FrmMain::toggleINDIConnect() {
    if ( m_tglINDIConnect->get_active() ) {
        connectINDI();
    } else {
        disconnectINDI();
    }
}

bool FrmMain::updateINDIStatus() {
    g_debug("Updating INDI status");
    if ( ! m_proxy ) {
        m_lblINDIStatus->set_text("Not connected");
        return true;
    }
    try {
        Glib::VariantContainerBase result = m_proxy->call_sync("getDevices");
    } catch(const Glib::Error& e) {
        m_lblINDIStatus->set_text("Error");
        return true;
    }
    m_lblINDIStatus->set_text("Connected");
    return true;
}

bool FrmMain::updateINDICameraMenu() {
    Glib::VariantContainerBase result;
    try {
        result = m_proxy->call_sync("getDevices");
    } catch(const Glib::Error& e) {
        showError(DBUS_ERROR, "Error communicating with kstars/INDI: " + e.what());
        return false;
    }
    g_debug("INDI camera list: %s", result.print().c_str());
    if ( ! result.is_of_type(Glib::VariantType("(as)")) ) {
        showError(DBUS_ERROR, "Got wrong type on DBUS: " + result.get_type_string());
        return false;
    }
    Glib::VariantBase thing = result.get_child(0);
    g_debug("Thing: Type: %s ; Content: %s", thing.get_type_string().c_str(), thing.print(true).c_str());
    auto inside = Glib::VariantBase::cast_dynamic<Glib::Variant<std::vector<Glib::ustring>>>(thing);
    for (gsize ii=0; ii<inside.get_n_children(); ii++) {
        g_debug("Entry %lu: %s", ii, inside.get_child(ii).c_str());
    }

    std::vector<Glib::ustring> cameras;
    for (gsize ii=0; ii<inside.get_n_children(); ii++) {
        auto cameraName = inside.get_child(ii);
        g_debug("Entry %lu: %s", ii, cameraName.c_str());
        auto params = Glib::VariantContainerBase::create_tuple(Glib::Variant<Glib::ustring>::create(cameraName));
        Glib::VariantContainerBase deviceResult;
        try {
            deviceResult = m_proxy->call_sync("getProperties", params);
        } catch(const Glib::Error &e) {
            showError(DBUS_ERROR, "Error getting device properties: " + e.what());
            return false;
        }
        auto content = Glib::VariantBase::cast_dynamic<Glib::Variant<std::vector<Glib::ustring>>>(deviceResult.get_child(0));
        Glib::ustring wantProperty = cameraName + ".RECORD_STREAM.RECORD_ON";
        for (gsize jj=0; jj<content.get_n_children(); jj++) {
            auto propertyName = content.get_child(jj);
            g_debug("%lu: %s", jj+1, propertyName.c_str());
            if ( propertyName.compare(wantProperty) == 0 ) {
                cameras.push_back(cameraName);
                break;
            }
        }
    }
    g_debug("Getting selected camera");
    auto selectedCamera = m_cmbINDICamera->get_active();
    g_debug("Getting camera name");
    if ( selectedCamera->children() ) {
        auto value = selectedCamera->get_value(m_Columns.m_colName);
    }
    g_debug("Clearing list");
    m_INDICameraModel->clear();
    g_debug("Cameras:");
    int activeIndex = 0;
    int num = 0;
    for (auto it = cameras.begin(); it != cameras.end(); it++) {
        g_debug("%s", it->c_str());
        auto row = *(m_INDICameraModel->append());
        row[m_Columns.m_colName] = *it;
        if ( m_selectedCamera == *it ) {
            activeIndex = num;
        }
        num++;
    }
    m_cmbINDICamera->set_active(activeIndex);

    return true;
}

void FrmMain::setSelectedCamera() {
    const auto iter = m_cmbINDICamera->get_active();
    if ( ! iter ) {
        return;
    }
    m_selectedCamera = (*iter)[m_Columns.m_colName];
    g_debug("Set selected camera to \"%s\"", m_selectedCamera.c_str());
}

}
