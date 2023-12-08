export interface config {
    enable: boolean;
    url: string;
    identity: string;
    enable_auth: boolean;
    pass: string;
    cert_id: number;
}

export interface reset {}

export interface state {
    charge_point_state: number;
    charge_point_status: number;
    next_profile_eval: number;
    connector_state: number;
    connector_status: number;
    tag_id: string;
    parent_tag_id: string;
    tag_expiry_date: number;
    tag_timeout: number;
    cable_timeout: number;
    txn_id: number;
    txn_confirmed_time: number;
    txn_start_time: number;
    current: number;
    txn_with_invalid_id: boolean;
    unavailable_requested: boolean;
    message_in_flight_type: number;
    message_in_flight_id_high: number;
    message_in_flight_id_low: number;
    message_in_flight_len: number;
    message_timeout: number;
    txn_msg_retry_timeout: number;
    message_queue_depth: number;
    status_queue_depth: number;
    txn_msg_queue_depth: number;
    connected: boolean;
    connected_change_time: number;
    last_ping_sent: number;
    pong_timeout: number;
}

export interface configuration {
    AuthorizeRemoteTxRequests: string;
    ClockAlignedDataInterval: string;
    ConnectionTimeOut: string;
    ConnectorPhaseRotation: string;
    ConnectorPhaseRotationMaxLength: string;
    GetConfigurationMaxKeys: string;
    HeartbeatInterval: string;
    LocalAuthorizeOffline: string;
    LocalPreAuthorize: string;
    MessageTimeout: string;
    MeterValuesAlignedData: string;
    MeterValuesAlignedDataMaxLength: string;
    MeterValuesSampledData: string;
    MeterValuesSampledDataMaxLength: string;
    MeterValueSampleInterval: string;
    NumberOfConnectors: string;
    ResetRetries: string;
    StopTransactionOnEVSideDisconnect: string;
    StopTransactionOnInvalidId: string;
    StopTransactionMaxMeterValues: string;
    StopTxnAlignedData: string;
    StopTxnAlignedDataMaxLength: string;
    StopTxnSampledData: string;
    StopTxnSampledDataMaxLength: string;
    SupportedFeatureProfiles: string;
    TransactionMessageAttempts: string;
    TransactionMessageRetryInterval: string;
    UnlockConnectorOnEVSideDisconnect: string;
    WebSocketPingInterval: string;
    ChargeProfileMaxStackLevel: string;
    ChargingScheduleAllowedChargingRateUnit: string;
    ChargingScheduleMaxPeriods: string;
    ConnectorSwitch3to1PhaseSupported: string;
    MaxChargingProfilesInstalled: string;
}

export interface change_configuration {
    key: string;
    value: string;
}
