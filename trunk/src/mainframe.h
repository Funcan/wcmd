#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

#include "fileselector.h"

class MainFrame: public wxFrame
{
public:

    MainFrame(const wxString& title, char **args);
    virtual ~MainFrame();

    // Handlers
    void ShowHidden();
    void BookmarAdd();
    void update_fs(int idx1=-1, int idx2=-1, wxWindowID id=-1);
    void delete_files();
    void open_terminal();
    void set_active_sp(wxWindowID id);
    FSDisplayPane *get_sp_o();
    FSDisplayPane *get_sp();
    wxString get_o_wd();
    void copy_or_move(bool copy=true);
    int copy_or_move_single(wxString &src, wxString &dest, bool copy);
    void exchange_sp();
    void open_in_other();
    void compare_items();
    void OnThreadCompletion(wxCommandEvent& event);
    void OnPositionChanged(wxSplitterEvent& event);
    void show_file_info();
    void update_status();
    void create_softlink();
private:
    FSDisplayPane *sp1;
    FSDisplayPane *sp2;

    wxSizer *sizer;

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
    void compress_files(wxCommandEvent &evt);
    void decompress_files(wxCommandEvent &evt);
    void create_displayer(char **args);
    void create_toolbar();
    void read_set_size();
    void Append_Servers();
    void OnServerActivated(wxCommandEvent &evt);
    vector<string> bookmark_list;
    wxMenu *bookmark_menu;
    wxMenuBar *menuBar;
    wxMenuItem *menu_item_view_hidden;
    wxMenuItem *menu_item_view_debug;
    wxMenuItem *menuitem;
    wxMenu *submenu_ssh;
    wxMenu *submenu_sftp;
    wxMenu *submenu_ftp;
    wxMenu *submenu_smb;
    DECLARE_EVENT_TABLE()
};






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
