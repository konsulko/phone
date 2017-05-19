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

#include "ofono_voicecall_interface.h"

OrgOfonoVoiceCall *ofono_voicecall_new(gchar *op)
{
	return org_ofono_voice_call_proxy_new_for_bus_sync(
		G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
		"org.ofono", op, NULL, NULL);
}

void ofono_voicecall_free(OrgOfonoVoiceCall *voice_call)
{
	g_object_unref(G_OBJECT(voice_call));
}

void ofono_voicecall_hangup(OrgOfonoVoiceCall *voice_call)
{
	GError *error = NULL;

	org_ofono_voice_call_call_hangup_sync(voice_call, NULL, &error);
}
