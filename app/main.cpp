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

#include <QtCore/QDebug>
#include <QtCore/QCommandLineParser>
#include <QtCore/QUrlQuery>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQuickControls2/QQuickStyle>

#ifdef HAVE_LIBHOMESCREEN
#include <libhomescreen.hpp>
#endif

#include <telephony.h>

int main(int argc, char *argv[])
{
#ifdef HAVE_LIBHOMESCREEN
    LibHomeScreen libHomeScreen;

    if (!libHomeScreen.renderAppToAreaAllowed(0, 1)) {
        qWarning() << "renderAppToAreaAllowed is denied";
        return -1;
    }
#endif

    QGuiApplication app(argc, argv);

    QQuickStyle::setStyle("AGL");

    QQmlApplicationEngine engine;

    QCommandLineParser parser;
    parser.addPositionalArgument("port", app.translate("main", "port for binding"));
    parser.addPositionalArgument("secret", app.translate("main", "secret for binding"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);
    QStringList positionalArguments = parser.positionalArguments();
    if (positionalArguments.length() == 2) {
	int port = positionalArguments.takeFirst().toInt();
	QString secret = positionalArguments.takeFirst();
	QUrl bindingAddress;
	bindingAddress.setScheme(QStringLiteral("ws"));
	bindingAddress.setHost(QStringLiteral("localhost"));
	bindingAddress.setPort(port);
	bindingAddress.setPath(QStringLiteral("/api"));
	QUrlQuery query;
	query.addQueryItem(QStringLiteral("token"), secret);
	bindingAddress.setQuery(query);
	QQmlContext *context = engine.rootContext();
	context->setContextProperty(QStringLiteral("bindingAddress"), bindingAddress);
	Telephony *telephony = new Telephony(bindingAddress);
	context->setContextProperty("telephony", telephony);
    }

    engine.load(QUrl(QStringLiteral("qrc:/Phone.qml")));

    return app.exec();
}

