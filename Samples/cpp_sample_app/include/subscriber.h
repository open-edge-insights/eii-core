#include <signal.h>
#include <unistd.h>
#include <eis/config_manager/env_config.h>
#include "eis/msgbus/msgbus.h"
#include <atomic>

#ifndef _EIS_CPP_SUBSCRIBER_H
#define _EIS_CPP_SUBSCRIBER_H

class Subscriber
{
    private:
        recv_ctx_t* g_sub_ctx = NULL;
        void* g_msgbus_ctx = NULL;
        env_config_t* g_env_config_client = NULL;
        config_mgr_t* g_config_mgr = NULL;
        msg_envelope_t* msg = NULL;
        msg_envelope_serialized_part_t* parts = NULL;
        int num_parts = 0;
        std::atomic<bool> *loop;
    
    public:
        Subscriber(std::atomic<bool> *loop);
        ~Subscriber();
        bool init(char *topic_name);
        static void* start(void *arg);
        int subscribe();
};

#endif //_EIS_CPP_SUBSCRIBER_H
