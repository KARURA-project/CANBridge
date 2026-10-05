#pragma once
#include "../../internal/Controller.hpp"
#include <Arduino.h>
#include <atomic>
#include <esp_idf_version.h>
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,5,0)
#include <esp_twai.h>
#include <esp_twai_onchip.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#else
#include <driver/twai.h>
#endif
namespace canbridge {
class EspCan final : public Controller {
    int tx_,rx_;
    bool started_=false;
    bool installed_=false;
    bool enabled_=false;
    bool selfTest_=false;
    esp_err_t lastError_=ESP_OK;
    bool accessPassed_=false;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,5,0)
    twai_node_handle_t node_=nullptr;
    QueueHandle_t queue_=nullptr;
    Frame txStorage_;
    twai_frame_t txFrame_{};
    std::atomic<bool> loss_{false};
    static bool onReceive(twai_node_handle_t,const twai_rx_done_event_data_t *,void *);
#endif
public:
    EspCan(int txPin,int rxPin):tx_(txPin),rx_(rxPin) {}
    Result begin(const CommonConfig &) override;
    Result beginDiagnostic(const CommonConfig &, DiagnosticReport &) override;
    Result end() override;
    Result send(const Frame &) override;
    Result receive(Frame &) override;
    Result pollHealth(Health &) override;
};
}
