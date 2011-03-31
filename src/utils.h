#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <error.h>
#include <errno.h>
#include <dirent.h>
#include <algorithm>
#include <sys/types.h>
#include <sys/wait.h>
#include <algorithm>
#include <cctype>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <syslog.h>
#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/mimetype.h>
#include <wx/filename.h>


#define char2wxstr(str) wxString(str, wxConvUTF8)
#define str2wxstr(str)  wxString(str.c_str(), wxConvUTF8)


using namespace std;

typedef enum _filetype{
    t_null = NULL,
    t_file = 1,
    t_dir,
    t_img,
    t_pdf,
    t_video,
    t_audio,
} filetype;

class ItemEntry {
public:
    ItemEntry(wxString path);
    ItemEntry(wxString dir_name, wxString file_name);
    virtual ~ItemEntry();
    bool         is_dir();
    wxString     get_ext();
    wxString     get_fullname();
    wxString     get_name();
    wxString     get_fullpath();
    wxString     get_size_str();
    wxULongLong  get_size();
    unsigned long long  get_file_size();
    wxString     get_date();
    wxString     get_parent();
    bool         is_text();
    bool         is_image();
    bool         is_file_exist();

    wxFileName  *fn;
    int          orig_id;

};

#ifdef DEBUG
#define PDEBUG(fmt, args...)                                \
    printf("%s(%d)-%s:\t",__FILE__,__LINE__,__FUNCTION__);        \
    printf("\033[31m"fmt"\033[0m", ##args);
#else
#define PDEBUG(fmt, args...)  ;
#endif

#define oops(ch, args...)                                               \
    {fprintf(stderr,ch,##args);perror("Reason from system call: ");return -1;}


bool    is_dir_exist(const string &path);
bool    is_dir_exist(const char *path);
bool    is_file_exist(const string &path);
bool    name_is_valid(wxString &name);
void    format_time(const time_t *mytime, char *tmp);
string  get_extname(const char *name);
void    resort_time_based(vector<ItemEntry *> &file_list);
void    resort_based_ext(vector<ItemEntry *> &file_list);
void    resort_size_based(vector<ItemEntry *> &file_list);
void    reverse_list(vector<ItemEntry *> &file_list);
filetype get_file_type (const string &path);
string get_content(wxString &path);
int strsplit(const string str, const string sip, vector<string> &item_list);
wxString size_2_wxstr(unsigned long long size);
int sort_name(ItemEntry *item1, ItemEntry *item2);
int sort_ext(ItemEntry *item1, ItemEntry *item2);
int sort_ext2(ItemEntry *item1, ItemEntry *item2);
int sort_time(ItemEntry *item1, ItemEntry *item2);
int sort_time2(ItemEntry *item1, ItemEntry *item2);
int sort_size(ItemEntry *item1, ItemEntry *item2);
int sort_size2(ItemEntry *item1, ItemEntry *item2);
void reverse_list(vector<ItemEntry *> &file_list);
unsigned long long WX_2_LL(wxLongLong n);
unsigned long long WX_2_LL(wxULongLong n);
#endif /* _UTILS_H_ */
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
