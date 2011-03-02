#ifndef _FILESELECTOR_H_
#define _FILESELECTOR_H_
#include <algorithm>
#include <cctype>
#include <dirent.h>
#include <errno.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <wx/aboutdlg.h>
#include <wx/event.h>
#include <wx/imaglist.h>
#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/thread.h>
#include <wx/wx.h>

#include "global.h"
#include "misc.h"
#include "utils.h"

#include "resources/mimetype/folder.xpm"
#include "resources/mimetype/generic.xpm"
#include "resources/mimetype/image.xpm"

extern  wxWindowID active_id;

class MyListCtrl: public wxListCtrl {

public:
    MyListCtrl(wxWindow *parent, wxWindowID id);
    ~MyListCtrl();
private:
    void OnPopupClick(wxCommandEvent &evt);
    void OnTip(wxMouseEvent &evt);
    void process_right_click(wxMouseEvent &evt);
    DECLARE_EVENT_TABLE()
};

class FSDisplayPane: public wxPanel {
public:
    FSDisplayPane(wxWindow *parent, wxWindowID id, string &path);
    virtual ~FSDisplayPane(){};
    int    cur_idx;
    int    delete_file();
    int    edit_file(bool create=false);
    int    open_terminal();
    int    view_file();
    void  goto_parent_dir();
    void   goto_dir();
    int    wrap_open(string &path, bool create);
    const string get_cwd();
    string get_selected_item();
    void   focus_first();
    void   focus_last();
    void   focus_next();
    void   focus_prev();
    void   rename_file();
    void   select_all();
    void   set_cwd(string &path);
    void   set_selected();
    void   update_list(int selected_item, bool reload_dir=true);
    int    get_selected_files(vector<string> &list);
    void   select_same_ext();
    void   deselect_same_ext();
    void  focus_list();
    void toggle_search();
    void OnTip(wxMouseEvent &evt);
    void create_dir();
    void real_sort(int idx);
private:
    void delete_single_file(string &path);
    void OnColumbDrag(wxListEvent &evt);
    void OnItemSelected(wxListEvent &evt);
    void OnKeydown(wxListEvent &evt);
    void OnMySort(wxListEvent &evt);
    void OnTextChanged(wxCommandEvent &evt);
    void OnTextEnter(wxCommandEvent &evt);
    void item_activated(wxListEvent &evt);
    void toggle_color(int idx, bool hicolor);
    void clean_resource();
    void activate_item(int idx);
    void process_right_click(wxMouseEvent &evt);
    vector<item *>::iterator iter;
    MyListCtrl     *lst;
    int             item_count;
    DIR *dirp;
    DIR *dirp_old;
    wxStaticText   *cwd_info, *dirinfo;
    wxTextCtrl     *quick_search;
    string          cwd;
    string          old_path;
    vector<item *>  file_list;
    vector<item *>  selected_list;
    vector<item *>  filtered_list;
    vector<item *>  tmp_list;
    vector<int>     sel_idx;
    wxFont          font;
    wxColour        fg_hi_col;
    wxColour        bg_hi_col;
    wxColour        bg_def_col;
    wxColour        fg_def_col;
    DECLARE_EVENT_TABLE()
};


class FileSelector: public wxSplitterWindow
{
public:
    FileSelector(wxWindow *parent, char **args);
    virtual ~FileSelector();
    FSDisplayPane *sp1;
    FSDisplayPane *sp2;
};


DECLARE_EVENT_TYPE(wxEVT_MY_EVENT, -1)


class FuncHelper: public wxThread
{
public:
    void Create(wxWindow *wn, string &path)
    {
        PDEBUG ("called, cmd: %s\n", path.c_str());
        cmd = path + " >/dev/null 2>&1";
        this->wn = wn;
        wxThread::Create();
        PDEBUG ("Exit\n");
    };
	void OnExit()
    {
        PDEBUG ("Leave thread!\n");
        return;
	}
    void Run()
    {
        PDEBUG ("called.\n");
        wxThread::Run();
        PDEBUG ("bye.\n");
    }
private:
	void* Entry();
    string cmd;
    wxWindow *wn;
};


#define GetMainFrame()  GetParent()->GetParent()


#endif /* _FILESELECTOR_H_ */
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
