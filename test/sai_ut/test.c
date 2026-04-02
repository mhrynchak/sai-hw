// gcc -o unit_test -I /usr/include/sai -L /usr/lib test.c -lsai

#include <stdio.h>
#include <stdlib.h>
#include "sai.h"

#define MAX_PORTS 64

sai_object_id_t g_port_list[MAX_PORTS];
uint32_t g_port_count = 0;

const char* test_profile_get_value(
    _In_ sai_switch_profile_id_t profile_id,
    _In_ const char* variable)
{
    return 0;
}

int test_profile_get_next_value(
    _In_ sai_switch_profile_id_t profile_id,
    _Out_ const char** variable,
    _Out_ const char** value)
{
    return -1;
}

const service_method_table_t test_services = {
    test_profile_get_value,
    test_profile_get_next_value
};

char * concat_str_oid_msg(char *msg, sai_object_id_t lag_oid) {
    char *concat_msg = NULL;
    asprintf(&concat_msg, "%s 0x%lX", msg, lag_oid);
    return concat_msg;
}

void log_message(char *msg, char* err, sai_status_t status) {
    if (status == SAI_STATUS_SUCCESS) {
        if (msg) {
            printf("%s, status: %d\n", msg, status);
        }
    } else {
        if (err) {
            printf("%s, status: %d\n", err, status);
        }
        exit(EXIT_FAILURE);
    }
}

sai_switch_api_t * test_sai_api_switch() {
    sai_status_t              status;
    sai_switch_api_t         *switch_api;
    sai_object_id_t           vr_oid;
    sai_attribute_t           attrs[2];
    sai_switch_notification_t notifications;

    status = sai_api_query(SAI_API_SWITCH, (void**)&switch_api);
    log_message("Retrieved SAI_API_SWITCH",
                "Failed to retrieve SAI_API_SWITCH",
                status);

    status = switch_api->initialize_switch(0, "HW_ID", 0, &notifications);
    log_message("Initialized switch",
                "Failed to initialize switch",
                status);

    attrs[0].id = SAI_SWITCH_ATTR_PORT_LIST;
    attrs[0].value.objlist.list = g_port_list;
    attrs[0].value.objlist.count = MAX_PORTS;
    
    status = switch_api->get_switch_attribute(1, attrs);
    log_message("Retrieved switch attributes",
                "Failed to retrieve switch attributes",
                status);
    
    g_port_count = attrs[0].value.objlist.count;

    // for (int32_t ii = 0; ii < attrs[0].value.objlist.count; ii++) {
    //     printf("Port #%d OID: 0x%lX\n", ii, attrs[0].value.objlist.list[ii]);
    // }

    return switch_api;
}

void test_sai_api_lag() {
    sai_status_t              status;
    sai_lag_api_t             *lag_api;

    status = sai_api_query(SAI_API_LAG, (void**)&lag_api);
    log_message("Retrieved SAI_API_LAG",
                "Failed to retrieve SAI_API_LAG",
                status);

    sai_object_id_t lag_oid_1;
    status = lag_api->create_lag(&lag_oid_1, 0, NULL);

    sai_attribute_t attrs_1[2];
    attrs_1[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs_1[0].value.oid = lag_oid_1;
    attrs_1[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attrs_1[1].value.oid = g_port_list[0];
    
    sai_object_id_t lag_member_oid_1;
    status = lag_api->create_lag_member(&lag_member_oid_1, 2, attrs_1);

    sai_attribute_t attrs_2[2];
    attrs_2[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs_2[0].value.oid = lag_oid_1;
    attrs_2[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attrs_2[1].value.oid = g_port_list[1];
    
    sai_object_id_t lag_member_oid_2;
    status = lag_api->create_lag_member(&lag_member_oid_2, 2, attrs_2);

    sai_object_id_t lag_oid_2;
    status = lag_api->create_lag(&lag_oid_2, 0, NULL);

    sai_attribute_t attrs_3[2];
    attrs_3[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs_3[0].value.oid = lag_oid_2;
    attrs_3[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attrs_3[1].value.oid = g_port_list[2];

    sai_object_id_t lag_member_oid_3;
    status = lag_api->create_lag_member(&lag_member_oid_3, 2, attrs_3);

    sai_attribute_t attrs_4[2];
    attrs_4[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs_4[0].value.oid = lag_oid_2;
    attrs_4[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attrs_4[1].value.oid = g_port_list[3];

    sai_object_id_t lag_member_oid_4;
    status = lag_api->create_lag_member(&lag_member_oid_4, 2, attrs_4);
    
    status = lag_api->get_lag_attribute(lag_oid_1, 0, NULL);
    status = lag_api->get_lag_attribute(lag_oid_2, 0, NULL);

    status = lag_api->get_lag_member_attribute(lag_member_oid_1, 0, NULL);
    status = lag_api->get_lag_member_attribute(lag_member_oid_3, 0, NULL);
    
    status = lag_api->remove_lag_member(lag_member_oid_2);
    status = lag_api->get_lag_attribute(lag_oid_1, 0, NULL);

    status = lag_api->remove_lag_member(lag_member_oid_3);
    status = lag_api->get_lag_attribute(lag_oid_2, 0, NULL);

    status = lag_api->remove_lag_member(lag_member_oid_1);
    status = lag_api->remove_lag_member(lag_member_oid_4);

    status = lag_api->remove_lag(lag_oid_2);
    status = lag_api->remove_lag(lag_oid_1);
}

int main() {
    sai_status_t              status;

    status = sai_api_initialize(0, &test_services);
    log_message("Initialized sai_api",
                "Failed to initialize sai_api",
                status);

    sai_switch_api_t * switch_api = NULL; 
    switch_api = test_sai_api_switch();
    
    test_sai_api_lag();

    switch_api->shutdown_switch(0);

    status = sai_api_uninitialize();

    return 0;
}
