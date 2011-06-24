#include "fileselector.h"
#include <wx/log.h>
#include <wx/process.h>
#include <wx/mimetype.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "mainframe.h"

const wxString PERL_PATH(_("/usr/bin/perl"));
const wxString MIME_OPEN_PATH(_("/usr/bin/mimeopen"));

class MyProcess : public wxProcess
{
public:
    MyProcess(FSDisplayPane *parent, const wxString& cmd, bool up_flag)
        : wxProcess(parent), m_cmd(cmd)
        {
            m_parent = parent;
            this->up_flag = up_flag;
        }
    virtual void OnTerminate(int pid, int status);

protected:
    FSDisplayPane *m_parent;
    wxString m_cmd;
    bool up_flag, err_flag;
};

void MyProcess::OnTerminate(int pid, int status)
{
    wxLogStatus(wxT("Process %u ('%s') terminated with exit code %d."),
                pid, m_cmd.BeforeFirst(wxT(' ')).c_str(), status);

    if (status != 0)
        err_flag = true;
    else
        err_flag = false;

    m_parent->OnAsyncTermination(up_flag, err_flag, m_cmd);
}

wxWindowID active_id;

FSDisplayPane::FSDisplayPane(wxWindow *parent, wxWindowID id, wxString &path): \
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
    dir = new wxDir;

    cwd_info = new wxStaticText(this, -1, _(""));
    if (path.IsEmpty()) {
        if (id == ID_Sp1) {
            cwd = str2wxstr(config.get_config("auto_last_path_l"));
            if (!wxDirExists(cwd))
                cwd = char2wxstr(getenv("HOME"));
            cwd_info->SetLabel(cwd);
        }
        else {
            cwd = str2wxstr(config.get_config("auto_last_path_r"));
            if (!wxDirExists(cwd))
                cwd = char2wxstr(getenv("HOME"));
            cwd_info->SetLabel(cwd);
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
/**
 * Get filelist of current dir.
 * @return int
 */
int FSDisplayPane::get_cur_filelist()
{
    ItemEntry *entry;

    vector<ItemEntry *> tmp;
    vector<ItemEntry *>::iterator iter;

    DIR *dp;
    struct dirent *den;
    bool show_hidded = false;
    if (config.get_config("show_hidden") != "false") {
        show_hidded = true;
    }
    dp = opendir(cwd.mb_str(wxConvUTF8));
    if (dp == NULL) {
        return -1;
    }

    struct stat status;
    char *d_name = NULL;
    while ((den = readdir(dp)) != NULL) {
        d_name = den->d_name;
        if (strlen(d_name) == 0 )
            continue;
        else {
            if (*d_name == '.') {
                if (strlen(d_name) == 1 || show_hidded == false ||
                    *(d_name + 1) == '.') {
                    continue;
                }
            }
        }
        entry = new ItemEntry(cwd, char2wxstr(d_name));
        if (stat(entry->get_fullpath().mb_str(wxConvUTF8), &status) == 0) {
            if (S_ISDIR(status.st_mode)) {
                file_list.push_back(entry);
            }
            else{
                tmp.push_back(entry);
            }
        }
        else {
            if (errno != ELOOP && errno != EMLINK) {
                cout << "Can not get status of entry: " <<      \
                    entry->get_fullpath().mb_str(wxConvUTF8)    \
                     << ".  Reason: " << strerror(errno)  << endl;
            }
            continue;
        }
    }
    closedir(dp);

    stable_sort(file_list.begin(), file_list.end(), sort_name);
    if (tmp.size()) {
        stable_sort(tmp.begin(), tmp.end(), sort_name);
        for (iter = tmp.begin(); iter != tmp.end(); iter++) {
            file_list.push_back(*iter);
        }
    }

    return 0;
}

void FSDisplayPane::restore_cwd()
{
    if (old_path.IsEmpty()) {
        cwd = char2wxstr(getenv("HOME"));
    }
    else {
        cwd = old_path;
    }
    old_path = get_parent_dir(cwd);
}

void FSDisplayPane::update_list(int selected_item, bool reload_dir)
{
    if (reload_dir) {
        clean_resource();
        if (wxDirExists(cwd) == false) {
            msg = _("Dir: ") + cwd + _("can not be accessed!\n");
            title = _("Error!");
            show_err_dialog();
            restore_cwd();
            update_list(-1);
            return ;
        }

        // Close old dir after enter new dir.
        bool flag;
        if (config.get_config("show_hidden") == "false")
            flag = false;
        else
            flag = true;

        int ret = get_cur_filelist();
        if (ret != 0) {
            msg = _("Failed to open dir: ") + cwd  +            \
                _("\nReason:") + char2wxstr(strerror(errno));
            title = _("Error");
            show_err_dialog();
            restore_cwd();
            update_list(-1);
            return ;
        }
    }
    cur_list.clear();
    cur_list = file_list;
    show_list(selected_item);
}

/**
 * Display current list in treectrl
 * @param selected_item - Number of selected item.
 * @param filter -  filter
 * @return void
 */
void FSDisplayPane::show_list(int selected_item, wxString filter)
{
    Freeze();
    msg.Clear();
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

    int size = cur_list.size();
    for (int i = 0; i < size;  i++) {
        idx ++;
        cur_list[i]->orig_id = idx;
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
 * Updates the Dirinfo label.
 * @return void
 */
void FSDisplayPane::update_dir_info()
{
    wxLongLong disk_size = 0, free_size = 0;
    wxULongLong selected_size = 0;
    int selected_number = 0;
    wxString mmsg;
    for (iter = selected_list.begin(); iter<selected_list.end();iter++) {
        selected_size += (*iter)->get_size();
        ++selected_number;
    }
    if (wxGetDiskSpace(cwd, &disk_size, &free_size) == false) {
        disk_size = 0;
        free_size = 0;
    }

    mmsg.Printf(wxT("Selected items: %d,"), selected_number);
    mmsg += _("  size: ") + size_2_wxstr(WX_2_LL(selected_size)) \
        + _(",\tDisk Space:") + size_2_wxstr(WX_2_LL((disk_size))) +    \
        _(", Free space: ") + size_2_wxstr(WX_2_LL((free_size)));
    dirinfo->SetLabel(mmsg);
    wxLogStatus(_("Active Directory:") + cwd);
}


/**
 * Cleans up the allocated resouces.
 * @return void
 */
void FSDisplayPane::clean_resource()
{
    ItemEntry *entry;
    for (iter = file_list.begin(); iter < file_list.end(); iter++) {
        entry = (*iter);
        delete entry;
    }

    file_list.clear();
    selected_list.clear();
}

/**
 * Chooses the files in current list.
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
 * Chooses all files of the  of same type as the
 *      selected one.
 * @return void
 */
void FSDisplayPane::select_same_ext()
{
    if (cur_idx == 0) {
        return;
    }
    selected_list.clear();
    wxString ext_name = file_list[cur_idx - 1]->get_ext();
    for (unsigned int idx = 0; idx < file_list.size(); idx++) {
        if (ext_name.Cmp(file_list[idx]->get_ext()) == 0) {
            selected_list.push_back(file_list[idx]);
            lst->select_entry(idx+1);
        }
    }
}

/**
 * Deselect files  of the same type.
 * @return void
 */
void FSDisplayPane::deselect_same_ext()
{
    if (cur_idx == 0) {
        return;
    }
    wxString ext_name = file_list[cur_idx - 1]->get_ext();
    for (int idx = selected_list.size() - 1; idx > 0; idx--) {
        if (ext_name.Cmp(file_list[idx]->get_ext()) == 0) {
            lst->deselect_entry(selected_list[idx]->orig_id);
            selected_list.erase(selected_list.begin()+idx);
        }
    }
}

const wxString FSDisplayPane::get_cwd()
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
        vector<int>::iterator iter;
        if (!sel_idx.empty()) {
            iter = sel_idx.end();
            selected_item = *(--iter) + 1;
            sel_idx.pop_back();
        }
        old_path = cwd;
        cwd = get_parent_dir(cwd);
        update_list(selected_item);
        return;
    }
    idx--; // Strip the first item.
    if (wxDirExists(cur_list[idx]->fn->GetFullPath())) {
        old_path = cwd;
        cwd = cur_list[idx]->fn->GetFullPath();
        sel_idx.push_back(idx);
        update_list(selected_item);
    }
    else {
        wxString path = cur_list[idx]->fn->GetFullPath();
        if (wrap_open(path, 0) < 0) {
            cout << "ERROR: failed to openfile " << path.char_str() << endl;
        }
    }
 }

int FSDisplayPane::open_with_plugin(const char *file_name)
{
    int len = strlen(file_name);
    char *ptr;
    char ext_name [1024] = {'\0'};
    char plugin_path[1024] = {'\0'};

    memset(ext_name, 0, 1024);
    strncpy(ext_name, file_name, len);
    for (ptr = ext_name + len - 1; ptr >= ext_name; ptr--) {
        if (*ptr == '.') {
            ptr++;
            break;
        }
    }

    sprintf(plugin_path, "%s/.config/wcmd/plugins/%s.so", getenv("HOME"),
            ptr);
    if (access(plugin_path, F_OK) == -1) {
        return -1;
    }
    else {
        MyThreadFunc *func = new MyThreadFunc(file_name, plugin_path);
        if ( func->Create() != wxTHREAD_NO_ERROR )
        {
            wxLogError(wxT("Can't create thread!"));
            return -1;
        }
        if (func->Run() != wxTHREAD_NO_ERROR)
        {
            wxLogError(wxT("Can't create thread!"));
            return -1;
        }
        return 0;
    }
    return 0;
}

/**
 * Wrapped open, run a child process to open selected files.
 * @param path -  path
 * @param create - Flag create
 * @return int
 * TODO: Enable multi-file edit and edit according to file type!
 */
int FSDisplayPane::wrap_open(wxString &path, bool create)
{
    int ret = -1;
    wxMessageDialog *dlg;

    if ((wxFileExists(path) == false) && !create) {
        msg = _("Can not access file: ") + path;
        dlg = new wxMessageDialog(this, msg, _("Open Error!"), wxOK);
        dlg->ShowModal();
        delete dlg;
        return -1;
    }

    ret = open_with_plugin(strdup(((const char *)path.mb_str(wxConvUTF8))));
    if (ret != -1)
        return ret;

    // Failed to open through plugins, try  other ways.
    wxString cmd;
    if (wxFileExists(PERL_PATH) && wxFileExists(MIME_OPEN_PATH)) {
        cmd = PERL_PATH + _(" ") + MIME_OPEN_PATH + _(" -n ") + path;
        ret = do_async_execute(cmd);
        return ret;
    }
    wxString ext = path.AfterLast(wxT('.'));
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
        cmd = ft->GetOpenCommand(_("\"") + path + _("\""));
        if (cmd.Len() != 0) {
            ret = do_async_execute(cmd);
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
            title = _("Open failed!");
            show_err_dialog();
        }
        delete ft;
    }
    return ret;
}

int FSDisplayPane::do_async_execute(const wxString &cmd, bool up_flag)
{
    MyProcess * const process = new MyProcess(this, cmd, up_flag);
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

void FSDisplayPane::OnAsyncTermination(bool up_flag, bool err_flag,
                                       wxString cmd)
{
    if (up_flag) {
        ((MainFrame *)(GetParent())->GetParent())->update_fs();
    }
    else
        update_list(-1);
    if (err_flag) {
        title = _("Operation Failed!");
        msg = _("Failed to execute command:\n") + cmd;
        show_err_dialog();
    }
}

void FSDisplayPane::set_selected()
{

    if (cur_idx == 0) {
        return;
    }
    bool found = false;
    ItemEntry *entry = file_list[cur_idx-1];
    if (!selected_list.empty()) {
        for (iter = selected_list.begin(); iter<selected_list.end();iter++) {
            if ((void *)entry == (void *)(*iter)) {
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

    wxFileName *fn = file_list[cur_idx-1]->fn;
    wxString path_name = fn->GetPath();
    wxString old_name = fn->GetFullName();
    DirnameDlg *dlg = \
        new DirnameDlg(this, _("Enter new file name:"), old_name);
    int ret = dlg->ShowModal();

    if (ret == wxID_OK) {
        if (!name_is_valid(dlg->fn)) {
            msg = _("New name is empty or not valid!");
            title = _("Error");
            show_err_dialog();
            return ;
        }
        wxString new_name = wxFileName(cwd, dlg->fn).GetFullPath();
        if (wxRenameFile(fn->GetFullPath(), new_name,  true)  == false) {
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
            wxString tmp(cwd);
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
    case 53: { // 5.
        open_terminal();
        break;
    }
    case 349:
    case 350:
        break;
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
        wxFileName fn(cwd, dlg->fn);
        if (fn.Mkdir(fn.GetFullPath()) == false) {
            msg = _("Failed to create directory: ") + fn.GetFullPath();
            title = _("Error");
            show_err_dialog();
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
 * View file according to Magic Key!!
 * @return int
 */
int FSDisplayPane::view_file()
{
    if (cur_idx <= 0) {
        msg = _("Fileviwer should be only applied to files!");
        title = _("Error!");
        show_err_dialog();
        return -1;
    }

    int ret = 0;
    ItemEntry *entry = cur_list[cur_idx-1];
    wxString path =  entry->get_fullpath();
    wxString cmd;

    if (entry->is_dir()) {
        msg = _("Fivwer should be only applied to files!");
        title = _("Error!");
        show_err_dialog();
        ret = -1;
        goto end;
    }
    if (!entry->is_file_exist()) {
        msg = _("File does not exist!");
        title = _("Error!");
        show_err_dialog();
        ret = -1;
        goto end;
    }

    if (entry->is_image()) {
        cmd = str2wxstr(config.get_config("img_viewer")) + _(" \"") +  \
            path + _("\"");
    }
    else {
        cmd = _("xterm -e \"");
        if (entry->is_text()) {
            cmd += _("less ") + path;
        }
        else {
            cmd += _("hexdump -C ") + path + _(" | less");
        }
        cmd += _("\"");
    }

    ret = do_async_execute(cmd);
end:
    return ret;
}

/**
 * @name edit_file
 * @param create - Flag create
 * @return int
 */
int FSDisplayPane::edit_file(bool create)
{
    if (cur_idx == 0) {
        return -1;
    }

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
            path = cwd + _("/") + path;
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
        ItemEntry *entry = cur_list[cur_idx-1];
        path =  entry->get_fullpath();
        if ((entry->is_file_exist() == false) && !create) {
            msg = _("File does not exist!");
            title = _("Error");
            show_err_dialog();
            return -1;
        }
    }
    cmd = str2wxstr(config.get_config("editor")) + _(" \"") + path + _("\"");
    return do_async_execute(cmd);
}

int FSDisplayPane::get_selected_files(vector<ItemEntry *> &list)
{
    list.clear();
    if (selected_list.empty() && cur_idx != 0) {
        list.push_back(file_list[cur_idx-1]);
    }
    else {
        for (iter = selected_list.begin(); iter < selected_list.end();iter++){
            list.push_back(*iter);
        }
    }
    return 0;
}

int FSDisplayPane::compress_files()
{
    vector<ItemEntry *> list;
    if(get_selected_files(list) || list.empty()) {
        msg = _("Source not selected!");
        title = _("Error");
        show_err_dialog();
        return -1;
    }
    CompressDlg *dlg = \
        new CompressDlg(this, _("Enter file name (without extension):"));
    int ret = dlg->ShowModal();
    if (ret == wxID_OK) {
        wxString path = dlg->fn;
        COMPRESS_TYPE type = (COMPRESS_TYPE)dlg->type;
        if (type < 0 || type >= TYPE_CNT) {
            msg = msg.Format(_("Wrong type: %d"), (int) type);
            title = _("Error");
            show_err_dialog();
            return -1;
        }
        if (!name_is_valid(path)) {
            msg = _("New name is empty or it is not valid!");
            show_err_dialog();
            return -1;
        }
        if (chdir(cwd.mb_str(wxConvUTF8))) {
            msg = _("Failed to change dir into") + cwd;
            title = _("Error");
            show_err_dialog();
            return -2;
        }

        path += _(".") + type_2_ext(type);
        if (wxFileExists(path) || wxDirExists(path)) {
            msg = _("File already exist!");
            show_err_dialog();
            return -1;
        }

        return do_async_execute(get_compress_cmd(path, list, type));
    }
    return 0;
}

int FSDisplayPane::decompress_files()
{
    vector<ItemEntry *> list;
    if(get_selected_files(list) || list.empty()) {
        msg = _("Source not selected!");
        title = _("Error");
        show_err_dialog();
        return -1;
    }

    if (chdir(cwd.mb_str(wxConvUTF8))) {
        msg = _("Failed to change dir into") + cwd;
        title = _("Error");
        show_err_dialog();
        return -2;
    }

    return do_async_execute(get_decompress_cmd(list));
}

wxString FSDisplayPane::get_decompress_cmd(vector<ItemEntry *> &list)
{
    wxString cmd, option, ext;
    vector<ItemEntry *>::iterator iter;

    for (iter = list.begin(); iter < list.end(); iter++) {
        ext = (*iter)->get_ext();
        if (ext.Cmp(_("bz2")) == 0 ||
            ext.Cmp(_("gz")) == 0) {
            if (((*iter) -> get_name()).EndsWith(_(".tar"))) {
                ext = _("tar.") + ext;
            }
            else {
                ext = _("7z");
            }
        }
        get_cmd_option(type_2_ext(ext), cmd, option, false);
        cmd += option;
        cmd += _(" \"") + (*iter)->get_fullname() + _("\" ");
        if (iter != list.end() - 1) {
            cmd += _("&& ");
        }
    }

    return cmd;
}

/**
 * Get command string to compress files.
 *
 * @param list -  list of files to be compressed.
 * @param dstname - dest name.
 * @param type -  type of extension.
 * @return command to be executed.
 */
wxString FSDisplayPane::get_compress_cmd(wxString &dstname,
                                         vector<ItemEntry *> &list,
                                         COMPRESS_TYPE type)
{
    wxString cmd, options;
    get_cmd_option(type, cmd, options, true);
    cmd += options + dstname;
    vector<ItemEntry *>::iterator iter;
    for (iter = list.begin(); iter < list.end(); iter++) {
        cmd += _(" \"") + (*iter)->get_fullname() + _("\" ");
    }
    return cmd;
}

/**
 * Get cmd and option based on file type.
 *
 * @param type - Type of
 *
 * @param cmd -  cmd
 *
 * @param option -  option
 *
 * @param compress - Flag compress
 * @return int
 */
void FSDisplayPane::get_cmd_option(COMPRESS_TYPE type, wxString &cmd,
                                  wxString &option, bool compress)
{
    switch (type) {
    case TAR: {
        cmd = _("tar ");
        if (compress)
            option = _("-cvf ");
        else
            option = _("-xvf ");
        break;
    }
    case GZIP: {
        cmd = _("tar ");
        if (compress)
            option = _("-czvzf ");
        else
            option = _("-xzvf ");
        break;
    }
    case BZIP2: {
        cmd = _("tar ");
        if (compress)
            option = _("-cjvf ");
        else
            option = _("-xjvf ");
        break;
    }
    case ZIP: {
        if (compress) {
            cmd = _("zip ");
            option = _("-r ");
        }
        else {
            cmd = _("7z ");
            option = _("x -y");
        }
        break;
    }
    case P7Z: {
        cmd = _("7z ");
        if (compress) {
            option = _("a ");
        }
        else {
            option = _("x -y ");
        }
        break;
    }
    default:
        ;
    }
}

int FSDisplayPane::open_terminal()
{
    string cmd = config.get_config("app_terminal");
    if (wxSetWorkingDirectory(cwd) == false) {
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
 * Delete selected files .
 * @return int : which item should be focused.
 */
int FSDisplayPane::delete_files()
{

    wxString path;
    int idx = 0;
    if (selected_list.empty()) {
        if (cur_idx == 0)
            return 0;
        path =  file_list[cur_idx-1]->fn->GetFullPath();
        delete_single_file(path);
        cur_idx--;
        idx = cur_idx;
    }
    else {
        wxString cmd = _("rm -rf ");
        wxString pwd = cwd;
        idx = (*selected_list.begin())->orig_id -1 ;
        for (iter=selected_list.begin(); iter<selected_list.end(); iter++){
            if (idx > (*iter)->orig_id) {
                idx = (*iter)->orig_id -1;
            }
            cmd += _(" \"") + (*iter)->fn->GetFullPath() + _("\"");
        }
        do_async_execute(cmd);
    }
    if (idx < 0)
        idx = 0;
    return idx;
}

/**
 * Delete single file.
 * @param path -  path
 * @return void
 * XXX: call do_async_execute() to delete can have chance to call update_fs()
 *      after command executed.
 */
void FSDisplayPane::delete_single_file(wxString &path)
{
    wxString cmd;
    cmd = _("rm -rf \"") + path + _("\"");
    do_async_execute(str2wxstr(cmd));
}

void FSDisplayPane::OnTextChanged(wxCommandEvent &evt)
{

    wxString name;
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

    for (idx = 0; idx < tmp_list.size(); idx++) {
        name = (tmp_list[idx])->fn->GetFullName();
        if (name.Lower().Find(cur_target.Lower()) !=  wxNOT_FOUND) {
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
 * When Enter pressed, close quick_search.
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

void FSDisplayPane::set_cwd(wxString path)
{
    cwd = path;
}

wxString FSDisplayPane::get_selected_item()
{
    wxString path;

    if (cur_idx == 0)
        path = (wxFileName::DirName(cwd)).GetFullPath();
    else
        path = file_list[cur_idx-1]->fn->GetFullPath();
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
 * Sort and display curr_list.
 * @param idx - Identifier of
 * @return void
 */
void FSDisplayPane::sort_and_show(int idx)
{
    switch (idx) {
    case COL_NAME: { // Name Column
        reverse_list(cur_list);
        show_list(cur_idx);
        break;
    }
    case COL_EXT: { // Ext Name
        resort_based_ext(cur_list);
        show_list(cur_idx);
        break;
    }
    case COL_SIZE: { // Size
        resort_size_based(cur_list);
        show_list(cur_idx);
        break;
    }
    case COL_TIME: { // Time
        resort_time_based(cur_list);
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
    cwd = (wxFileName::DirName(old_path)).GetFullPath();
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
        wxFileName fn(dlg->fn);
        wxString path(dlg->fn);
        if (fn.IsRelative()) // Relative path.
            path = cwd + _("/") + fn.GetFullPath();

        if (wxDirExists(path)) {
            old_path = cwd;
            cwd = path;
        }
        else {
            msg = path + _(" does not exist!\n");
            title =  _("Error");
            show_err_dialog();
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

void FSDisplayPane::set_focus()
{
    lst->SetFocus();
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
