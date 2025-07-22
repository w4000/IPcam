#ifndef __API_SYSTEM_INFORMATION__
#define __API_SYSTEM_INFORMATION__

int api_system_information(char *request_method, char *data, int id, char *attr, char *resp);
int api_system_factory(char *request_method, char *data, int id, char *attr, char *resp);
int api_system_factory_mac(char *request_method, char *data, int id, char *attr, char *resp);
#endif /*__API_SYSTEM_INFORMATION__*/
