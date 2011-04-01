#include "mylistctrl.h"
#include "fileselector.h"

#include "resources/mimetype/folder.xpm"
#include "resources/mimetype/generic.xpm"

#define LST_STYLE                                                       \
    wxLC_REPORT | wxBORDER_NONE | wxLC_EDIT_LABELS | wxLC_SORT_ASCENDING


MyListCtrl::MyListCtrl(wxWindow * parent, wxWindowID id):\
    wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize, LST_STYLE)
{
    wxImageList *imageList = NULL;
    wxString mime_plain(_("txt"));
    wxIconLocation icon_path;
    wxString mimetype;
    imageList = new wxImageList(22, 22);

    imageList->Add(wxIcon(folder, wxBITMAP_TYPE_XPM));  // 0
    imageList->Add(wxIcon(generic, wxBITMAP_TYPE_XPM)); // 1

    this->AssignImageList(imageList, wxIMAGE_LIST_SMALL);
    this->InsertColumn(0, _(""));
    this->InsertColumn(1, _("        File Name"));
    this->InsertColumn(2, _("Ext"));
    this->InsertColumn(3, _("Size"));
    this->InsertColumn(4, _("Last Modify"));

    this->SetColumnWidth(0, 25);
    this->SetColumnWidth(1, 340);
    this->SetColumnWidth(2, 80);
    this->SetColumnWidth(3, 90);
    this->SetColumnWidth(4, 95);
}

MyListCtrl::~MyListCtrl()
{
}

void MyListCtrl::append_item(int idx, ItemEntry *entry)
{
    wxString ext_name, msg;
    if (wxDirExists(entry->fn->GetFullPath()) == false) {
        this->InsertItem(idx, 1);
        this->SetItem(idx, 2, entry->get_ext());
    }
    else
        this->InsertItem(idx, 0);

    this->SetItemData(idx, idx);

    this->SetItem(idx, 1, entry->get_fullname());
    this->SetItem(idx, 3, entry->get_size_str());
    this->SetItem(idx, 4, entry->get_date());
}


void MyListCtrl::select_entry(int idx)
{
    this->SetItemState(idx,
                       wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED,
                       wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
}

void MyListCtrl::deselect_entry(int idx)
{
    this->SetItemState(idx, 0,
                       wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
}

void MyListCtrl::OnPopupClick(wxCommandEvent &evt)
{


    FSDisplayPane *fs = (FSDisplayPane *)GetParent();
    int id = evt.GetId();
	switch(id) {
    case ID_PopupMenu_Create_File:{
        fs->edit_file(true);
        break;
    }
    case ID_PopupMenu_Create_Dir:{
        fs->create_dir();
        break;
    }
    case ID_PopupMenu_Sort_Name:
    case ID_PopupMenu_Sort_Ext:
    case ID_PopupMenu_Sort_Time:
    case ID_PopupMenu_Sort_Size: {
        fs->sort_and_show(id - ID_PopupMenu_Sort_Start);
        break;
    }
    case ID_PopupMenu_Open:{
        fs->activate_item(-1);
        break;
    }
    default:

        break;
    }

}


void MyListCtrl::process_right_click(wxMouseEvent &evt)
{


	wxMenu *menu, *submenu;
    wxMenuItem *item;
    menu = new wxMenu();

    int flags = wxLIST_HITTEST_ABOVE;
    long item_id = this->HitTest(evt.GetPosition(), flags, NULL);

    if ((item_id == wxNOT_FOUND)) { // Right click in blank area.
        item = new wxMenuItem(menu, -1, _("Create:"));
        submenu = new wxMenu();
        submenu->Append(ID_PopupMenu_Create_File, _("File"));
        submenu->Append(ID_PopupMenu_Create_Dir, _("Directory"));
        submenu->Connect(wxEVT_COMMAND_MENU_SELECTED,
                         (wxObjectEventFunction)&MyListCtrl::OnPopupClick, NULL,
                         this);
        item->SetSubMenu(submenu);
        menu->Append(item);

        item = new wxMenuItem(menu, -1, _("Sort by:"));
        submenu = new wxMenu();
        submenu->Append(ID_PopupMenu_Sort_Name, _("Name"));
        submenu->Append(ID_PopupMenu_Sort_Name, _("Extension name"));
        submenu->Append(ID_PopupMenu_Sort_Name, _("Size"));
        submenu->Append(ID_PopupMenu_Sort_Name, _("Time"));
        submenu->Connect(wxEVT_COMMAND_MENU_SELECTED,
                         (wxObjectEventFunction)&MyListCtrl::OnPopupClick, NULL,
                         this);
        item->SetSubMenu(submenu);
        menu->Append(item);
    }
    else { // Right click on files or directories.
        menu->Append(ID_PopupMenu_Open, _("Open"));
        menu->Connect(wxEVT_COMMAND_MENU_SELECTED,
                      (wxObjectEventFunction)&MyListCtrl::OnPopupClick, NULL,
                      this);
    }

	PopupMenu(menu);
}

BEGIN_EVENT_TABLE(MyListCtrl, wxListCtrl)
EVT_RIGHT_DOWN(MyListCtrl::process_right_click)
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
