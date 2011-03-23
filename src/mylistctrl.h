#ifndef _MYLISTCTRL_H_
#define _MYLISTCTRL_H_

#include <wx/aboutdlg.h>
#include <wx/event.h>
#include <wx/imaglist.h>
#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/wx.h>
#include <wx/iconloc.h>

#include "utils.h"
#include "global.h"



class MyListCtrl: public wxListCtrl {

public:
    MyListCtrl(wxWindow *parent, wxWindowID id);
    ~MyListCtrl();
    void append_item(int idx, ItemEntry *entry);
    void select_entry(int idx);
    void deselect_entry(int idx);
private:
    void OnPopupClick(wxCommandEvent &evt);
    void process_right_click(wxMouseEvent &evt);
    DECLARE_EVENT_TABLE()
};

#endif /* _MYLISTCTRL_H_ */
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
