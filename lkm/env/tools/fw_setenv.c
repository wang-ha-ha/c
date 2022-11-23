#include <stdio.h>
#include <string.h>

extern unsigned short crc16_ccitt(const void *buf, int len);
extern void fw_printenv(int argc, char *argv[]);
extern void fw_setenv(int argc, char *argv[]);

void usage(void)
{
    fprintf(stderr,
"Usage: fw_printenv [NAME]\n\n"

"    Print environment variables NAME if specified, or print all variables\n\n"

"Usage:   fw_setenv NAME [VALUE]       # Set or unset environment variable NAME\n"
"         fw_setenv -s FILE            # Set variables from FILE\n\n"
"         fw_setenv -c                 # Clear all variables\n\n"
"         fw_setenv                    # Equal to fw_printenv with no args \n\n"

"Example: fw_setenv wifi_ssid ovlinux  # Set wifi_ssid=ovlinux\n"
"         fw_setenv wifi_ssid          # Unset wifi_ssid\n");
}

int main(int argc, char *argv[])
{
    if (argc >= 2 && strcmp(argv[1], "-h") == 0) {
        usage();
        return 0;
    }

    if (strstr(argv[0], "fw_printenv")) {
        fw_printenv(argc, argv);
    }
    else if (strstr(argv[0], "fw_setenv")) {
        fw_setenv(argc, argv);
    }
    else
        usage();

    return 0;
}