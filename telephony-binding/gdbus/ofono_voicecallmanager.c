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

#include <string.h>

#define _GNU_SOURCE
#include <afb/afb-binding.h>

#include "ofono_voicecallmanager_interface.h"

static const struct afb_binding_interface *interface;

static void call_added(OrgOfonoVoiceCallManager *manager,
		       gchar *op,
		       GVariant *properties)
{
	GVariantIter *iter;
	gchar *key;
	GVariant *value;
	const gchar *state, *clip = NULL;

	g_variant_get(properties, "a{sv}", &iter);
	while (g_variant_iter_loop(iter, "{sv}", &key, &value)) {
		if (!strcmp(key, "State"))
			state = g_variant_get_string(value, NULL);
		else if (!strcmp(key, "LineIdentification")) {
			clip = g_variant_get_string(value, NULL);
		}
	}

	if (!strcmp(state, "incoming")) {
		g_signal_emit_by_name(manager, "incoming-call", op, clip ? clip : "");
	}
}

const OrgOfonoVoiceCallManager
*ofono_voicecallmanager_init(const struct afb_binding_interface *iface,
			     const gchar *op,
			     void (*incoming_call)(OrgOfonoVoiceCallManager *manager,gchar *))
{
	interface = iface;

	OrgOfonoVoiceCallManager *manager = org_ofono_voice_call_manager_proxy_new_for_bus_sync(
			G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
			"org.ofono", op, NULL, NULL);

	g_signal_new("incoming-call",
		     G_TYPE_OBJECT,
		     G_SIGNAL_RUN_LAST,
		     0,
		     NULL,
		     NULL,
		     g_cclosure_marshal_generic,
		     G_TYPE_NONE,
		     2,
		     G_TYPE_STRING,
		     G_TYPE_STRING);

	if (g_signal_connect(G_OBJECT(manager), "incoming-call", G_CALLBACK(incoming_call), NULL) <= 0) {
		ERROR(interface, "Failed to connect to signal call-added\n");
	}

	if (g_signal_connect(manager, "call-added", G_CALLBACK(call_added), NULL) <= 0) {
		ERROR(interface, "Failed to connect to signal call-added\n");
	}

	return manager;
}

gchar *ofono_voicecallmanager_dial(OrgOfonoVoiceCallManager *manager,
				   const gchar *number,
				   const gchar *cid)
{
	gchar *out = NULL;
	GError *error = NULL;

	if (!manager) {
		ERROR(interface, "Ofono VoiceCallmanager uninitialized\n");
		return NULL;
	}

	org_ofono_voice_call_manager_call_dial_sync(manager, number, cid, &out, NULL, &error);
	if (error != NULL)
		out = NULL;

	return out;
}

void ofono_voicecallmanager_hangup_all(OrgOfonoVoiceCallManager *manager)
{
	GError *error = NULL;

	if (!manager) {
		ERROR(interface, "Ofono VoiceCallmanager uninitialized\n");
		return;
	}

	org_ofono_voice_call_manager_call_hangup_all_sync(manager, NULL, &error);
}
