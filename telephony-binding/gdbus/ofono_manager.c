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

#define _GNU_SOURCE

#include <string.h>

#include <afb/afb-binding.h>

#include "ofono_manager.h"
#include "ofono_manager_interface.h"

struct ofono_manager_modem
{
	const gchar *path;
	const gchar *name;
	const gchar *type;
	gboolean powered;
	gboolean online;
};

static OrgOfonoManager *manager;
static struct ofono_manager_modem default_modem;
static const struct afb_binding_interface *interface;

void ofono_manager_init(const struct afb_binding_interface *iface)
{
	GVariant *out_arg = NULL, *next, *value;
	GError *error = NULL;
	gchar *path, *key;
	GVariantIter *iter, *iter2;

	interface = iface;

	if (manager) {
		ERROR(interface, "Ofono Manager already initialized\n");
		return;
	}

	manager = org_ofono_manager_proxy_new_for_bus_sync(
		G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
		"org.ofono", "/", NULL, NULL);

	if (!manager) {
		ERROR(interface, "Ofono Manager not initialized\n");
		return;
	}

	org_ofono_manager_call_get_modems_sync(manager, &out_arg, NULL, &error);
	if (error == NULL) {
		g_variant_get(out_arg, "a(oa{sv})", &iter);
		next = g_variant_iter_next_value(iter);
		g_variant_get(next, "(oa{sv})", &path, &iter2);
		default_modem.path = path;
		while (g_variant_iter_loop(iter2, "{sv}", &key, &value)) {
			if (!strcmp(key, "Name"))
				default_modem.name = g_variant_get_string(value, NULL);
			else if (!strcmp(key, "Type"))
				default_modem.type = g_variant_get_string(value, NULL);
			else if (!strcmp(key, "Powered"))
				default_modem.powered = g_variant_get_boolean(value);
			else if (!strcmp(key, "Online"))
				default_modem.online = g_variant_get_boolean(value);
		}
	}
}


const gchar *ofono_manager_get_default_modem_path(void)
{
	if (!manager) {
		ERROR(interface, "Ofono Manager not initialized\n");
	}

	return default_modem.path;
}

const gchar *ofono_manager_get_default_modem_name(void)
{
	if (!manager) {
		ERROR(interface, "Ofono Manager not initialized\n");
	}

	return default_modem.name;
}

const gchar *ofono_manager_get_default_modem_type(void)
{
	if (!manager) {
		ERROR(interface, "Ofono Manager not initialized\n");
	}

	return default_modem.type;
}

gboolean ofono_manager_get_default_modem_powered(void)
{
	if (!manager) {
		ERROR(interface, "Ofono Manager not initialized\n");
	}

	return default_modem.powered;
}

gboolean ofono_manager_get_default_modem_online(void)
{
	if (!manager) {
		ERROR(interface, "Ofono Manager not initialized\n");
	}

	return default_modem.online;
}
