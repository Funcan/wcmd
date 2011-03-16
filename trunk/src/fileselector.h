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
#include <wx/wx.h>
#include <wx/iconloc.h>

#include "global.h"
#include "misc.h"
#include "utils.h"
#include "mylistctrl.h"


#define char2wxstr(str) wxString(str, wxConvUTF8)
#define str2wxstr(str)  wxString(str.c_str(), wxConvUTF8)

extern  wxWindowID active_id;
void init_imglist();


class FSDisplayPane: public wxPanel {
public:
    FSDisplayPane(wxWindow *parent, wxWindowID id, string &path);
    virtual ~FSDisplayPane(){};
    int    cur_idx;
    int    delete_files();
    int    edit_file(bool create=false);
    int    open_terminal();
    int    view_file();
    void   goto_parent_dir();
    void   goto_dir();
    int    wrap_open(wxString &path, bool create);
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
    void   show_list(int selected_item, wxString filter=_(""));
    int    get_selected_files(vector<string> &list);
    void   select_same_ext();
    void   deselect_same_ext();
    void   focus_list();
    void   toggle_search();
    void   create_dir();
    void   sort_and_show(int idx);
    void   activate_item(int idx);
    void   OnAsyncTermination(bool up_both_fs = false);
    int    do_async_execute(const wxString &cmd);
    void   update_dir_info();
    const string get_cwd();
private:
    int       cur_sort;
    void               delete_single_file(string &path);
    void               OnColumbDrag(wxListEvent &evt);
    void               OnItemSelected(wxListEvent &evt);
    void               OnKeydown(wxListEvent &evt);
    void               OnMySort(wxListEvent &evt);
    void               OnTextChanged(wxCommandEvent &evt);
    void               OnTextEnter(wxCommandEvent &evt);
    void               item_activated(wxListEvent &evt);
    void               toggle_color(int idx, bool hicolor);
    void               clean_resource();
    void               process_right_click(wxMouseEvent &evt);
    void               show_err_dialog();
    unsigned long long WX_2_LL(wxLongLong n);

    MyListCtrl     *lst;
    int             item_count;
    DIR            *dirp;
    DIR            *dirp_old;
    wxStaticText   *cwd_info, *dirinfo;
    wxMessageDialog *dlg;
    wxTextCtrl     *quick_search;
    string          cwd;
    string          old_path;
    vector<item *>::iterator  iter;
    vector<item *>  file_list;
    vector<item *>  selected_list;
    vector<item *>  tmp_list;
    vector<item *>  cur_list;
    vector<int>     sel_idx;
    wxString        cur_target, old_target, msg, cmd, title;
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
    void update_fs();
};


DECLARE_EVENT_TYPE(wxEVT_MY_EVENT, -1)

#define GetMainFrame() GetParent()->GetParent()


#endif                          /* _FILESELECTOR_H_ */
/*
 * Editor modelines
 *
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 *
 * ex: set shiftwidth = 4 tabstop=4 expandtab
 * :indentSize        = 4:tabSize=4:noTabs=true:
 */
