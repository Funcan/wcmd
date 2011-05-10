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
#include <wx/artprov.h>

#include "global.h"
#include "misc.h"
#include "utils.h"
#include "mylistctrl.h"

extern  wxWindowID active_id;
void init_imglist();


class FSDisplayPane: public wxPanel {
public:
    FSDisplayPane(wxWindow *parent, wxWindowID id, wxString &path);
    virtual ~FSDisplayPane(){};
    int    cur_idx;
    int    delete_files();
    int    edit_file(bool create=false);
    int    open_terminal();
    int    view_file();
    void   goto_parent_dir();
    void   goto_dir();
    int    wrap_open(wxString &path, bool create);
    wxString get_selected_item();
    void   focus_first();
    void   focus_last();
    void   focus_next();
    void   focus_prev();
    void   rename_file();
    void   select_all();
    void   set_cwd(wxString path);
    void   set_selected();
    void   update_list(int selected_item, bool reload_dir=true);
    void   show_list(int selected_item, wxString filter=_(""));
    int    get_selected_files(vector<wxString> &list);
    int    get_selected_files(vector<ItemEntry *> &list);
    void   select_same_ext();
    void   deselect_same_ext();
    void   focus_list();
    void   toggle_search();
    void   create_dir();
    void   sort_and_show(int idx);
    void   activate_item(int idx);
    void   OnAsyncTermination(bool up_flag = false, bool err_flag = false,
                              wxString cmd=_(""));
    int    do_async_execute(const wxString &cmd);
    int    compress_files();
    void   update_dir_info();
    void   set_focus();
    const wxString get_cwd();
    wxString          cwd,  old_path;
private:
    int       cur_sort;
    void restore_cwd();
    void delete_single_file(wxString &path);
    void OnColumbDrag(wxListEvent &evt);
    void OnItemSelected(wxListEvent &evt);
    void show_err_dialog();
    void OnKeydown(wxListEvent &evt);
    void OnMySort(wxListEvent &evt);
    void OnTextChanged(wxCommandEvent &evt);
    void OnTextEnter(wxCommandEvent &evt);
    void item_activated(wxListEvent &evt);
    void toggle_color(int idx, bool hicolor);
    void clean_resource();
    void process_right_click(wxMouseEvent &evt);
    int __open_with_plugin(const char *file_name, const char *plugin_path);
    int open_with_plugin(const char *file_name);
    int get_cur_filelist();

    MyListCtrl     *lst;
    int             item_count;
    wxDir *dir;
    wxStaticText   *cwd_info, *dirinfo;
    wxMessageDialog *dlg;
    wxTextCtrl     *quick_search;
    vector<ItemEntry *>::iterator  iter;
    vector<ItemEntry *>  file_list;
    vector<ItemEntry *>  selected_list;
    vector<ItemEntry *>  tmp_list;
    vector<ItemEntry *>  cur_list;
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
