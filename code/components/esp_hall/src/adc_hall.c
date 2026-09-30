#include "adc_hall.h"
#include "esp_log.h"

const static char *TAG = "ADC_HALL";

static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

static void adc_calibration_deinit(adc_cali_handle_t handle)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
#endif
}

adc_cali_handle_t adc_calibration(adc_atten_t atten, adc_channel_t channel){
    adc_cali_handle_t adc_cali_handle = NULL;

    adc_calibration_init(ADC_UNIT_1, channel, atten, &adc_cali_handle);

    return adc_cali_handle;
}

adc_oneshot_unit_handle_t adc_init(void){
    adc_oneshot_unit_handle_t adc_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    return adc_handle;
}

void adc_add(adc_oneshot_unit_handle_t adc_handle, adc_atten_t atten, adc_channel_t channel){
    adc_oneshot_chan_cfg_t config = {
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, channel, &config));
    
}

esp_err_t adc_read(adc_oneshot_unit_handle_t adc_handle, adc_channel_t channel, int *raw){
    if(!raw)
        return ESP_ERR_INVALID_ARG;
    
    esp_err_t adc_result = adc_oneshot_read(adc_handle, channel, raw);
    if(adc_result != ESP_OK)
        return adc_result;

    return ESP_OK;
}

esp_err_t adc_get_voltage(adc_cali_handle_t adc_calibration_handle, int raw, int *voltage){
    if(!voltage){
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t adc_voltage_result = adc_cali_raw_to_voltage(adc_calibration_handle, raw, voltage);

    return adc_voltage_result;
}

esp_err_t adc_remove(adc_oneshot_unit_handle_t adc_handle, adc_cali_handle_t *adc_calibration_handle){
    esp_err_t result = adc_oneshot_del_unit(adc_handle);
    
    if(adc_calibration_handle){
        adc_calibration_deinit(*adc_calibration_handle);
    }

    return result;
}