#include "test.h"
#include "ip_driver.h"

int main(void) {
    printf("Hello from test!\n");
    
    #ifdef TEST_DRIVER
        int result = test_ip_driver();
        
        assert(result == -1);
    #endif

    return 0;
}

int test_ip_driver(void)
{
    int result = -1;
    /* Path to floppy.img */
    const char * img_path = "E:\\Workspace\\project\\clone\\FATFileSystem\\images\\floppy.img";
    
    /* Initialize IP Driver */
    ip_driver_init(img_path);
    
    /* Read from IP Driver from sector 0 */
    unsigned char buffer[512];
    result = ip_driver_read(0x0, buffer, 512);
    assert(result == 0);
    printf("Read from IP Driver from sector 0: %s\n", buffer);
    
    #ifdef HEX_PRINT
        /* Print to IP Driver */
        for(int i = 0, j = 1; i < 512; i++, j++) {
            printf("%02X ", buffer[i]);
            if ((j > 0) && (j % 16 == 0)) {
                printf("\n");
            }
        }
        printf("\n");
    #endif

    /* Read from IP Driver from sector 7 */
    result = ip_driver_read(0x700, buffer, 512);
    assert(result == 0);
    printf("Read from IP Driver from sector 7: %s\n", buffer);
    
    #ifdef HEX_PRINT
        /* Print to IP Driver */
        for(int i = 0, j = 1; i < 512; i++, j++) {
            printf("%02X ", buffer[i]);
            if ((j > 0) && (j % 16 == 0)) {
                printf("\n");
            }
        }
        printf("\n");
    #endif
    
    /* Close IP Driver */
    ip_driver_close();

    /* TODO: Read from IP Driver */
    result = ip_driver_read(0x0, buffer, 512);
    /* Expected result: -1 */
    assert(result == -1);

    return result;
}