#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include "utils.h"
#include "misc.h"


using namespace std;

typedef enum _CONFIG_TYPE{
    TYPE_STR = 0,
    TYPE_INT,
    TYPE_BOOL,
    TYPE_LIST,
} CONFIG_TYPE;


typedef struct _config_entry {
    string desc;
    string value;
    CONFIG_TYPE type;
} config_entry;


class Config {
public:
    Config();
    virtual ~Config();
    void   set_config(const string key, const string val);
    string get_config(const string key);
    void   dump2file();
    void read_configs();
    void read_bookmarks();
    vector<config_entry> entry_list;
private:
    int    splitstr(string &str, string &key, string &val);
    void   strip(string &str);
    int    checkpath(const string &path);
    void build_config_list(string &home);
    void dump_config();
    void dump_bookmarks();
    string bookmark_path, config_path;
};

extern Config config;
extern vector<string> bookmarks;

#endif /* _CONFIG_H_ */
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
