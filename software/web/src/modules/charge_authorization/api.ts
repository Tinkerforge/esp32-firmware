import { CMAuthType } from "modules/cm_networking/generated/cm_auth_type.enum";

export /*no-api*/ type ChargeAuth = {
    seen_at: number,
    auth_info:   [CMAuthType.None, null]
               | [CMAuthType.Lost, null]
               | [CMAuthType.NFC, {tag_id: string, tag_type: number}]
               | [CMAuthType.InjectedNFC, {tag_id: string, tag_type: number}]
               | [CMAuthType.EV, {mac: string}]
               | [CMAuthType.InjectedEV, {mac: string}]
};

export type last_seen = ChargeAuth[];
