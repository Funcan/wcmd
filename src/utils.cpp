#include "utils.h"

using namespace std;

static bool time_sort=true;
static bool size_sort=true;
static bool ext_sort=true;



#define KB       1024
#define MB       KB*KB
#define GB       KB*KB*KB

#define B2K(n)       n/KB
#define B2M(n)       n/(KB*KB)
#define B2G(n)       n/(KB*KB*KB)


ItemEntry::ItemEntry(wxString path)
{
    fn = new wxFileName(path);
    orig_id = 0;
}

ItemEntry::ItemEntry(wxString dir_name, wxString file_name)
{
    fn = new wxFileName(dir_name, file_name);
    orig_id = 0;
}

ItemEntry:: ~ItemEntry()
{
    delete fn;
}

bool ItemEntry::is_dir()
{
    return wxDirExists(fn->GetFullPath());
}

wxString ItemEntry::get_ext()
{
    wxString ext = _("");
    if (fn->HasExt()) {
        ext = fn->GetExt();
    }
    return ext;
}

wxString ItemEntry::get_fullname()
{
    wxString name = _("");
    if (fn->HasName()) {
        name = fn->GetFullName();
    }
    return name;
}

wxString ItemEntry::get_name()
{
    wxString name = _("");
    if (fn->HasName()) {
        name = fn->GetName();
    }
    return name;
}

wxString ItemEntry::get_fullpath()
{
    return fn->GetFullPath();
}

wxString ItemEntry::get_date()
{
    return fn->GetModificationTime().FormatISODate();
}

wxString ItemEntry::get_size_str()
{
    wxString size;
    if (is_dir()) {
        size = wxT("4KB");
    }
    else {
        size = fn->GetHumanReadableSize();
    }
    return size;
}

wxString ItemEntry::get_parent()
{
    return (wxFileName::DirName(fn->GetFullPath())).GetFullPath();
}

wxULongLong ItemEntry::get_size()
{
    wxULongLong size;
    if (is_dir())
        size = wxDir::GetTotalSize(fn->GetFullPath());
    else
        size = fn->GetSize();
    return size;
}
/**
 * @name get_file_size - Get file size (return 0 if item is a directory.)
 * @return unsigned long long
 */
unsigned long long ItemEntry::get_file_size()
{
    unsigned long long size;
    if (is_dir())
        size = 0;
    else
        size = WX_2_LL(fn->GetSize());
    return size;
}

bool ItemEntry::is_image()
{
    bool ret = false;
    wxString mt;
    wxString path = fn->GetFullPath();
    if (fn->HasExt()) {
        wxFileType *ft = \
            wxTheMimeTypesManager->GetFileTypeFromExtension(fn->GetExt());
        if (ft != NULL) {
            if (ft->GetMimeType(&mt)) { // Can be treated as text file
                if (mt.Find(_("image/")) != wxNOT_FOUND)
                    ret = true;
            }
        }
    }
    return ret;
}

bool ItemEntry::is_text()
{
    bool ret = false;
    wxString mt;
    wxString path = fn->GetFullPath();
    if (fn->HasExt()) {
        wxFileType *ft = \
            wxTheMimeTypesManager->GetFileTypeFromExtension(fn->GetExt());
        if (ft != NULL) {
            if (ft->GetMimeType(&mt)) { // Can be treated as text file
                if (mt.Find(_("text")) != wxNOT_FOUND)
                    ret = true;
            }
        }
    }
    return ret;
}

bool ItemEntry::is_file_exist()
{
    return wxFileExists(fn->GetFullPath());
}


/**
 * @name endswith - Whether a string endswith specified subtring.
 * @param fullString -  full string
 * @param ending -  ending
 * @return bool
 */
bool endswith(string const &fullString, string const ending)
{
    unsigned int lastMatchPos = fullString.rfind(ending);
    bool isEnding = lastMatchPos != string::npos;
    unsigned int i;
    for(i = lastMatchPos + ending.length();
        (i < fullString.length()) && isEnding; i++)
        {
            if( (fullString[i] != '\n') &&
                (fullString[i] != '\r') )
                {
                    isEnding = false;
                }
        }

    return isEnding;
}
/**
 * @name string_to_lower - Converts string into lower case.
 * @param str -  string
 * @return string
 */
string string_to_lower(const string str)
{
    string new_str = str;
    transform(new_str.begin(), new_str.end(), new_str.begin(), ::tolower);
    return new_str;
}

/**
 * get_real_path - Get absolute path of in_path.
 * @in_path - Character in path
 *
 * Return: char*
 */
char *get_real_dirname(const char *in_path)
{
    char rest_name[1024] = {'\0'}, tmp[1024] = {'\0'};
    char *base_name = NULL, *rp = NULL,  *real_path = NULL;
    char *dir_name = dirname(strdup(in_path));
    int real_path_len = 0;

    while ((rp = realpath(dir_name, NULL)) == NULL) {
        memset(tmp, 0, 1024);
        base_name = basename(dir_name);
        if (strlen(rest_name)) {
            sprintf(tmp, "%s/%s", base_name, rest_name);
            strncpy(rest_name, tmp, strlen(tmp)+1);
        }
        else {
            sprintf(rest_name, "%s", base_name);
        }
        dir_name = dirname(dir_name);
    }

    real_path_len = strlen(rp) + strlen(rest_name) + 2;
    real_path = (char *)calloc(real_path_len, sizeof(char));
    sprintf(real_path, "%s/%s", rp, rest_name);
    return real_path;
}


bool name_is_valid(wxString &fn)
{
    if (fn.IsEmpty() || fn.Cmp(_("..")) == 0 || fn.Cmp(_(".")) == 0)
        return false;
    else
        return true;
}


bool is_file_exist(const string &path)
{
    return (access(path.c_str(), F_OK) == -1)?false:true;
}


bool is_dir_exist(const string &path)
{
    struct stat st;
    if ((stat(path.c_str(), &st) == -1) || !S_ISDIR(st.st_mode))
        return false;
    else
        return true;
}

bool is_dir_exist(const char * path)
{
    struct stat st;
    if ((stat(path, &st) == -1) || !S_ISDIR(st.st_mode))
        return false;
    else
        return true;
}

string  get_extname(const char *name)
{
    string extname;
    string bname(basename(strdup(name)));
    int pos = bname.rfind(".");
    if (pos == -1 || pos == 0)
        extname = "";
    else
        extname = bname.substr(pos+1);
    return extname;
}

void format_time(const time_t *mytime, char *tmp)
{
    struct tm *mytm = gmtime(mytime);
     sprintf(tmp, "%d/%d/%d %d:%d", mytm->tm_mon, mytm->tm_mday,
            mytm->tm_year+1900, mytm->tm_hour, mytm->tm_min);
    return ;
}

/**
 * @name sort_name - Sort function.
 * @param item1 -  item 1
 * @param item2 -  item 2
 * @return int
 */
int sort_name(ItemEntry *item1, ItemEntry *item2)
{
    return item1->get_name().Cmp(item2->get_name()) < 0 ? 1:0;
}

int sort_name2(ItemEntry *item1, ItemEntry *item2)
{
    return item1->get_name().Cmp(item2->get_name()) < 0 ? 0:1;
}

int sort_ext(ItemEntry *item1, ItemEntry *item2)
{
    return item1->get_ext().Cmp(item2->get_ext()) < 0 ? 0:1;
}

int sort_ext2(ItemEntry *item1, ItemEntry *item2)
{
    return item1->get_ext().Cmp(item2->get_ext()) < 0 ? 1:0;
}

int sort_time(ItemEntry *item1, ItemEntry *item2)
{
    return item1->fn->GetModificationTime().IsEarlierThan(\
        item2->fn->GetModificationTime());
}

int sort_time_2(ItemEntry *item1, ItemEntry *item2)
{
    return item1->fn->GetModificationTime().IsLaterThan(item2->fn->GetModificationTime());
}

int sort_size(ItemEntry *item1, ItemEntry *item2)
{
    int ret = item1->get_file_size() > item2->get_file_size();
    return ret;
}

int sort_size2(ItemEntry *item1, ItemEntry *item2)
{
    int ret = item1->get_file_size() <= item2->get_file_size();
    return ret;
}

void resort_time_based(vector<ItemEntry *> &file_list)
{
    time_sort = !time_sort;
    if (time_sort == true)
        stable_sort(file_list.begin(), file_list.end(), sort_time);
    else
        stable_sort(file_list.begin(), file_list.end(), sort_time_2);
    return;
}

void resort_size_based(vector<ItemEntry *> &file_list)
{
    size_sort = !size_sort;
    if (size_sort == true)
        stable_sort(file_list.begin(), file_list.end(), sort_size);
    else
        stable_sort(file_list.begin(), file_list.end(), sort_size2);
    return;
}

void resort_based_ext(vector<ItemEntry *> &file_list)
{
    ext_sort = !ext_sort;
    if (ext_sort == true)
        stable_sort(file_list.begin(), file_list.end(), sort_ext);
    else
        stable_sort(file_list.begin(), file_list.end(), sort_ext2);
    return;
}


void reverse_list(vector<ItemEntry *> &file_list)
{
    vector<ItemEntry *> tmp_list;
    vector<ItemEntry *>::iterator iter;
    for ( iter = file_list.begin(); iter < file_list.end(); iter++) {
        tmp_list.insert(tmp_list.begin(), *iter);
    }
    file_list.clear();
    for (iter = tmp_list.begin(); iter < tmp_list.end(); iter++) {
        file_list.push_back(*iter);
    }
}

int sort_length(string str1, string str2)
{
    return str1.size() < str2.size()?1:0;
}

int strsplit(const string str, const string sip, vector<string> &item_list)
{
    int startpos=0;
    int endpos=0;
    endpos = str.find_first_of(sip, startpos);
    while (endpos != -1) {
        if (endpos != 0) // If str begins with sip, skip the first one.
            item_list.push_back(str.substr(startpos, endpos-startpos));
        startpos = endpos+1; //jump past sep
        endpos = str.find_first_of("|", startpos); // find next
        if(endpos == -1){
            item_list.push_back(str.substr(startpos));
        }
    }
    stable_sort(item_list.begin(), item_list.end(), sort_length);
    return item_list.size();
}

/**
 * @name get_content - Get content of file.
 * @param path -  path
 * @return string
 */
string get_content(wxString &path)
{
    int fd = open(path.mb_str(), O_RDONLY);
    string val;
    if (fd != -1) {
        struct stat stats;
        if (stat(path.mb_str(), &stats) == 0){
            char *addr = (char *)mmap(NULL, stats.st_size, PROT_READ,
                                      MAP_PRIVATE,  fd, 0);
            if (addr != MAP_FAILED) {
                val = string(addr);
                munmap(addr, stats.st_size);
            }
        }
        close(fd);
    }
    return val;
}

/**
 * @name size_2_wxstr - Conver size into wxString.
 * @param size - Number of size
 * @return wxString
 */
wxString size_2_wxstr(unsigned long long size)
{
    wxString desc;
    if (size < KB) {
        desc.Printf(wxT("%llu B"), size);
    }
    else if (size < MB){
        desc.Printf(wxT("%'.1f KB"), (double)B2K(size));
    }
    else if (size < GB) {
        desc.Printf(wxT("%'.1f MB"), (double)B2M(size));
    }
    else {
        desc.Printf(wxT("%'.1f GB"), (double)B2G(size));
    }
    return desc;
}


unsigned long long WX_2_LL(wxLongLong n)
{
    unsigned long long hi;
    hi = n.GetHi();
    hi <<= 32;
    hi += n.GetLo();
    return hi;
}

unsigned long long WX_2_LL(wxULongLong n)
{
    unsigned long long hi;
    hi = n.GetHi();
    hi <<= 32;
    hi += n.GetLo();
    return hi;
}

wxString get_parent_dir(wxString path)
{
    wxString tmp = wxFileName::DirName(path).GetFullPath().BeforeLast(wxT('/')).BeforeLast(wxT('/'));
    if (tmp.IsEmpty()) {
        tmp = _("/");
    }
    return tmp;
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
