/*
 * Copyright (C) 2018 Burak Akguel, Christian Gesse, Fabian Ruhland, Filip Krakowski, Michael Schoettner
 * Heinrich-Heine University
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef HHUOS_SERIALSERVICE_H
#define HHUOS_SERIALSERVICE_H

#include <kernel/KernelService.h>
#include <lib/String.h>
#include <devices/ports/Port.h>
#include <lib/util/HashMap.h>

class PortService : public KernelService {

private:

    Util::HashMap<String, Port*> portMap;

public:

    PortService() = default;

    void registerPort(Port *port);

    Port *getPort(const String &name);

    bool isPortAvailable(const String &name);

    static constexpr const char* SERVICE_NAME = "PortService";
};

#endif