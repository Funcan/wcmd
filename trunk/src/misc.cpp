#include "misc.h"
#include "fileselector.h"
// #include "resources/wxviewer.xpm"
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/wait.h>

static wxPoint bookmark_point(-1, -1);

SimpleDialog:: SimpleDialog(wxWindow *parent, wxString name, const wxPoint &pt) : \
    wxDialog(parent, -1, name, pt)
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


// Bookmark management
BookmarkManage::BookmarkManage(wxWindow *parent, wxString title):\
SimpleDialog(parent, title, bookmark_point)
{
    SetMinSize(wxSize(400, 300));
    draw();
    post_draw();
    Fit();
}

void BookmarkManage::draw()
{
    wxStaticText *label;
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *hbox;
    wxCheckBox *check;
    vector<string>::iterator iter;
    unsigned int idx = 0;
    wxString str;
    for (; idx < bookmarks.size(); idx++) {
        str.Printf(wxT("%002d. "), idx + 1);
        hbox = new wxBoxSizer(wxHORIZONTAL);
        check = new wxCheckBox((wxWindow *)this,
                               (wxWindowID)ID_BookmarkManage+idx, str);
        hbox->Add(check, 0, wxEXPAND|wxLEFT|wxRIGHT, 5);
        check_list.push_back(check);
        label = new wxStaticText(this, -1, wxString(bookmarks[idx].c_str(),
                                                    wxConvUTF8));
        hbox->Add(label, 0, wxEXPAND|wxLEFT|wxRIGHT, 5);
        vbox->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT, 5);
    }
    sizer->Add(vbox, 1, wxEXPAND|wxALL, 5);
}

void BookmarkManage::post_draw()
{
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    wxButton *btn = new wxButton(this, ID_BookmarkManageAdd, _("Add"));
    hbox->Add(btn, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);

    btn = new wxButton(this, ID_BookmarkManageDel, _("Delete"));
    hbox->Add(btn, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);

    btn = new wxButton(this, wxID_CANCEL, _("Cancel"));
    hbox->Add(btn, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);

    btn = new wxButton(this, wxID_OK, _("OK"));
    hbox->Add(btn, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);

    sizer->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);
}


void BookmarkManage::OnAdd(wxCommandEvent &evt)
{
    wxDirDialog *dlg = new wxDirDialog((wxWindow *)this,  _("Add a bookmark"),
                                       wxString(getenv("HOME"), wxConvUTF8),
                                       wxDD_DIR_MUST_EXIST);
    int ret = dlg->ShowModal();
    if (ret == wxID_OK) {
        wxString path = dlg->GetPath();
        string val = string(path.mb_str());

        unsigned int idx1;
        for (idx1 = 0; idx1 < bookmarks.size(); idx1++) {
            if (bookmarks[idx1] == val) {
                wxMessageDialog *ddlg = \
                    new wxMessageDialog(this, _("Bookmark existed!"),
                                        _("Error"), wxOK);
                ddlg->ShowModal();
                delete(ddlg);
                return ;
            }
        }
        bookmarks.push_back(val);
        config.dump2file();
        EndModal(ID_BookmarkRedraw);
    }
}

void BookmarkManage::OnDel(wxCommandEvent &evt)
{
    int idx = 0;
    for (idx = check_list.size() - 1; idx >= 0; idx--) {
        if (check_list[idx]->GetValue()) {
            bookmarks.erase(bookmarks.begin() + idx);
        }
    }
    config.dump2file();
    EndModal(ID_BookmarkRedraw);
}

void BookmarkManage::OnOK(wxCommandEvent &evt)
{
    config.dump2file();
    bookmark_point = this->GetPosition();
    EndModal(wxID_OK);
}


void BookmarkManage::OnCancel(wxCommandEvent &evt)
{
    bookmark_point = this->GetPosition();
    EndModal(wxID_CANCEL);
}

BEGIN_EVENT_TABLE(BookmarkManage, wxDialog)
EVT_BUTTON(wxID_OK, BookmarkManage::OnOK)
EVT_BUTTON(wxID_CANCEL, BookmarkManage::OnCancel)
EVT_BUTTON(ID_BookmarkManageAdd, BookmarkManage::OnAdd)
EVT_BUTTON(ID_BookmarkManageDel, BookmarkManage::OnDel)
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
    if (btn1->GetValue())
        config.set_config(this->key, "true");
    else
        config.set_config(this->key, "false");
}

BEGIN_EVENT_TABLE(BoolEntry, wxPanel)
EVT_RADIOBUTTON(-1, BoolEntry::OnBtn)
END_EVENT_TABLE()

MyThreadFunc::MyThreadFunc(const char *fn, const char *pp): \
    wxThread()
{
    memset(file_name, 0, 1024);
    memset(plugin_path, 0, 1024);
    strcpy(file_name, fn);
    strcpy(plugin_path, pp);
}

void *MyThreadFunc::Entry()
{
    void *handle;
    int (*func)(const char *);
    char *error;
    char fn[64], ext_name[64];
    char *base_name = basename(strdup(this->plugin_path));

    memset(fn, 0, 64);
    memset(ext_name, 0, 64);
    strncpy(ext_name, base_name, strlen(base_name) - 3);

    handle = dlopen(this->plugin_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        return NULL;
    }

    snprintf(fn, 5+strlen(base_name)-2, "open_%s", base_name);
    dlerror();    /* Clear any existing error */

    func = (int (*)(const char *)) dlsym(handle, fn);

    if ((error = dlerror()) != NULL)  {
        fprintf(stderr, "%s\n", error);
        return 0;
    }
    char *tmp = strdup(file_name);
    func((const char *)tmp);
    dlclose(handle);
    return NULL;
}

void MyThreadFunc::OnExit()
{
    delete this;
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
