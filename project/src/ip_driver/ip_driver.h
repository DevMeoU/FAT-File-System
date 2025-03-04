#ifndef IP_DRIVER_H
#define IP_DRIVER_H

/*********************************************************************
 * Include
 *********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Function prototypes
 *********************************************************************/
int ip_driver_init(const char *img_path);
int ip_driver_read(unsigned int offset, unsigned char *buffer, size_t size);
int ip_driver_write(unsigned int offset, const unsigned char *buffer, size_t size);
void ip_driver_close();

/*********************************************************************
 * Close
 *********************************************************************/
#ifdef __cplusplus
}
#endif

#endif // IP_DRIVER_H

