import { TileType } from "./tile_type.enum";

type TileTypeEmpty = [TileType.Empty, null];
type TileTypeCharger = [TileType.Charger, number];
type TileTypeChargeManagement = [TileType.ChargeManagement, null];
type TileTypeMeter = [TileType.Meter, number];
type TileTypeDayAheadPrices = [TileType.DayAheadPrices, number];
type TileTypeSolarForecast = [TileType.SolarForecast, number];
type TileTypeEnergyManagerStatus = [TileType.EnergyManagerStatus, null];
type TileTypeHeatingStatus = [TileType.HeatingStatus, null];

type TileConfig =
    TileTypeEmpty |
    TileTypeCharger |
    TileTypeChargeManagement |
    TileTypeMeter |
    TileTypeDayAheadPrices |
    TileTypeSolarForecast |
    TileTypeEnergyManagerStatus |
    TileTypeHeatingStatus;

export interface config {
    enable: boolean;
    tiles: TileConfig[]
}
