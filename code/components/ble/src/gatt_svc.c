#include "gatt_svc.h"
#include "common.h"
#include "pid.h"
#include "oled.h"

/* Automation IO service */
static const ble_uuid16_t auto_io_svc_uuid = BLE_UUID16_INIT(0x1815);

static uint16_t led_chr_val_handle;

static const ble_uuid128_t led_chr_uuid =
    BLE_UUID128_INIT(
        0x23, 0xd1, 0xbc, 0xea,
        0x5f, 0x78, 0x23, 0x15,
        0xde, 0xef, 0x12, 0x12,
        0x25, 0x15, 0x00, 0x00
    );

/* Kp characteristic */
static int led_chr_access(uint16_t conn_handle,
                          uint16_t attr_handle,
                          struct ble_gatt_access_ctxt *ctxt,
                          void *arg)
{
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {

        char value[10];

        if (ctxt->om->om_len >= sizeof(value)) {
            return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }

        memcpy(value, ctxt->om->om_data, ctxt->om->om_len);
        value[ctxt->om->om_len] = '\0';

        char identifier = value[0];

        float val = strtof(&value[1], NULL);

        switch(identifier){
            case 'p':
                set_kp(val);
                printf("Kp: %.2f\n\n", get_kp());
                break;
            case 'i':
                set_ki(val);
                printf("Ki: %.2f\n\n", get_ki());
                break;
            case 'd':
                set_kd(val);
                printf("Kd: %.2f\n\n", get_kd());
                break;
            default:
                printf("Command not found.");
        }

        oled_print(value, 0);

        return 0;
    }

    return BLE_ATT_ERR_UNLIKELY;
}

/* GATT services table */
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {

    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &auto_io_svc_uuid.u,
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {
                    .uuid = &led_chr_uuid.u,
                    .access_cb = led_chr_access,
                    .flags = BLE_GATT_CHR_F_WRITE,
                    .val_handle = &led_chr_val_handle,
                },
                {0},
            },
    },

    {0},
};

/*
 * Handle GATT attribute register events
 */
void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op) {

    case BLE_GATT_REGISTER_OP_SVC:
        ESP_LOGD(TAG,
                 "registered service %s with handle=%d",
                 ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                 ctxt->svc.handle);
        break;

    case BLE_GATT_REGISTER_OP_CHR:
        ESP_LOGD(TAG,
                 "registered characteristic %s "
                 "def_handle=%d val_handle=%d",
                 ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                 ctxt->chr.def_handle,
                 ctxt->chr.val_handle);
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        ESP_LOGD(TAG,
                 "registered descriptor %s with handle=%d",
                 ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                 ctxt->dsc.handle);
        break;

    default:
        assert(0);
        break;
    }
}

/*
 * GATT server initialization
 */
int gatt_svc_init(void)
{
    int rc;

    ble_svc_gatt_init();

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}