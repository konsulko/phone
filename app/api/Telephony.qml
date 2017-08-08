/*
 * Copyright (C) 2016 The Qt Company Ltd.
 * Copyright (C) 2017 Konsulko Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import QtQuick 2.6
import QtWebSockets 1.0

WebSocket {
    id: root
    active: true
    url: bindingAddress

    property string statusString: "waiting..."
    property string apiString: "telephony"
    property var verbs: []
    property string payloadLength: "9999"

    readonly property var msgid: {
        "call": 2,
        "retok": 3,
        "reterr": 4,
        "event": 5
    }

    onTextMessageReceived: {
        var json = JSON.parse(message)
        var request = json[2].request
        var response = json[2].response
        switch (json[0]) {
            case msgid.call:
                break
            case msgid.retok:
                root.statusString = request.status
                var verb = verbs.shift()
                if (verb == "dial") {
                    console.debug("Dial response")
                } else if (verb == "hangup") {
                    console.debug("Hangup response")
                }
                break
            case msgid.reterr:
                root.statusString = "Bad return value, binding probably not installed"
                break
            case msgid.event:
                var payload = JSON.parse(JSON.stringify(json[2]))
                var event = payload.event
                var data = payload.data
                console.debug("event: " + event)
                if (event == "telephony/incomingCall") {
                    callClipColp = data.clip
                    callStatus = "incoming"
                } else if (event == "telephony/dialingCall") {
                    callClipColp = data.colp
                    callStatus = "dialing"
                } else if (event == "telephony/terminatedCall") {
                    callStatus = "disconnected"
                } else if (event == "telephony/callStateChanged") {
                    callStatus = data.state
                }
                break
        }
    }

    onStatusChanged: {
        switch (status) {
            case WebSocket.Open:
                sendSocketMessage("subscribe", { value: "callStateChanged" })
                sendSocketMessage("subscribe", { value: "dialingCall" })
                sendSocketMessage("subscribe", { value: "incomingCall" })
                sendSocketMessage("subscribe", { value: "terminatedCall" })
                break
            case WebSocket.Error:
                root.statusString = "WebSocket error: " + root.errorString
                break
        }
    }

    function sendSocketMessage(verb, parameter) {
        var requestJson = [ msgid.call, payloadLength, apiString + '/' + verb, parameter ]
        verbs.push(verb)
        sendTextMessage(JSON.stringify(requestJson))
    }

    function dial(number) {
        var parameterJson = { value: number }
        sendSocketMessage("dial", parameterJson)
    }

    function answer() {
        var parameterJson = 'None'
        sendSocketMessage("answer", parameterJson)
    }

    function hangup() {
        var parameterJson = 'None'
        sendSocketMessage("hangup", parameterJson)
    }
}
