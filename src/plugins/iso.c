#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int open_iso(const char *path)
{
    int ret = 0;
    char template[256] = {'\0'};
    sprintf(template, "/tmp/ISO-XXXXXX");
    if (mktemp(template) == NULL) {
        fprintf(stderr, "ERROR: failed to make temporaty filename!\n");
        ret = -1;
        goto end;
    }

end:
    return ret;
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
