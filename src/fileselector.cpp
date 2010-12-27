#include "fileselector.h"


wxWindowID active_id;


FileSelector:: FileSelector(wxWindow *parent, char **args): \
    wxSplitterWindow(parent, -1, wxDefaultPosition)
{
    string path1, path2;

    if (args[0] && strlen(args[0]))
        if (is_dir_exist(args[0]))
            path1=string(args[0]);

    if (args[1] && strlen(args[1]))
        if (is_dir_exist(args[1]))
            path2=string(args[1]);

    sp1 = new FSDisplayPane(this, ID_Sp1, path1);
    sp2 = new FSDisplayPane(this, ID_Sp2, path2);
    this->SplitVertically(sp1, sp2);
    this->Show(true);
    sp1->SetFocus();
    active_id = ID_Sp1;
}

FileSelector::~FileSelector()
{
}





FSDisplayPane::FSDisplayPane(wxWindow *parent, wxWindowID id, string &path): \
	wxPanel(parent, id)
{
    long style = wxLC_REPORT | wxBORDER_NONE | wxLC_EDIT_LABELS |\
        wxLC_SORT_ASCENDING;
    font = wxFont(11, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
                  wxFONTWEIGHT_NORMAL);
    SetFont(font);
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    wxPoint point;
    int height;
    sizer->Add(hbox, 0, wxEXPAND|wxALL, 0);

    dirp = NULL;
    dirp_old = NULL;
    cwd_info = new wxStaticText(this, -1, _(""));
    if (path.empty()) {
        if (id == ID_Sp1) {
            cwd = config.get_config("auto_last_path_l");
            if (!is_dir_exist(cwd))
                cwd = string(getenv("HOME"));
            cwd_info->SetLabel(wxString(cwd.c_str(), wxConvUTF8));
        }
        else {
            cwd = config.get_config("auto_last_path_r");
            if (!is_dir_exist(cwd))
                cwd = string(getenv("HOME"));
            cwd_info->SetLabel(wxString(cwd.c_str(), wxConvUTF8));
        }
    }
    else {
        cwd = path;
    }

    sizer->Add(cwd_info, 0, wxEXPAND|wxBOTTOM, 2);

    lst = new wxListCtrl(this, id+1, wxDefaultPosition,
                         wxDefaultSize, style );
    wxImageList *imageList = new wxImageList(22, 22);
    imageList->Add(wxIcon(folder, wxBITMAP_TYPE_XPM));
    imageList->Add(wxIcon(generic, wxBITMAP_TYPE_XPM));
    imageList->Add(wxIcon(image, wxBITMAP_TYPE_XPM));
    lst->AssignImageList(imageList, wxIMAGE_LIST_SMALL);

    lst->InsertColumn(0, _(""));
    lst->InsertColumn(1, _("        File Name"));
    lst->InsertColumn(2, _("Ext Name"));
    lst->InsertColumn(3, _("Size"));
    lst->InsertColumn(4, _("Last Modify"));
    lst->InsertColumn(5, _("Mode"));

    lst->SetColumnWidth(0, 25);
    lst->SetColumnWidth(1, 210);
    lst->SetColumnWidth(2, 72);
    lst->SetColumnWidth(3, 68);
    lst->SetColumnWidth(4, 118);
    lst->SetColumnWidth(5, 64);

    sizer->Add(lst, 1, wxEXPAND|wxALL, 0);

    dirinfo = new wxStaticText(this, -1, _("Directory info!"));
    sizer->Add(dirinfo, 0, wxEXPAND|wxALL, 5);
    this->SetSizer(sizer);

    point = GetPosition();
    height = (GetParent()->GetParent())->GetSize().GetHeight();
    PDEBUG ("Height: %d\n", height);
    point += wxPoint(0, height-140);
    quick_search = new wxTextCtrl(this, id+2, _(""), point, wxDefaultSize,
                                  wxTE_PROCESS_ENTER);
    quick_search->Show(false);
    this->Show(true);

    update_list(0);
    cur_idx = 0;
    fg_hi_col = wxColour(255, 255, 255);
    bg_hi_col = wxColour(0x20, 0x4a, 0x87);
    bg_def_col = wxColour(0xe7, 0xed, 0xf6);
    fg_def_col = wxColour(0,0,0);
}

void FSDisplayPane::update_list(int selected_item, bool reload_dir)
{
    PDEBUG ("CWD: %s, reload = %s, seleced_item: %d\n",
            cwd.c_str(), reload_dir?"True":"False", selected_item);

    wxString msg;
    if (reload_dir) {
        PDEBUG ("reload dir!\n");

        clean_resource();
        if (dirp != NULL){
            dirp_old = dirp;
        }
        if ((dirp = opendir(cwd.c_str())) == NULL) {
            wxString msg = _("ERROR: failed to open :") +\
                wxString(cwd.c_str(), wxConvUTF8) +\
                _("\nReason: ") + wxString(strerror(errno), wxConvUTF8);
            wxMessageDialog *dlg = \
                new wxMessageDialog(this, msg,  _("Error"), wxOK);
            dlg->ShowModal();
            delete(dlg);
            if (old_path.empty()) {
                char ccwd[1024] = {'\0'};
                if (getcwd(ccwd, 1024) == NULL){
                    sprintf(ccwd, "%s", getenv("HOME"));
                }
                cwd = string(ccwd);
            }
            else {
                cwd = old_path;
            }
            dirp = opendir(cwd.c_str());
            update_list(-1);
            return ;
        }
        if (fchdir(dirfd(dirp)) == -1) {
            fprintf(stderr, "ERROR: Failed to change directory into: %s\n",
                    cwd.c_str());
            fprintf(stderr, "reason: %s\n", strerror(errno));
            closedir(dirp);
            return;
        };

        // Close old dir after enter new dir.
        if (dirp_old) {
            int fd = dirfd(dirp_old);
            closedir(dirp_old);
            close(fd);
        }

        bool flag;
        if (config.get_config("show_hidden") == "false")
            flag = false;
        else
            flag = true;

        PDEBUG ("Show Hidden files?: %d\n", flag);


        string reason;
        int ret = get_filelist(".", file_list, reason, flag);
        if (ret != 0) {
            msg = _("Failed to open dir: ") + wxString(cwd.c_str(),
                                                       wxConvUTF8) +    \
                _(" Reason:") + wxString(reason.c_str(), wxConvUTF8);
            wxMessageDialog *dlg = new wxMessageDialog(this, msg, _("Error"),
                                                       wxOK);
            dlg->ShowModal();
            delete(dlg);
            return ;
        }
    }

    PDEBUG ("Begin fill\n");
    Freeze();
    cwd_info->SetLabel(wxString(cwd.c_str(), wxConvUTF8));
    lst->DeleteAllItems();

    // Add parent dir ("..")
    int idx = 0;
    lst->InsertItem(idx, 0);
    lst->SetItemData(idx, idx);
    lst->SetItem(idx, 1, _(".."));
    // lst->SetItem(idx, 2, msg);

    int size = file_list.size();
    char tmp[18];
    for (int i = 0; i < size;  i++) {
        idx ++;
        file_list[i]->orig_id = idx;
        if (file_list[i]->type == t_file){
            lst->InsertItem(idx, 1);
            lst->SetItem(idx, 2,
                         wxString(get_extname(file_list[i]->name).c_str(),
                                  wxConvUTF8));
        }
        else
            lst->InsertItem(idx, 0);
        lst->SetItemData(idx, idx);
        lst->SetItem(idx, 1, wxString(file_list[i]->name, wxConvUTF8));
        msg.Printf(wxT("%d"), file_list[i]->size);
        lst->SetItem(idx, 3, msg);
        memset (tmp, 0, 18);
        format_time(&file_list[i]->ctime, tmp);
        lst->SetItem(idx, 4,  wxString(tmp, wxConvUTF8));
        msg.Printf(wxT("%lo"), file_list[i]->mode & 0x1ff);
        lst->SetItem(idx, 5, msg);
    }
    PDEBUG ("After fill.\n");

    if (selected_item < 0)
        selected_item = cur_idx;
    if (file_list.empty())
        selected_item = 0;
    else if (selected_item > file_list.size())
        selected_item = file_list.size();
    lst->SetItemState(selected_item,
                      wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED,
                      wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
    cur_idx = selected_item;
    msg.Printf(wxT("%d files."), file_list.size());
    dirinfo->SetLabel(msg);

    lst->SetColumnWidth(3, -1);
    lst->SetColumnWidth(4, -1);
    lst->SetColumnWidth(5, -2);

    item_count = idx + 1;
    Thaw();
    PDEBUG ("leave\n");
}

void FSDisplayPane::clean_resource()
{
    PDEBUG ("Called.\n");
    for (iter = file_list.begin(); iter < file_list.end(); iter++) {
        free (*iter);
    }

    file_list.clear();
    selected_list.clear();
}

void FSDisplayPane::select_all()
{
    PDEBUG ("called.\n");
    selected_list.clear();
    // int idx;
    for (int idx = 0; idx < file_list.size(); idx++) {
        selected_list.push_back(file_list[idx]);
        lst->SetItemState(idx+1, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED,
                          wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
    }
    //    lst->SetItemState(idx, 0, (wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED));
}

void FSDisplayPane::select_same_ext()
{
    if (cur_idx == 0) {
        return;
    }
    selected_list.clear();
    string ext_name = get_extname(file_list[cur_idx - 1]->name);
    for (int idx = 0; idx < file_list.size(); idx++) {
        if (strstr(file_list[idx]->name, ext_name.c_str())) {
            selected_list.push_back(file_list[idx]);
            lst->SetItemState(idx+1,
                              wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED,
                              wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
        }
    }
}

void FSDisplayPane::deselect_same_ext()
{
    PDEBUG ("called");
    if (cur_idx == 0) {
        return;
    }
    string ext_name = get_extname(file_list[cur_idx - 1]->name);
    for (int idx = selected_list.size() - 1; idx > 0; idx--) {
        if (strstr(selected_list[idx]->name, ext_name.c_str())) {
            lst->SetItemState(selected_list[idx]->orig_id, 0,
                              wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
            selected_list.erase(selected_list.begin()+idx);
        }
    }
    PDEBUG ("leave\n");
}

const string FSDisplayPane::get_cwd()
{
    return cwd;
}

void FSDisplayPane::OnItemSelected(wxListEvent &evt)
{
    PDEBUG ("Selected item: %ld\n", evt.GetData() );
    cur_idx = evt.GetData();
    active_id = GetId();
    evt.Skip();
}

void FSDisplayPane::item_activated(wxListEvent &evt)
{
    int idx = evt.GetData();
    PDEBUG ("Item actived: %d\n", idx);
    quick_search->Clear();
    quick_search->Show(false);

    int selected_item = 0;

    // The first item should be processed sepratedly, cause dir ".." is not
    // stored in the file_list.


    if (idx == 0) {
        PDEBUG ("Selected parent director!\n");
        cwd = string(dirname(strdup(cwd.c_str())));
        vector<int>::iterator iter;
        if (!sel_idx.empty()) {
            iter = sel_idx.end();
            selected_item = *(--iter) + 1;
            sel_idx.pop_back();
        }
        update_list(selected_item);
        return;
    }
    idx--; // Strip the first item.
    if (file_list[idx]->type == t_dir) {
        PDEBUG ("Idx: %d, name: %s\n", idx, file_list[idx]->name);

        old_path = cwd;
        cwd = old_path + "/" + string(file_list[idx]->name);
        cwd = realpath(cwd.c_str(), NULL);
        sel_idx.push_back(idx);
        update_list(selected_item);
    }
    else {
        string path = cwd + "/" + string(file_list[idx]->name);
        if (wrap_open(path, 0) < 0) {
            PDEBUG ("Failed to open file: %s\n", path.c_str());
        }
    }
    evt.Skip();
}

/**
 * @name wrap_open - Wrapped open, run a child process to open selected files.
 * @param path -  path
 * @param create - Flag create
 * @return int
 * TODO: Enable multi-file edit and edit according to file type!
 */
int FSDisplayPane::wrap_open(string &path, bool create)
{
    int ret = 0;
    if (access(path.c_str(), F_OK) == -1 && !create) {
        oops ("Failed to open file: %s\n", path.c_str());
    }

    string cmd, key;
    filetype ftype = get_file_type(path);
    PDEBUG ("ftype: %d\n", ftype);
    switch (ftype) {
    case t_img: {
        key = "img_editor";
        break;
    }
    case t_pdf: {
        key = "pdf_reader";
        break;
    }
    case t_video: {
        key = "video_player";
        break;
    }
    default:
        key = "editor";
        break;
    }
    cmd = config.get_config(key) +  " \"" + path + "\"";
    PDEBUG ("cmd = %s\n", cmd.c_str());
    FuncHelper *func  = new FuncHelper();
    func->Create((wxWindow *)GetMainFrame(), cmd);
    func->Run();
    return 0;
}

void FSDisplayPane::set_selected()
{
    PDEBUG ("called;\n");
    if (cur_idx == 0) {
        return;
    }
    bool found = false;
    item *entry = file_list[cur_idx-1];
    if (!selected_list.empty()) {
        for (iter = selected_list.begin(); iter<selected_list.end();iter++) {
            if (strcmp(entry->name, (*iter)->name) == 0) {
                PDEBUG ("Found entry: %s\n",entry->name);
                found = true;
                break;
            }
        }
    }
    if (!found) {
        selected_list.push_back(entry);
        toggle_color(cur_idx, true);
    }
    else {
        selected_list.erase(iter);
        toggle_color(cur_idx, false);
    }
}


void FSDisplayPane::toggle_color(int idx, bool hicolor)
{
    if (hicolor){
        PDEBUG ("Hicolor\n");
        lst->SetItemTextColour(idx, fg_hi_col);
        lst->SetItemBackgroundColour(idx, bg_hi_col);
    }
    else{
        PDEBUG ("Dehicolor\n");
        lst->SetItemTextColour(idx, fg_def_col);
        lst->SetItemBackgroundColour(idx, bg_def_col);
    }
}

void FSDisplayPane::rename_file()
{
    wxString msg;
    wxMessageDialog *ddlg;
    if (cur_idx == 0) {
        PDEBUG ("Rename .. is not allowed!\n");
        msg = _("Rename .. is not allowed!\n");
        ddlg = \
            new wxMessageDialog(this, msg, _("Error"),
                                wxOK);
        ddlg->ShowModal();
        return;
    }
    // wxString old(wxT(basename(strdup(cwd.c_str()))));
    wxString old(file_list[cur_idx-1]->name, wxConvUTF8);
    DirnameDlg *dlg = \
        new DirnameDlg(this, _("Enter new file name:"), old);
    int ret = dlg->ShowModal();
    if (ret == wxID_OK) {
        string fn = string(dlg->fn.mb_str(wxConvUTF8));
        if (!name_is_valid(fn)) {
            msg = _("New name is empty or it is not valid!");
            ddlg = \
                new wxMessageDialog(this, msg, _("Error"),
                                    wxOK);
            ddlg->ShowModal();
            delete(ddlg);
            return ;
        }
        string cmd = "mv \"" + cwd + "/" + string(file_list[cur_idx-1]->name)\
            +  "\" \"" + cwd + "/" + fn + "\"";
        cout << cmd << endl;
        if ((ret = system(cmd.c_str())) != 0 ) {
            msg = _("Failed to rename file: ") + \
                wxString(fn.c_str(),wxConvUTF8);
            ddlg = \
                new wxMessageDialog(this, msg, _("Error"),
                                    wxOK);
            ddlg->ShowModal();
            delete(dlg);
            return ;
        }
        update_list(cur_idx);
    }
    delete(dlg);
}

void FSDisplayPane::OnKeydown(wxListEvent &evt)
{
    int keycode = evt.GetKeyCode();
    PDEBUG ("called: KeyCode: %d\n", keycode);
    switch (keycode) {
    case 8: { // Backspace.
        wxString msg = quick_search->GetValue();
        if (msg.Len() != 0 && msg.Len() != 1)
            quick_search->SetValue(msg.Remove(msg.Len()-1));
        else
            quick_search->Show(false);
        break;
    }
    case 32: { // SPACE, Mark item as selected.
        set_selected();
        break;
    }
    case 53: {
        open_terminal();
        break;
    }
    case 27: { // ESC
        quick_search->Clear();
        quick_search->Show(false);
        update_list(cur_idx);
        break;
    }
    case 349:
    case 350:
        break;
    case WXK_F2: { // F2, rename.
        rename_file();
        break;
    }
    case WXK_F3: { // F3
        view_file();
        break;
    }
    case WXK_F4: { //F4
        if (cur_idx == 0) {
            return;
        }
        edit_file(false);
        break;
    }
    case WXK_F7: { // F7
        DirnameDlg *dlg = \
            new DirnameDlg(this, _("Enter new directory name:"));
        int ret = dlg->ShowModal();
        if (ret == wxID_OK) {
            string fn = cwd + "/" + string(dlg->fn.mb_str(wxConvUTF8));
            PDEBUG ("MKDIR: %s\n", fn.c_str());
            if ((ret = mkdir(fn.c_str(), 0777))  ==1 ) {
                wxString msg = _("Failed to create directory: ") + \
                    wxString(fn.c_str(),wxConvUTF8) + \
                    _(" Reason:") + wxString(strerror(errno), wxConvUTF8);
                wxMessageDialog *ddlg = \
                    new wxMessageDialog(this, msg, _("Error"),
                                        wxOK);
                ddlg->ShowModal();
                delete(ddlg);
                return ;
            }
            update_list(cur_idx);
        }
        delete(dlg);
        break;
    }
    default:
        if (keycode >= 65 && keycode <= 122) {
            char c[2] = {'\0'};
            sprintf(c, "%c", keycode);
            if (!quick_search->IsShown()) {
                PDEBUG ("Set %d unselected!\n", cur_idx);

                lst->SetItemState(cur_idx, 0,
                                  (wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED));
            }
            quick_search->Show(true);
            quick_search->AppendText(wxString(c, wxConvUTF8));
        }
        break;
    }
    evt.Skip();
}

/**
 * @name view_file - View file according to Magic Key!!
 * @return int
 */
int FSDisplayPane::view_file()
{
    if (cur_idx == 0 || file_list[cur_idx-1]->type == 2) {
        oops ("View File called on dir!\n");
    }
    string path =  cwd + "/" + file_list[cur_idx-1]->name;
    if (!is_file_exist(path)) {
        oops ("File %s does not exist!\n", path.c_str());
    }
    if (is_image(path)) {
        string cmd = config.get_config("img_viewer") + " \"" + path + "\"";
        FuncHelper *func = new FuncHelper();
        func->Create((wxWindow *)GetMainFrame(), cmd);
        func->Run();
        return 0;
    }
    else {
        TextViewer *viewer = new TextViewer(GetParent(), path,
                                            file_list[cur_idx-1]->size);
        viewer->Show(true);
    }
    return 0;
}

/**
 * @name edit_file
 * @param create - Flag create
 * @return int
 */
int FSDisplayPane::edit_file(bool create)
{
    string path;
    if (create) {
        DirnameDlg *dlg = \
            new DirnameDlg(this, _("Enter new file name:"));
        int ret = dlg->ShowModal();
        if (ret == wxID_OK) {
            path = string(dlg->fn.mb_str(wxConvUTF8));
            if (!name_is_valid(path)) {
                wxString msg = _("New name is empty or it is not valid!");
                wxMessageDialog *ddlg = \
                    new wxMessageDialog(this, msg, _("Error"),
                                        wxOK);
                ddlg->ShowModal();
                delete(ddlg);
                return -1;
            }
            path = cwd + "/" + path;
            if (!access(path.c_str(), F_OK)) {
                wxMessageDialog *ddlg = \
                    new wxMessageDialog(this, _("File exised!"), _("Error"),
                                        wxOK);
                ddlg->ShowModal();
                delete(ddlg);
                return -1;
            }
        }
        else {
            return 0;
        }
    }
    else {
        path =  cwd + "/" + file_list[cur_idx-1]->name;
        if (access(path.c_str(), F_OK) == -1 && !create) {
            wxString msg = _("File does not exist!");
            wxMessageDialog *ddlg = \
                new wxMessageDialog(this, msg, _("Error"),
                                    wxOK);
            ddlg->ShowModal();
            delete(ddlg);
            return -1;
        }
    }
    string cmd = config.get_config("editor") + " \"" + path + "\"";
    PDEBUG ("CMD: %s\n", cmd.c_str());
    FuncHelper *func = new FuncHelper();
    func->Create((wxWindow *)GetMainFrame(), cmd);
    func->Run();
    return 0;
}

int FSDisplayPane::get_selected_files(vector<string> &list)
{
    list.clear();
    string fn;
    if (selected_list.empty()) {
        fn = cwd + "/" + string(file_list[cur_idx-1]->name);
        list.push_back(fn);
    }
    else {
        for (iter = selected_list.begin(); iter < selected_list.end();iter++){
            fn = cwd + "/" + string((*iter)->name);
            list.push_back(fn);
        }
    }
    return 0;
}

int FSDisplayPane::open_terminal()
{
    string cmd = config.get_config("app_terminal");
    if (chdir(cwd.c_str()) == -1) {
        fprintf(stderr, "ERROR: failed to change dir!\n");
    }
    if (cmd.empty()) {
        wxMessageDialog *ddlg = \
            new wxMessageDialog(this, _("App_terminal not signed!"),
                                _("Error"), wxOK);
        ddlg->ShowModal();
        delete ddlg;
        return -1;
    }
    FuncHelper *func = new FuncHelper();
    func->Create((wxWindow *)GetMainFrame(), cmd);
    func->Run();
    return 0;
}
/**
 * @name del_file - Move file to trashdir, or delete it if it's already in
 * 					trash dir.
 * @return int : which item should be focused.
 */
int FSDisplayPane::delete_file()
{
    PDEBUG ("called.\n");
    string path, cmd;
    int idx = 0;
    if (selected_list.empty()) {
        if (cur_idx == 0)
            return 0;
        PDEBUG ("Delete the on at curser\n!");
        path =  cwd + "/" + string(file_list[cur_idx-1]->name);
        delete_single_file(path);
        cur_idx--;
        idx = cur_idx;
    }
    else {
        PDEBUG ("delete from selected list!\n");

        idx = (*selected_list.begin())->orig_id -1 ;
        for (iter=selected_list.begin(); iter<selected_list.end(); iter++){
            if (idx > (*iter)->orig_id) {
                idx = (*iter)->orig_id -1;
            }
            path =  cwd + "/" + string((*iter)->name);
            delete_single_file(path);
        }
    }
    if (idx < 0)
        idx = 0;
    return idx;
}

/**
 * @name delete_single_file - Delete single file.
 * @param path -  path
 * @return void
 */
void FSDisplayPane::delete_single_file(string &path)
{
    string cmd;
    PDEBUG ("called: %s\n", path.c_str());

    if (path.find(string(getenv("HOME")) + "/tmp/deleted") != -1)
        cmd = "rm -rf \"" + path + "\"";
    else
        cmd = "mv \"" + path + "\"" + " ~/tmp/deleted";
    FuncHelper *func = new FuncHelper();
    func->Create((wxWindow *)GetMainFrame(), cmd);
    func->Run();
}

// Ug, it does not work .
void FSDisplayPane::OnSetFocus(wxFocusEvent &evt)
{
    PDEBUG ("WindowID: %d\n", this->GetId());
}

void FSDisplayPane::OnTextChanged(wxCommandEvent &evt)
{
    wxString target = quick_search->GetValue();
    if (target.Len() == 0) {
        PDEBUG ("Empty search target!\n");
        // quick_search->Clear();
        // quick_search->Show(false);
        return;
    }
    string tmp = string(target.mb_str(wxConvUTF8));
    transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
    string name;
    int idx = 0;
    bool found = false;
    for (idx = 0; idx < file_list.size(); idx++) {
        name = (file_list[idx])->name;
        transform(name.begin(), name.end(), name.begin(), ::tolower);
        if (name.find(tmp) != -1) {
            idx++;
            found = true;
            break;
        }
    }

    if (found) {
        lst->SetItemState(cur_idx, 0,
                          (wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED));
        lst->SetItemState(idx,
                          wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED,
                          wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
        wxPoint point;
        lst->GetItemPosition(idx, point);
        cur_idx = idx;
        lst->ScrollList(0, point.y-200);
    }
    else {
        lst->SetItemState(cur_idx, 0,
                          (wxLIST_STATE_SELECTED));
    }
}

void FSDisplayPane::OnTextEnter(wxCommandEvent &evt)
{
    quick_search->Clear();
    quick_search->Show(false);
    lst->SetItemState(cur_idx,
                      wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED,
                      wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
    lst->SetFocus();
}

void FSDisplayPane::OnColumbDrag(wxListEvent &evt)
{
#ifdef DEBUG
    for (int i = 0; i < 6; i++) {
        PDEBUG ("Column: %d, width: %d\n",i, lst->GetColumnWidth(i));
    }
#endif
    evt.Skip();
}

void FSDisplayPane::set_cwd(string &path)
{
    cwd = path;
}

string FSDisplayPane::get_selected_item()
{
    string path;

    if (cur_idx == 0)
        path = string(dirname(strdup(cwd.c_str())));
    else
        path = cwd + "/" + string(file_list[cur_idx-1]->name);
    return path;
}

void FSDisplayPane:: focus_first()
{
    lst->SetItemState(cur_idx, 0,
                      (wxLIST_STATE_SELECTED));
    cur_idx = 0;
    lst->SetItemState(cur_idx,
                      wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED,
                      wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
}

void FSDisplayPane:: focus_last()
{
    lst->SetItemState(cur_idx, 0,
                      (wxLIST_STATE_SELECTED));
    cur_idx = item_count - 1;
    lst->SetItemState(cur_idx,
                      wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED,
                      wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
}
void FSDisplayPane:: focus_prev()
{
    lst->SetItemState(cur_idx, 0,
                      (wxLIST_STATE_SELECTED));
    cur_idx -- ;
    if (cur_idx < 0)
        cur_idx = 0;
    lst->SetItemState(cur_idx,
                      wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED,
                      wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);

}
void FSDisplayPane:: focus_next()
{
    lst->SetItemState(cur_idx, 0,
                      (wxLIST_STATE_SELECTED));
    cur_idx ++;
    if (cur_idx == item_count)
        cur_idx = item_count - 1;
    lst->SetItemState(cur_idx,
                      wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED,
                      wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
}

void FSDisplayPane::OnMySort(wxListEvent &evt)
{
    int col = evt.GetColumn();
    PDEBUG ("called, col: %d\n", col);
    switch (col) {
    case 1: { // Name Column
        reverse_list(file_list);
        update_list(cur_idx, false);
        break;
    }
    case 2: { // Ext Name
        update_list(cur_idx, false);
        break;
    }
    case 3: { // Size
        resort_size_based(file_list);
        update_list(cur_idx, false);
        break;
    }
    case 4: { // Time
        resort_time_based(file_list);
        update_list(cur_idx, false);
        break;
    }
    default:
        break;
    }
    evt.Skip();
}


void  FSDisplayPane:: goto_dir()
{
    PDEBUG ("called.\n");
    DirnameDlg *dlg = \
        new DirnameDlg(this, _("Enter new directory name:"));
    int ret = dlg->ShowModal();
    if (ret == wxID_OK) {
        string path(dlg->fn.mb_str(wxConvUTF8));
        if (path.find("/") != 0) // Relative path.
            path = cwd + "/" + path;
        if (is_dir_exist(path)) {
            old_path = cwd;
            cwd = path;
        }
        else {
            wxString msg = wxString(path.c_str(), wxConvUTF8)+\
                _("\nDirecory does not exist!\n");
            wxMessageDialog *ddlg = \
                new wxMessageDialog(this, msg, _("Error"), wxOK);
            ddlg->ShowModal();
            delete ddlg;
            return ;
        }
    }
    delete dlg;
    update_list(-1);
}


BEGIN_EVENT_TABLE(FSDisplayPane, wxPanel)
EVT_LIST_ITEM_SELECTED(-1, FSDisplayPane::OnItemSelected)
EVT_LIST_ITEM_ACTIVATED(-1, FSDisplayPane::item_activated)
EVT_LIST_KEY_DOWN(-1, FSDisplayPane::OnKeydown)
EVT_LIST_COL_CLICK(-1, FSDisplayPane::OnMySort)
EVT_SET_FOCUS(FSDisplayPane::OnSetFocus)
EVT_KILL_FOCUS(FSDisplayPane::OnSetFocus)
EVT_TEXT(-1, FSDisplayPane::OnTextChanged)
EVT_TEXT_ENTER(-1, FSDisplayPane::OnTextEnter)
EVT_LIST_COL_END_DRAG(-1, FSDisplayPane::OnColumbDrag)
END_EVENT_TABLE()

void *FuncHelper::Entry()
{
    PDEBUG ("Begin executing: %s.\n", cmd.c_str());
    if(system(cmd.c_str()))
        fprintf(stderr, "ERROR: failed to execute command!\n");
    wxCommandEvent event( wxEVT_MY_EVENT, GetId() );
    event.SetEventObject(wn);
    (wn->GetEventHandler())->AddPendingEvent(event);
    PDEBUG ("Leaving Thread!\n");
    return 0;
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
