"""
esp32-firmware
Copyright (C) 2025 Julius Dill <julius@tinkerforge.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
from dataclasses import dataclass, field
from typing import Optional

import xmlschema


# =============================================================================
# Configuration Constants
# =============================================================================

GENERATE_DEFAULT_CONSTRUCTOR = False

# Reserved C++ keywords and names that need prefixing
CPP_FORBIDDEN_NAMES = frozenset(["switch", "auto", "SAR", "delete"])

# Type mappings from XSD types to C++ types
XSD_TO_CPP_TYPE_MAP = {
    "string": "std::string",
    "str": "std::string",
    "int": "int",
    "integer": "int",
    "float": "float",
    "double": "double",
    "boolean": "bool",
    "bool": "bool",
    "duration": "std::string",
    "datetime10": "std::string",
    "datetime": "std::string",
    "date": "std::string",
    "time": "std::string",
    "anytype": "std::string",
    "unsignedshort": "uint16_t",
    "unsignedint": "uint32_t",
    "unsignedbyte": "uint8_t",
    "unsignedlong": "uint64_t",
    "hexbinary": "std::string",
}

# Types to ignore during processing
IGNORED_STRUCT_TYPES = frozenset(["CmdType", "DatagramType", "PayloadType", "FilterType"])
IGNORED_ELEMENT_TYPES = frozenset(["filtertype", "payloadtype", "datagramtype", "hexbinary"])
IGNORED_ELEMENT_NAMES = frozenset(["device", "entity", "feature", "hexbinary"])

# Whitelisted XSD schema feature groups to process.
# Only schemas matching these groups (and their _overview variants) will be processed.
WHITELISTED_SCHEMA_GROUPS = frozenset([
    # Core infrastructure
    "CommonDataTypes",
    "CommandCommonDefinitions",
    "CommandFrame",
    "Datagram",
    "Version",
    "Result",
    # Node/Session management
    "NodeManagement",
    "SubscriptionManagement",
    "BindingManagement",
    # Device feature groups
    "DeviceClassification",
    "DeviceConfiguration",
    "DeviceDiagnosis",
    # Use-case specific feature groups
    "ElectricalConnection",
    "Measurement",
    "LoadControl",
    "Bill",
    "Identification",
    "TimeSeries",
    "IncentiveTable",
    "UseCaseInformation",
])

# Whitelisted function names for the SpineDataTypeHandler.
# Only these functions (and their associated types) will be included in the handler class.
WHITELISTED_FUNCTIONS = frozenset([
    "nodeManagementDetailedDiscoveryData",
    "nodeManagementUseCaseData",
    "nodeManagementSubscriptionData",
    "nodeManagementSubscriptionRequestCall",
    "nodeManagementSubscriptionDeleteCall",
    "nodeManagementBindingData",
    "nodeManagementBindingRequestCall",
    "nodeManagementBindingDeleteCall",
    "deviceDiagnosisHeartbeatData",
    "deviceDiagnosisStateData",
    "deviceClassificationManufacturerData",
    "deviceConfigurationKeyValueDescriptionListData",
    "deviceConfigurationKeyValueListData",
    "identificationListData",
    "measurementDescriptionListData",
    "measurementConstraintsListData",
    "measurementListData",
    "electricalConnectionDescriptionListData",
    "electricalConnectionParameterDescriptionListData",
    "electricalConnectionPermittedValueSetListData",
    "electricalConnectionCharacteristicListData",
    "loadControlLimitDescriptionListData",
    "loadControlLimitConstraintsListData",
    "loadControlLimitListData",
    "billDescriptionListData",
    "billConstraintsListData",
    "billListData",
    "timeSeriesDescriptionListData",
    "timeSeriesConstraintsListData",
    "timeSeriesListData",
    "incentiveTableDescriptionData",
    "incentiveTableConstraintsData",
    "incentiveTableData",
    "resultData",
])

# Types directly referenced by application code (outside the handler).
# These are used as extra root types for dependency-based pruning.
ADDITIONAL_REQUIRED_TYPES = frozenset([
    "HeaderType",                         # Used in handle_message() signatures
    "BindingManagementEntryListDataType", # Stored as member in use-case class
])


# =============================================================================
# C++ Code Templates
# =============================================================================

class CppTemplates:
    """Container for C++ code templates used in code generation."""

    SPINE_HEADER = """
// This file is generated by generator.py. Changes will be overwritten

#pragma once

#include "module.h"
#include "config.h"
#include <string>
#include <vector>
#include <optional>

namespace ArduinoJson
{
template <typename T> struct Converter<std::vector<T>>
{
    static void toJson(const std::vector<T> &src, JsonVariant dst);
    static std::vector<T> fromJson(JsonVariantConst src);
    static bool checkJson(JsonVariantConst src);
};
} // namespace ArduinoJson

/**
 * An alternative optional implementation that automatically fills itself if needed.
 * @tparam T The typename of the optional value.
 */
template <typename T>
class SpineOptional
{
    mutable T value{};
    mutable bool is_set = false;

public:
    using value_type = T;
    SpineOptional() = default;
    SpineOptional(const SpineOptional&) = default;
    SpineOptional(SpineOptional&&) noexcept = default;
    SpineOptional& operator=(const SpineOptional&) = default;
    SpineOptional& operator=(SpineOptional&&) noexcept = default;
    

    SpineOptional(const T &v) : value(v), is_set(true)
    {
    }

    /**
     * Assign value to the optional and mark it as set.
     * @param v the value assigned
     * @return
     */
    SpineOptional &operator=(const T &v)
    {
        value = v;
        is_set = true;
        return *this;
    }

    explicit operator bool() const noexcept
    {
        return is_set;
    }

    /**
     *
     * @return The value contained in the optional.
     */
    T &operator*()
    {
        return value;
    }

    /**
     *
     * @return The value contained in the optional.
     */
    const T &operator*() const
    {
        return value;
    }

    /**
     *
     * @return The pointer to the value contained in the optional. If the optional is not set, it will be default constructed and set.
     */
    T *operator->()
    {
        if (!is_set) {
            value = T{};
        }
        is_set = true;
        return &value;
    }

    /**
     *
     * @return The pointer to the value contained in the optional. If the optional is not set, it will be default constructed and set.
     */
    const T *operator->() const
    {
        if (!is_set) {
            value = T{};
        }
        is_set = true;
        return &value;
    }

    T &get()
    {
        is_set = true;
        return value;
    }

    const T &get() const
    {
        is_set = true;
        return value;
    }

    bool has_value() const
    {
        return is_set;
    }

    /**
     * Resets the optional to an unset state and clears the value.
     */
    void reset()
    {
        is_set = false;
        value = T{};
    }

    bool isNull() const
    {
        return !is_set;
    }

    /**
     * Resets the optional to a set state and default constructs the value.
     */
    void emplace()
    {
        is_set = true;
        value = T{};
    }

    friend bool operator==(const SpineOptional<T> &opt, const T &v)
    {
        return opt.is_set && opt.value == v;
    }

    friend bool operator==(const T &v, const SpineOptional<T> &opt)
    {
        return opt.is_set && opt.value == v;
    }

    friend bool operator!=(const SpineOptional<T> &opt, const T &v)
    {
        return !(opt == v);
    }

    friend bool operator!=(const T &v, const SpineOptional<T> &opt)
    {
        return !(v == opt);
    }
};

struct DateTimeStruct
{
    // Added manually
    SpineOptional<uint16_t> year;
    SpineOptional<uint8_t> month;
    SpineOptional<uint8_t> day;
    SpineOptional<uint8_t> hour;
    SpineOptional<uint8_t> minute;
    SpineOptional<uint8_t> second;
    SpineOptional<uint16_t> millisecond;
    SpineOptional<int8_t> timezone; // Offset from UTC in hours, 0 if time is UTC, empty if no TZ is given
};
"""

    SPINE_IMPLEMENTATION = """
// This file is generated by generator.py. Changes will be overwritten
#include "spine_types.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"


namespace ArduinoJson
{
template <typename T>
void Converter<std::vector<T>>::toJson(const std::vector<T> &src, JsonVariant dst)
{
    JsonArray array = dst.to<JsonArray>();
    for (T item : src)
        array.add(item);
}

template <typename T>
std::vector<T> Converter<std::vector<T>>::fromJson(JsonVariantConst src)
{
    std::vector<T> dst;
    for (T item : src.as<JsonArrayConst>())
        dst.push_back(item);    
    return dst;
}

template <typename T>
bool Converter<std::vector<T>>::checkJson(JsonVariantConst src)
{
    JsonArrayConst array = src;
    bool result = array;
    for (JsonVariantConst item : array)
        result &= item.is<T>();
    return result;
}


} // namespace ArduinoJson

static const char *enumValueToName(const char *const *names, size_t count, int value) {
    if (value >= 0 && static_cast<size_t>(value) < count) return names[value];
    return "EnumUndefined";
}

static int enumNameToValue(const char *const *names, size_t count, const String &s, int fallback) {
    for (size_t i = 0; i < count; ++i) {
        if (s == names[i]) return static_cast<int>(i);
    }
    return fallback;
}
"""

    SHIP_HEADER = """
// This file is generated by generator.py. Changes will be overwritten
#pragma once
#include "module.h"
#include "config.h"
#include <string>
#include <vector>
#include <optional>
#include "spine_types.h"
"""

    SHIP_IMPLEMENTATION = """
// This file is generated by generator.py. Changes will be overwritten
#include "ship_types_new.h"
"""


# =============================================================================
# Data Classes
# =============================================================================

@dataclass
class SpineType:
    """
    Represents a C++ type generated from XSD schema.

    Attributes:
        type_name: The kind of C++ construct ("define", "using", "enum", "struct", "class")
        name: The name of the type
        code: The generated C++ header code
        depends_on: List of type names this type depends on
        to_json_code: Generated toJson implementation code
        from_json_code: Generated fromJson implementation code
        upper_limit: Optional upper limit for numeric types
        lower_limit: Optional lower limit for numeric types
        regex_restriction: Optional regex pattern restriction
    """
    type_name: str
    name: str
    code: str = ""
    depends_on: list[str] = field(default_factory=list)
    to_json_code: str = ""
    from_json_code: str = ""
    upper_limit: Optional[float] = None
    lower_limit: Optional[float] = None
    regex_restriction: Optional[str] = None


@dataclass
class ElementInfo:
    """Holds information about a struct element/member."""
    variable_type: str
    variable_name: str
    is_vector: bool


# =============================================================================
# Code Generator State
# =============================================================================

class GeneratorState:
    """
    Manages the state of code generation.

    This class encapsulates the mutable state that was previously stored
    in global variables, making the code more testable and maintainable.
    """

    def __init__(self):
        self.cpp_datatypes: list[SpineType] = []
        self.type_function_mapping: list[tuple[str, str]] = []
        self.unprocessed_elements: int = 0

    def reset(self):
        """Reset the generator state for a new processing run."""
        self.cpp_datatypes = []
        self.type_function_mapping = []
        self.unprocessed_elements = 0

    def add_datatype(self, datatype: SpineType):
        """Add a new datatype to the collection."""
        self.cpp_datatypes.append(datatype)

    def add_function_mapping(self, element_name: str, element_type: str):
        """Add a new function-to-type mapping."""
        self.type_function_mapping.append((element_name, element_type))

    def increment_unprocessed(self):
        """Increment the count of unprocessed elements."""
        self.unprocessed_elements += 1


# Global state instance (for backward compatibility)
_generator_state = GeneratorState()


# =============================================================================
# Utility Functions
# =============================================================================

def remove_namespace(name: Optional[str]) -> str:
    """
    Remove XML namespace prefix from a name.

    Args:
        name: The name potentially containing a namespace URI

    Returns:
        The name without the namespace prefix
    """
    if name is None:
        return "None"
    if "}" in name:
        return name.split("}", 1)[1]
    return name


def to_cpp_datatype(type_name) -> str:
    """
    Convert an XSD type name to its C++ equivalent.

    Args:
        type_name: The XSD type name (can be a string or have __name__ attribute)

    Returns:
        The corresponding C++ type name
    """
    if hasattr(type_name, "__name__"):
        type_name_str = str(type_name.__name__)
    else:
        type_name_str = str(type_name)

    result = XSD_TO_CPP_TYPE_MAP.get(type_name_str.lower())
    if result is not None:
        return result
    return type_name_str


def make_variable_name(name: str) -> str:
    """
    Convert a name to a valid C++ variable name.

    Cleans up the name to only contain alphanumeric characters and underscores,
    and adds a prefix if the name starts with a non-letter or is a reserved word.

    Args:
        name: The original name

    Returns:
        A valid C++ variable name
    """
    # Replace non-alphanumeric characters with underscores
    cleaned_name = ''.join(char if char.isalnum() else '_' for char in name)

    # Ensure the name starts with a letter and is not a reserved keyword
    if (not cleaned_name[0].isalpha() or
            cleaned_name[0] in CPP_FORBIDDEN_NAMES or
            cleaned_name in CPP_FORBIDDEN_NAMES):
        cleaned_name = 'var_' + cleaned_name

    return cleaned_name


# =============================================================================
# Deduplication and Sorting
# =============================================================================

def remove_duplicate_types(spine_types: list[SpineType]) -> list[SpineType]:
    """
    Remove duplicate types from the list, keeping the first occurrence.

    Args:
        spine_types: List of SpineType objects

    Returns:
        Deduplicated list of SpineType objects
    """
    seen: set[str] = set()
    result: list[SpineType] = []

    for spine_type in spine_types:
        if spine_type.name not in seen:
            seen.add(spine_type.name)
            result.append(spine_type)

    return result


def remove_duplicate_function_mappings(
        mappings: list[tuple[str, str]]
) -> list[tuple[str, str]]:
    """
    Remove duplicate function mappings from the list.

    Args:
        mappings: List of (function_name, type_name) tuples

    Returns:
        Deduplicated list of mappings
    """
    seen: set[tuple[str, str]] = set()
    result: list[tuple[str, str]] = []

    for mapping in mappings:
        if mapping not in seen:
            seen.add(mapping)
            result.append(mapping)

    return result


def sort_and_resolve_dependencies(
        datatypes: list[SpineType],
        all_datatypes: list[SpineType]
) -> list[SpineType]:
    """
    Sort datatypes by type order and resolve dependencies using topological sort.

    Args:
        datatypes: List of SpineType objects to sort
        all_datatypes: Complete list of all datatypes for dependency resolution

    Returns:
        Sorted list of SpineType objects
    """
    type_order = {"define": 0, "using": 1, "enum": 2, "struct": 3, "class": 4}

    # Build dependency map
    dependency_map = {dt.name: dt.depends_on for dt in all_datatypes}

    sorted_datatypes: list[SpineType] = []
    visited: set[str] = set()

    def visit(datatype: SpineType):
        if datatype.name in visited:
            return
        visited.add(datatype.name)

        # Visit dependencies first
        for dependency in dependency_map.get(datatype.name, []):
            dependent_datatype = next(
                (d for d in all_datatypes if d.name == dependency), None
            )
            if dependent_datatype:
                visit(dependent_datatype)

        sorted_datatypes.append(datatype)

    # Sort by type order first
    datatypes.sort(key=lambda x: type_order.get(x.type_name, 5))

    # Then perform topological sort
    for datatype in datatypes:
        visit(datatype)

    return sorted_datatypes


def prune_unreachable_types(
        datatypes: list[SpineType],
        type_function_mapping: list[tuple[str, str]],
        additional_roots: frozenset[str] = frozenset()
) -> list[SpineType]:
    """
    Remove types that are not transitively reachable from handler function
    types or additional root types.
    """

    # Build root set: handler types + additional required types
    root_types = set(func[1] for func in type_function_mapping) | set(additional_roots)

    # Build name -> depends_on lookup
    dep_map: dict[str, list[str]] = {dt.name: dt.depends_on for dt in datatypes}
    all_type_names = {dt.name for dt in datatypes}

    # Walk transitive dependencies via BFS
    reachable: set[str] = set()
    queue = list(root_types & all_type_names)
    # Also seed with additional roots that may not be in function mapping
    for r in additional_roots:
        if r in all_type_names and r not in queue:
            queue.append(r)

    while queue:
        current = queue.pop()
        if current in reachable:
            continue
        reachable.add(current)
        for dep in dep_map.get(current, []):
            if dep not in reachable and dep in all_type_names:
                queue.append(dep)

    pruned = [dt for dt in datatypes if dt.name in reachable]

    print(f"Pruning: {len(datatypes)} types -> {len(pruned)} reachable "
          f"({len(datatypes) - len(pruned)} removed)")
    print(f"  Root types: {len(root_types)} "
          f"({len(root_types & all_type_names)} found in generated types)")

    return pruned


# =============================================================================
# Enum Type Processing
# =============================================================================

class EnumTypeGenerator:
    """Handles generation of C++ enum types from XSD simple types."""

    def __init__(self, state: GeneratorState):
        self.state = state

    def generate(self, simple_type) -> Optional[SpineType]:
        """
        Generate C++ enum code from an XSD simple type with enumeration.

        Args:
            simple_type: The XSD simple type with enumeration

        Returns:
            SpineType containing the generated enum code, or None if invalid
        """
        if simple_type.enumeration is None:
            return None

        enum_type_name = remove_namespace(simple_type.name)
        new_type = SpineType(type_name="enum", name=enum_type_name)

        # Prepare enumeration list with undefined entry
        enum_list = list(simple_type.enumeration)
        enum_undefined_name = "EnumUndefined"
        enum_list.append(enum_undefined_name)
        enum_list = list(dict.fromkeys(enum_list))  # Remove duplicates

        # Generate enum definition
        new_type.code = self._generate_enum_definition(
            enum_type_name, enum_list, enum_undefined_name, simple_type
        )

        # Generate function prototypes
        new_type.code += self._generate_function_prototypes(enum_type_name)

        # Generate implementations
        new_type.to_json_code = self._generate_to_json_code(
            enum_type_name, enum_list, enum_undefined_name
        )
        new_type.from_json_code = self._generate_from_json_code(
            enum_type_name, enum_list, enum_undefined_name
        )

        return new_type

    def _generate_enum_definition(
            self, enum_type_name: str, enum_list: list,
            enum_undefined_name: str, simple_type
    ) -> str:
        """Generate the enum class definition."""
        code = (
            f"/**\n* Datatype {enum_type_name} as defined in "
            f"{simple_type.schema.name}\n*/\n"
            f"enum class {enum_type_name} {{\n"
        )

        for enumeration in enum_list:
            enum_string_name = str(enumeration)
            enum_variable_name = make_variable_name(enum_string_name)

            if enum_string_name == enum_undefined_name:
                code += (
                    f"\t{enum_variable_name}, "
                    "// This is not part of the spec but its needed for error handling\n"
                )
            else:
                code += f"\t{enum_variable_name},\n"

        code += "};\n"
        return code

    def _generate_function_prototypes(self, enum_type_name: str) -> str:
        """Generate function prototypes for the enum."""
        return f"""/**
 * Convert the enum {enum_type_name} to its String representation
 */
String convertToString(const {enum_type_name} &src);

/**
 * Convert the enum {enum_type_name} to JSON (uses convertToString)
 */
bool convertToJson(const {enum_type_name} &src, JsonVariant& dst);

/**
 * Convert a string to a {enum_type_name}
 */
void convertFromString(const String &src, {enum_type_name} &dst);

/**
 * Convert a JSON variant containing a string to a {enum_type_name} (uses convertFromString)
 */
void convertFromJson(const JsonVariantConst& src, {enum_type_name} &dst);

"""

    def _generate_to_json_code(
            self, enum_type_name: str, enum_list: list, enum_undefined_name: str
    ) -> str:
        """Generate static names array, convertToString and convertToJson implementations."""
        # Static names array (enum values are sequential 0..N-1, so index == enum value)
        names_var = f"{enum_type_name}_names"
        code = f"static const char *const {names_var}[] = {{\n"

        for enumeration in enum_list:
            enum_string_name = str(enumeration)
            if enum_string_name == enum_undefined_name:
                continue
            code += f"\t\"{enum_string_name}\",\n"

        code += "};\n"
        code += f"static constexpr size_t {enum_type_name}_count = sizeof({names_var}) / sizeof({names_var}[0]);\n\n"

        # convertToString implementation (O(1) index lookup)
        code += (
            f"String convertToString(const {enum_type_name} &src) {{\n"
            f"\treturn enumValueToName({names_var}, {enum_type_name}_count, static_cast<int>(src));\n}}\n\n"
        )

        # convertToJson implementation
        code += (
            f"bool convertToJson(const {enum_type_name} &src, JsonVariant& dst) {{\n"
            f"\treturn dst.set(convertToString(src));\n}}\n\n"
        )

        return code

    def _generate_from_json_code(
            self, enum_type_name: str, enum_list: list, enum_undefined_name: str
    ) -> str:
        """Generate convertFromString and convertFromJson implementations."""
        names_var = f"{enum_type_name}_names"
        undefined_var = make_variable_name(enum_undefined_name)

        # convertFromString implementation (linear scan of names array)
        from_string_impl = (
            f"void convertFromString(const String &src, {enum_type_name} &dst) {{\n"
            f"\tdst = static_cast<{enum_type_name}>(enumNameToValue({names_var}, {enum_type_name}_count, src, static_cast<int>({enum_type_name}::{undefined_var})));\n}}\n\n"
        )

        # convertFromJson implementation
        from_json_impl = (
            f"void convertFromJson(const JsonVariantConst& src, {enum_type_name} &dst) {{\n"
            f"\tString s = src.as<const char*>();\n"
            f"\tconvertFromString(s, dst);\n}}\n\n"
        )

        return from_string_impl + from_json_impl


# =============================================================================
# Complex Type Processing
# =============================================================================

class ComplexTypeGenerator:
    """Handles generation of C++ struct types from XSD complex types."""

    def __init__(self, state: GeneratorState):
        self.state = state

    def generate(self, complex_type) -> Optional[SpineType]:
        """
        Generate C++ struct code from an XSD complex type.

        Args:
            complex_type: The XSD complex type to process

        Returns:
            SpineType containing the generated struct code, or None if skipped
        """
        struct_type_name = remove_namespace(complex_type.name)

        # Skip ignored types
        if struct_type_name in IGNORED_STRUCT_TYPES:
            return None

        # Handle simple content types
        if (hasattr(complex_type.content, "content_type_label") and
                complex_type.content.content_type_label == "simple"):
            return self._generate_using_type(complex_type, struct_type_name)

        # Handle sequence types
        if complex_type.content.model == "sequence":
            return self._generate_struct_type(complex_type, struct_type_name)

        return None

    def _generate_using_type(self, complex_type, type_name: str) -> SpineType:
        """Generate a 'using' type alias for simple content types."""
        new_type = SpineType(type_name="using", name=type_name)
        cpp_type = to_cpp_datatype(complex_type.base_type.python_type)
        new_type.code = f"using {type_name} = {cpp_type}; \n"
        return new_type

    def _generate_struct_type(self, complex_type, struct_type_name: str) -> Optional[SpineType]:
        """Generate a struct type from a sequence complex type."""
        new_type = SpineType(type_name="struct", name=struct_type_name)

        # Extract elements
        elements = self._extract_elements(complex_type, struct_type_name)
        if elements is None:
            return None

        # Generate struct code
        new_type.code = self._generate_struct_definition(
            struct_type_name, elements, complex_type
        )
        new_type.to_json_code = self._generate_struct_to_json(struct_type_name, elements)
        new_type.from_json_code = self._generate_struct_from_json(struct_type_name, elements)

        # Track dependencies
        for elem in elements:
            if elem.variable_type == to_cpp_datatype(elem.variable_type):
                new_type.depends_on.append(elem.variable_type)

        return new_type

    def _extract_elements(self, complex_type, struct_type_name: str) -> Optional[list[ElementInfo]]:
        """Extract element information from a complex type."""
        elements: list[ElementInfo] = []

        for elem in complex_type.content:
            extracted = self._process_element(elem, struct_type_name)
            if extracted is None:
                return None
            elements.extend(extracted)

        return elements

    def _process_element(
            self, elem, struct_type_name: str
    ) -> Optional[list[ElementInfo]]:
        """Process a single element and return its information."""
        if hasattr(elem, "model"):
            return self._process_model_element(elem, struct_type_name)
        else:
            return self._process_simple_element(elem, struct_type_name)

    def _process_model_element(
            self, elem, struct_type_name: str
    ) -> Optional[list[ElementInfo]]:
        """Process an element that has a model attribute."""
        if elem.model != "sequence":
            if struct_type_name == "FilterType":
                return []  # FilterType is handled manually
            print(f"unprocessed element: {struct_type_name} has model type but is not sequence")
            self.state.increment_unprocessed()
            return None

        if len(elem) == 0:
            print(
                f"unprocessed element: {struct_type_name} has model type sequence "
                "but no type can be determined"
            )
            self.state.increment_unprocessed()
            return None

        elements: list[ElementInfo] = []
        for sub_elem in elem:
            # Unwrap single-element sequences
            if len(sub_elem) == 1:
                sub_elem = sub_elem[0]
            elif len(sub_elem) > 1:
                print("Sub element too long")

            is_vec = sub_elem.max_occurs != 1
            variable_name = make_variable_name(remove_namespace(sub_elem.name))

            if not hasattr(sub_elem, "type") and hasattr(sub_elem, "local_name"):
                variable_type = remove_namespace(sub_elem.local_name)
            else:
                variable_type = remove_namespace(sub_elem.type.name)

            elements.append(ElementInfo(variable_type, variable_name, is_vec))

        return elements

    def _process_simple_element(
            self, elem, struct_type_name: str
    ) -> Optional[list[ElementInfo]]:
        """Process a simple element without model attribute."""
        variable_name = remove_namespace(elem.name)
        variable_type = remove_namespace(elem.type.name)

        # Handle special cases for None type
        if variable_type is None or variable_type == "None":
            result = self._resolve_none_type(elem, variable_name, struct_type_name)
            if result is None:
                return None
            return result

        # Check if there's a matching enum type
        variable_type = self._check_for_enum_type(variable_type)

        is_vec = elem.max_occurs is None
        return [ElementInfo(variable_type, variable_name, is_vec)]

    def _resolve_none_type(
            self, elem, variable_name: str, struct_type_name: str
    ) -> Optional[list[ElementInfo]]:
        """Try to resolve a None variable type."""
        if elem.type.base_type is None:
            # Handle special version case
            if variable_name == "version":
                return self._handle_version_element(elem)
            elif variable_name == "dnsSd_mDns":
                return [ElementInfo("ElementTagType", variable_name, False)]
            elif variable_name == "dns":
                print("Not processed AccessMethodsType.dns as its optional and a bit fiddly to implement")
                self.state.increment_unprocessed()
                return []
            else:
                print(f"Still an unprocessed unknown element: {struct_type_name}.{variable_name}")
                self.state.increment_unprocessed()
                return None
        else:
            variable_type = remove_namespace(elem.type.base_type.name)
            if variable_type is None or variable_type == "None":
                print("Variable type is still none")
                self.state.increment_unprocessed()
                return None
            is_vec = elem.max_occurs is None
            return [ElementInfo(variable_type, variable_name, is_vec)]

    def _handle_version_element(self, elem) -> list[ElementInfo]:
        """Handle the special version element with two sub-elements."""
        version_1_name = elem[0].local_name
        version_1_type = remove_namespace(elem[0].type.name)
        version_2_name = elem[1].local_name
        version_2_type = remove_namespace(elem[1].type.name)

        return [
            ElementInfo(version_1_type, version_1_name, False),
            ElementInfo(version_2_type, version_2_name, False),
        ]

    def _check_for_enum_type(self, variable_type: str) -> str:
        """Check if there's a matching enum type for the variable type."""
        hypothetical_enum_type = re.sub(r'Type$', 'EnumType', variable_type)

        for enum_type in self.state.cpp_datatypes:
            if enum_type.type_name == "enum" and enum_type.name == hypothetical_enum_type:
                return hypothetical_enum_type

        return variable_type

    def _generate_struct_definition(
            self, struct_type_name: str, elements: list[ElementInfo], complex_type
    ) -> str:
        """Generate the struct definition code."""
        code = (
            f"/**\n * Datatype {struct_type_name} as defined in "
            f"{complex_type.schema.name}\n*/\n"
            f"struct {struct_type_name} {{ \n"
        )

        # Generate member variables
        for elem in elements:
            cpp_type = self._get_cpp_member_type(elem)
            code += f"\tSpineOptional<{cpp_type}> {make_variable_name(elem.variable_name)}{{}};\n"

        # Generate copy constructor
        code += f"\n\t{struct_type_name}(const {struct_type_name}& other) = default;\n"

        # Generate default constructor
        if not GENERATE_DEFAULT_CONSTRUCTOR:
            code += f"\n\t{struct_type_name}() = default;\n}};\n"
        else:
            code += self._generate_explicit_default_constructor(struct_type_name, elements)

        # Add function prototypes
        code += self._generate_struct_function_prototypes(struct_type_name)

        return code

    def _get_cpp_member_type(self, elem: ElementInfo) -> str:
        """Get the C++ type for a struct member."""
        cpp_type = to_cpp_datatype(elem.variable_type)

        # Handle edge case
        if cpp_type == "SpecificationVersionDataType":
            cpp_type = "SpecificationVersionType"

        if elem.is_vector:
            cpp_type = f"std::vector<{cpp_type}>"

        return cpp_type

    def _generate_explicit_default_constructor(
            self, struct_type_name: str, elements: list[ElementInfo]
    ) -> str:
        """Generate an explicit default constructor with member initialization."""
        code = f"\n\t{struct_type_name}()\n\t\t:"

        for elem in elements:
            cpp_type = self._get_cpp_member_type(elem)
            variable_name_cpp = make_variable_name(elem.variable_name)
            code += f"\n\t\t{variable_name_cpp}({cpp_type}{{}}),"

        code = code[:-1]  # Remove last comma
        code += "\n\t{}\n};\n"
        return code

    def _generate_struct_function_prototypes(self, struct_type_name: str) -> str:
        """Generate function prototypes for struct conversion."""
        return f"""/**
 * Convert a {struct_type_name} to its JSON representation
 * @param src The {struct_type_name} to convert
 * @param dst The JSON variant to fill with the converted data.
 * @return true if the conversion was successful, false otherwise.
 */
bool convertToJson(const {struct_type_name} &src, JsonVariant& dst);
/**
 * Convert a JSON representation to a {struct_type_name}
 * @param src The JSON variant to convert
 * @param dst The {struct_type_name} to fill with the converted data.
 */
void convertFromJson(const JsonVariantConst& src, {struct_type_name} &dst);

"""

    def _generate_struct_to_json(
            self, struct_type_name: str, elements: list[ElementInfo]
    ) -> str:
        """Generate the convertToJson implementation for a struct."""
        if not elements:
            return (
                f"bool convertToJson(const {struct_type_name} &src, JsonVariant& dst) {{\n"
                f"\tif (!dst.to<JsonObject>()) {{return false;}}\n\treturn true;\n}}\n"
            )

        code = (
            f"bool convertToJson(const {struct_type_name} &src, JsonVariant& dst) {{\n"
            f"\tif (!dst.to<JsonObject>()) {{return false;}}\n"
        )

        for elem in elements:
            variable_name_cpp = make_variable_name(elem.variable_name)
            variable_name_string = elem.variable_name
            code += (
                f"\tif (src.{variable_name_cpp}.has_value()) {{\n"
                f"\t\tdst[\"{variable_name_string}\"] = *src.{variable_name_cpp};\n"
                f"\t}}\n"
            )

        code += "\n\treturn true;\n}\n"
        return code

    def _generate_struct_from_json(
            self, struct_type_name: str, elements: list[ElementInfo]
    ) -> str:
        """Generate the convertFromJson implementation for a struct."""
        if not elements:
            return (
                f"void convertFromJson(const JsonVariantConst& src, {struct_type_name} &dst) {{\n"
                f"\n}}\n"
            )

        code = (
            f"void convertFromJson(const JsonVariantConst& src, {struct_type_name} &dst) {{\n"
            f"\t\n\t\t"
        )

        for elem in elements:
            cpp_type = self._get_cpp_member_type(elem)
            if cpp_type is None or cpp_type == "None":
                print("Variable type is None while making complex type")
                self.state.increment_unprocessed()
                continue

            variable_name_cpp = make_variable_name(elem.variable_name)
            variable_name_string = elem.variable_name
            code += (
                f"\tif (!src[\"{variable_name_string}\"].isNull()) {{\n"
                f"\t\tdst.{variable_name_cpp} = src[\"{variable_name_string}\"]"
                f".as<decltype(dst.{variable_name_cpp})::value_type>();\n"
                f"\t}} else {{\n"
                f"\t\tdst.{variable_name_cpp}.reset();\n\t}}\n"
            )

        code += "\n\t\n\n}\n"
        return code


# =============================================================================
# Data Handler Class Generation
# =============================================================================

class DataHandlerGenerator:
    """Generates the SpineDataTypeHandler class code."""

    def __init__(self, state: GeneratorState):
        self.state = state

    def generate(self) -> tuple[str, str]:
        """
        Generate the SpineDataTypeHandler class.

        Returns:
            Tuple of (header_code, implementation_code)
        """
        return self._generate_header(), self._generate_implementation()

    def _generate_header(self) -> str:
        """Generate the header code for SpineDataTypeHandler."""
        function_enum_entries = ''.join(
            f"\n\t\t\t{func[0]}," for func in self.state.type_function_mapping
        )
        type_enum_entries = ''.join(
            f"\n\t\t\t{func[1]}," for func in self.state.type_function_mapping
        )
        # Only generate data members for types referenced by function mappings
        handler_type_names = sorted(set(func[1] for func in self.state.type_function_mapping))
        data_types = ''.join(
            f"\n\t\tSpineOptional<{type_name}> {type_name.lower()}{{}};"
            for type_name in handler_type_names
        )

        return f"""
class SpineDataTypeHandler {{
    public:

        {data_types}
        enum class Function {{{function_enum_entries}
        None
        }};  
        enum class Type {{{type_enum_entries}
        None
        }};
        
        
        Function last_cmd;
        
        Type type_from_function(Function function);
        static Function function_from_string(const String function_name);
        Function handle_cmd(JsonObjectConst obj);
        static String function_to_string(Function function);
        void last_cmd_to_json(JsonVariant &dst);
        void reset();
}};
    """

    def _generate_implementation(self) -> str:
        """Generate the implementation code for SpineDataTypeHandler."""
        function_to_type_mapping = ''.join(
            f"\n\t\tcase SpineDataTypeHandler::Function::{func[0]}: "
            f"return SpineDataTypeHandler::Type::{func[1]};"
            for func in self.state.type_function_mapping
        )
        string_to_function_mapping = ''.join(
            f"\n\tif (function_name == \"{func[0]}\") "
            f"return SpineDataTypeHandler::Function::{func[0]};"
            for func in self.state.type_function_mapping
        )
        function_to_string_mapping = ''.join(
            f"\n\tif (function == SpineDataTypeHandler::Function::{func[0]}) "
            f"return \"{func[0]}\";"
            for func in self.state.type_function_mapping
        )
        last_cmd_to_json_mapping = ''.join(
            f"""
    if (last_cmd == SpineDataTypeHandler::Function::{func[0]}) {{
        dst["{func[0]}"] = *{func[1].lower()};
    }}
    """
            for func in self.state.type_function_mapping
        )
        cmd_types = ''.join(
            f"""
    if (obj["{func[0]}"]) {{
        last_cmd = function_from_string("{func[0]}");
        {func[1].lower()} =  obj["{func[0]}"].as<decltype({func[1].lower()})::value_type>();  
        return last_cmd;
    }}"""
            for func in self.state.type_function_mapping
        )
        # Only reset data members that are actually part of the handler
        handler_type_names = sorted(set(func[1] for func in self.state.type_function_mapping))
        reset_calls = ''.join(
            f"\n\t\t{type_name.lower()}.reset();"
            for type_name in handler_type_names
        )

        return f"""
SpineDataTypeHandler::Type SpineDataTypeHandler::type_from_function(SpineDataTypeHandler::Function function) {{
    switch(function) {{{function_to_type_mapping}
    default:    
        return SpineDataTypeHandler::Type::None;
    }}
    return SpineDataTypeHandler::Type::None;
}}

SpineDataTypeHandler::Function SpineDataTypeHandler::function_from_string(const String function_name) {{
{string_to_function_mapping}
    return SpineDataTypeHandler::Function::None;
}}

SpineDataTypeHandler::Function SpineDataTypeHandler::handle_cmd(JsonObjectConst obj) {{
    {cmd_types}
    return SpineDataTypeHandler::Function::None;
}}
String SpineDataTypeHandler::function_to_string(Function function) {{
    {function_to_string_mapping}
    return "Unknown function";
}}

void SpineDataTypeHandler::last_cmd_to_json(JsonVariant &dst) {{
    {last_cmd_to_json_mapping}
}}

void SpineDataTypeHandler::reset() {{
    last_cmd = Function::None;
    {reset_calls}
}}
    """


# =============================================================================
# Schema Processing
# =============================================================================

class SchemaProcessor:
    """Processes XSD schemas and generates C++ code."""

    def __init__(self, state: GeneratorState):
        self.state = state
        self.enum_generator = EnumTypeGenerator(state)
        self.complex_generator = ComplexTypeGenerator(state)
        self.data_handler_generator = DataHandlerGenerator(state)

    def process_schema(self, xml_schema):
        """
        Process an XSD schema and extract types.

        Args:
            xml_schema: The parsed XSD schema
        """
        self._process_simple_types(xml_schema)
        self._process_complex_types(xml_schema)
        self._process_elements(xml_schema)

    def _process_simple_types(self, xml_schema):
        """Process all simple types from the schema."""
        for simple_type in xml_schema.simple_types:
            if simple_type.derivation == "restriction":
                self._process_restriction_type(simple_type)
            elif hasattr(simple_type, 'member_types'):
                self._process_union_type(simple_type)
            else:
                print(f"Unidentified Type {simple_type.name}")
                self.state.increment_unprocessed()

    def _process_restriction_type(self, simple_type):
        """Process a restriction-based simple type (enum or using)."""
        if simple_type.enumeration is not None:
            # It's an enum
            new_type = self.enum_generator.generate(simple_type)
            if new_type:
                self.state.add_datatype(new_type)
        elif hasattr(simple_type, 'base_type'):
            # It's a using type alias
            new_type = self._create_using_type(simple_type)
            if new_type:
                self.state.add_datatype(new_type)
        else:
            self.state.increment_unprocessed()
            print(
                f"simple type: {simple_type.name} has restriction but no enumeration. "
                "Not sure what to do with this"
            )

    def _create_using_type(self, simple_type) -> SpineType:
        """Create a using type alias from a simple type."""
        type_name = remove_namespace(simple_type.name)
        cpp_type = to_cpp_datatype(simple_type.base_type.python_type)

        new_type = SpineType(type_name="using", name=type_name)
        new_type.code = f"using {type_name} = {cpp_type};\n"

        # Track restrictions (for documentation/validation purposes)
        found_restriction = False
        if simple_type.base_type.max_value is not None:
            new_type.upper_limit = simple_type.base_type.max_value
        if simple_type.base_type.min_value is not None:
            new_type.lower_limit = simple_type.base_type.min_value
            found_restriction = True
        if cpp_type == "std::string":
            found_restriction = True
        if hasattr(simple_type.base_type.python_type, 'name'):
            if simple_type.base_type.python_type.name == "duration":
                found_restriction = True

        if not found_restriction:
            print(f"Unknown restriction to basetype {simple_type.name}")

        return new_type

    def _process_union_type(self, simple_type):
        """Process a union simple type."""
        type_name = remove_namespace(simple_type.name)
        cpp_type = to_cpp_datatype(simple_type.python_type)

        new_type = SpineType(type_name="using", name=type_name)
        new_type.code = f"using {type_name} = {cpp_type};\n"
        self.state.add_datatype(new_type)

    def _process_complex_types(self, xml_schema):
        """Process all complex types from the schema."""
        for complex_type in xml_schema.complex_types:
            new_type = self.complex_generator.generate(complex_type)
            if new_type:
                self.state.add_datatype(new_type)

    def _process_elements(self, xml_schema):
        """Process schema elements and create function mappings."""
        target_dict = getattr(xml_schema.elements, 'target_dict',
                              getattr(xml_schema.elements, '_target_dict', None))
        if target_dict is None:
            print("WARNING: Could not access elements target_dict")
            return

        for element_name in target_dict:
            if "{http://www.w3.org/2001/XMLSchema}" in element_name:
                continue

            element_name_short = remove_namespace(element_name)
            element_type = remove_namespace(
                target_dict[element_name].type.name
            )

            if (element_name_short.lower() not in IGNORED_ELEMENT_NAMES and
                    element_type.lower() not in IGNORED_ELEMENT_TYPES):
                self.state.add_function_mapping(element_name_short, element_type)


# =============================================================================
# XSD Directory Processing
# =============================================================================

def _get_schema_group(filename: str) -> str:
    name = filename[:-4]  # strip .xsd
    prefix = "EEBus_SPINE_TS_"
    if name.startswith(prefix):
        name = name[len(prefix):]
    if name.endswith("_overview"):
        name = name[:-9]
    return name


def process_xsd_directory(
        input_dir: str,
        output_header_path: str,
        output_cpp_path: str,
        handling_type: str = "spine",
        code_header: str = "",
        code_implementation: str = ""
):
    """
    Process all XSD files in a directory and generate C++ code.

    Args:
        input_dir: Directory containing XSD files
        output_header_path: Path for the output header file
        output_cpp_path: Path for the output implementation file
        handling_type: Either "spine" or "ship"
        code_header: Header code to prepend to the header file
        code_implementation: Header code to prepend to the implementation file
    """
    # Reset global state
    _generator_state.reset()

    processor = SchemaProcessor(_generator_state)

    print("Loading schema...")
    for xsd_file in sorted(os.listdir(input_dir)):
        if not xsd_file.endswith(".xsd"):
            continue

        # Filter to whitelisted schema groups only
        group = _get_schema_group(xsd_file)
        if group not in WHITELISTED_SCHEMA_GROUPS:
            print(f"Skipping (not whitelisted): {xsd_file}")
            continue

        print(f"Processing: {xsd_file}")
        schema = xmlschema.XMLSchema(os.path.join(input_dir, xsd_file))
        if schema.version != "1.3.0":
            print("WARNING: Schema version is not 1.3.0. This might cause issues. The processing will continue.")
        processor.process_schema(schema)

    # Deduplicate and sort
    print("Deduplicating and sorting code...")
    print(f"Elements Before: {len(_generator_state.cpp_datatypes)}")
    print(f"Functions before: {len(_generator_state.type_function_mapping)}")

    _generator_state.cpp_datatypes = remove_duplicate_types(_generator_state.cpp_datatypes)
    _generator_state.type_function_mapping = remove_duplicate_function_mappings(
        _generator_state.type_function_mapping
    )

    # Filter function mappings to only whitelisted functions
    pre_filter_count = len(_generator_state.type_function_mapping)
    _generator_state.type_function_mapping = [
        mapping for mapping in _generator_state.type_function_mapping
        if mapping[0] in WHITELISTED_FUNCTIONS
    ]

    print(f"Elements After: {len(_generator_state.cpp_datatypes)}")
    print(f"Functions after dedup: {pre_filter_count}")
    print(f"Functions after whitelist: {len(_generator_state.type_function_mapping)}")

    # Prune types not reachable from handler functions or application code
    print("Pruning unreachable types...")
    _generator_state.cpp_datatypes = prune_unreachable_types(
        _generator_state.cpp_datatypes,
        _generator_state.type_function_mapping,
        ADDITIONAL_REQUIRED_TYPES,
    )

    print("Resolving Type Dependencies...")
    _generator_state.cpp_datatypes = sort_and_resolve_dependencies(
        _generator_state.cpp_datatypes,
        _generator_state.cpp_datatypes
    )

    # Generate data handler class for SPINE
    data_handler_header = ""
    data_handler_cpp = ""
    if handling_type == "spine":
        handler_gen = DataHandlerGenerator(_generator_state)
        data_handler_header, data_handler_cpp = handler_gen.generate()

    # Write output files
    _write_output_files(
        output_header_path,
        output_cpp_path,
        code_header,
        code_implementation,
        data_handler_header,
        data_handler_cpp
    )

    # Post-process for SHIP mode
    if handling_type == "ship":
        _postprocess_ship_files(output_header_path, output_cpp_path)

    # Report results
    _report_results(output_header_path, output_cpp_path, input_dir)


def _write_output_files(
        output_header_path: str,
        output_cpp_path: str,
        code_header: str,
        code_implementation: str,
        data_handler_header: str,
        data_handler_cpp: str
):
    """Write the generated code to output files."""
    print("Writing code to files...")

    with open(output_header_path, "w") as h, open(output_cpp_path, "w") as cpp:
        h.write(code_header)
        cpp.write(code_implementation)
        h.write("\n\n\n\n")

        for datatype in _generator_state.cpp_datatypes:
            h.write(datatype.code)
            cpp.write(datatype.to_json_code)
            cpp.write(datatype.from_json_code)

        h.write("\n\n\n\n")
        h.write(data_handler_header)
        cpp.write(data_handler_cpp)


def _postprocess_ship_files(output_header_path: str, output_cpp_path: str):
    """Post-process SHIP files to rename HeaderType to SpineHeaderType."""
    print("Running in SHIP Mode. Adapting code a bit")

    for file_path in [output_header_path, output_cpp_path]:
        with open(file_path, "r") as f:
            content = f.read()
        content = content.replace("HeaderType", "SpineHeaderType")
        with open(file_path, "w") as f:
            f.write(content)


def _report_results(output_header_path: str, output_cpp_path: str, input_dir: str):
    """Report the results of code generation."""
    with open(output_header_path, "r") as h, open(output_cpp_path, "r") as cpp:
        print(f"Wrote {len(h.readlines())} LOC to header ")
        print(f"Wrote {len(cpp.readlines())} LOC to cpp")

    print(f"Unprocessed elements: {_generator_state.unprocessed_elements}")
    print(f"Processed all schemas in {input_dir} and wrote to {output_header_path} and {output_cpp_path}")


# =============================================================================
# CLI Interface
# =============================================================================

def parse_arguments() -> argparse.Namespace:
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(
        description="Generate C++ code for SPINE and SHIP types from XSD files."
    )

    parser.add_argument(
        '--no-spine',
        action='store_true',
        help='Do not process SPINE XSD files. Defaults to processing SPINE files.'
    )
    parser.add_argument(
        '--spine-input',
        default='SPINE',
        help='Input directory for SPINE XSD files. Defaults to "SPINE".'
    )
    parser.add_argument(
        '--spine-output',
        default='spine_types',
        help=(
            'Output filename for SPINE C++ code (without file extension). '
            'Defaults to "spine_types", which generates "spine_types.h" and "spine_types.cpp".'
        )
    )
    # Commented out SHIP arguments for potential future use
    # parser.add_argument('--no-ship', action='store_true',
    #                     help='Do not process SHIP XSD files.')
    # parser.add_argument('--ship-input', default='SHIP',
    #                     help='Input directory for SHIP XSD files.')
    # parser.add_argument('--ship-output', default='ship_types_new',
    #                     help='Output filename for SHIP C++ code.')

    return parser.parse_args()


def main():
    """Main entry point for the generator."""
    args = parse_arguments()

    if not args.no_spine:
        print("Processing SPINE XSD files...")
        spine_path = args.spine_input
        output_header = args.spine_output + ".h"
        output_cpp = args.spine_output + ".cpp"

        # Update implementation header if custom output name is used
        implementation_header = CppTemplates.SPINE_IMPLEMENTATION.replace(
            "spine_types.h", output_header
        )

        process_xsd_directory(
            spine_path,
            output_header,
            output_cpp,
            handling_type="spine",
            code_header=CppTemplates.SPINE_HEADER,
            code_implementation=implementation_header
        )

        # Run clang-format on the generated files
        clang_format = shutil.which("clang-format")
        if clang_format:
            print("Running clang-format on generated files...")
            subprocess.run([clang_format, "-i", output_header, output_cpp], check=True)
        else:
            print("WARNING: clang-format not found, skipping formatting.")

        print("Done processing SPINE XSD files.")

    # SHIP processing (currently disabled)
    # if not args.no_ship:
    #     print("Processing SHIP XSD files...")
    #     ship_path = args.ship_input
    #     output_header = args.ship_output + ".h"
    #     output_cpp = args.ship_output + ".cpp"
    #     process_xsd_directory(
    #         ship_path, output_header, output_cpp,
    #         handling_type="ship",
    #         code_header=CppTemplates.SHIP_HEADER,
    #         code_implementation=CppTemplates.SHIP_IMPLEMENTATION
    #     )
    #     if args.no_spine:
    #         print("WARNING: SHIP code is generated without SPINE code. "
    #               "SHIP requires SPINE to function properly.")
    #     print("Done processing SHIP XSD files.")

    print("Done!")


if __name__ == "__main__":
    main()
