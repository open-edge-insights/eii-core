#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "eis/msgbus/msgbus.h"
#include "eis/utils/logger.h"
#include "eis/utils/json_config.h"
#include <eis/config_manager/env_config.h>
#include <atomic>

#ifndef _EIS_CPP_SERVER_H
#define _EIS_CPP_SERVER_H

class Server
{
    private:
        void* g_msgbus_ctx_server = NULL; 
        recv_ctx_t* g_service_ctx = NULL;
        env_config_t* g_env_config_client = NULL;
        config_mgr_t* g_config_mgr = NULL;
        msg_envelope_serialized_part_t* parts = NULL;
        int num_parts = 0;
        msg_envelope_t* msg = NULL;
        char* m_app_name = getenv("AppName");
        std::atomic<bool> *loop;
    
    public:
        Server(std::atomic<bool> *loop);
        ~Server();
        bool init(char *service_name);
        static void* start(void *arg);
        int server_service();
};

#endif //_EIS_CPP_SERVER_H

