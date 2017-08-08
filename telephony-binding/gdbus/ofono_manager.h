/*
 * Copyright (C) 2017 Konsulko Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <glib.h>

#define _GNU_SOURCE
#include <afb/afb-binding.h>
#include <afb/afb-service-itf.h>

int ofono_manager_init(void);
const gchar *ofono_manager_get_default_modem_path(void);
const gchar *ofono_manager_get_default_modem_name(void);
const gchar *ofono_manager_get_default_modem_type(void);
gboolean ofono_manager_get_default_modem_powered(void);
gboolean ofono_manager_get_default_modem_online(void);
