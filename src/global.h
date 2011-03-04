#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <vector>
#include <wx/wx.h>
#include "utils.h"

enum{
    ID_Quit = 1,
    ID_About,
    ID_Option,

    ID_View = 31,
    ID_Edit,
    ID_Copy,
    ID_Move,
    ID_NewDie,
    ID_NewFile,
    ID_Terminal,

    ID_View_ShowHidden,

    ID_Delete,
    ID_SplitWindow = 101,
    ID_Sp1,
    ID_Lst1,
    ID_Sp2,
    ID_Search1,
    ID_Lst2,
    ID_search2,

    ID_BookmarkEdit = 9999,
    ID_BookmarkAdd = 10000,

    ID_BookmarkManage = 20000,

    ID_BookmarkManageAdd = 20100,
    ID_BookmarkManageDel,
    ID_BookmarkRedraw = 20200,

    ID_PopupMenu_Create_File = 20300,
    ID_PopupMenu_Create_Dir,
    ID_PopupMenu_Sort_Start = 20400,
    ID_PopupMenu_Sort_Name,
    ID_PopupMenu_Sort_Size,
    ID_PopupMenu_Sort_Time,
    ID_PopupMenu_Sort_Ext,
    ID_PopupMenu_Open = 20500,

};

enum {
    COL_IMG = 0,
    COL_NAME,
    COL_EXT,
    COL_SIZE,
    COL_TIME,
    COL_MODE,
};

typedef enum _FS_id{
    fs1 = 1,
    fs2,
} FS_id;



#endif /* _GLOBAL_H_ */
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
