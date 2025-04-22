

import json

import xmlschema
import os






class Spine_type:
    #@brief: Type_name: either "define", "using", "enum", "struct" or "class"
    type_name = ""
    depends_on = []
    name = ""
    code = ""
    to_json_code = ""
    from_json_code = ""
    upper_limit = None
    lower_limit = None
    #this is technically required but we dont check it
    regex_restriction = None

    def __init__(self, type_name, name, code):
        self.type_name = type_name
        self.name = name
        self.code = code
        self.depends_on = []

cpp_datatypes : list[Spine_type] = []


def remove_namespace(name):
    if name is not None:
        if "}" in name:
            return name.split("}", 1)[1]  # Entfernt den Namespace-URI
    else:
        return "None"
    return name

def to_cpp_datatype(type_name):
    type_name = str(type_name.__name__)
    type_mapping = {
        "string": "std::string",
        "str": "std::string",
        "int": "int",
        "float": "float",
        "double": "double",
        "boolean": "bool",
        "bool": "bool",
        "Duration": "uint64_t",
        "DateTime10": "uint64_t",
    }
    if type_name not in type_mapping:
        print("Unknown datatype:", type_name)
    return type_mapping.get(type_name, "Null")

def remove_duplicate_objects(spine_types: list[Spine_type]):
    seen = set()
    result = []
    for spine_type in spine_types:
        if spine_type.name not in seen:
            seen.add(spine_type.name)
            result.append(spine_type)
    return result

def sort_and_resolve_dependencies(spine_types: list[Spine_type]):
    type_order = {"define": 0, "using": 1, "enum": 2, "struct": 3, "class": 4}

    # Erstelle eine Abhängigkeits-Map
    dependency_map = {datatype.name: datatype.depends_on for datatype in cpp_datatypes}

    # Topologische Sortierung
    sorted_datatypes = []
    visited = set()

    def visit(datatype):
        if datatype.name in visited:
            return
        visited.add(datatype.name)
        for dependency in dependency_map.get(datatype.name, []):
            dependent_datatype = next((d for d in cpp_datatypes if d.name == dependency), None)
            if dependent_datatype:
                visit(dependent_datatype)
        sorted_datatypes.append(datatype)

    # Sortiere zuerst nach `type_order`, dann nach Abhängigkeiten
    cpp_datatypes.sort(key=lambda x: type_order.get(x.type_name, 5))
    for datatype in cpp_datatypes:
        visit(datatype)

    return sorted_datatypes

def make_variable_name(name: str):
    forbidden_names = ["switch", "auto", "SAR"]
    #Cleanup the variable so it can be used in C++
    name = ''.join(char if char.isalnum() else '_' for char in name)
    # Stelle sicher, dass der Name mit einem Buchstaben beginnt
    if not name[0].isalpha() or name[0] in forbidden_names:
        name = 'var_' + name
    return name

def process_complex_type(complex_type):
    if hasattr(complex_type.content, 'content'):
        new_type = Spine_type("struct", remove_namespace(complex_type.name), "")
        new_type.code = "struct " + remove_namespace(complex_type.name) + " {\n"
        for member in complex_type.content.content:
            if hasattr(member, 'type'):
                new_type.code += f"    {remove_namespace(member.type.name)} {member.local_name};\n"
            elif member.model == "sequence":
                if len(member.content) != 1:
                    pass

            else:
                print(f"Member {member} of type {complex_type.name} has no type")
        new_type.code += "};"
        pass
    else:
        #print(complex_type.name)
        pass
    # print("Generating code for complex type", remove_namespace(complex_type.name))

def process_schema(xml_schema):
    unprocessed_elements = 0
    print("Generating code...")
    for simple_type in xml_schema.simple_types:

        if simple_type.derivation == "restriction":
            if simple_type.enumeration is not None:
                new_type = Spine_type("enum", remove_namespace(simple_type.name), "")
                new_type.code = "enum " + remove_namespace(simple_type.name) + " {\n"

                for enumeration in simple_type.enumeration:
                    new_type.code += "\t" + make_variable_name(str(enumeration)) + ",\n"
                new_type.code += "};\n"
                cpp_datatypes.append(new_type)
            elif hasattr(simple_type, 'base_type'):
                new_type = Spine_type("using", remove_namespace(simple_type.name), "")
                new_type.code = "using " + remove_namespace(simple_type.name) + " = " + to_cpp_datatype(simple_type.base_type.python_type) + ";\n"
                found_restriction = False
                if simple_type.base_type.max_value is not None:
                    new_type.max_value = simple_type.base_type.max_value
                    pass
                if simple_type.base_type.min_value is not None:
                    found_restriction = True
                    new_type.min_value = simple_type.base_type.min_value
                    found_restriction = True
                if to_cpp_datatype(simple_type.base_type.python_type) == "std::string":
                    found_restriction = True
                if hasattr(simple_type.base_type.python_type, 'name'):
                    if simple_type.base_type.python_type.name == "duration":
                        found_restriction = True
                if not found_restriction:
                    print("Unknown restriction to basetype", simple_type.name)
                cpp_datatypes.append(new_type)
                pass
            else:
                unprocessed_elements += 1
                print("simple type: " + simple_type.name + " has restriction but no enumeration")
        else:
            if hasattr(simple_type, 'member_types'):
                new_type = Spine_type("struct", remove_namespace(simple_type.name), "")
                new_type.code = "struct " + remove_namespace(simple_type.name) + " {\n"
                for member_type in simple_type.member_types:
                    if member_type.derivation == "restriction":
                        new_type.code += "\t" + remove_namespace(member_type.name) + " " + str(member_type.local_name).lower()  +";\n"
                    else:
                        new_type.code += "\t" + to_cpp_datatype(member_type.python_type) + " " + member_type.local_name  +";\n"
                new_type.code += "};\n"
                cpp_datatypes.append(new_type)
            else:
                print("No restriction to basetype", simple_type.name)
                unprocessed_elements += 1
    print("not processed "+ str(unprocessed_elements) + " of " + str(len(schema.simple_types)) + " simple types")
    for complex_type in xml_schema.complex_types:
        process_complex_type(complex_type)

    for element in xml_schema.elements:
        pass
        #print("Generating code for element", remove_namespace(element))



print("Loading schema...")

schema_path = os.path.join("SPINE", "EEBus_SPINE_TS_NodeManagement.xsd")
# Schema laden
#schema = xmlschema.XMLSchema(schema_path)
#process_schema(schema)

spine_path = "SPINE"

for xsd in os.listdir(spine_path):
    if xsd.endswith(".xsd"):
        print("Found xsd file: " + xsd)
        schema = xmlschema.XMLSchema(os.path.join(spine_path, xsd))
        process_schema(schema)



print("Deduplicating and sorting code...")
print("Before: ", len(cpp_datatypes))
cpp_datatypes = remove_duplicate_objects(cpp_datatypes)
cpp_datatypes = sort_and_resolve_dependencies(cpp_datatypes)
print("After: ", len(cpp_datatypes))
with open("spine_types.h", "w") as f:
    f.write("// This file is generated by pre_generator.py\n")
    f.write("#include \"module.h\"\n")
    f.write("#include \"config.h\"\n")
    #include "config.h"
    for datatype in cpp_datatypes:
        f.write(datatype.code)



print("Done!")
