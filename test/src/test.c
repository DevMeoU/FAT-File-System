#include <stdio.h>
#include "ip_driver.h"

int main(void) {
    printf("Hello from test!\n");
    /* Path to floppy.img */
    const char * img_path = "E:\\Workspace\\project\\clone\\FATFileSystem\\images\\floppy.img";
    
    /* Initialize IP Driver */
    ip_driver_init(img_path);
    
    /* Read from IP Driver */
    unsigned char buffer[512];
    ip_driver_read(0x1000, buffer, 512);
    printf("Read from IP Driver: %s\n", buffer);
    
    /* Print to IP Driver */

    for(int i = 0, j = 0; i < 512; i++, j++) {
        printf("%02X ", buffer[i]);
        if (j != 0 && j % 16 == 0) {
            printf("\n");
        }
    }
    
    return 0;
}