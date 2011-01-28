#include "wx/wx.h"
#include "mainframe.h"
#include "utils.h"

class MyApp: public wxApp
{
    virtual bool OnInit();
    int FilterEvent(wxEvent& event);
private:
    MainFrame *frame;
    char **c_args;
};


const char *APP_NAME = "wcmd";

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    openlog(APP_NAME, LOG_CONS, LOG_DAEMON);
    PDEBUG ("Begin\n");
    if (argc > 3) {
        fprintf(stderr, "ERROR: two arguments at most!"
                "Others will be skipped.\n");
    }
    c_args = (char **)calloc(2, sizeof(char *));
    for (int i = 0; i < 2; i++) {
        if (i+1 < argc) {
            wxString tmp = wxString(argv[i+1]);
            c_args[i] = (char *)calloc(tmp.Length()+1, 1);
            memcpy(c_args[i], tmp.char_str(), tmp.Length());
        }
    }

    string trash_dir = string(getenv("HOME")) + "/tmp/deleted" ;
    if (!is_dir_exist(trash_dir)) {
        string cmd = "mkdir ~/tmp/deleted";
        if(system(cmd.c_str()) != 0)
            fprintf(stderr, "ERROR: Failed to create trash dir!\n");
    }
    frame = new MainFrame( _("wcmd"), c_args);
    wxImage::AddHandler( new wxPNGHandler );
    wxImage::AddHandler( new wxJPEGHandler );
    wxImage::AddHandler( new wxGIFHandler );
    wxImage::AddHandler( new wxXPMHandler );
    frame->Show(true);
    SetTopWindow(frame);
    return true;
}

/**
 * Hotkeys will be moved to here from fileselector.cpp.
 */
int MyApp::FilterEvent(wxEvent& event)
{
    if ( event.GetEventType()==wxEVT_KEY_DOWN) {
        wxKeyEvent evt = (wxKeyEvent&)event;
        int keycode = evt.GetKeyCode();

        if (evt.ControlDown()) {
            switch (keycode) {
            case 96: { // Ctrl + `, Go to HOME dir
                string path = string(getenv("HOME"));
                frame->get_sp()->set_cwd(path);
                frame->get_sp()->update_list(0);
                return true;
            }
            case 92: { // Ctrl + \, Go to root dir.
                string path = "/";
                frame->get_sp()->set_cwd(path);
                frame->get_sp()->update_list(0);
                return true;
            }
            case 65: { // Ctrl + A, Select all.
                frame->get_sp()->select_all();
                return true;
            }
            case 44: { // Ctrl + , , goto first
                frame->get_sp()->focus_first();
                return true;
            }
            case 46: { // Ctrl + . , goto last
                frame->get_sp()->focus_last();
                return true;
            }
            case 66: { // Ctrl + B: Add to bookmark
                frame->BookmarAdd();
                    return true;
                }
            case 78: { // Ctrl + n
                frame->get_sp()->focus_next();
                return true;
            }
            case 80: { // Ctrl + p
                frame->get_sp()->focus_prev();
                return true;
            }
            case 71: { // Ctrl + g: Goto direcoty.
                frame->get_sp()->goto_dir();
                return true;
            }
            case 72: { // Ctrl + h, toggle hidden files.
                frame->ShowHidden();
                return true;
            }
            case 73: {
                frame->show_file_info();
                return true;
            }
            case 82: { // Ctrl + R: Reload current directory.
                frame->update_fs();
                return true;
                }
            case 87: {
                exit(1);
                break;
            }
            default:
                return -1;
            }
        }
        else if (evt.ShiftDown()){
            switch (keycode) {
            case WXK_RETURN: {
                frame->open_in_other();
                frame->exchange_sp();
                return true;
            }
            case WXK_F4: {
                frame->get_sp()->edit_file(true);
                return true;
            }
            default:
                return -1;
            }
        }
        else if (evt.AltDown()) {
            switch (keycode) {
            case WXK_NUMPAD_ADD: {
                frame->get_sp()->select_same_ext();
                return true;
            }
            case WXK_NUMPAD_SUBTRACT: {
                frame->get_sp()->deselect_same_ext();
                return true;
            }
            case 315:{
                frame->get_sp()->goto_parent_dir();
                return true;
            }
            default:
                fprintf(stderr, "ERROR: Key: %d not implemente!\n", keycode);
                break;
            }
        }
        else{
            switch (keycode) {
            case WXK_F11: { // 10
                frame->compare_items();
                return true;
            }
            case 9:{
                frame->exchange_sp();
                return false;
            }
            case WXK_F5: { //F5
                frame->copy_or_move(true);
                return true;
            }
            case WXK_F6: { //F6
                frame->copy_or_move(false);
                return true;
            }
            case WXK_F8: { //F8
                frame->get_sp()->delete_file();
                return true;
            }
            case 27: { // ESC
                frame->get_sp()->toggle_search();
                return true;
            }
            case 315: { // Up arrow.
                frame->get_sp()->focus_prev();
                return true;
            }
            case 317: { // Down arrow.
                frame->get_sp()->focus_next();
                return true;
            }
            default:
                break;
            }
            return  -1;
        }
    }
    return -1;
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
