#ifndef _MYFRAME_H_
#define _MYFRAME_H_

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
#include "resources/mimetype/folder.xpm"
#include "resources/mimetype/generic.xpm"
#include "resources/mimetype/image.xpm"
#include "utils.h"
#include "resources/wxcommandor.xpm"
#include "resources/bookmark_add.xpm"
#include "resources/bookmark_mgt.xpm"
#include "resources/buttons/btn_copy.xpm"
#include "resources/buttons/btn_cut.xpm"
#include "resources/buttons/btn_move.xpm"
#include "resources/buttons/btn_delete.xpm"
#include "resources/buttons/btn_edit.xpm"
#include "resources/buttons/btn_new.xpm"
#include "resources/buttons/btn_open.xpm"
#include "resources/buttons/btn_paste.xpm"
#include "resources/buttons/btn_view.xpm"
#include "resources/buttons/btn_terminal.xpm"


class FSDisplayPane: public wxPanel {
public:
    FSDisplayPane(wxWindow *parent, wxWindowID id, string &path);
    virtual ~FSDisplayPane(){};
    int    cur_idx;
    int    delete_file();
    int    edit_file(bool create=false);
    int    open_terminal();
    int    view_file();
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
private:
    void delete_single_file(string &path);
    void OnColumbDrag(wxListEvent &evt);
    void OnItemSelected(wxListEvent &evt);
    void OnKeydown(wxListEvent &evt);
    void OnMySort(wxListEvent &evt);
    void OnSetFocus(wxFocusEvent &evt);
    void OnTextChanged(wxCommandEvent &evt);
    void OnTextEnter(wxCommandEvent &evt);
    void item_activated(wxListEvent &evt);
    void toggle_color(int idx, bool hicolor);
    void clean_resource();

    vector<item *>::iterator iter;
    wxListCtrl     *lst;
    int             item_count;
    DIR *dirp;
    DIR *dirp_old;
    wxStaticText   *cwd_info, *dirinfo;
    wxTextCtrl     *quick_search;
    string          cwd;
    string          old_path;
    vector<item *>  file_list;
    vector<item *>  selected_list;
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




class MainFrame: public wxFrame
{
public:

    MainFrame(const wxString& title, char **args);
    virtual ~MainFrame();


    // Handlers
    void ShowHidden();
    void BookmarAdd();
    void update_fs(int idx1=-1, int idx2=-1, wxWindowID id=-1);
    void delete_file();
    void open_terminal();
    void set_active_sp(wxWindowID id);
    FSDisplayPane *get_sp_o();
    FSDisplayPane *get_sp();
    void copy_or_move(bool copy=true);
    int copy_or_move_single(string &src, string &dest, bool copy);
    void exchange_sp();
    void open_in_other();
    void compare_items();
    void OnThreadCompletion(wxCommandEvent& event);
    void OnPositionChanged(wxSplitterEvent& event);

private:
    FSDisplayPane *sp1;
    FSDisplayPane *sp2;

    void Append_Bookmark(int id, string item);
    void Show_Hidden(wxCommandEvent &evt);
    void OnBookmarkAdd(wxCommandEvent &evt);
    void OnBookmarkEdit(wxCommandEvent &evt);
    void OnBookmarkClicked(wxCommandEvent &evt);
    void create_menubar();
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnOption (wxCommandEvent& event);
    void OnView(wxCommandEvent & event);
    void OnEdit(wxCommandEvent & event);
    void OnMove(wxCommandEvent & event);
    void OnDelete(wxCommandEvent & event);
    void OnCopy(wxCommandEvent & event);
    void OpenTerminal(wxCommandEvent & event);
    vector<string> bookmark_list;
    wxMenu *bookmarks;
    wxMenuBar *menuBar;
    FileSelector *fs;
    wxMenuItem *menu_item_view_hidden;
    DECLARE_EVENT_TABLE()
};



class FuncHelper: public wxThread
{
public:
    void Create(MainFrame *fm, string &path)
    {
        PDEBUG ("called, cmd: %s\n", path.c_str());
        cmd = path + " >/dev/null 2>&1";
        mainframe = fm;
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
    MainFrame *mainframe;
};

#define GetMainFrame()  (MainFrame *)GetParent()->GetParent()


#endif /* _MYFRAME_H_ */
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
