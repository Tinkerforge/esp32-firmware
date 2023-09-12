import { CronTrigger } from "../cron/cron_defs";

export interface RtcCronTrigger {
    0: CronTrigger.Cron,
    1: {
        mday: number,
        wday: number,
        hour: number,
        minute: number
    }
}

export function init() {
    // FIXME
}
