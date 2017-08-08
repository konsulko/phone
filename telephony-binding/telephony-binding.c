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

#define AFB_BINDING_VERSION 2
#include <afb/afb-binding.h>
#include <afb/afb-service-itf.h>

#include "ofono_manager.h"
#include "ofono_voicecallmanager.h"
#include "ofono_voicecall.h"

static OrgOfonoVoiceCallManager *vcm;
static OrgOfonoVoiceCall *incoming_call, *voice_call;
static struct afb_event call_state_changed_event;
static struct afb_event dialing_call_event;
static struct afb_event incoming_call_event;
static struct afb_event terminated_call_event;

static void dial(struct afb_req request)
{
	struct json_object *query, *val;
	const char *number;

	query = afb_req_json(request);
	json_object_object_get_ex(query, "value", &val);
	if (json_object_is_type(val, json_type_string)) {
		number = json_object_get_string(val);
		if (voice_call) {
			AFB_ERROR("dial: cannot dial with active call");
			afb_req_fail(request, "active call", NULL);
		} else {
			AFB_DEBUG("dial: %s...\n", number);
			if (ofono_voicecallmanager_dial(vcm, (gchar *)number, "")) {
				afb_req_success(request, NULL, NULL);
			} else {
				AFB_ERROR("dial: failed to dial number\n");
				afb_req_fail(request, "failed dial", NULL);
			}
		}
	} else {
		AFB_ERROR("dial: no phone number parameter\n");
		afb_req_fail(request, "no number", NULL);
	}
}

static void hangup(struct afb_req request)
{
	if (voice_call) {
		AFB_DEBUG("Hangup voice call\n");
		ofono_voicecall_hangup(voice_call);
		afb_req_success(request, NULL, NULL);
	} else if (incoming_call) {
		AFB_DEBUG("Reject incoming call\n");
		ofono_voicecall_hangup(incoming_call);
		afb_req_success(request, NULL, NULL);
	} else {
		AFB_ERROR("Hangup: no active call");
		afb_req_fail(request, "failed hangup", NULL);
	}
}

static void answer(struct afb_req request)
{
	if (incoming_call) {
		AFB_DEBUG("Answer voice call\n");
		voice_call = incoming_call;
		ofono_voicecall_answer(voice_call);
	} else {
		AFB_ERROR("Answer: no incoming call");
	}
}

static void subscribe(struct afb_req request)
{
	const char *value = afb_req_value(request, "value");
	if(value) {
		if (!strcasecmp(value, "callStateChanged")) {
			afb_req_subscribe(request, call_state_changed_event);
		} else if (!strcasecmp(value, "dialingCall")) {
			afb_req_subscribe(request, dialing_call_event);
		} else if (!strcasecmp(value, "incomingCall")) {
			afb_req_subscribe(request, incoming_call_event);
		} else if (!strcasecmp(value, "terminatedCall")) {
			afb_req_subscribe(request, terminated_call_event);
		} else {
			afb_req_fail(request, "failed", "Invalid event");
			return;
		}
	}

	afb_req_success(request, NULL, NULL);
}

static void unsubscribe(struct afb_req request)
{
	const char *value = afb_req_value(request, "value");
	if(value) {
		if (!strcasecmp(value, "callStateChanged")) {
			afb_req_unsubscribe(request, call_state_changed_event);
		} else if (!strcasecmp(value, "dialingCall")) {
			afb_req_unsubscribe(request, dialing_call_event);
		} else if (!strcasecmp(value, "incomingCall")) {
			afb_req_unsubscribe(request, incoming_call_event);
		} else if (!strcasecmp(value, "terminatedCall")) {
			afb_req_unsubscribe(request, terminated_call_event);
		} else {
			afb_req_fail(request, "failed", "Invalid event");
			return;
		}
	}

	afb_req_success(request, NULL, NULL);
}

static void call_state_changed_cb(OrgOfonoVoiceCall *vc, gchar *state)
{
	struct json_object *call_state;
	call_state = json_object_new_object();
	json_object_object_add(call_state, "state", json_object_new_string(state));
	afb_event_push(call_state_changed_event, call_state);
}

static void incoming_call_cb(OrgOfonoVoiceCallManager *manager, gchar *op, gchar *clip)
{
	struct json_object *call_info;

	call_info = json_object_new_object();
	json_object_object_add(call_info, "clip", json_object_new_string(clip));
	afb_event_push(incoming_call_event, call_info);
	incoming_call = ofono_voicecall_new(op, call_state_changed_cb);
}

static void dialing_call_cb(OrgOfonoVoiceCallManager *manager, gchar *op, gchar *colp)
{
	struct json_object *call_info;

	call_info = json_object_new_object();
	json_object_object_add(call_info, "colp", json_object_new_string(colp));
	afb_event_push(dialing_call_event, call_info);
	voice_call = ofono_voicecall_new(op, call_state_changed_cb);
}

static void terminated_call_cb(OrgOfonoVoiceCallManager *manager, gchar *op)
{
	if (incoming_call) {
		ofono_voicecall_free(incoming_call);
		incoming_call = NULL;
	} else if (voice_call) {
		ofono_voicecall_free(voice_call);
	}
	voice_call = NULL;
	afb_event_push(terminated_call_event, NULL);
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

	call_state_changed_event = afb_daemon_make_event("callStateChanged");
	dialing_call_event = afb_daemon_make_event("dialingCall");
	incoming_call_event = afb_daemon_make_event("incomingCall");
	terminated_call_event = afb_daemon_make_event("terminatedCall");

	/* Start the main loop thread */
	pthread_create(&tid, NULL, main_loop_thread, NULL);

	ret = ofono_manager_init();
	if (ret == 0) {
		const gchar *modem_path = ofono_manager_get_default_modem_path();
		if (modem_path) {
			AFB_DEBUG("modem_path: %s\n", modem_path);
			vcm = ofono_voicecallmanager_init(modem_path,
					incoming_call_cb,
					dialing_call_cb,
					terminated_call_cb);
			if (!vcm) {
				AFB_ERROR("failed to initialize voice call manager\n");
				ret = -1;
			}
		} else {
			AFB_ERROR("default modem not set\n");
			ret = -1;
		}
	} else {
		AFB_ERROR("failed to initialize ofono manager: " \
				 "HFP device not connected or Bluetooth disabled\n");
	}

	return ret;
}

static const struct afb_verb_v2 verbs[]= {
	{
		.verb		= "dial",
		.callback	= dial,
		.auth		= NULL,
		.session	= AFB_SESSION_NONE,
	},
	{
		.verb		= "hangup",
		.callback	= hangup,
		.auth		= NULL,
		.session	= AFB_SESSION_NONE,
	},
	{
		.verb		= "answer",
		.callback	= answer,
		.auth		= NULL,
		.session	= AFB_SESSION_NONE,
	},
	{
		.verb		= "subscribe",
		.callback	= subscribe,
		.auth		= NULL,
		.session	= AFB_SESSION_NONE,
	},
	{
		.verb		= "unsubscribe",
		.callback	= unsubscribe,
		.auth		= NULL,
		.session	= AFB_SESSION_NONE,
	},
	{NULL}
};

static int preinit()
{
	AFB_NOTICE("Pre-init telephony service");

	return 0;
}

static int init()
{
	AFB_NOTICE("Initializing telephony service");

	return ofono_init();
}

const struct afb_binding_v2 afbBindingV2 = {
	.api = "telephony",
	.specification = NULL,
	.verbs = verbs,
	.preinit = preinit,
	.init = init,
};
