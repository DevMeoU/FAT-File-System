#include "test.h"
#include "ip_driver.h"

int main(void) {
    int measuring = 0;
    printf("Hello from test!\n");
    
    #ifdef TEST_DRIVER
        /* Test IP Driver */
        printf("Testing IP Driver...\n");
        fflush(stdout);
        measuring = measure_function(test_ip_driver);
        printf("Time measured: %dms\n", measuring);
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

int measure_function(int (*args)(void)) {
    clock_t start, end;
    double cpu_time_used;

    start = clock();
    // Code to measure
    int result = args();
    assert(result == -1);

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    return (int)(cpu_time_used * 1000); // Return time in milliseconds
}
