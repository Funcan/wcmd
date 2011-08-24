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

const int SENTRY = 0;
const int DENTRY = 1;

typedef enum _CONFIG_TYPE{
    TYPE_STR = 0,
    TYPE_INT,
    TYPE_BOOL,
    TYPE_LIST,
} CONFIG_TYPE;


typedef struct _plain_config {
    string desc;
    string value;
    CONFIG_TYPE type;
} plain_config;

typedef struct _desktop_entry {
    string name;
    string icon;
    string exec;
} desktop_entry;

typedef enum _SERVER_TYPE{
    SSH = 0,
    SFTP,
    FTP,
    SMB,
} SERVER_TYPE;

typedef struct _server_entry {
    SERVER_TYPE type;
    string name;
    string ip;
    string user;
    string passwd;
} server_entry;

class Config {
public:
    Config();
    virtual ~Config();
    void   set_config(const string key, const string val);
    void  *get_dentry(int type, bool reset=true);
    string get_config(const string key);
    void   dump2file();
    void   read_configs();
    void   read_bookmarks();
    void add_dentry(const string &name, const string &exec,
                    const string &icon="");
    void del_dentry(const string &name);
    vector<plain_config> entry_list;
    server_entry *get_sentry(int id);
private:
    int    splitstr(string &str, string &key, string &val);
    void   strip(string &str);
    int    checkpath(const string &path);
    void   build_config_list(string &home);
    void   dump_config();
    void   dump_bookmarks();
    string bookmark_path, config_path;

    vector<desktop_entry> dentry_list;
    vector<server_entry> server_list;

    vector <desktop_entry>::iterator dentry_iter;
    vector <server_entry>::iterator server_iter;

    unsigned int dentry_pos;
    unsigned int sentry_pos;

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
