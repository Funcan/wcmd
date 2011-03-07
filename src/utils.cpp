#include "utils.h"

using namespace std;

static bool time_sort=true;
static bool size_sort=true;
static bool ext_sort=true;

typedef struct _magic_filetype {
    filetype ftype;
    string magics;
} magic_filetype;


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


bool name_is_valid(string &fn)
{
    if (fn.empty() || fn.compare("..") == 0 || fn.compare(".") == 0 ||
        fn.find("/") != string::npos)
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


bool is_image(const string &filename)
{
    if (get_file_type(filename) == t_img)
        return true;
    else
        return false;
}


/**
 * @name sort_name - Sort function.
 * @param item1 -  item 1
 * @param item2 -  item 2
 * @return int
 */
int sort_name(item *item1, item *item2)
{
    char *p1 = item1->name;
    char *p2 = item2->name;
    while (*p1 && *p2) {
        if (*p1 == *p2){
            p1++;
            p2++;
            continue;
        }
        else if (*p1 < *p2)
            return 1;
        else
            return 0;
    }
    return strlen(item1->name)<strlen(item2->name)?1:0;
}

int sort_ext(item *item1, item *item2)
{
    char *p1 = item1->ext;
    char *p2 = item2->ext;
    while (*p1 && *p2) {
        if (*p1 == *p2){
            p1++;
            p2++;
            continue;
        }
        else if (*p1 < *p2)
            return 1;
        else
            return 0;
    }
    return strlen(item1->name)<strlen(item2->name)?1:0;
}

int sort_ext2(item *item1, item *item2)
{
    char *p1 = item1->ext;
    char *p2 = item2->ext;
    while (*p1 && *p2) {
        if (*p1 == *p2){
            p1++;
            p2++;
            continue;
        }
        else if (*p1 < *p2)
            return 0;
        else
            return 1;
    }
    return strlen(item1->name)<strlen(item2->name)?1:0;
}

int sort_time(item *item1, item *item2)
{
    return item1->ctime < item2->ctime?1:0;
}

int sort_time_2(item *item1, item *item2)
{
    return item1->ctime > item2->ctime?1:0;
}

int sort_size(item *item1, item *item2)
{
    return item1->size < item2->size?1:0;
}

int sort_size2(item *item1, item *item2)
{
    return item1->size > item2->size?1:0;
}

void resort_time_based(vector<item *> &file_list)
{
    time_sort = !time_sort;
    if (time_sort == true)
        stable_sort(file_list.begin(), file_list.end(), sort_time);
    else
        stable_sort(file_list.begin(), file_list.end(), sort_time_2);
    return;
}

void resort_size_based(vector<item *> &file_list)
{
    size_sort = !size_sort;
    if (size_sort == true)
        stable_sort(file_list.begin(), file_list.end(), sort_size);
    else
        stable_sort(file_list.begin(), file_list.end(), sort_size2);
    return;
}

void resort_based_ext(vector<item *> &file_list)
{
    ext_sort = !ext_sort;
    if (ext_sort == true)
        stable_sort(file_list.begin(), file_list.end(), sort_ext);
    else
        stable_sort(file_list.begin(), file_list.end(), sort_ext2);
    return;
}

int get_filelist(string path, vector<item *> &file_list, string &reason,
                 bool show_hiden)
{
    DIR *dirp = NULL;
    struct dirent *dp = NULL;
    if ((dirp = opendir(path.c_str())) == NULL) {
        reason = strerror(errno);
        return -1;
    }

    item *fn;
    int size = sizeof(item);
    struct stat stats;
    vector<item *> tmp;
    vector<item *>::iterator iter;
    iter = file_list.begin();
    while ((dp = readdir(dirp)) != NULL) {
        if (stat(dp->d_name, &stats) == -1) {
            continue;
        }
        if ((!show_hiden && *(dp->d_name) == '.') ||
            strcmp(dp->d_name, "..") == 0 || strcmp(dp->d_name, ".")== 0) {
            continue;
        }
        fn = new item;
        fn = (item *)calloc(1, size);
        fn->name = strdup(dp->d_name);
        fn->ext = strdup(get_extname(fn->name).c_str());
        fn->size = stats.st_size;
        fn->mode = stats.st_mode;
        fn->ctime = stats.st_ctime;
        if (S_ISDIR(fn->mode)){
            fn->type = t_dir;
            file_list.push_back(fn);
        }
        else{
            fn->type = t_file;
            tmp.push_back(fn);
        }
    }
    stable_sort(file_list.begin(), file_list.end(), sort_name);
    if (tmp.size()) {
        stable_sort(tmp.begin(), tmp.end(), sort_name);
        for (iter = tmp.begin(); iter != tmp.end(); iter++) {
            file_list.push_back(*iter);
        }
    }
    closedir(dirp);
    return 0;
}

void reverse_list(vector<item *> &file_list)
{
    vector<item *> tmp_list;
    vector<item *>::iterator iter;
    for (iter = file_list.begin(); iter < file_list.end(); iter++) {
        tmp_list.insert(tmp_list.begin(), *iter);
    }
    file_list.clear();
    for (iter = tmp_list.begin(); iter < tmp_list.end(); iter++) {
        file_list.push_back(*iter);
    }
}

magic_filetype mtype[] = {
    { t_img, "image data"},
    { t_pdf, "PDF document"},
    { t_video, "RealMedia file|Microsoft ASF|MPEG|RIFFOgg data|Matroska data"},
    { t_audio, "Audio file"},
    { t_null, ""},
};

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

void get_magics(const string str, vector<string> &item_list)
{
    string sip("|");
    if (!strsplit(str, sip, item_list))
        fprintf(stderr, "Faild to get magics for: %s\n",
                str.c_str());
    return;
}

filetype get_file_type (const string &path)
{
    int i = 0;
    string cmd;
    vector <string> item_list;

    while (mtype[i].ftype != t_null) {
        if (mtype[i].magics.find("|") == string::npos) {
            cmd = "file \"" + path + "\" | grep \"" + mtype[i].magics+\
                "\" >/dev/null 2>&1";
            if (system(cmd.c_str()) == 0)
                return mtype[i].ftype;
        }
        else {
            get_magics(mtype[i].magics, item_list);
            vector<string>::iterator iter;
            for (iter = item_list.begin(); iter < item_list.end(); iter++) {
                cmd = "file \"" + path + "\" | grep \"" + *iter + \
                    "\" >/dev/null 2>&1";
                if (system(cmd.c_str()) == 0)
                    return mtype[i].ftype;
            }
        }
        i++;
    }
    return t_file;
}
/**
 * @name get_content - Get content of file.
 * @param path -  path
 * @return string
 */
string get_content(string &path)
{
    int fd = open(path.c_str(), O_RDONLY);
    string val;
    if (fd != -1) {
        struct stat stats;
        if (stat(path.c_str(), &stats) == 0){
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
