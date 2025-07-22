#include <stdio.h>
#include <string.h>
#include "fcgi_common.h"
#include "api_system_information.h"


#include <stdlib.h>

int is_valid_ip(const char *ip) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip, &(sa.sin_addr));
    return result != 0;
}


int __system (const char *command) {
	int ret;


	// pthread_mutex_lock (& system_lock);
	ret = system (command);
	// pthread_mutex_unlock (& system_lock);

	return ret;
}


#if 0//w4000
static int put_system_factory_mac(char *data, int id, char *attr, char *resp)
{
    int ret;
    json_error_t error;
    json_t *root = NULL;
    json_t *factory = NULL;
    char *mac = NULL;
    char buf[1024];

    _CAMSETUP setup;
    _CAMSETUP_EXT setup_ext;

    // 입력된 data 출력
    printf("Received data: %s\n", data);

    load_setup(&setup, &setup_ext);

    root = json_loads(data, JSON_DISABLE_EOF_CHECK, &error);
    if (root == NULL) {
        printf("JSON parsing error: %s (line %d, column %d)\n", error.text, error.line, error.column);
        ret = 400;
        goto Fail;
    }

    factory = json_object_get(root, "factory");
    if (!json_is_object(factory)) {
        printf("JSON 'factory' object not found.\n");
        ret = 400;
        goto Fail;
    }

    mac = (char*)json_obj_str(factory, "mac");
    if (mac == NULL || strlen(mac) != 17) { // MAC 주소 길이 확인
        printf("Invalid MAC address: %s\n", mac ? mac : "NULL");
        ret = 400;
        goto Fail;
    }

    // MAC 주소 설정
    printf("Setting MAC address: %s\n", mac);
    setup_ini_load();
    setup_ini_set_str("cam_setup", "NET_MAC", mac);

    if (fork() == 0) {
        execlp("fw_setenv", "fw_setenv", "ethaddr", mac, (char *)NULL);
        exit(1); // 실패 시 종료
    }
    wait(NULL);

    notify_update_setup(UPDATE_SYSTEM_REBOOT);

    json_decref(root);
    return 200;

Fail:
    if (root)
        json_decref(root);
    return ret;
}


#else

static int put_system_factory_mac(char *data, int id, char *attr, char *resp)
{
	int ret;
	json_error_t error;
	json_t *root;
	json_t *factory = NULL;

	char *mac = NULL;
	char buf[1024];
	// char cmd[1024];

	
	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;

	load_setup(&setup, &setup_ext);
	root = json_loads(data, JSON_DISABLE_EOF_CHECK, &error);


	if(root == NULL) {
		ret = 400;
		goto Fail;
	}


	const char *key;
	json_object_foreach(root, key, factory) {
		if(strncasecmp(key, "factory", 7) == 0) {
			break;
		}
		factory = NULL;
	}


	if(factory == NULL) {
		ret = 400;
		goto Fail;
	}

	mac = (char*)json_obj_str(factory, "mac");


	// fw_setenv ethaddr 00:55:7B:B5:70:F7
	if (mac != NULL)
	{
		// medel_spec_value_add("NET_MAC", mac);
		setup_ini_load();
		setup_ini_set_str("cam_setup","NET_MAC", mac);

		sprintf(buf, "fw_setenv ethaddr %s", mac);
		system(buf);
		sleep(1);
	}

	notify_update_setup(UPDATE_SYSTEM_REBOOT);

	json_decref(root);
	return 200;
Fail:


	json_decref(root);
	return 520;
}


#endif

static int get_system_factory_mac(char *data, int id, char *attr, char *resp)
{
	json_t *root;
	json_error_t error;
	json_t *factory = json_object();
	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;
	

#if 0
#else 
#endif 
		load_setup(&setup, &setup_ext);

		char mac[64];
		unsigned char mac_addr[6];


		__if_get_mac(ETHERNET_DEV, mac_addr);
		sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
			mac_addr [0], mac_addr [1], mac_addr [2], mac_addr [3], mac_addr [4], mac_addr [5]);
			
		factory = json_pack_ex(&error, 0, "{s:s}",	\
				"mac", mac);

		root = json_pack_ex(&error, 0, "{s:o*}", "factory",
				factory);


	if(json_to_string(root, resp) < 0) {
		goto Fail;
	}


	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;

}


static int put_system_factory(char *data, int id, char *attr, char *resp)
{
	int ret;
	json_error_t error;
	json_t *root;
	json_t *factory = NULL;
	json_t *user = NULL;
	json_t *system = NULL;
	json_t *network = NULL;
	json_t *interface = NULL;
	json_t *port = NULL;

	char *dhcp;
	char *ip = NULL;
	char *mask = NULL;
	char *gateway = NULL;
	// char *mac = NULL;
	int rtsp_port;
	int http_port;


	char *userid = NULL;
	char *password = NULL;

	char *name = NULL;
	char *manufacturer = NULL;
	char *model = NULL;

	char buf[1024];
	//char cmd[1024];

	
	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;

	load_setup(&setup, &setup_ext);
	root = json_loads(data, JSON_DISABLE_EOF_CHECK, &error);


	if(root == NULL) {
		ret = 400;
		goto Fail;
	}

	const char *key;
	json_object_foreach(root, key, factory) {
		if(strncasecmp(key, "factory", 7) == 0) {
			break;
		}
		factory = NULL;
	}

	if(factory == NULL) {
		ret = 400;
		goto Fail;
	}





	// json_t *value2 = NULL;
	// json_t *value3 = NULL;

	system = json_object_get(factory, "system");
	if(system == NULL) {
		goto Fail;
	}

	user = json_object_get(system, "user");
	if(user == NULL) {
		goto Fail;
	}	
	


	network = json_object_get(factory, "network");
	if(network == NULL) {
		goto Fail;
	}

	interface = json_object_get(network, "interface");
	if(interface == NULL) {
		goto Fail;
	}


	port = json_object_get(network, "port");
	if(port == NULL) {
		goto Fail;
	}


	userid = (char*)json_obj_str(user, "id");
	password = (char*)json_obj_str(user, "password");
	name = (char*)json_obj_str(system, "name");
	manufacturer = (char*)json_obj_str(system, "manufacturer");
	model = (char*)json_obj_str(system, "model");


	//sprintf (cmd, "echo \"%s : %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__);
	//__system (cmd);


	//sprintf (cmd, "echo \"userid: %s\" | cat >> /mnt/ipm/log.txt", userid);
	//__system (cmd);
	//sprintf (cmd, "echo \"password: %s\" | cat >> /mnt/ipm/log.txt", password);
	//__system (cmd);
	//sprintf (cmd, "echo \"name: %s\" | cat >> /mnt/ipm/log.txt", name);
	//__system (cmd);
	//sprintf (cmd, "echo \"manufacturer: %s\" | cat >> /mnt/ipm/log.txt", manufacturer);
	//__system (cmd);
	//sprintf (cmd, "echo \"model: %s\" | cat >> /mnt/ipm/log.txt", model);
	//__system (cmd);


	if (userid != NULL && password != NULL && name != NULL && manufacturer != NULL && model != NULL)
	{
		//sprintf (cmd, "echo \"%s : %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__);
		//__system (cmd);
		setup_ini_set_str("SYSTEM_USER_NAME", userid);
		//sprintf (cmd, "echo \"%s : %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__);
		//__system (cmd);
		setup_ini_set_str("SYSTEM_USER_PASSWD", password);
		//sprintf (cmd, "echo \"%s : %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__);
		//__system (cmd);
		setup_ini_set_str("INFO_MODEL_NAME", name);
		//sprintf (cmd, "echo \"%s : %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__);
		//__system (cmd);
		setup_ini_set_str("SYSTEM_MANUFACTURER", manufacturer);
		//sprintf (cmd, "echo \"%s : %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__);
		//__system (cmd);
		setup_ini_set_str("INFO_MODEL_NAME2", model);
		//sprintf (cmd, "echo \"%s : %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__);
		//__system (cmd);
	}

	//sprintf (cmd, "echo \"%s : %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__);
	//__system (cmd);




	dhcp = (char*)json_obj_str(interface, "dhcp");
	ip = (char*)json_obj_str(interface, "address");
	mask = (char*)json_obj_str(interface, "subnet_mask");
	gateway = (char*)json_obj_str(interface, "gateway");
	// mac = (char*)json_obj_str(interface, "mac");
	//sprintf (cmd, "echo \"%s : %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__);
	//__system (cmd);
	if (!is_valid_ip(ip) && is_valid_ip(mask) && is_valid_ip(gateway))
	{
		goto Fail;
	}


	//sprintf (cmd, "echo \"%s : %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__);
	//__system (cmd);
	




	if (dhcp != NULL && ip != NULL && mask != NULL && gateway != NULL)
	{
		setup_ini_set_str("NET_DHCP", dhcp);
		setup_ini_set_str("NET_IPADDR", ip);
		setup_ini_set_str("NET_SUSBNET", mask);
		setup_ini_set_str("NET_GW", gateway);
	}
	//sprintf (cmd, "echo \"%s : %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__);
	//__system (cmd);
	// if (mac != NULL)
	// {
	// 	medel_spec_value_add("NET_MAC", mac);
	// 	// system("")
	// }



	http_port = json_obj_int(port, "web");
	rtsp_port = json_obj_int(port, "rtsp");

	//sprintf (cmd, "echo \"%s : %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__);
	//__system (cmd);

	if (http_port > 0 && rtsp_port > 0)
	{
		sprintf(buf, "%d", http_port);
		setup_ini_set_str("WEB_PORT", buf);
		sprintf(buf, "%d", rtsp_port);
		setup_ini_set_str("RTSP_PORT", buf);
	}
	//sprintf (cmd, "echo \"%s : %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__);
	//__system (cmd);

	update_setup(&setup, &setup_ext);
	notify_update_setup(UPDATE_SYSTEM_DEFAULT);




	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

static int get_system_factory(char *data, int id, char *attr, char *resp)
{
	json_t *root;
	json_error_t error;
	json_t *factory = json_object();
	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;
	// char buf[1024];

	//char cmd[1024];

	//sprintf (cmd, "echo \"!!!!!!!!!!!!!!!!!!!!!!!!!!!!		func:%s:line %d > load setup\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__);
	//system(cmd);
	load_setup(&setup, &setup_ext);

	char ip[64];
	char netmask[64];
	char gateway[64];
	
	// char mac[64];
	// unsigned char mac_addr[6];

	sprintf(ip, "%s", inet_ntoa(setup.net.ip));
	sprintf(netmask, "%s", inet_ntoa(setup.net.netmask));
	sprintf(gateway, "%s", inet_ntoa(setup.net.gateway));

	// __if_get_mac(ETHERNET_DEV, mac_addr);
	// sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
	// 	mac_addr [0], mac_addr [1], mac_addr [2], mac_addr [3], mac_addr [4], mac_addr [5]);

	//sprintf (cmd, "echo \"func:%s:line %d > ini load\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__);
	//system(cmd);
	// setup_ini_load();
	//sprintf (cmd, "echo \"func:%s:line %d \" | cat >> /mnt/ipm/log.txt", __func__, __LINE__);
	//system(cmd);


	factory = json_pack_ex(&error, 0, "{s:{s:{s:s, s:s}, s:s, s:s, s:s},s:{s:{s:s, s:s, s:s, s:s},s:{s:i, s:i}}}",	\
			"system", "user", "id", setup_ini_get_str2("cam_setup", "SYSTEM_USER_NAME"), "password", setup_ini_get_str2("cam_setup", "SYSTEM_USER_PASSWD"), "name", setup_ini_get_str2("cam_setup", "INFO_MODEL_NAME"),"manufacturer", setup_ini_get_str2("cam_setup", "SYSTEM_MANUFACTURER"),"model", setup_ini_get_str2("cam_setup", "INFO_MODEL_NAME2"),	\
			"network","interface","dhcp", (setup.net.type == NETTYPE_LAN)? "FALSE":"TRUE", "address", ip,"subnet_mask", netmask,"gateway", gateway,	\
			"port", "web", setup.net.port_web,"rtsp", setup_ext.rtsp_info.rtsp_port);



	root = json_pack_ex(&error, 0, "{s:o*}", "factory",
			factory);


		
	if(json_to_string(root, resp) < 0) {
		goto Fail;
	}



	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;

}

static int get_system_information(char *data, int id, char *attr, char *resp)
{
	json_t *root;
	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;

	char language[128];
	load_setup(&setup, &setup_ext);



	switch(setup.lang) {
		case 1://LANG_KOR:
			sprintf(language, "kor"); break;
		case 2://LANG_FRE:
			sprintf(language, "jpn"); break;
		case 0://LANG_ENG:
			sprintf(language, "eng"); break;
		default:
			sprintf(language, "eng"); break;
	}

	{
		root = json_pack("{s:{s:s}}",
				"system info",
				"lan", language);
	}
		
	if(json_to_string(root, resp) < 0) {
		goto Fail;
	}
		
	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

int api_system_information(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_system_information(data, id, attr, resp);
	}
	
	return 405;
}


int api_system_factory(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_system_factory(data, id, attr, resp);
	}
	
	if(strncasecmp(request_method, "PUT", 3) == 0) {
		return put_system_factory(data, id, attr, resp);
	}

	return 405;
}

int api_system_factory_mac(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_system_factory_mac(data, id, attr, resp);
	}
	
	if(strncasecmp(request_method, "PUT", 3) == 0) {
		return put_system_factory_mac(data, id, attr, resp);
	}

	return 405;
}

