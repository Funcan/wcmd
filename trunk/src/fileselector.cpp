#include "fileselector.h"
#include "wx/log.h"
#include "wx/process.h"
#include <wx/mimetype.h>

class MyProcess : public wxProcess
{
public:
    MyProcess(FSDisplayPane *parent, const wxString& cmd)
        : wxProcess(parent), m_cmd(cmd)
        {
            m_parent = parent;
        }
    virtual void OnTerminate(int pid, int status);

protected:
    FSDisplayPane *m_parent;
    wxString m_cmd;
    bool flag;
};

void MyProcess::OnTerminate(int pid, int status)
{
    wxLogStatus(wxT("Process %u ('%s') terminated with exit code %d."),
                pid, m_cmd.BeforeFirst(wxT(' ')).c_str(), status);

    if (m_cmd.StartsWith(_("mv")) || m_cmd.StartsWith(_("cp")))
        flag = true;
    else
        flag = false;

    m_parent->OnAsyncTermination(flag);
}

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

void FileSelector::update_fs()
{
    sp1->update_list(sp1->cur_idx);
    sp2->update_list(sp2->cur_idx);
}



FSDisplayPane::FSDisplayPane(wxWindow *parent, wxWindowID id, string &path): \
	wxPanel(parent, id)
{
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

    lst = new MyListCtrl(this, id+1);
    sizer->Add(lst, 1, wxEXPAND|wxALL, 0);

    dirinfo = new wxStaticText(this, -1, _("Directory info!"));
    sizer->Add(dirinfo, 0, wxEXPAND|wxALL, 5);
    this->SetSizer(sizer);

    point = GetPosition();
    height = (GetParent()->GetParent())->GetSize().GetHeight();
    point += wxPoint(0, height-140);
    quick_search = new wxTextCtrl(this, id+2, _(""), point, wxDefaultSize,
                                  wxTE_PROCESS_ENTER);
    quick_search->Show(false);
    this->Show(true);

    cur_sort = COL_NAME;
    update_list(0);
    fg_hi_col = wxColour(255, 255, 255);
    bg_hi_col = wxColour(0x20, 0x4a, 0x87);
    bg_def_col = wxColour(0xe7, 0xed, 0xf6);
    fg_def_col = wxColour(0,0,0);
}

void FSDisplayPane::update_list(int selected_item, bool reload_dir)
{
    if (reload_dir) {
        clean_resource();
        if (dirp != NULL){
            dirp_old = dirp;
        }
        if ((dirp = opendir(cwd.c_str())) == NULL) {
            msg = _("ERROR: failed to open :") +\
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
    cur_list.clear();
    cur_list = file_list;
    show_list(selected_item);
}

/**
 * @name show_list - Display current list in treectrl
 * @param selected_item - Number of selected item.
 * @param filter -  filter
 * @return void
 */
void FSDisplayPane::show_list(int selected_item, wxString filter)
{
    Freeze();
    if (filter.Len() != 0)
        msg = _("\t\tFILTER: ") + filter;
    cwd_info->SetLabel(wxString(cwd.c_str(), wxConvUTF8) + msg);
    lst->DeleteAllItems();

    msg.Clear();

    // Add parent dir ("..")
    int idx = 0;
    lst->InsertItem(idx, 0);
    lst->SetItemData(idx, idx);
    lst->SetItem(idx, 1, _(".."));
    // lst->SetItem(idx, 2, msg);

    int size = cur_list.size();
    for (int i = 0; i < size;  i++) {
        idx ++;
        cur_list[i]->orig_id = idx;
        // Insert Image Column & Extention Colum.
        lst->append_item(idx, cur_list[i]);
    }

    if (selected_item < 0) // Selecte the first one by default.
        selected_item = cur_idx;
    if (cur_list.empty()) // If list is empty, select ".."
        selected_item = 0;
    else if (selected_item > (int)cur_list.size()) // Select the last one.
        selected_item = cur_list.size();

    lst->select_entry(selected_item);
    cur_idx = selected_item;
    update_dir_info();
    item_count = idx + 1;
    Thaw();

}

/**
 * @name update_dir_info - Updates the Dirinfo label.
 * @return void
 */
void FSDisplayPane::update_dir_info()
{
    wxLongLong disk_size = 0, free_size = 0;
    int selected_size = 0, selected_number = 0;
    wxString mmsg;
    for (iter = selected_list.begin(); iter<selected_list.end();iter++) {
        selected_size += (*iter)->size;
        ++selected_number;
    }
    if (wxGetDiskSpace(str2wxstr(cwd), &disk_size, &free_size) == false) {
        disk_size = 0;
        free_size = 0;
    }
    mmsg.Printf(wxT("Selected items: %d,"), selected_number);
    mmsg += _("  size: ") + size_2_wxstr(selected_size) +  \
        _(",\tDisk Space:") + size_2_wxstr(WX_2_LL((disk_size))) + \
        _(", Free space: ") + size_2_wxstr(WX_2_LL((free_size)));
    dirinfo->SetLabel(mmsg);
}

unsigned long long FSDisplayPane::WX_2_LL(wxLongLong n)
{
    unsigned long long hi;
    hi = n.GetHi();
    hi <<= 32;
    hi += n.GetLo();
    return hi;
}

/**
 * @name clean_resource - Cleans up the allocated resouces.
 * @return void
 */
void FSDisplayPane::clean_resource()
{

    for (iter = file_list.begin(); iter < file_list.end(); iter++) {
        free (*iter);
    }

    file_list.clear();
    selected_list.clear();
}

/**
 * @name select_all - Chooses the files in current list.
 * @return void
 */
void FSDisplayPane::select_all()
{

    selected_list.clear();
    // int idx;
    for (unsigned int idx = 0; idx < file_list.size(); idx++) {
        selected_list.push_back(file_list[idx]);
        lst->select_entry(idx+1);
    }
}

/**
 * @name select_same_ext - Chooses all files of the  of same type as the
 *      selected one.
 * @return void
 */
void FSDisplayPane::select_same_ext()
{
    if (cur_idx == 0) {
        return;
    }
    selected_list.clear();
    string ext_name(file_list[cur_idx - 1]->ext);
    for (unsigned int idx = 0; idx < file_list.size(); idx++) {
        if (strstr(file_list[idx]->name, ext_name.c_str())) {
            selected_list.push_back(file_list[idx]);
            lst->select_entry(idx+1);
        }
    }
}

/**
 * @name deselect_same_ext - Deselect files  of the same type.
 * @return void
 */
void FSDisplayPane::deselect_same_ext()
{
    if (cur_idx == 0) {
        return;
    }
    string ext_name(file_list[cur_idx - 1]->ext);
    for (int idx = selected_list.size() - 1; idx > 0; idx--) {
        if (strstr(selected_list[idx]->name, ext_name.c_str())) {
            lst->deselect_entry(selected_list[idx]->orig_id);
            selected_list.erase(selected_list.begin()+idx);
        }
    }
}

const string FSDisplayPane::get_cwd()
{
    return cwd;
}

void FSDisplayPane::OnItemSelected(wxListEvent &evt)
{
    cur_idx = evt.GetData();
    active_id = GetId();
    evt.Skip();
}

void FSDisplayPane::item_activated(wxListEvent &evt)
{

    int idx = evt.GetData();
    quick_search->Clear();
    quick_search->Show(false);
    activate_item(idx);
    evt.Skip();
}

/**
 * Active item specified by idx.
 */
void FSDisplayPane::activate_item(int idx)
{
    int selected_item = 0;
    if (idx < 0)
        idx = cur_idx;
    // The first item should be processed sepratedly, cause dir ".." is not
    // stored in the file_list.
    if (idx == 0) {

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
    if (cur_list[idx]->type == t_dir) {


        old_path = cwd;
        cwd = old_path + "/" + string(cur_list[idx]->name);
        cwd = realpath(cwd.c_str(), NULL);
        sel_idx.push_back(idx);
        update_list(selected_item);
    }
    else {
        wxString path = str2wxstr(cwd) + _("/") + \
            char2wxstr(cur_list[idx]->name);
        if (wrap_open(path, 0) < 0) {
            fprintf(stderr, "ERROR: failed to openfile: %s!\n",
                    cur_list[idx]->name);
        }
    }
 }

/**
 * @name wrap_open - Wrapped open, run a child process to open selected files.
 * @param path -  path
 * @param create - Flag create
 * @return int
 * TODO: Enable multi-file edit and edit according to file type!
 */
int FSDisplayPane::wrap_open(wxString &path, bool create)
{
    int ret = -1;
    wxString ext = path.AfterLast(wxT('.'));
    wxMessageDialog *dlg;

    if ((wxFileExists(path) == false) && !create) {
        msg = _("Can not access file: ") + path;
        dlg = new wxMessageDialog(this, msg, _("Open Error!"), wxOK);
        dlg->ShowModal();
        delete dlg;
        return -1;
    }

    wxFileType *ft = \
        wxTheMimeTypesManager->GetFileTypeFromExtension(ext);

    if (ft == NULL) {
        msg = _("Unknow filetype:\t") + ext +                 \
            _(".\nYou can stroke Ctrl+i to view file info ")+ \
            _("Or Press F3 to view as plain file!");
        dlg = new wxMessageDialog(this, msg, _("Open fail!"), wxOK);
        dlg->ShowModal();
        delete dlg;
    }
    else {
        wxString wxcmd = ft->GetOpenCommand(_("\"") + path + _("\""));
        if (wxcmd.Len() != 0) {
            ret = do_async_execute(wxcmd);
        }
        else {
            msg.Clear();
            wxString mt;
            if (ft->GetMimeType(&mt)) { // Can be treated as text file
                if (mt.Find(_("text")) != wxNOT_FOUND)
                    return edit_file(false);
                }
            else {
                mt = _("Unkonwn!");
            }

            msg =  _("No program registered!\nFile Name: ") +       \
                str2wxstr(path) +                                   \
                _(".\nFile mimeTypes is:\t") + mt +                \
                _("\nYou can stroke Ctrl+i to view file info ")+    \
                _("Or Press F3 to view as plain file!");


            dlg = new wxMessageDialog(this, msg, _("Open fail!"), wxOK);
            dlg->ShowModal();
            delete dlg;
        }
        delete ft;
    }
    return ret;
}

int FSDisplayPane::do_async_execute(const wxString &cmd)
{
    MyProcess * const process = new MyProcess(this, cmd);
    long m_pidLast = wxExecute(cmd, wxEXEC_ASYNC, process);
    if ( !m_pidLast ) {
        wxLogError(wxT("Execution of '%s' failed."), cmd.c_str());
        delete process;
    }
    else {
        wxLogStatus(wxT("Process %ld (%s) launched."), m_pidLast,
                    cmd.c_str());
    }
    return (int)m_pidLast;
}

void FSDisplayPane::OnAsyncTermination(bool up_both_fs)
{
    if (up_both_fs)
        ((FileSelector *)GetParent())->update_fs();
    else
        update_list(-1);
}

void FSDisplayPane::set_selected()
{

    if (cur_idx == 0) {
        PDEBUG ("Should not select parent direcoty!\n");
        return;
    }
    bool found = false;
    item *entry = file_list[cur_idx-1];
    if (!selected_list.empty()) {
        for (iter = selected_list.begin(); iter<selected_list.end();iter++) {
            if (strcmp(entry->name, (*iter)->name) == 0) {
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
    update_dir_info();
}


void FSDisplayPane::toggle_color(int idx, bool hicolor)
{
    if (hicolor){

        lst->SetItemTextColour(idx, fg_hi_col);
        lst->SetItemBackgroundColour(idx, bg_hi_col);
    }
    else{

        lst->SetItemTextColour(idx, fg_def_col);
        lst->SetItemBackgroundColour(idx, bg_def_col);
    }
}

void FSDisplayPane::rename_file()
{
    wxMessageDialog *ddlg;
    if (cur_idx == 0) {
        msg = _("Rename .. is not allowed!\n");
        ddlg = \
            new wxMessageDialog(this, msg, _("Error"),
                                wxOK);
        ddlg->SetFocus();
        ddlg->ShowModal();
        return;
    }
    // wxString old(wxT(basename(strdup(cwd.c_str()))));
    wxString old(file_list[cur_idx-1]->name, wxConvUTF8);
    DirnameDlg *dlg = \
        new DirnameDlg(this, _("Enter new file name:"), old);
    int ret = dlg->ShowModal();
    if (ret == wxID_OK) {
        if (!name_is_valid(dlg->fn)) {
            msg = _("New name is empty or not valid!");
            ddlg = \
                new wxMessageDialog(this, msg, _("Error"),
                                    wxOK);
            ddlg->ShowModal();
            delete(ddlg);
            return ;
        }
        if (wxRenameFile(char2wxstr(file_list[cur_idx-1]->name), dlg->fn,
                         true)  == false) {
            msg = _("Failed to rename file: ") + dlg->fn;
            ddlg =                                          \
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

void FSDisplayPane::toggle_search()
{
    quick_search->Clear();
    quick_search->Show(false);
    tmp_list.clear();
    update_list(cur_idx);
    lst->SetFocus();
}

void FSDisplayPane::OnKeydown(wxListEvent &evt)
{
    int keycode = evt.GetKeyCode();
    switch (keycode) {
    case 8: { // Backspace.
        if (quick_search->IsShown()) { // In quick search, delete one chr.
            msg = quick_search->GetValue();
            if (msg.Len() != 0 && msg.Len() != 1)
                quick_search->SetValue(msg.Remove(msg.Len()-1));
            else
                quick_search->Show(false);
        }
        else { // Not in quick search, back into previous directory.
            string tmp(cwd);
            cwd = old_path;
            old_path = tmp;
            update_list(-1);
        }
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
        create_dir();
        break;
    }
    default:
        if (keycode >= 65 && keycode <= 122) {
            char c[2] = {'\0'};
            sprintf(c, "%c", keycode);
            if (!quick_search->IsShown()) {
                lst->deselect_entry(cur_idx);
            }
            quick_search->Show(true);
            quick_search->SetFocus();
            quick_search->AppendText(wxString(c, wxConvUTF8));
        }
        break;
    }
    evt.Skip();
}

void FSDisplayPane::create_dir()
{
    DirnameDlg *dlg = \
        new DirnameDlg(this, _("Enter new directory name:"));
    int ret = dlg->ShowModal();
    if (ret == wxID_OK) {
        string fn = cwd + "/" + string(dlg->fn.mb_str(wxConvUTF8));

        if ((ret = mkdir(fn.c_str(), 0777))  ==1 ) {
            msg = _("Failed to create directory: ") + \
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
}

void FSDisplayPane::show_err_dialog()
{
    dlg = new wxMessageDialog(this, msg, title,  wxOK);
    dlg->ShowModal();
    delete(dlg);
}

/**
 * @name view_file - View file according to Magic Key!!
 * @return int
 */
int FSDisplayPane::view_file()
{
    if (cur_idx == 0 || cur_list[cur_idx-1]->type == 2) {
        msg = _("Fivwer should be applied to files!");
        title = _("Error!");
        show_err_dialog();

    }
    string path =  cwd + "/" +  cur_list[cur_idx-1]->name;
    if (!wxFileExists(str2wxstr(path))) {
        msg = _("File does not exist!");
        title = _("Error!");
        show_err_dialog();
    }
    if (is_image(path)) {
        string cmd = config.get_config("img_viewer") + " \"" + path + "\"";
        wxExecute(str2wxstr(cmd));
        return 0;
    }
    else {
        TextViewer *viewer = new TextViewer(GetParent(), path,
                                            cur_list[cur_idx-1]->size);
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
    wxString path;
    title = _("Error!");
    if (create) {
        DirnameDlg *dlg = \
            new DirnameDlg(this, _("Enter new file name:"));
        int ret = dlg->ShowModal();
        if (ret == wxID_OK) {
            path = dlg->fn;
            if (!name_is_valid(path)) {
                msg = _("New name is empty or it is not valid!");
                show_err_dialog();
                return -1;
            }
            path = str2wxstr(cwd) + _("/") + path;
            if (wxFileExists(path) || wxDirExists(path)) {
                msg = _("File already exist!");
                show_err_dialog();
                return -1;
            }
        }
        else {
            return 0;
        }
    }
    else {
        if (cur_idx == 0) {
            msg = _("Can not edit directory!");
            show_err_dialog();
            return -1;
        }
        path =  str2wxstr(cwd) + _("/") +           \
            char2wxstr(cur_list[cur_idx-1]->name);
        if ((wxFileExists(path) == false) && !create) {
            msg = _("File does not exist!");
            show_err_dialog();
            return -1;
        }
    }
    cmd = str2wxstr(config.get_config("editor")) + _(" \"") + path + _("\"");
    do_async_execute(str2wxstr(cmd));
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
    return do_async_execute(str2wxstr(cmd));
}
/**
 * @name del_file - Move file to trashdir, or delete it if it's already in
 * 					trash dir.
 * @return int : which item should be focused.
 */
int FSDisplayPane::delete_files()
{

    string path;
    int idx = 0;
    if (selected_list.empty()) {
        if (cur_idx == 0)
            return 0;

        path =  cwd + "/" + string(file_list[cur_idx-1]->name);
        delete_single_file(path);
        cur_idx--;
        idx = cur_idx;
    }
    else {
        wxString cmd = _("rm -rf ");
        wxString pwd = str2wxstr(cwd);
        idx = (*selected_list.begin())->orig_id -1 ;
        for (iter=selected_list.begin(); iter<selected_list.end(); iter++){
            if (idx > (*iter)->orig_id) {
                idx = (*iter)->orig_id -1;
            }
            cmd += _(" \"") + pwd + _("/") + char2wxstr((*iter)->name) + _("\"");
        }
        do_async_execute(cmd);
    }
    if (idx < 0)
        idx = 0;
    return idx;
}

/**
 * @name delete_single_file - Delete single file.
 * @param path -  path
 * @return void
 * XXX: call do_async_execute() to delete can have chance to call update_fs()
 *      after command executed.
 */
void FSDisplayPane::delete_single_file(string &path)
{
    string cmd;
    cmd = "rm -rf \"" + path + "\"";
    do_async_execute(str2wxstr(cmd));
}

void FSDisplayPane::OnTextChanged(wxCommandEvent &evt)
{

    string tmp, name;
    unsigned int idx;

    lst->deselect_entry(cur_idx);

    if (!quick_search->IsShown()) {
        old_target.Clear();

        goto ret;
        return;
    }

    cur_target = quick_search->GetValue();
    if (cur_target.Len() == 0) {

        old_target.Clear();
        // cur_list = file_list;
        goto ret;
    }

    // Use orignal file_list for search, if:
    //  1. This is a new search.
    //  2. User stroked Backspace.
    if (cur_target.Len() < old_target.Len())
        tmp_list = file_list;
    else
        tmp_list = cur_list;

    cur_list.clear();

    tmp = string(cur_target.mb_str(wxConvUTF8));
    transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
    for (idx = 0; idx < tmp_list.size(); idx++) {
        name = (tmp_list[idx])->name;
        transform(name.begin(), name.end(), name.begin(), ::tolower);
        if (name.find(tmp) != string::npos) {
            cur_list.push_back(tmp_list[idx]);
        }
    }

ret:
    if (cur_idx > (signed int)cur_list.size() || cur_idx < 1)
        cur_idx = 1;
    if (cur_list.empty())
        cur_idx = 0;
    old_target = cur_target;
    show_list(cur_idx, cur_target);
}
/**
 * @name OnTextEnter - When Enter pressed, close quick_search.
 * @param evt -  evt
 * @return void
 */
void FSDisplayPane::OnTextEnter(wxCommandEvent &evt)
{
     quick_search->Clear();
    quick_search->Show(false);
    lst->select_entry(cur_idx);
    lst->SetFocus();

    if (cur_idx != 0)
        activate_item(cur_idx);
}

void FSDisplayPane::OnColumbDrag(wxListEvent &evt)
{
#ifdef DEBUG
    for (int i = 0; i < 6; i++) {

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
    lst->deselect_entry(cur_idx);
    cur_idx = 0;
    lst->select_entry(cur_idx);
    wxPoint point;
    lst->GetItemPosition(cur_idx, point);
    lst->ScrollList(0, point.y);
}

void FSDisplayPane:: focus_last()
{
    lst->deselect_entry(cur_idx);
    cur_idx = item_count - 1;
    lst->select_entry(cur_idx);
    wxPoint point;
    lst->GetItemPosition(cur_idx, point);
    lst->ScrollList(0, point.y);
}
void FSDisplayPane:: focus_prev()
{
    lst->deselect_entry(cur_idx);
    cur_idx--;
    if (cur_idx < 0)
        cur_idx = 0;

    lst->select_entry(cur_idx);

}
void FSDisplayPane:: focus_next()
{
    lst->deselect_entry(cur_idx);
    cur_idx ++;
    if (cur_idx == item_count)
        cur_idx = item_count - 1;
    lst->select_entry(cur_idx);
}

void FSDisplayPane::OnMySort(wxListEvent &evt)
{
    int col = evt.GetColumn();

    sort_and_show(col);
    evt.Skip();
}

/**
 * @name real_sort - Sort and display curr_list.
 * @param idx - Identifier of
 * @return void
 */
void FSDisplayPane::sort_and_show(int idx)
{
    switch (idx) {
    case COL_NAME: { // Name Column
        reverse_list(file_list);
        show_list(cur_idx);
        break;
    }
    case COL_EXT: { // Ext Name
        resort_based_ext(file_list);
        show_list(cur_idx);
        break;
    }
    case COL_SIZE: { // Size
        resort_size_based(file_list);
        show_list(cur_idx);
        break;
    }
    case COL_TIME: { // Time
        resort_time_based(file_list);
        show_list(cur_idx);
        break;
    }
    default:
        break;
    }
}

void  FSDisplayPane:: goto_parent_dir()
{
    old_path = cwd;
    cwd = string(dirname(strdup(cwd.c_str())));
    update_list(-1);
}

void  FSDisplayPane::focus_list()
{
    lst->SetFocus();
}

void  FSDisplayPane:: goto_dir()
{

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
            msg = wxString(path.c_str(), wxConvUTF8)+\
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

void FSDisplayPane::process_right_click(wxMouseEvent &evt)
{


    wxMessageDialog *dlg = \
        new wxMessageDialog(this, _("Test"), _("Test"), wxOK);
    dlg->ShowModal();
}


BEGIN_EVENT_TABLE(FSDisplayPane, wxPanel)
EVT_LIST_COL_CLICK(-1, FSDisplayPane::OnMySort)
EVT_LIST_COL_END_DRAG(-1, FSDisplayPane::OnColumbDrag)
EVT_LIST_ITEM_ACTIVATED(-1, FSDisplayPane::item_activated)
EVT_LIST_ITEM_SELECTED(-1, FSDisplayPane::OnItemSelected)
EVT_LIST_KEY_DOWN(-1, FSDisplayPane::OnKeydown)
EVT_TEXT(-1, FSDisplayPane::OnTextChanged)
EVT_TEXT_ENTER(-1, FSDisplayPane::OnTextEnter)
EVT_RIGHT_DOWN(FSDisplayPane::process_right_click)
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
