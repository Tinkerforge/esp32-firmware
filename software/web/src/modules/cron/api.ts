import {RtcCronTrigger} from '/home/freddy/tf/esp32-firmware/software/web/src/modules/rtc/cron_trigger'
import {ChargeLimitsCronAction} from '/home/freddy/tf/esp32-firmware/software/web/src/modules/charge_limits/cron_action'
import {ChargeLimitsCronTrigger} from '/home/freddy/tf/esp32-firmware/software/web/src/modules/charge_limits/cron_trigger'
import {MqttCronAction} from '/home/freddy/tf/esp32-firmware/software/web/src/modules/mqtt/cron_action'
import {MqttCronTrigger} from '/home/freddy/tf/esp32-firmware/software/web/src/modules/mqtt/cron_trigger'
import {EvseGpOutputCronAction} from '/home/freddy/tf/esp32-firmware/software/web/src/modules/evse_v2/cron_action'
import {EvseSdCronTrigger} from '/home/freddy/tf/esp32-firmware/software/web/src/modules/evse_v2/cron_trigger'
import {EvseGpioCronTrigger} from '/home/freddy/tf/esp32-firmware/software/web/src/modules/evse_v2/cron_trigger'
import {EvseButtonCronTrigger} from '/home/freddy/tf/esp32-firmware/software/web/src/modules/evse_v2/cron_trigger'
import {MeterCronAction} from '/home/freddy/tf/esp32-firmware/software/web/src/modules/meter/cron_action'
import {NfcCronAction} from '/home/freddy/tf/esp32-firmware/software/web/src/modules/nfc/cron_action'
import {NfcCronTrigger} from '/home/freddy/tf/esp32-firmware/software/web/src/modules/nfc/cron_trigger'
import {ChargeManagerCronAction} from '/home/freddy/tf/esp32-firmware/software/web/src/modules/charge_manager/cron_action'
import {EvseCronAction} from '/home/freddy/tf/esp32-firmware/software/web/src/modules/evse_common/cron_action'
import {EvseLedCronAction} from '/home/freddy/tf/esp32-firmware/software/web/src/modules/evse_common/cron_action'
import {EvseStateCronTrigger} from '/home/freddy/tf/esp32-firmware/software/web/src/modules/evse_common/cron_trigger'


export interface config {
    tasks: task[]
}

interface task {
    trigger: cron_trigger,
    action: cron_action
}

export interface cron_trigger {
    0: [0, {}] | RtcCronTrigger |
             ChargeLimitsCronTrigger |
             MqttCronTrigger |
             EvseSdCronTrigger |
             EvseGpioCronTrigger |
             EvseButtonCronTrigger |
             NfcCronTrigger |
             EvseStateCronTrigger
}

export interface cron_action {
    0: ChargeLimitsCronAction |
            MqttCronAction |
            EvseGpOutputCronAction |
            MeterCronAction |
            NfcCronAction |
            ChargeManagerCronAction |
            EvseCronAction |
            EvseLedCronAction
}