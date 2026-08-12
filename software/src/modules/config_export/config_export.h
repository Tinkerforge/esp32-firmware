/* esp32-firmware
 * Copyright (C) 2026 Erik Fleckstein <erik@tinkerforge.com>
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

#pragma once

#include "module.h"
#include "config.h"
#include "tools.h"

struct WebServerRequest;
struct WebServerRequestReturnProtect;

class ConfigExport final : public IModule
{
public:
    ConfigExport() {}
    void pre_setup() override;
    void register_urls() override;

    enum ExportType {
        JSON,
        Text,
        Binary
    };

    void allow_path(const char *path, ExportType type);

    void import();
    String import_json_update(const String &file, ConfigRoot *config);

private:
    struct Reg {
        const char *path;
        ExportType type;
    };

    CoolArray<Reg, 2> allowed_paths;

    enum class SanitizePathError {
        OK,
        IsDirectory,
        RealpathFailed,
        PathNotFound,
        FilenameTooLong
    };

    SanitizePathError sanitize_path(const char *path, size_t prefix_len, const Reg **out_reg, char *out_path);
    WebServerRequestReturnProtect receive_file(WebServerRequest &request, const char *import_path);
};
