#include "mainframe.h"
#include "utils.h"


MainFrame::MainFrame(const wxString& title, char ** args): \
    wxFrame( NULL, -1, title, wxDefaultPosition, wxSize(1200,850))
{
    PDEBUG ("called.\n");

    // Set Size;
    int x, y;
    string tmp = config.get_config("auto_size_x");
    if (tmp.empty())
        x = 1200;
    else
        x = atoi(tmp.c_str());

    tmp = config.get_config("auto_size_y");
    if (tmp.empty())
        y = 850;
    else
        y = atoi(tmp.c_str());

    SetSize(wxSize(x, y));
    // Menubar
    create_menubar();

    wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    // Toobar
    wxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    wxBitmapButton *button = new wxBitmapButton(this, ID_View,
                                                wxBitmap(btn_view));
    hbox->Add(button, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);

    button = new wxBitmapButton(this, ID_Edit, wxBitmap(btn_edit));
    hbox->Add(button, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);

    button = new wxBitmapButton(this, ID_Copy, wxBitmap(btn_copy));
    hbox->Add(button, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);

    button = new wxBitmapButton(this, ID_Move, wxBitmap(btn_move));
    hbox->Add(button, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);

    button = new wxBitmapButton(this, ID_Delete, wxBitmap(btn_delete));
    hbox->Add(button, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);

    button = new wxBitmapButton(this, ID_Terminal, wxBitmap(btn_terminal));
    hbox->Add(button, 0, wxEXPAND|wxLEFT|wxRIGHT, 10);

    sizer->Add(hbox, 0, wxEXPAND|wxTOP|wxBOTTOM, 2);

    fs = new FileSelector(this, args);
    sp1 = fs->sp1;
    sp2 = fs->sp2;
    active_id = ID_Sp1;
    sizer->Add(fs, 1, wxEXPAND|wxALL, 5);
    this->SetSizer(sizer);
    SetIcon(wxIcon(wxcommandor, wxBITMAP_TYPE_XPM));
    Centre();
}

MainFrame::~MainFrame()
{
    wxSize size = GetSize();
    int x = size.GetWidth();
    int y = size.GetHeight();
    char tmp[8] = {'\0'};
    sprintf(tmp, "%d", x);
    config.set_config("auto_size_x", string(tmp));
    memset(tmp, 0, 8);
    sprintf(tmp, "%d", y);
    config.set_config("auto_size_y", string(tmp));
    config.set_config("auto_last_path_l", sp1->get_cwd());
    config.set_config("auto_last_path_r", sp2->get_cwd());
}


void MainFrame::create_menubar()
{
    wxMenu *menu = new wxMenu;
    menuBar = new wxMenuBar;

    // File
    menu->Append( ID_Quit, _("E&xit") );

    menuBar->Append( menu, _("&File") );

    // View
    menu = new wxMenu;
    menu_item_view_hidden = menu->AppendCheckItem(ID_View_ShowHidden,
                                                  _("&Show Hidden Files"));
    menuBar->Append(menu, _("&View") );

    // Edit
    menu = new wxMenu;
    menu->Append(ID_Option, _("&Otions"));
    menuBar->Append(menu, _("&Edit") );

    bookmark_menu = new wxMenu;
    menuitem = new wxMenuItem(bookmark_menu, ID_BookmarkAdd,
                                          _("Add Current Directory"));
    menuitem->SetBitmap(wxBitmap(bookmark_add));
    bookmark_menu->Append(menuitem);

    menuitem = new wxMenuItem(bookmark_menu, ID_BookmarkEdit,
                              _("Edit Bookmarks"));
    menuitem->SetBitmap(wxBitmap(bookmark_mgt));
    bookmark_menu->Append(menuitem);
    bookmark_menu->AppendSeparator();
    if (! bookmarks.empty()) {
        int i;
        for (i = 0; i < bookmarks.size(); i++) {
            Append_Bookmark(ID_BookmarkAdd + i + 1, bookmarks[i]);
        }
    }
    menuBar->Append(bookmark_menu, _("Bookmarks"));

    // Help
    menu = new wxMenu;
    menu->Append( ID_About, _("&About...") );
    menuBar->Append(menu, _("&Help") );
    SetMenuBar( menuBar );
}

void MainFrame::BookmarAdd()
{
    Freeze();
    string path = get_sp()->get_cwd();
    bookmarks.push_back(path);
    Append_Bookmark(bookmarks.size() + ID_BookmarkAdd + 1, path);
    Thaw();
}

void MainFrame::Append_Bookmark(int id, string item)
{
    menuitem = new wxMenuItem(bookmark_menu, id,
                              wxString(item.c_str(), wxConvUTF8));
    menuitem->SetBitmap(wxBitmap(folder));
    bookmark_menu->Append(menuitem);
    Connect(id, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( MainFrame::OnBookmarkClicked));
    menuBar->Refresh();
}

void MainFrame::OnBookmarkClicked(wxCommandEvent &evt)
{
    int idx =  evt.GetId() - ID_BookmarkAdd - 1;
    get_sp()->set_cwd(bookmarks[idx]);
    get_sp()->update_list(-1);
    Thaw();
}


void MainFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(TRUE);
}

void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxAboutDialogInfo info;
    info.SetName(_("wcmd"));
    info.SetVersion(_("0.1.0"));
    info.SetDescription(_("wcmd is a simple file manaber writen with\
 wxwidgets."));
    info.SetCopyright(_T("(C) 2010 Yang, Yingchao <yangyingchao@gmail.com>"));
    wxAboutBox(info);
}

void MainFrame::OnOption (wxCommandEvent& WXUNUSED(event))
{
    PDEBUG ("called.\n");
    PrefDialog *pref = new PrefDialog(this, _("Preferences"));
    if (pref->ShowModal() == wxID_OK)
        update_fs();
    delete(pref);
}


void MainFrame::OnView(wxCommandEvent & event)
{
    PDEBUG ("Called\n");
}

void MainFrame::OnEdit(wxCommandEvent & event)
{
    PDEBUG ("Called\n");
    get_sp()->edit_file();
}

void MainFrame::OnMove(wxCommandEvent & event)
{
    PDEBUG ("Called\n");
    copy_or_move(false);
}
void MainFrame::OnCopy(wxCommandEvent & event)
{
    PDEBUG ("Called\n");
    copy_or_move();
}


void MainFrame::OnDelete(wxCommandEvent & event)
{
    PDEBUG ("Called\n");
    get_sp()->delete_file();
    event.Skip();
}

void MainFrame::ShowHidden()
{
    PDEBUG ("ShowHidden? %s\n", config.get_config("show_hidden").c_str());

    if (config.get_config("show_hidden") == "false") {
        config.set_config("show_hidden", "true");
        menu_item_view_hidden -> Check(true);
    }
    else {
        config.set_config("show_hidden", "false");
        menu_item_view_hidden -> Check(false);
    }
    PDEBUG ("ShowHidden? %s\n", config.get_config("show_hidden").c_str());

    update_fs();
}

void MainFrame::Show_Hidden(wxCommandEvent &evt)
{
    ShowHidden();
    evt.Skip();
}

void MainFrame::OnBookmarkAdd(wxCommandEvent &evt)
{
    BookmarAdd();
    evt.Skip();
}


void MainFrame::OnBookmarkEdit(wxCommandEvent &evt)
{
    BookmarkManage *mgt = new BookmarkManage(this);
    int ret;
    ret = mgt->ShowModal();
    delete(mgt);
    if (ret == ID_BookmarkRedraw) {
        OnBookmarkEdit(evt);
    }
    else {
        int i;
        wxMenuItemList list = bookmark_menu->GetMenuItems();
        if (!list.empty()) // Remove old menu items.
            for (i = 0; i < list.size(); i++)
                bookmark_menu->Remove((wxMenuItem *)list[i]);

        if (! bookmarks.empty()) // Add new menu items.
            for (i = 0; i < bookmarks.size(); i++)
                Append_Bookmark(ID_BookmarkAdd + i + 1, bookmarks[i]);
    }
}



void MainFrame::compare_items()
{
    string cmd = config.get_config("diff_tool");
    if (cmd.empty()) {
        wxMessageDialog *ddlg = \
            new wxMessageDialog(this, _("Diff tool is not configured!"),
                                _("Error"), wxOK);
        ddlg->ShowModal();
        delete(ddlg);
        return ;
    }
    cmd += " \"" + sp1->get_selected_item() + "\" \"" + \
        sp2->get_selected_item()+"\"";

    FuncHelper *func = new FuncHelper();
    func->Create((wxWindow *)this, cmd);
    func->Run();
    return;
}

void MainFrame::OnThreadCompletion(wxCommandEvent& event)
{
    PDEBUG ("called.\n");
    update_fs();
}

void MainFrame::open_in_other()
{
    string path = get_sp()->get_selected_item();
    get_sp_o()->set_cwd(path);
    get_sp_o()->update_list(-1);
    get_sp_o()->focus_list();
}

FSDisplayPane *MainFrame::get_sp()
{
    PDEBUG ("called.\n");

    if (active_id == ID_Sp1)
        return sp1;
    else
        return sp2;
}

void MainFrame::exchange_sp()
{
    PDEBUG ("called:\n");

    if (active_id == ID_Sp1)
        active_id = ID_Sp2;
    else
        active_id = ID_Sp1;
    PDEBUG ("After: %d\n", active_id);
}

FSDisplayPane *MainFrame::get_sp_o()
{
    if (active_id == ID_Sp1)
        return sp2;
    else
        return sp1;
}

void MainFrame::update_fs(int idx1, int idx2, wxWindowID id)
{
    PDEBUG ("called.\n");
    if (id == -1) // Get active_id first, it will be changed in "update_list"
        id = active_id;
    sp1->update_list(idx1==-1?sp1->cur_idx:idx1);
    sp2->update_list(idx2==-1?sp2->cur_idx:idx2);
    // Restore active_id.
    if (id == ID_Sp1){
        sp1->SetFocus();
        active_id = id;
    }
    else {
        active_id = id;
        sp2->SetFocus();
    }
    PDEBUG ("leave.\n");
}

void MainFrame::set_active_sp(wxWindowID id)
{
    PDEBUG ("active id: %d\n", id);
    active_id = id;
}

void MainFrame::OpenTerminal(wxCommandEvent &evt)
{
    get_sp()->open_terminal();
}

void MainFrame::copy_or_move(bool copy)
{
    PDEBUG ("called.\n");
    vector<string> src_list;
    string dest = get_sp_o()->get_cwd();
    if (get_sp()->get_selected_files(src_list) || src_list.empty()) {
        wxMessageDialog *dlg = \
            new wxMessageDialog(this, _("Failed to get selected files!"),
                                _("Error"), wxOK);
        dlg->ShowModal();
        delete(dlg);
        return ;
    }
    vector<string>::iterator iter;
    wxString msg;
    for (iter = src_list.begin(); iter < src_list.end(); iter++) {
        PDEBUG ("SRC: %s\n", (*iter).c_str());
        dest += "/";
        copy_or_move_single(*iter, dest, copy);
    }
    int idx = get_sp()->cur_idx;
    if (!copy) {
        idx--;
        if (idx < 0)
            idx = 0;
    }
}

/**
 * @name copy_or_move_single - Copy or move file from src to dst.
 * @param src -  src
 * @param dst -  dst
 * @param copy - Flag copy, true to copy, false to move.
 * @return int: 0  - success.
 *              -2 - Source or desty is empty.
 */
int MainFrame::copy_or_move_single(string &src, string &dest, bool copy)
{
    PDEBUG ("called.\n");
    int ret = 0;
    if (src.empty() || dest.empty()) {
        fprintf(stderr, "ERROR: Source or desty is empty!\n");
        return -2;
    }
    size_t pos = src.rfind("/");
    string fake_dest;
    wxMessageDialog *dlg;
    wxString msg = wxString(fake_dest.c_str(), wxConvUTF8);
    wxWindowID id;
    fake_dest.assign(src, pos+1, src.length()-pos);
    fake_dest = dest + "/" + fake_dest;
    PDEBUG ("src: %s, fake: %s\n", src.c_str(), fake_dest.c_str());
    if (is_file_exist(fake_dest)) {
        msg += _(" already exited! Overwrite?");
        dlg = new wxMessageDialog(this, msg, _("Overwrite"), wxID_OK|wxID_CANCEL);
        id = dlg->ShowModal();
        if (id == wxID_CANCEL){
            delete(dlg);
            return 0;
        }
    }
    string cmd;
    cmd =  "cp -aRf \"" + src + "\"  \"" +  dest +"\"";
    if (!copy)
        cmd += "&& rm -rf \"" + src + "\"";
    FuncHelper *func = new FuncHelper();
    func->Create((wxWindow *)this, cmd);
    func->Run();
    return ret;
}

void MainFrame::show_file_info()
{
    string path = get_sp()->get_selected_item();
    InfoViewer *info = new InfoViewer(this, path);
    info->Show();
}

DEFINE_EVENT_TYPE(wxEVT_MY_EVENT)


BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(ID_Quit, MainFrame::OnQuit)
EVT_MENU(ID_About, MainFrame::OnAbout)
EVT_MENU(ID_Option, MainFrame::OnOption)
EVT_MENU(ID_BookmarkAdd, MainFrame::OnBookmarkAdd)
EVT_MENU(ID_BookmarkEdit, MainFrame::OnBookmarkEdit)
EVT_MENU(ID_View_ShowHidden, MainFrame::Show_Hidden)
EVT_BUTTON(ID_View, MainFrame::OnView)
EVT_BUTTON(ID_Edit, MainFrame::OnEdit)
EVT_BUTTON(ID_Copy, MainFrame::OnCopy)
EVT_BUTTON(ID_Delete, MainFrame::OnDelete)
EVT_BUTTON(ID_Terminal, MainFrame::OpenTerminal)
EVT_COMMAND  (-1, wxEVT_MY_EVENT, MainFrame::OnThreadCompletion)
END_EVENT_TABLE()



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
