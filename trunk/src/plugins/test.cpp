#include <libgen.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <vector>
using namespace std;


typedef struct _ext_base {
    string ext;
    int (* func)(const char *path);
} ext_base;

vector <ext_base *> list;


int __open_with_plugin(const char *file_name, const char *plugin_path)
{
    void *handle;
    int (*func)(const char *);
    ext_base *tmp;
    char *error;
    static char fn[64], ext_name[64];
    char *base_name = basename(strdup(plugin_path));
    memset(fn, 0, 64);
    memset(ext_name, 0, 64);
    strncpy(ext_name, base_name, strlen(base_name) - 3);

    handle = dlopen(plugin_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        return -1;
    }

    snprintf(fn, 5+strlen(base_name)-2, "open_%s", base_name);
    dlerror();    /* Clear any existing error */

    func = (int (*)(const char *)) dlsym(handle, fn);

    if ((error = dlerror()) != NULL)  {
        fprintf(stderr, "%s\n", error);
        return 0;
    }
    func(file_name);
    list.push_back(tmp);
    // dlclose(handle);
}

int open_with_plugin(const char *file_name)
{
    int len = strlen(file_name);
    char *ptr;
    char ext_name [1024] = {'\0'};
    char plugin_path[1024] = {'\0'};

    memset(ext_name, 0, 1024);
    strncpy(ext_name, file_name, len);
    for (ptr = ext_name + len - 1; ptr >= ext_name; ptr--) {
        if (*ptr == '.') {
            ptr++;
            break;
        }
    }

    sprintf(plugin_path, "%s/.config/wcmd/plugins/%s.so", getenv("HOME"),
            ptr);
    cout << "Check for " << plugin_path << endl;
    if (access(plugin_path, F_OK) == -1) {
        cout << "Not found!" << endl;;
        return -1;
    }
    else {
        pid_t childpid = fork();
        if (childpid == -1) {
            fprintf(stderr, "ERROR: failed to fork!\n");
            return -1;
        }
        else if (childpid > 0) {
            sleep(0.5);
            waitpid(childpid, NULL, NULL);
            return 0;
        }
        else {
            if ((childpid = fork())  == -1) {
                fprintf(stderr, "ERROR: failed to fork!\n");
                return -1;
            }
            else if (childpid == 0){
                return 0;
            }
            else {
                setsid();
                return __open_with_plugin(file_name, plugin_path);
            }
        }
    }

}

int
main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "ERROR: usage: %s args\n", argv[0]);
        return 0;
    }
    int i;
    for (i = 0; i < argc; i++) {
        open_with_plugin(argv[i]);
    }


    exit(EXIT_SUCCESS);
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
