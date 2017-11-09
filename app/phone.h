/*
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

#ifndef PHONE_H
#define PHONE_H

#include <QSoundEffect>

#include <telephony.h>

class Phone : public QObject
{
	Q_OBJECT

	public:
		explicit Phone(Telephony *telephony, QObject *parent = Q_NULLPTR);
		void onCallStateChanged(QString);

	private:
		Telephony *m_telephony;
		QSoundEffect m_ringtone;
};

#endif // PHONE_H
