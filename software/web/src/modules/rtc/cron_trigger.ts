import { CronTriggerID } from "../cron/cron_defs";

export type RtcCronTrigger = [
    CronTriggerID.Cron,
    {
        mday: number,
        wday: number,
        hour: number,
        minute: number
    }
];

export function init() {
    // FIXME
}
