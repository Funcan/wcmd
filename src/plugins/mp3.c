#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int open_mp3(const char *path)
{
    char cmd[256] = {'\0'};
    sprintf(cmd, "killall mpg123 && mpg123 %s || mpg123 %s", path, path);
    return system(cmd);
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
