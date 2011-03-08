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


typedef struct _item {
    char *name;
    char *ext;
    int size;
    mode_t mode;
    time_t ctime;
    filetype type;
    int orig_id;
} item;

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
bool    is_image(const string &filename);
bool    name_is_valid(wxString &name);
void    format_time(const time_t *mytime, char *tmp);
string  get_extname(const char *name);
void    resort_time_based(vector<item *> &file_list);
void    resort_based_ext(vector<item *> &file_list);
void    resort_size_based(vector<item *> &file_list);
void    reverse_list(vector<item *> &file_list);
int 	get_filelist(string path, vector<item *> &file_list, string &reason,
                 bool show_hiden);
filetype get_file_type (const string &path);
string get_content(string &path);
int strsplit(const string str, const string sip, vector<string> &item_list);
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
