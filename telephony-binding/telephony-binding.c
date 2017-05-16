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

#include <glib.h>
#include <json-c/json.h>
#include <string.h>

#include <afb/afb-binding.h>
#include <afb/afb-service-itf.h>

#include "ofono_manager.h"
#include "ofono_voicecallmanager.h"
#include "ofono_voicecall.h"

static const struct afb_binding_interface *interface;

static OrgOfonoVoiceCallManager *vcm;
static gchar *incoming_call;
static gchar *voice_call;

static void dial(struct afb_req request)
{
	struct json_object *query, *val, *ret;
	const char *number;

	query = afb_req_json(request);
	json_object_object_get_ex(query, "value", &val);
	if (json_object_is_type(val, json_type_string)) {
		number = json_object_get_string(val);
		if (voice_call) {
			ERROR(interface, "dial: cannot dial with active call");
			afb_req_fail(request, "active call", NULL);
		} else {
			DEBUG(interface, "dial: %s...\n", number);
			voice_call = ofono_voicecallmanager_dial(vcm, (gchar *)number, "");
			DEBUG(interface, "voice_call: %s\n", voice_call);
			if (voice_call) {
				ret = json_object_new_object();
				json_object_object_add(ret, "VoiceCall", json_object_new_string(voice_call));
				afb_req_success(request, ret, NULL);
			} else {
				ERROR(interface, "dial: failed to dial number\n");
				afb_req_fail(request, "failed dial", NULL);
			}
		}
	} else {
		ERROR(interface, "dial: no phone number parameter\n");
		afb_req_fail(request, "no number", NULL);
	}
}

static void hangup(struct afb_req request)
{
	if (voice_call) {
		DEBUG(interface, "Hangup voice call: %s\n", voice_call);
		ofono_hangup(voice_call);
		afb_req_success(request, NULL, NULL);
		voice_call = NULL;
	} else {
		ERROR(interface, "Hangup: no active call");
		afb_req_fail(request, "failed hangup", NULL);
	}
}


static void incoming_call_cb(OrgOfonoVoiceCallManager *manager, gchar *op, gchar *clip) {
	WARNING(interface, "Incoming call from %s\n", clip);
	incoming_call = strdup(op);
}

static void *main_loop_thread(void *unused)
{
	GMainLoop *loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);
	return NULL;
}

static int ofono_init(void)
{
	pthread_t tid;
	int ret = 0;

	/* Start the main loop thread */
	pthread_create(&tid, NULL, main_loop_thread, NULL);

	ofono_manager_init(interface);
	const gchar *modem_path = ofono_manager_get_default_modem_path();
	DEBUG(interface, "modem_path: %s\n", modem_path);
	vcm = ofono_voicecallmanager_init(interface, modem_path, incoming_call_cb);
	if (!vcm) {
		ERROR(interface, "Failed to initialize voice call manager\n");
		ret = -1;
	}

	return ret;
}

static const struct afb_verb_desc_v1 verbs[]= {
	{
		.name		= "dial",
		.session	= AFB_SESSION_NONE,
		.callback	= dial,
		.info		= "Dial phone"
	},
	{
		.name		= "hangup",
		.session	= AFB_SESSION_NONE,
		.callback	= hangup,
		.info		= "Hangup phone"
	},
	{NULL}
};

static const struct afb_binding binding_desc = {
	.type = AFB_BINDING_VERSION_1,
	.v1 = {
		.info = "telephony service",
		.prefix = "telephony",
		.verbs = verbs
	}
};

const struct afb_binding *afbBindingV1Register (const struct afb_binding_interface *itf)
{
	interface = itf;

	return &binding_desc;
}

int afbBindingV1ServiceInit(struct afb_service service)
{
	DEBUG(interface, "Initializing telephony service\n");

	return ofono_init();
}