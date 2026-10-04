#ifdef ARDUINO_ARCH_ESP32
#include "EspCan.hpp"
namespace canbridge {
Result EspCan::begin(const CommonConfig &c) {
    if(started_) return Result::AlreadyStarted;
    if(tx_<0 || rx_<0 || !c.bitrate) return Result::InvalidConfig;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,5,0)
    queue_=xQueueCreate(64,sizeof(Frame));
    if(!queue_) return Result::DriverError;
    twai_onchip_node_config_t cfg{};
    cfg.io_cfg.tx=static_cast<gpio_num_t>(tx_); cfg.io_cfg.rx=static_cast<gpio_num_t>(rx_);
    cfg.io_cfg.quanta_clk_out=GPIO_NUM_NC; cfg.io_cfg.bus_off_indicator=GPIO_NUM_NC;
    cfg.bit_timing.bitrate=c.bitrate; cfg.tx_queue_depth=1;
    cfg.flags.enable_listen_only=c.listenOnly;
    if(twai_new_node_onchip(&cfg,&node_)!=ESP_OK) { vQueueDelete(queue_);queue_=nullptr; return Result::DriverError; }
    loss_.store(false, std::memory_order_relaxed);
    twai_event_callbacks_t cb{}; cb.on_rx_done=onReceive;
    if(twai_node_register_event_callbacks(node_,&cb,this)!=ESP_OK || twai_node_enable(node_)!=ESP_OK) {
        twai_node_delete(node_);node_=nullptr;vQueueDelete(queue_);queue_=nullptr;return Result::DriverError;
    }
#else
    twai_timing_config_t timing{};
    switch(c.bitrate) {
    case 1000000: timing=TWAI_TIMING_CONFIG_1MBITS();break;
    case 500000: timing=TWAI_TIMING_CONFIG_500KBITS();break;
    case 250000: timing=TWAI_TIMING_CONFIG_250KBITS();break;
    case 125000: timing=TWAI_TIMING_CONFIG_125KBITS();break;
    default: return Result::Unsupported;
    }
    twai_general_config_t cfg=TWAI_GENERAL_CONFIG_DEFAULT(static_cast<gpio_num_t>(tx_),static_cast<gpio_num_t>(rx_),c.listenOnly ? TWAI_MODE_LISTEN_ONLY : TWAI_MODE_NORMAL);
    cfg.rx_queue_len=64; cfg.tx_queue_len=8;
    twai_filter_config_t filter=TWAI_FILTER_CONFIG_ACCEPT_ALL();
    if(twai_driver_install(&cfg,&timing,&filter)!=ESP_OK) return Result::DriverError;
    if(twai_start()!=ESP_OK) {twai_driver_uninstall();return Result::DriverError;}
#endif
    started_=true;return Result::Ok;
}
Result EspCan::end() {
    if(!started_) return Result::NotStarted;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,5,0)
    if(twai_node_disable(node_)!=ESP_OK) return Result::DriverError;
    if(twai_node_delete(node_)!=ESP_OK) return Result::DriverError;
    node_=nullptr;vQueueDelete(queue_);queue_=nullptr;
#else
    if(twai_stop()!=ESP_OK) return Result::DriverError;
    if(twai_driver_uninstall()!=ESP_OK) return Result::DriverError;
#endif
    started_=false;return Result::Ok;
}
Result EspCan::send(const Frame &f) {
    if(!started_) return Result::NotStarted;
    if(!valid(f)) return Result::InvalidFrame;
    esp_err_t err;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,5,0)
    // Keep frame and payload alive until the SDK has completed transmission.
    const esp_err_t pending = twai_node_transmit_wait_all_done(node_,0);
    if(pending!=ESP_OK) return pending==ESP_ERR_TIMEOUT ? Result::Busy : Result::DriverError;
    txStorage_=f;txFrame_={};
    txFrame_.header.id=f.id;txFrame_.header.ide=f.extended;txFrame_.header.rtr=f.remote;
    txFrame_.header.dlc=f.length;
    txFrame_.buffer=txStorage_.data.data();txFrame_.buffer_len=f.length;
    err=twai_node_transmit(node_,&txFrame_,0);
#else
    twai_message_t m{};m.identifier=f.id;m.extd=f.extended;m.rtr=f.remote;m.data_length_code=f.length;
    for(unsigned i=0;i<f.length;++i)m.data[i]=f.data[i];
    err=twai_transmit(&m,0);
#endif
    return err==ESP_OK ? Result::Ok : err==ESP_ERR_TIMEOUT ? Result::Busy : Result::DriverError;
}
Result EspCan::receive(Frame &f) {
    if(!started_) return Result::NotStarted;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,5,0)
    Frame out;
    if(xQueueReceive(queue_,&out,0)!=pdTRUE)return Result::Empty;
    f=out;return Result::Ok;
#else
    twai_message_t m{};auto err=twai_receive(&m,0);
    if(err==ESP_ERR_TIMEOUT)return Result::Empty;
    if(err!=ESP_OK || m.data_length_code>8)return Result::DriverError;
    Frame out;out.id=m.identifier;out.extended=m.extd;out.remote=m.rtr;out.length=m.data_length_code;
    for(unsigned i=0;i<out.length;++i)out.data[i]=m.data[i];
    f=out;return Result::Ok;
#endif
}
Result EspCan::pollHealth(Health &h) {
    if(!started_)return Result::NotStarted;
    h=Health{};
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,5,0)
    twai_node_status_t status{};twai_node_record_t record{};
    if(twai_node_get_info(node_,&status,&record)!=ESP_OK)return Result::DriverError;
    h.busOff=status.state==TWAI_ERROR_BUS_OFF;h.errorPassive=status.state==TWAI_ERROR_PASSIVE;
    h.receiveLoss=loss_.load(std::memory_order_relaxed);h.raw=record.bus_err_num;
#else
    twai_status_info_t s{};
    if(twai_get_status_info(&s)!=ESP_OK)return Result::DriverError;
    h.busOff=s.state==TWAI_STATE_BUS_OFF;
    h.errorPassive=s.tx_error_counter>=128 || s.rx_error_counter>=128;
    h.receiveLoss=s.rx_missed_count || s.rx_overrun_count;h.raw=s.bus_error_count;
#endif
    // Receive loss stays latched until end()/begin(); no automatic recovery.
    return Result::Ok;
}
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,5,0)
bool EspCan::onReceive(twai_node_handle_t node,const twai_rx_done_event_data_t *,void *ctx) {
    auto &self=*static_cast<EspCan *>(ctx);
    std::uint8_t bytes[64]{};twai_frame_t raw{};raw.buffer=bytes;raw.buffer_len=sizeof(bytes);
    if(twai_node_receive_from_isr(node,&raw)!=ESP_OK){self.loss_.store(true, std::memory_order_relaxed);return false;}
#if SOC_TWAI_SUPPORT_FD
    if(raw.header.fdf){self.loss_.store(true, std::memory_order_relaxed);return false;}
#endif
    if(raw.header.dlc>8){self.loss_.store(true, std::memory_order_relaxed);return false;}
    Frame out;out.id=raw.header.id;out.extended=raw.header.ide;out.remote=raw.header.rtr;out.length=raw.header.dlc;
    for(unsigned i=0;i<out.length;++i)out.data[i]=bytes[i];
    BaseType_t woken=pdFALSE;
    if(xQueueSendFromISR(self.queue_,&out,&woken)!=pdTRUE)self.loss_.store(true, std::memory_order_relaxed);
    return woken==pdTRUE;
}
#endif
}
#endif
