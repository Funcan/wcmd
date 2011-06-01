#include "wxconfig.h"

const string  gtk_prefix("file://");
const int PRE_LEN = gtk_prefix.length();
vector<string> bookmarks;

Config::Config()
{
    string home = getenv("HOME");
    config_path = home + "/.config/wcmd/config";
    bookmark_path = home + "/.gtk-bookmarks";

    build_config_list(home);
    vector<config_entry>::iterator iter;
    int ret;
    if ((ret = checkpath(config_path)) == -1) {
        fprintf(stderr, "ERROR: failed to check path!\n");
    }
    else {
        read_configs();
        read_bookmarks();
    }
}


Config::~Config()
{
    dump2file();
}

void Config::build_config_list(string &home)
{
    config_entry auto_last_path_l = {
        "auto_last_path_l", home, TYPE_STR
    };
    entry_list.push_back(auto_last_path_l);

    config_entry auto_last_path_r = {
        "auto_last_path_r", home, TYPE_STR
    };
    entry_list.push_back(auto_last_path_r);

    config_entry auto_size_x = {
        "auto_size_x", "1280", TYPE_STR
    };
    entry_list.push_back(auto_size_x);

    config_entry auto_size_y = {
        "auto_size_y", "850", TYPE_STR
    };
    entry_list.push_back(auto_size_y);

    config_entry show_hidden = {
        "show_hidden", "false", TYPE_BOOL
    };
    entry_list.push_back(show_hidden);

    config_entry editor = {
        "editor", "/usr/bin/emacsclient",TYPE_STR
    };
    entry_list.push_back(editor);

    config_entry app_terminal = {
        "app_terminal", "sakura", TYPE_STR
    };
    entry_list.push_back(app_terminal);

    config_entry img_editor = {
        "img_editor", "gimp", TYPE_STR
    };
    entry_list.push_back(img_editor);

    config_entry img_viewer = {
        "img_viewer", "display", TYPE_STR
    };
    entry_list.push_back(img_viewer);

    config_entry pdf_reader = {
        "pdf_reader", "evince", TYPE_STR
    };
    entry_list.push_back(pdf_reader);

    config_entry video_player = {
        "video_player", "mplayer", TYPE_STR
    };
    entry_list.push_back(video_player);

    config_entry diff_tool = {
        "diff_tool", "meld", TYPE_STR
    };
    entry_list.push_back(diff_tool);
}

string Config::get_config(const string key)
{
    vector<config_entry>::iterator iter;
    string val = "";
    for (iter = entry_list.begin(); iter < entry_list.end(); iter++) {
        if ((*iter).desc == key) {
            val = (*iter).value;
            return val;
        }
    }
    return "";
}

void Config::set_config(const string key, const string val)
{
    vector<config_entry>::iterator iter;
    for (iter = entry_list.begin(); iter < entry_list.end(); iter++) {
        if ((*iter).desc == key) {
                (*iter).value = val;
                break;
        }
    }
}

void Config::dump2file()
{
    dump_config();
    dump_bookmarks();
}

void Config::dump_config()
{
    ofstream fout;
    string path = string(getenv("HOME"));
    string tmp;
    path += "/.config/wcmd/config";
    fout.open(path.c_str());
    if(fout == NULL){
        fprintf(stderr, "ERROR: Failed to open config file!\n");
        exit(0);
    }

    vector<config_entry>::iterator iter;

    for (iter = entry_list.begin(); iter < entry_list.end(); iter++) {

        tmp = (*iter).desc + " = " + (*iter).value;
        fout << tmp << endl;
    }
    fout.close();
}

void Config::dump_bookmarks()
{
    ofstream fout;
    string path = string(getenv("HOME"));
    path += "/.gtk-bookmarks";
    fout.open(path.c_str());
    if(fout == NULL){
        fprintf(stderr, "ERROR: Failed to open config file!\n");
        exit(0);
    }

    vector<string>::iterator iter;
    for (iter = bookmarks.begin(); iter < bookmarks.end(); iter++) {

        fout << gtk_prefix + *iter << endl;
    }

    fout.close();
}

/**
 * Reads configurations, fill them into enter_list.
 * @return void
 */
void Config::read_configs()
{
    ifstream fin;
    fin.open(config_path.c_str());
    if (!fin) {

    }
    else {
        int ret;
        string str, key, val;
        vector<config_entry>::iterator iter;
        while (getline(fin, str) != NULL) {
            if ((ret = splitstr(str, key, val)) != 0) {
                fprintf(stderr, "ERROR: Failed to parse string\n");
                continue;
            }
            for (iter=entry_list.begin();iter<entry_list.end();iter++){
                if (key == (*iter).desc) {
                    (*iter).value = val;
                    break;
                }
            }
        }
    }
}

/**
 * Read bookmarks, fill them into bookmarks.
 * @return void
 */
void Config::read_bookmarks()
{
    ifstream fin;
    fin.open(bookmark_path.c_str());
    if (!fin) {

    }
    else {
        string str, key, val;
        while (getline(fin, str) != NULL) {

            str = str.substr(PRE_LEN, str.length() - PRE_LEN);
            if (is_dir_exist(str))
                bookmarks.push_back(str);
            else
                fprintf(stderr, "ERROR: invalid bookmark! %s\n", str.c_str());
        }
    }
}

int Config::checkpath(const string &path)
{
    if (!is_dir_exist(dirname(strdup(path.c_str())))) {
        char cmd[256] = {'\0'};
        sprintf(cmd, "mkdir -p %s", path.c_str());
        return system(cmd);
    }
    return 0;
}

int Config::splitstr(string &str, string &key, string &val)
{
    size_t pos = str.find("=");
    if (pos == string::npos ){

        return -1;
    }
    key.assign(str,0, pos-1);
    val.assign(str, pos+1, str.length() - pos);
    strip(key);
    strip(val);
    if (key.empty() || val.empty()) {

        return -1;
    }
    return 0;
}

void Config::strip(string &str)
{
    unsigned int i = 0;
    while (str[i] == ' ' && i < str.length()) {
        i++;
    }
    str.erase(0, i);
}

Config config;
