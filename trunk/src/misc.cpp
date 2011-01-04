#include "misc.h"

#ifdef DEBUG
#define PDEBUG(fmt, args...)                                            \
    printf("%ld: %s(%d)-%s:\t",time(NULL), __FILE__,__LINE__,__FUNCTION__); \
    printf("\033[31m"fmt"\033[0m", ##args);
#else
#define PDEBUG(fmt, args...)  ;
#endif

#define oops(ch, args...)                                               \
    {fprintf(stderr,ch,##args);perror("Reason from system call: ");return -1;}


SimpleDialog:: SimpleDialog(wxWindow *parent, wxString name) :    \
    wxDialog(parent, -1, name)
{
    sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);
}

void SimpleDialog::post_draw()
{
    wxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    wxButton *btn = new wxButton(this, wxID_CANCEL, _("Cancel"));
    hbox->Add(btn, 0, wxEXPAND|wxLEFT, 10);
    hbox->Add(150, 0, 0);
    btn = new wxButton(this, wxID_OK, _("OK"));
    hbox->Add(btn, 0, wxEXPAND|wxRIGHT, 10);
    sizer->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);
}

DirnameDlg::DirnameDlg(wxWindow *parent, wxString name, wxString msg):   \
    SimpleDialog(parent, name)
{
    draw();
    post_draw();
    txt->SetValue(msg);
    txt->SetFocus();
    Fit();
}

void DirnameDlg::draw()
{
    txt = new wxTextCtrl(this, -1, _(""));
    sizer->Add(txt, 1, wxEXPAND|wxALL, 6);
}

void DirnameDlg::OnOK(wxCommandEvent &evt)
{
    PDEBUG ("called.\n");
    fn = txt->GetValue();
    EndModal(wxID_OK);
}

void DirnameDlg::OnCancel(wxCommandEvent &evt)
{
    EndModal(wxID_CANCEL);
}



BEGIN_EVENT_TABLE(DirnameDlg, wxDialog)
EVT_BUTTON(wxID_OK, DirnameDlg::OnOK)
EVT_BUTTON(wxID_CANCEL, DirnameDlg::OnCancel)
END_EVENT_TABLE()


PrefDialog::PrefDialog(wxWindow *parent, wxString title):\
SimpleDialog(parent, title)
{
    draw();
    post_draw();
    Fit();
}

void PrefDialog:: draw()
{
    string key;
    wxStaticText *label;
    wxGridBagSizer *grid = new wxGridBagSizer(5, 5);
    vector<config_entry>::iterator iter;
    int idx = 0;
    for (iter = config.entry_list.begin(); iter < config.entry_list.end();
         iter++) {
        key = (*iter).desc;
        if (key.find("auto_") == 0) {
            continue;
        }
        label = new wxStaticText(this, -1, wxString(key.c_str(), wxConvUTF8));
        grid->Add(label, wxGBPosition(idx, 0), wxGBSpan(1,1));
        switch ((*iter).type) {
        case TYPE_STR: {
            TextEntry *entry = new TextEntry(this, key);
            grid->Add(entry, wxGBPosition(idx, 1), wxGBSpan(1,4));
            break;
        }
        case TYPE_BOOL: {
            BoolEntry *entry = new BoolEntry(this, key);
            grid->Add(entry, wxGBPosition(idx, 1), wxGBSpan(1,4));
            break;
        }
        default:
            break;
        }
        idx++;
    }
    sizer->Add(grid, 1, wxEXPAND|wxALL, 5);
}

void PrefDialog::OnOK(wxCommandEvent &evt)
{
    config.dump2file();
    EndModal(wxID_OK);
}


void PrefDialog::OnCancel(wxCommandEvent &evt)
{
    EndModal(wxID_CANCEL);
}
BEGIN_EVENT_TABLE(PrefDialog, wxDialog)
EVT_BUTTON(wxID_OK, PrefDialog::OnOK)
EVT_BUTTON(wxID_CANCEL, PrefDialog::OnCancel)
END_EVENT_TABLE()

TextEntry::TextEntry(wxWindow *parent, string key):\
wxPanel(parent, -1)
{
    this->key = key;
    string val = config.get_config(key);
    wxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);

    txt = new wxTextCtrl(this, -1, wxString(val.c_str(), wxConvUTF8),
                         wxDefaultPosition, wxSize(200, -1));
    sizer->Add(txt, 1, wxEXPAND|wxALL, 6);
    wxButton *btn = new wxButton(this, -1, _("Browse"), wxDefaultPosition,
                                 wxSize(60,-1));
    sizer->Add(btn, 0, wxEXPAND|wxALL, 6);
    SetSizer(sizer);
    Fit();
}

void TextEntry::OnBtn(wxCommandEvent &evt)
{
    wxFileDialog *dlg = new wxFileDialog(this, _("Please choose a file!"),
                                         _("/"));
    if (dlg->ShowModal() == wxID_OK) {
        wxString vv = dlg->GetPath();
        string val = string(vv.mb_str());
        config.set_config(this->key, val);
        txt->SetValue(vv);
    }
}

BEGIN_EVENT_TABLE(TextEntry, wxPanel)
EVT_BUTTON(-1, TextEntry::OnBtn)
END_EVENT_TABLE()


BoolEntry:: BoolEntry(wxWindow *parent, string key):wxPanel(parent, -1)
{
    this->key = key;
    string val = config.get_config(key);
    wxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    btn1 = new wxRadioButton(this, -1, _("True"), wxDefaultPosition,
                             wxDefaultSize, wxRB_GROUP);
    sizer->Add(btn1, -1, wxEXPAND|wxALL, 6);
    btn2 = new wxRadioButton(this, -1, _("False"));
    sizer->Add(btn2, -1, wxEXPAND|wxALL, 6);
    if (val == "true") {
        btn1->SetValue(true);
        btn2->SetValue(false);
    }
    else {
        btn1->SetValue(false);
        btn2->SetValue(true);
    }
    SetSizer(sizer);
}

void BoolEntry::OnBtn(wxCommandEvent &evt)
{
    PDEBUG ("called.\n");

    if (btn1->GetValue())
        config.set_config(this->key, "true");
    else
        config.set_config(this->key, "false");
}

BEGIN_EVENT_TABLE(BoolEntry, wxPanel)
EVT_RADIOBUTTON(-1, BoolEntry::OnBtn)
END_EVENT_TABLE()



MyImagePanel::MyImagePanel(wxWindow* parent, string path) :\
wxPanel(parent, wxID_ANY)
{
    PDEBUG ("called: path: %s\n", path.c_str());
    fn = wxString(path.c_str(), wxConvUTF8);
    Show(true);
}

/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */

void MyImagePanel::paintEvent(wxPaintEvent & evt)
{
    // depending on your system you may need to look at double-buffered dcs
    wxPaintDC dc(this);
    render(dc);
}

/*
 * Alternatively, you can use a clientDC to paint on the panel
 * at any time. Using this generally does not free you from
 * catching paint events, since it is possible that e.g. the window
 * manager throws away your drawing when the window comes to the
 * background, and expects you will redraw it when the window comes
 * back (by sending a paint event).
 */
void MyImagePanel::paintNow()
{
    // depending on your system you may need to look at double-buffered dcs
    wxClientDC dc(this);
    render(dc);
}

/*
 * Here we do the actual rendering. I put it in a separate
 * method so that it can work no matter what type of DC
 * (e.g. wxPaintDC or wxClientDC) is used.
 */
void MyImagePanel::render(wxDC&  dc)
{
    PDEBUG ("called.\n");
    dc.Clear();
    if (fn.Length() > 4) { // XXX.png|jpg|jpeg
        long type;
        if (fn.Find(_T("jpg")) != wxNOT_FOUND ||
            fn.Find(_T("jpeg")) != wxNOT_FOUND||
            fn.Find(_T("JPG")) != wxNOT_FOUND ||
            fn.Find(_T("JPEG"))!= wxNOT_FOUND) {
            PDEBUG ("Filetype: jpeg!\n");

            type = wxBITMAP_TYPE_JPEG;
        }
        else if (fn.Find(_T("png")) != wxNOT_FOUND ||
                 fn.Find(_T("PNG")) != wxNOT_FOUND) {
            PDEBUG ("Filetype: png!\n");

            type = wxBITMAP_TYPE_PNG;
        }
        else if (fn.Find(_T("gif")) != wxNOT_FOUND ||
                 fn.Find(_T("GIF")) != wxNOT_FOUND){
            type = wxBITMAP_TYPE_GIF;
        }
        else if (fn.Find(_T("xpm")) != wxNOT_FOUND ||
                 fn.Find(_T("XPM")) != wxNOT_FOUND){
            type = wxBITMAP_TYPE_XPM;
            PDEBUG ("FileType: Image, xpm!\n");
        }
        else
            type = wxBITMAP_TYPE_ANY;
        image = wxImage(fn, type);
        int h, w, x, y;
        h = image.GetHeight();
        w = image.GetWidth();
        x = ((wxWindow *)GetParent())->GetSize().GetWidth();
        y = ((wxWindow *)GetParent())->GetSize().GetHeight();
        if (h > y || w > x)
            image=wxImage(fn, type).Scale(x, y);
    }
    else {
        image = wxImage(250, 250, false);
    }

    dc.DrawBitmap( image, 0, 0, false );
    PDEBUG ("end\n");
}

BEGIN_EVENT_TABLE(MyImagePanel, wxPanel)
EVT_PAINT(MyImagePanel::paintEvent)
END_EVENT_TABLE()


TextViewer::TextViewer(wxWindow *parent, string path, int length): \
wxFrame(parent, -1, _("TextViewer: ")+wxString(path.c_str(), wxConvUTF8),
        wxDefaultPosition, wxSize(750,920))
{
    long style = wxTE_READONLY|wxTE_MULTILINE|wxTE_RICH;
    txt = new wxTextCtrl(this, -1, _(""), wxDefaultPosition,
                         wxDefaultSize, style);
    wxFont font = wxFont(11, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL,
                         wxFONTWEIGHT_NORMAL);

    txt->SetDefaultStyle(wxTextAttr(wxColour(0,0,0),
                                    wxColour(255,255,255), font));
    txt->SetFont(font);
    wxString val;
    if (length == 0)
        val = _("Empty!");
    else if (length > 1024*1024) {
        wxMessageDialog *dlg = \
            new wxMessageDialog(this,
                                _("File is larger than 1M, Realy open it?"),
                                _("Warnning"), wxID_OK|wxID_CANCEL);
        int ret = dlg->ShowModal();
        if (ret == wxID_CANCEL)
            val = _("Skipped for big file");
    }

    if (val.IsEmpty()){
        val = wxString(get_content(path).c_str(), wxConvUTF8);
        if (val.IsEmpty()) {
            string output("/tmp/wcmd_txt_view");
            string cmd = "hexdump -C \"" + path + "\" > " + output;
            if (system(cmd.c_str()) != 0)
                val = _("Failed to read content!");
            else {
                val = wxString(get_content(output).c_str(),wxConvUTF8);
                unlink(output.c_str());
            }
        }
    }
    txt->SetValue(val);
    txt->ShowPosition(0);
    SetIcon(wxIcon(wxviewer, wxBITMAP_TYPE_XPM));
    Show(true);
}


InfoViewer::InfoViewer(wxWindow *parent, string path): \
    wxFrame(parent, -1, _("InfoViewer: ")+wxString(path.c_str(), wxConvUTF8),
    wxDefaultPosition, wxSize(750,300))
{
    long style = wxTE_READONLY|wxTE_MULTILINE|wxTE_RICH;
    txt = new wxTextCtrl(this, -1, _(""), wxDefaultPosition,
                         wxDefaultSize, style);
    wxFont font = wxFont(11, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL,
                         wxFONTWEIGHT_NORMAL);

    txt->SetDefaultStyle(wxTextAttr(wxColour(0,0,0),
                                    wxColour(255,255,255), font));
    txt->SetFont(font);

    wxString val;
    string output("/tmp/wcmd_info_view");
    string cmd = "stat \"" + path + "\" > " + output;
    cmd += "&& echo \"FileType:\" >> " + output;
    cmd += "&& file \"" + path + "\" >> " + output;
    if (system(cmd.c_str()) != 0)
        val = _("Failed to read content!");
    else {
        val = wxString(get_content(output).c_str(),wxConvUTF8);
        unlink(output.c_str());
    }
    txt->SetValue(val);
    txt->ShowPosition(0);
    SetIcon(wxIcon(wxviewer, wxBITMAP_TYPE_XPM));
    Show(true);
}


/*
 * Editor modelines
 *
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 *
 * ex: set shiftwidth=4 tabstop=4 expandtab
 * :indentSize=4:tabSize=4:noTabs=true:
 */
