#include "sai.h"
#include "stub_sai.h"
#include "assert.h"

#define MAX_NUMBER_OF_LAG_MEMBERS 16
#define MAX_NUMBER_OF_LAGS 5

typedef struct _lag_member_db_entry_t {
    bool            is_used;
    sai_object_id_t port_oid;
    sai_object_id_t lag_oid;
} lag_member_db_entry_t;

typedef struct _lag_db_entry_t {
    bool            is_used;
    sai_object_id_t oid;
    sai_object_id_t members_ids[MAX_NUMBER_OF_LAG_MEMBERS];
} lag_db_entry_t;

struct lag_db_t {
    lag_db_entry_t        lags[MAX_NUMBER_OF_LAGS];
    lag_member_db_entry_t members[MAX_NUMBER_OF_LAG_MEMBERS];
} lag_db;

static const sai_attribute_entry_t lag_attribs[] = {
    { SAI_LAG_ATTR_PORT_LIST, false, false, false, true,
      "List of ports in LAG", SAI_ATTR_VAL_TYPE_OBJLIST },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_attribute_entry_t lag_member_attribs[] = {
    { SAI_LAG_MEMBER_ATTR_LAG_ID, true, true, false, true,
      "LAG ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_LAG_MEMBER_ATTR_PORT_ID, true, true, false, true,
      "PORT ID", SAI_ATTR_VAL_TYPE_OID },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t lag_vendor_attribs[] = {
    { SAI_LAG_ATTR_PORT_LIST,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};

static const sai_vendor_attribute_entry_t lag_member_vendor_attribs[] = {
    { SAI_LAG_MEMBER_ATTR_LAG_ID,
      { true, false, false, false },
      { true, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_LAG_MEMBER_ATTR_PORT_ID,
      { true, false, false, false },
      { true, false, false, false },
      NULL, NULL,
      NULL, NULL }
};

sai_status_t stub_create_lag(
    _Out_ sai_object_id_t* lag_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    sai_status_t status;

    status = check_attribs_metadata(attr_count, attr_list, lag_attribs, lag_vendor_attribs, SAI_OPERATION_CREATE);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed attributes check\n");
        return status;
    }

    // verify that this LAG is not used
    uint32_t ii = 0;
    for (; ii < MAX_NUMBER_OF_LAGS; ii++) {
        if (!lag_db.lags[ii].is_used) {
            break;
        }
    }

    if (ii == MAX_NUMBER_OF_LAGS) {
        printf("Cannot create LAG: limit is reached\n");
        return SAI_STATUS_FAILURE;
    }

    uint32_t lag_db_id = ii;

    char list_str[MAX_LIST_VALUE_STR_LEN];  
    sai_attr_list_to_str(attr_count, attr_list, lag_attribs, MAX_LIST_VALUE_STR_LEN, list_str);

    status = stub_create_object(SAI_OBJECT_TYPE_LAG, lag_db_id, lag_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot create LAG OID\n");
        return status;
    }

    lag_db.lags[lag_db_id].is_used = true;
    lag_db.lags[lag_db_id].oid = *lag_id;

    printf("Created LAG 0x%lX (%s) at lag_db_id(%u)\n", *lag_id, list_str, lag_db_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_remove_lag(
    _In_ sai_object_id_t  lag_id)
{
    sai_status_t status;
    uint32_t     lag_db_id;
    status = stub_object_to_type(lag_id, SAI_OBJECT_TYPE_LAG, &lag_db_id);
    
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot get LAG DB ID.\n");
        return status;
    }

    // Check if LAG exists
    if (lag_db_id >= MAX_NUMBER_OF_LAGS ||
        !lag_db.lags[lag_db_id].is_used) {
        printf("Invalid LAG OID\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    // Check if any members are still attached
    for (uint32_t i = 0; i < MAX_NUMBER_OF_LAG_MEMBERS; i++) {
        if (lag_db.lags[lag_db_id].members_ids[i] != SAI_NULL_OBJECT_ID) {
            printf("Cannot remove LAG 0x%lX: members still attached\n", lag_id);
            return SAI_STATUS_OBJECT_IN_USE;
        }
    }

    lag_db.lags[lag_db_id].is_used = false;
    lag_db.lags[lag_db_id].oid = SAI_NULL_OBJECT_ID;
    memset(lag_db.lags[lag_db_id].members_ids, 0, sizeof(lag_db.lags[lag_db_id].members_ids));
    
    printf("Removed LAG 0x%lX\n", lag_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_set_lag_attribute(
    _In_ sai_object_id_t  lag_id,
    _In_ const sai_attribute_t *attr)
{
    printf("Called stub_set_lag_attribute\n");
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_get_lag_attribute(
    _In_ sai_object_id_t lag_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    printf("Called stub_get_lag_attribute\n");
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_create_lag_member(
    _Out_ sai_object_id_t* lag_member_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    sai_status_t status;

    status = check_attribs_metadata(attr_count, attr_list, lag_member_attribs, lag_member_vendor_attribs, SAI_OPERATION_CREATE);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed attributes check\n");
        return status;
    }

    uint32_t ii = 0;
    for (; ii < MAX_NUMBER_OF_LAG_MEMBERS; ii++) {
        if (!lag_db.members[ii].is_used) {
            break;
        }
    }

    if (ii == MAX_NUMBER_OF_LAG_MEMBERS) {
        printf("Cannot create LAG member: limit is reached\n");
        return SAI_STATUS_FAILURE;
    }

    uint32_t lag_member_db_id = ii;

    char list_str[MAX_LIST_VALUE_STR_LEN];
    sai_attr_list_to_str(attr_count, attr_list, lag_member_attribs, MAX_LIST_VALUE_STR_LEN, list_str);

    const sai_attribute_value_t *lag_id;
    uint32_t lag_id_idx;
    status = find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_LAG_ID, &lag_id, &lag_id_idx);
    if (status != SAI_STATUS_SUCCESS) {
        printf("LAG_ID attribute not found.\n");
        return status;
    }

    const sai_attribute_value_t *port_id;
    uint32_t port_id_idx;
    status = find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_PORT_ID, &port_id, &port_id_idx);
    if (status != SAI_STATUS_SUCCESS) {
        printf("PORT_ID attribute not found.\n");
        return status;
    }

    uint32_t lag_db_id = MAX_NUMBER_OF_LAGS;
    for (uint32_t i = 0; i < MAX_NUMBER_OF_LAGS; i++) {
        if (lag_db.lags[i].is_used &&
            lag_db.lags[i].oid == lag_id->oid) {
            lag_db_id = i;
            // printf("found lag_db_id: %u\n", lag_db_id);
            break;
        }
    }

    if (lag_db_id == MAX_NUMBER_OF_LAGS) {
        printf("Invalid LAG OID\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    status = stub_create_object(SAI_OBJECT_TYPE_LAG_MEMBER, lag_member_db_id, lag_member_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot create LAG member OID\n");
        return status;
    }

    lag_db.members[lag_member_db_id].is_used = true;
    lag_db.members[lag_member_db_id].lag_oid = lag_id->oid;
    lag_db.members[lag_member_db_id].port_oid = port_id->oid;

    lag_db.lags[lag_db_id].members_ids[lag_member_db_id] = *lag_member_id;

    printf("Created LAG member 0x%lX (%s)\n", *lag_member_id, list_str);
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_remove_lag_member(
    _In_ sai_object_id_t  lag_member_id)
{
    sai_status_t status;
    uint32_t     lag_member_db_id;
    status = stub_object_to_type(lag_member_id, SAI_OBJECT_TYPE_LAG_MEMBER, &lag_member_db_id);
    
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot get LAG member DB ID.\n");
        return status;
    }

    uint32_t lag_db_id = MAX_NUMBER_OF_LAGS;
    for (uint32_t i = 0; i < MAX_NUMBER_OF_LAGS; i++) {
        if (lag_db.lags[i].is_used &&
            lag_db.lags[i].oid == lag_db.members[lag_member_db_id].lag_oid) {
            lag_db_id = i;
            // printf("found lag_db_id: %u\n", lag_db_id);
            break;
        }
    }

    if (lag_db_id == MAX_NUMBER_OF_LAGS) {
        printf("Invalid LAG OID\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    lag_db.members[lag_member_db_id].is_used = false;
    lag_db.members[lag_member_db_id].lag_oid = SAI_NULL_OBJECT_ID;
    lag_db.members[lag_member_db_id].port_oid = SAI_NULL_OBJECT_ID;
    
    lag_db.lags[lag_db_id].members_ids[lag_member_db_id] = SAI_NULL_OBJECT_ID;

    printf("Removed LAG member 0x%lX\n", lag_member_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_set_lag_member_attribute(
    _In_ sai_object_id_t  lag_member_id,
    _In_ const sai_attribute_t *attr)
{
    printf("Called stub_set_lag_member_attribute\n");
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_get_lag_member_attribute(
    _In_ sai_object_id_t lag_member_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    printf("Called stub_get_lag_member_attribute\n");
    return SAI_STATUS_SUCCESS;
}

const sai_lag_api_t lag_api = {
    stub_create_lag,
    stub_remove_lag,
    stub_set_lag_attribute,
    stub_get_lag_attribute,
    stub_create_lag_member,
    stub_remove_lag_member,
    stub_set_lag_member_attribute,
    stub_get_lag_member_attribute
};
