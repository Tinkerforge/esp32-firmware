/* esp32-firmware
 * Copyright (C) 2025 Julius Dill <julius@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "entity_data.h"
#include "../eebus.h"
#include "../eebus_usecases.h"

extern EEBus eebus;

// ============================================================================
// EVSEEntity Implementation
// ============================================================================

LoadControlLimitDescriptionListDataType EVSEEntity::get_load_control_limit_description_list_data()
{
    LoadControlLimitDescriptionListDataType load_control_limit_description_list_data;
#ifdef EEBUS_ENABLE_LPC_USECASE
    eebus.usecases->limitation_of_power_consumption.get_loadcontrol_limit_description(&load_control_limit_description_list_data);
#endif
#ifdef EEBUS_ENABLE_LPP_USECASE
    eebus.usecases->limitation_of_power_production.get_loadcontrol_limit_description(&load_control_limit_description_list_data);
#endif
    return load_control_limit_description_list_data;
}

LoadControlLimitListDataType EVSEEntity::get_load_control_limit_list_data()
{
    LoadControlLimitListDataType load_control_limit_list_data;
#ifdef EEBUS_ENABLE_LPC_USECASE
    eebus.usecases->limitation_of_power_consumption.get_loadcontrol_limit_list(&load_control_limit_list_data);
#endif
#ifdef EEBUS_ENABLE_LPP_USECASE
    eebus.usecases->limitation_of_power_production.get_loadcontrol_limit_list(&load_control_limit_list_data);
#endif
    return load_control_limit_list_data;
}

DeviceConfigurationKeyValueDescriptionListDataType EVSEEntity::get_device_configuration_list_data()
{
    DeviceConfigurationKeyValueDescriptionListDataType device_configuration_description_list_data;
#ifdef EEBUS_ENABLE_LPC_USECASE
    eebus.usecases->limitation_of_power_consumption.get_device_configuration_description(&device_configuration_description_list_data);
#endif
#ifdef EEBUS_ENABLE_LPP_USECASE
    eebus.usecases->limitation_of_power_production.get_device_configuration_description(&device_configuration_description_list_data);
#endif
#ifdef EEBUS_ENABLE_MGCP_USECASE
    eebus.usecases->monitoring_of_grid_connection_point.get_device_configuration_description_list_data(&device_configuration_description_list_data);
#endif
    return device_configuration_description_list_data;
}

DeviceConfigurationKeyValueListDataType EVSEEntity::get_device_configuration_value_list_data()
{
    DeviceConfigurationKeyValueListDataType device_configuration_value_list_data;
#ifdef EEBUS_ENABLE_LPC_USECASE
    eebus.usecases->limitation_of_power_consumption.get_device_configuration_value(&device_configuration_value_list_data);
#endif
#ifdef EEBUS_ENABLE_LPP_USECASE
    eebus.usecases->limitation_of_power_production.get_device_configuration_value(&device_configuration_value_list_data);
#endif
#ifdef EEBUS_ENABLE_MGCP_USECASE
    eebus.usecases->monitoring_of_grid_connection_point.get_device_configuration_value_list_data(&device_configuration_value_list_data);
#endif
    return device_configuration_value_list_data;
}

DeviceDiagnosisStateDataType EVSEEntity::get_state_data()
{
    DeviceDiagnosisStateDataType state_data;
#ifdef EEBUS_ENABLE_EVSECC_USECASE
    eebus.usecases->evse_commissioning_and_configuration.get_device_diagnosis_state(&state_data);
#endif
    return state_data;
}

ElectricalConnectionCharacteristicListDataType EVSEEntity::get_electrical_connection_characteristic_list_data()
{
    ElectricalConnectionCharacteristicListDataType electrical_connection_characteristic_list_data;
#ifdef EEBUS_ENABLE_LPC_USECASE
    eebus.usecases->limitation_of_power_consumption.get_electrical_connection_characteristic(&electrical_connection_characteristic_list_data);
#endif
#ifdef EEBUS_ENABLE_LPP_USECASE
    eebus.usecases->limitation_of_power_production.get_electrical_connection_characteristic(&electrical_connection_characteristic_list_data);
#endif
    return electrical_connection_characteristic_list_data;
}

ElectricalConnectionDescriptionListDataType EVSEEntity::get_electrical_connection_description_list_data()
{
    ElectricalConnectionDescriptionListDataType electrical_connection_description_list_data;
#ifdef EEBUS_ENABLE_MPC_USECASE
    eebus.usecases->monitoring_of_power_consumption.get_electricalConnection_description_list_data(&electrical_connection_description_list_data);
#endif
#ifdef EEBUS_ENABLE_MGCP_USECASE
    eebus.usecases->monitoring_of_grid_connection_point.get_electrical_connection_description_list_data(&electrical_connection_description_list_data);
#endif
    return electrical_connection_description_list_data;
}

ElectricalConnectionParameterDescriptionListDataType EVSEEntity::get_electrical_connection_parameter_description_list_data()
{
    ElectricalConnectionParameterDescriptionListDataType electrical_connection_parameter_description_list_data;
#ifdef EEBUS_ENABLE_MPC_USECASE
    eebus.usecases->monitoring_of_power_consumption.get_electricalConnection_parameter_description_list_data(&electrical_connection_parameter_description_list_data);
#endif
#ifdef EEBUS_ENABLE_MGCP_USECASE
    eebus.usecases->monitoring_of_grid_connection_point.get_electrical_connection_parameter_description_list_data(&electrical_connection_parameter_description_list_data);
#endif
    return electrical_connection_parameter_description_list_data;
}

DeviceClassificationManufacturerDataType EVSEEntity::get_device_classification_manufacturer_data()
{
    DeviceClassificationManufacturerDataType manufacturer_data;
#ifdef EEBUS_ENABLE_EVSECC_USECASE
    EvseccUsecase::get_device_classification_manufacturer(&manufacturer_data);
#endif
    return manufacturer_data;
}

MeasurementDescriptionListDataType EVSEEntity::get_measurement_description_list_data()
{
    MeasurementDescriptionListDataType measurement_description_list_data;
#ifdef EEBUS_ENABLE_MPC_USECASE
    eebus.usecases->monitoring_of_power_consumption.get_measurement_description_list_data(&measurement_description_list_data);
#endif
#ifdef EEBUS_ENABLE_MGCP_USECASE
    eebus.usecases->monitoring_of_grid_connection_point.get_measurement_description_list_data(&measurement_description_list_data);
#endif
    return measurement_description_list_data;
}

MeasurementConstraintsListDataType EVSEEntity::get_measurement_constraints_list_data()
{
    MeasurementConstraintsListDataType measurement_constraints_list_data;
#ifdef EEBUS_ENABLE_MPC_USECASE
    eebus.usecases->monitoring_of_power_consumption.get_measurement_constraints_list_data(&measurement_constraints_list_data);
#endif
#ifdef EEBUS_ENABLE_MGCP_USECASE
    eebus.usecases->monitoring_of_grid_connection_point.get_measurement_constraints_list_data(&measurement_constraints_list_data);
#endif
    return measurement_constraints_list_data;
}

MeasurementListDataType EVSEEntity::get_measurement_list_data()
{
    MeasurementListDataType measurement_list_data;
#ifdef EEBUS_ENABLE_MPC_USECASE
    eebus.usecases->monitoring_of_power_consumption.get_measurement_list_data(&measurement_list_data);
#endif
#ifdef EEBUS_ENABLE_MGCP_USECASE
    eebus.usecases->monitoring_of_grid_connection_point.get_measurement_list_data(&measurement_list_data);
#endif
    return measurement_list_data;
}

BillDescriptionListDataType EVSEEntity::get_bill_description_list_data()
{
    BillDescriptionListDataType bill_description_list_data;
#ifdef EEBUS_ENABLE_EVCS_USECASE
    eebus.usecases->charging_summary.get_bill_description_list(&bill_description_list_data);
#endif
    return bill_description_list_data;
}

BillConstraintsListDataType EVSEEntity::get_bill_constraints_list_data()
{
    BillConstraintsListDataType bill_constraints_list_data;
#ifdef EEBUS_ENABLE_EVCS_USECASE
    eebus.usecases->charging_summary.get_bill_constraints_list(&bill_constraints_list_data);
#endif
    return bill_constraints_list_data;
}

BillListDataType EVSEEntity::get_bill_list_data()
{
    BillListDataType bill_list_data;
#ifdef EEBUS_ENABLE_EVCS_USECASE
    eebus.usecases->charging_summary.get_bill_list_data(&bill_list_data);
#endif
    return bill_list_data;
}

// ============================================================================
// EVEntity Implementation
// ============================================================================

DeviceConfigurationKeyValueDescriptionListDataType EVEntity::get_device_configuration_value_description_list()
{
    DeviceConfigurationKeyValueDescriptionListDataType device_configuration_value_list_data;
#ifdef EEBUS_ENABLE_EVCC_USECASE
    eebus.usecases->ev_commissioning_and_configuration.get_device_config_description(&device_configuration_value_list_data);
#endif
    return device_configuration_value_list_data;
}

DeviceConfigurationKeyValueListDataType EVEntity::get_device_configuration_value_list()
{
    DeviceConfigurationKeyValueListDataType device_configuration_value_list;
#ifdef EEBUS_ENABLE_EVCC_USECASE
    eebus.usecases->ev_commissioning_and_configuration.get_device_config_list(&device_configuration_value_list);
#endif
    return device_configuration_value_list;
}

IdentificationListDataType EVEntity::get_identification_list_data()
{
    IdentificationListDataType identification_list_data;
#ifdef EEBUS_ENABLE_EVCC_USECASE
    eebus.usecases->ev_commissioning_and_configuration.get_identification_list(&identification_list_data);
#endif
    return identification_list_data;
}

DeviceClassificationManufacturerDataType EVEntity::get_device_classification_manufacturer_data()
{
    DeviceClassificationManufacturerDataType manufacturer_data;
#ifdef EEBUS_ENABLE_EVCC_USECASE
    eebus.usecases->ev_commissioning_and_configuration.get_device_classification_manufacturer(&manufacturer_data);
#endif
    return manufacturer_data;
}

ElectricalConnectionParameterDescriptionListDataType EVEntity::get_electrical_connection_parameter_description_list_data()
{
    ElectricalConnectionParameterDescriptionListDataType electrical_connection_parameter_description_list_data;
#ifdef EEBUS_ENABLE_EVCEM_USECASE
    eebus.usecases->ev_charging_electricity_measurement.get_electrical_connection_parameters(&electrical_connection_parameter_description_list_data);
#endif
#ifdef EEBUS_ENABLE_EVCC_USECASE
    eebus.usecases->ev_commissioning_and_configuration.get_electrical_connection_parameter_description(&electrical_connection_parameter_description_list_data);
#endif
#ifdef EEBUS_ENABLE_OPEV_USECASE
    eebus.usecases->overload_protection_by_ev_charging_current_curtailment.get_electrical_connection_parameter_description_list_data(&electrical_connection_parameter_description_list_data);
#endif
    return electrical_connection_parameter_description_list_data;
}

ElectricalConnectionPermittedValueSetListDataType EVEntity::get_electrical_connection_permitted_list_data()
{
    ElectricalConnectionPermittedValueSetListDataType electrical_connection_permitted_value_set_list_data;
#ifdef EEBUS_ENABLE_EVCC_USECASE
    eebus.usecases->ev_commissioning_and_configuration.get_electrical_connection_permitted_values(&electrical_connection_permitted_value_set_list_data);
#endif
#ifdef EEBUS_ENABLE_OPEV_USECASE
    eebus.usecases->overload_protection_by_ev_charging_current_curtailment.get_electrical_connection_permitted_list_data(&electrical_connection_permitted_value_set_list_data);
#endif
    return electrical_connection_permitted_value_set_list_data;
}

ElectricalConnectionDescriptionListDataType EVEntity::get_electrical_connection_description_list_data()
{
    ElectricalConnectionDescriptionListDataType electrical_connection_description_list_data;
#ifdef EEBUS_ENABLE_EVCEM_USECASE
    eebus.usecases->ev_charging_electricity_measurement.get_electrical_connection_description(&electrical_connection_description_list_data);
#endif
    return electrical_connection_description_list_data;
}

DeviceDiagnosisStateDataType EVEntity::get_diagnosis_state_data()
{
    DeviceDiagnosisStateDataType diagnosis_state_data;
#ifdef EEBUS_ENABLE_EVCC_USECASE
    diagnosis_state_data = eebus.usecases->ev_commissioning_and_configuration.get_device_diagnosis_state();
#endif
    return diagnosis_state_data;
}

MeasurementDescriptionListDataType EVEntity::get_measurement_description_list_data()
{
    MeasurementDescriptionListDataType measurement_description_list_data;
#ifdef EEBUS_ENABLE_EVCEM_USECASE
    eebus.usecases->ev_charging_electricity_measurement.get_measurement_description_list(&measurement_description_list_data);
#endif
    return measurement_description_list_data;
}

MeasurementConstraintsListDataType EVEntity::get_measurement_constraints_list_data()
{
    MeasurementConstraintsListDataType measurement_constraints_list_data;
#ifdef EEBUS_ENABLE_EVCEM_USECASE
    eebus.usecases->ev_charging_electricity_measurement.get_measurement_constraints(&measurement_constraints_list_data);
#endif
    return measurement_constraints_list_data;
}

MeasurementListDataType EVEntity::get_measurement_list_data()
{
    MeasurementListDataType measurement_list_data;
#ifdef EEBUS_ENABLE_EVCEM_USECASE
    eebus.usecases->ev_charging_electricity_measurement.get_measurement_list(&measurement_list_data);
#endif
    return measurement_list_data;
}

LoadControlLimitDescriptionListDataType EVEntity::get_load_control_limit_description_list_data()
{
    LoadControlLimitDescriptionListDataType load_control_limit_description_list_data;
#ifdef EEBUS_ENABLE_OPEV_USECASE
    eebus.usecases->overload_protection_by_ev_charging_current_curtailment.get_load_control_limit_description_list_data(&load_control_limit_description_list_data);
#endif
    return load_control_limit_description_list_data;
}

LoadControlLimitListDataType EVEntity::get_load_control_limit_list_data()
{
    LoadControlLimitListDataType load_control_limit_list_data;
#ifdef EEBUS_ENABLE_OPEV_USECASE
    eebus.usecases->overload_protection_by_ev_charging_current_curtailment.get_load_control_limit_list_data(&load_control_limit_list_data);
#endif
    return load_control_limit_list_data;
}

LoadControlLimitConstraintsListDataType EVEntity::get_load_control_limit_constraints_list_data()
{
    LoadControlLimitConstraintsListDataType load_control_limit_constraints_list_data;
#ifdef EEBUS_ENABLE_OPEV_USECASE
    // TODO: Implement get_load_control_constraints_list_data in OPEV usecase
    //eebus.usecases->overload_protection_by_ev_charging_current_curtailment.get_load_control_constraints_list_data(&load_control_limit_constraints_list_data);
#endif
    return load_control_limit_constraints_list_data;
}

TimeSeriesDescriptionListDataType EVEntity::get_time_series_description_list_data()
{
    TimeSeriesDescriptionListDataType time_series_description_list_data;
#ifdef EEBUS_ENABLE_CEVC_USECASE
    eebus.usecases->coordinate_ev_charging.read_time_series_description(&time_series_description_list_data);
#endif
    return time_series_description_list_data;
}

TimeSeriesConstraintsListDataType EVEntity::get_time_series_constraints_list_data()
{
    TimeSeriesConstraintsListDataType time_series_constraints_list_data;
#ifdef EEBUS_ENABLE_CEVC_USECASE
    eebus.usecases->coordinate_ev_charging.read_time_series_constraints(&time_series_constraints_list_data);
#endif
    return time_series_constraints_list_data;
}

TimeSeriesListDataType EVEntity::get_time_series_list_data()
{
    TimeSeriesListDataType time_series_list_data;
#ifdef EEBUS_ENABLE_CEVC_USECASE
    eebus.usecases->coordinate_ev_charging.read_time_series_list(&time_series_list_data);
#endif
    return time_series_list_data;
}

IncentiveTableDescriptionDataType EVEntity::get_incentive_table_description_data()
{
    IncentiveTableDescriptionDataType incentive_table_description_data;
#ifdef EEBUS_ENABLE_CEVC_USECASE
    eebus.usecases->coordinate_ev_charging.read_incentive_table_description(&incentive_table_description_data);
#endif
    return incentive_table_description_data;
}

IncentiveTableConstraintsDataType EVEntity::get_incentive_table_constraints_data()
{
    IncentiveTableConstraintsDataType incentive_table_constraints_data;
#ifdef EEBUS_ENABLE_CEVC_USECASE
    eebus.usecases->coordinate_ev_charging.read_incentive_table_constraints(&incentive_table_constraints_data);
#endif
    return incentive_table_constraints_data;
}

IncentiveTableDataType EVEntity::get_incentive_table_data()
{
    IncentiveTableDataType incentive_table_data;
#ifdef EEBUS_ENABLE_CEVC_USECASE
    eebus.usecases->coordinate_ev_charging.read_incentive_table_data(&incentive_table_data);
#endif
    return incentive_table_data;
}
