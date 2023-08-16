export interface MqttCronAction {
    0: 2,
    1: {
        topic: string,
        payload: string,
        retain: boolean
    }
}