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
#include "resources/wxviewer.xpm"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "wxconfig.h"
#include "global.h"

using namespace std;

class SimpleDialog : public wxDialog {
public:
    SimpleDialog(wxWindow *parent, wxString title);
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


class MyImagePanel : public wxPanel
{

public:
    MyImagePanel(wxWindow* parent, string path);
    wxBitmap image;
    void paintEvent(wxPaintEvent & evt);
    void paintNow();
    void render(wxDC& dc);

private:
    wxString fn;
    DECLARE_EVENT_TABLE()
};

class TextViewer: public wxFrame {
public:
    TextViewer(wxWindow *parent, string path, int length);
    virtual ~TextViewer() {PDEBUG ("called!\n"); };
private:
    wxTextCtrl *txt;
    // MyImagePanel *img;
    // string fpath;
    // int type;
    // DECLARE_EVENT_TABLE()
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
