#ifndef _MISC_H_
#define _MISC_H_

#include <limits.h>
#include <stdlib.h>
#include <wx/wx.h>
#include <wx/dialog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <wx/gbsizer.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "wxconfig.h"
#include "utils.h"
#include "global.h"

using namespace std;

class SimpleDialog : public wxDialog {
public:
    SimpleDialog(wxWindow *parent, wxString title,
                 const wxPoint &pt=wxDefaultPosition);
    void post_draw();
    virtual void draw() {printf("Base class drawing\n");};
    virtual void OnOK(wxCommandEvent &evt){}
    void OnCancel(wxCommandEvent &evt) {};
    virtual ~SimpleDialog() {};
    wxSizer *sizer;
};


class DirnameDlg:public SimpleDialog {
public:
    DirnameDlg(wxWindow *parent, wxString title, wxString content=_(""));
    virtual void draw();
    virtual void OnOK(wxCommandEvent &evt);
    virtual void OnCancel(wxCommandEvent &evt);
    virtual ~DirnameDlg() {};
    wxString fn;
private:
    wxTextCtrl *txt;
    DECLARE_EVENT_TABLE()
};

class CompressDlg:public SimpleDialog {
public:
    CompressDlg(wxWindow *parent, wxString title, wxString content=_(""));
    virtual void draw();
    virtual void OnOK(wxCommandEvent &evt);
    virtual void OnCancel(wxCommandEvent &evt);
    virtual ~CompressDlg() {};
    wxString fn;
    int type;
private:
    wxTextCtrl *txt;
    wxChoice *choice;
    void OnChoice(wxCommandEvent &evt);
    DECLARE_EVENT_TABLE()
};


class PrefDialog:public SimpleDialog {
public:
    PrefDialog(wxWindow *parent, wxString title);
    virtual void draw();
    virtual void OnOK(wxCommandEvent &evt);
    virtual void OnCancel(wxCommandEvent &evt);
    virtual ~PrefDialog() {};
    wxString fn;
private:
    wxTextCtrl *txt;
    DECLARE_EVENT_TABLE()
};

class BookmarkManage: public SimpleDialog {
public:
    BookmarkManage(wxWindow *parent, wxString title=_("Bookmark Management"));
    virtual ~BookmarkManage() {};
    virtual void draw();
    virtual void OnOK(wxCommandEvent &evt);
    virtual void OnCancel(wxCommandEvent &evt);

private:
    void OnAdd(wxCommandEvent &evt);
    void OnDel(wxCommandEvent &evt);
    void post_draw();
    void redraw();
    vector<wxCheckBox *> check_list;
    vector<wxStaticText *> label_list;
    vector<wxButton *> btn_list;
    vector<wxBoxSizer *> box_list;
    DECLARE_EVENT_TABLE()

};

class TextEntry:public wxPanel {
public:
    TextEntry(wxWindow *parent, string key);
    virtual ~TextEntry(){};
    void OnBtn(wxCommandEvent &evt);
private:
    string key;
    wxTextCtrl *txt;
    DECLARE_EVENT_TABLE()
};

class BoolEntry:public wxPanel {
public:
    BoolEntry(wxWindow *parent, string key);
    virtual ~BoolEntry(){};
    void OnBtn(wxCommandEvent &evt);
private:
    string key;
    wxRadioButton *btn1, *btn2;
    DECLARE_EVENT_TABLE()
};


class MyThreadFunc: public wxThread {
public:
    MyThreadFunc(const char *fn, const char *pp);
    virtual ~MyThreadFunc(){};
    void *Entry();
    void OnExit();
private:
    char file_name[1024];
    char plugin_path[1024];
};
#endif /* _MISC_H_ */
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
