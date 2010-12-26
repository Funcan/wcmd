#include "wxconfig.h"

Config::Config()
{
    string home = getenv("HOME");

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

    config_entry bookmarks = {
        "auto_bookmarks", "", TYPE_STR
    };
    entry_list.push_back(bookmarks);

    int ret;
    string path = home + "/.config/wcmd/config";
    vector<config_entry>::iterator iter;
    if ((ret = checkpath(path)) == -1) {
        fprintf(stderr, "ERROR: failed to check path!\n");
    }
    else {
        ifstream fin;
        fin.open(path.c_str());
        if (!fin) {
            PDEBUG ("Failed to open file: %s\n", path.c_str());
        }
        else {
            string str, key, val;
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

#ifdef DEBUG
    for (iter=entry_list.begin();iter<entry_list.end();iter++){
        PDEBUG ("Name: %s, value: %s\n", (*iter).desc.c_str(),
                (*iter).value.c_str());
    }
#endif
    PDEBUG ("Leave\n");
}


Config::~Config()
{
    PDEBUG ("Bye bye!\n");
    dump2file();
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
    PDEBUG ("Called;\n");

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
        PDEBUG ("%s = %s\n", (*iter).desc.c_str(), (*iter).value.c_str());
        tmp = (*iter).desc + " = " + (*iter).value;
        fout << tmp << endl;
    }
    fout.close();
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
    if (pos == -1 ){
        PDEBUG ("No '=' found!\n");
        return -1;
    }
    key.assign(str,0, pos-1);
    val.assign(str, pos+1, str.length() - pos);
    strip(key);
    strip(val);
    if (key.empty() || val.empty()) {
        PDEBUG ("Key or val is empty!\n");
        return -1;
    }
    return 0;
}

void Config::strip(string &str)
{
    int i = 0;
    while (str[i] == ' ' && i < str.length()) {
        i++;
    }
    str.erase(0, i);
}

int Config::get_bookmarks(vector<string> &items)
{
    return strsplit(get_config("auto_bookmarks"), string("|"), items);
}

void Config::set_bookmarks(const vector<string> &items)
{
    string path;
    for (int i = 0; i < items.size(); i++) {
        path += "|" + items[i];
    }
    set_config("auto_bookmarks", path);
    dump2file(); // Write config file immediately after bookmark added.
}


Config config;
