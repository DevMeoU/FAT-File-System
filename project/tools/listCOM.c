#include <windows.h>
#include <stdio.h>
#include <string.h>

#define CONFIG_KEY "available_com_ports"
#define PORTS_CAPACITY 1024

static int build_config_path(char *out, size_t out_size) {
    char exe_path[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, exe_path, sizeof(exe_path));
    if (len == 0 || len >= sizeof(exe_path)) {
        return 0;
    }

    char *slash = strrchr(exe_path, '\\');
    if (slash == NULL) {
        return 0;
    }
    *slash = '\0'; /* project/tools */

    slash = strrchr(exe_path, '\\');
    if (slash == NULL) {
        return 0;
    }
    *slash = '\0'; /* project */

    slash = strrchr(exe_path, '\\');
    if (slash == NULL) {
        return 0;
    }
    *slash = '\0'; /* repository root */

    return snprintf(out, out_size, "%s\\shell_config.cfg", exe_path) > 0;
}

static void append_port(char *ports, size_t ports_size, const char *port) {
    size_t used = strlen(ports);
    size_t port_len = strlen(port);
    size_t extra_space = used > 0 ? 1u : 0u;

    if (used + extra_space + port_len + 1u > ports_size) {
        return;
    }

    if (used > 0) {
        strcat(ports, " ");
    }
    strcat(ports, port);
}

static int write_available_ports(const char *config_path, const char *ports) {
    char temp_path[MAX_PATH];
    char line[1024];
    int wrote_ports_key = 0;
    int last_line_had_newline = 1;

    if (snprintf(temp_path, sizeof(temp_path), "%s.tmp", config_path) <= 0) {
        return 0;
    }

    FILE *in = fopen(config_path, "rb");
    FILE *out = fopen(temp_path, "wb");
    if (out == NULL) {
        if (in != NULL) {
            fclose(in);
        }
        return 0;
    }

    if (in != NULL) {
        while (fgets(line, sizeof(line), in) != NULL) {
            size_t line_len = strcspn(line, "\r\n");
            line[line_len] = '\0';

            if (strncmp(line, CONFIG_KEY "=", strlen(CONFIG_KEY) + 1u) == 0) {
                continue;
            }

            fputs(line, out);
            fputc('\n', out);
            last_line_had_newline = 1;

            if (strncmp(line, "com_port=", strlen("com_port=")) == 0) {
                fprintf(out, CONFIG_KEY "=\"%s\"\n", ports);
                wrote_ports_key = 1;
            }
        }
        fclose(in);
    }

    if (!wrote_ports_key && !last_line_had_newline) {
        fputc('\n', out);
    }
    if (!wrote_ports_key) {
        fprintf(out, CONFIG_KEY "=\"%s\"\n", ports);
    }
    fclose(out);

    if (!MoveFileExA(temp_path, config_path, MOVEFILE_REPLACE_EXISTING)) {
        DeleteFileA(temp_path);
        return 0;
    }

    return 1;
}

int main(void) {
    HKEY hKey;
    DWORD index = 0;
    char valueName[256];
    char valueData[256];
    char ports[PORTS_CAPACITY] = "";
    char config_path[MAX_PATH];
    DWORD nameSize;
    DWORD dataSize;
    DWORD type;
    LONG result;

    result = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                           "HARDWARE\\DEVICEMAP\\SERIALCOMM",
                           0, KEY_READ, &hKey);

    if (result != ERROR_SUCCESS) {
        printf("Cannot open serial registry (error %ld).\n", result);
        return 1;
    }

    while (1) {
        nameSize = sizeof(valueName);
        dataSize = sizeof(valueData) - 1u;
        result = RegEnumValueA(hKey, index, valueName, &nameSize,
                               NULL, &type, (LPBYTE)valueData, &dataSize);

        if (result == ERROR_NO_MORE_ITEMS) {
            break;
        }

        if (result == ERROR_SUCCESS && type == REG_SZ) {
            valueData[dataSize] = '\0';
            printf("%s\n", valueData);
            append_port(ports, sizeof(ports), valueData);
        }
        index++;
    }

    RegCloseKey(hKey);

    if (!build_config_path(config_path, sizeof(config_path))) {
        printf("Cannot resolve shell_config.cfg path.\n");
        return 1;
    }

    if (!write_available_ports(config_path, ports)) {
        printf("Cannot update %s.\n", config_path);
        return 1;
    }

    return 0;
}
