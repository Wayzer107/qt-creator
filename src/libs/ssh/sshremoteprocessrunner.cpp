/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "sshremoteprocessrunner.h"

#include "sshconnectionmanager.h"

#include <utils/qtcassert.h>

/*!
    \class QSsh::SshRemoteProcessRunner

    \brief The SshRemoteProcessRunner class is a convenience class for
    running a remote process over an SSH connection.
*/

using namespace Utils;

namespace QSsh {
namespace Internal {
namespace {
enum State { Inactive, Connecting, Connected, ProcessRunning };
} // anonymous namespace

class SshRemoteProcessRunnerPrivate
{
public:
    SshRemoteProcessRunnerPrivate() : m_state(Inactive) {}

    SshRemoteProcessPtr m_process;
    SshConnection *m_connection;
    QString m_command;
    QString m_lastConnectionErrorString;
    QProcess::ExitStatus m_exitStatus;
    int m_exitCode;
    QString m_errorString;
    State m_state;
};

} // namespace Internal

using namespace Internal;

SshRemoteProcessRunner::SshRemoteProcessRunner(QObject *parent)
    : QObject(parent), d(new SshRemoteProcessRunnerPrivate)
{
}

SshRemoteProcessRunner::~SshRemoteProcessRunner()
{
    disconnect();
    setState(Inactive);
    delete d;
}

void SshRemoteProcessRunner::run(const QString &command, const SshConnectionParameters &sshParams)
{
    QTC_ASSERT(d->m_state == Inactive, return);

    runInternal(command, sshParams);
}

void SshRemoteProcessRunner::runInternal(const QString &command,
    const SshConnectionParameters &sshParams)
{
    setState(Connecting);

    d->m_lastConnectionErrorString.clear();
    d->m_errorString.clear();
    d->m_exitCode = -1;
    d->m_command = command;
    d->m_connection = SshConnectionManager::acquireConnection(sshParams);
    connect(d->m_connection, &SshConnection::errorOccurred,
            this, &SshRemoteProcessRunner::handleConnectionError);
    connect(d->m_connection, &SshConnection::disconnected,
            this, &SshRemoteProcessRunner::handleDisconnected);
    if (d->m_connection->state() == SshConnection::Connected) {
        handleConnected();
    } else {
        connect(d->m_connection, &SshConnection::connected, this, &SshRemoteProcessRunner::handleConnected);
        if (d->m_connection->state() == SshConnection::Unconnected)
            d->m_connection->connectToHost();
    }
}

void SshRemoteProcessRunner::handleConnected()
{
    QTC_ASSERT(d->m_state == Connecting, return);
    setState(Connected);

    d->m_process = d->m_connection->createRemoteProcess(d->m_command);
    connect(d->m_process.get(), &QtcProcess::started,
            this, &SshRemoteProcessRunner::handleProcessStarted);
    connect(d->m_process.get(), &QtcProcess::done,
            this, &SshRemoteProcessRunner::handleProcessFinished);
    connect(d->m_process.get(), &QtcProcess::readyReadStandardOutput,
            this, &SshRemoteProcessRunner::readyReadStandardOutput);
    connect(d->m_process.get(), &QtcProcess::readyReadStandardError,
            this, &SshRemoteProcessRunner::readyReadStandardError);
    d->m_process->start();
}

void SshRemoteProcessRunner::handleConnectionError()
{
    d->m_lastConnectionErrorString = d->m_connection->errorString();
    handleDisconnected();
    emit connectionError();
}

void SshRemoteProcessRunner::handleDisconnected()
{
    QTC_ASSERT(d->m_state == Connecting || d->m_state == Connected || d->m_state == ProcessRunning,
               return);
    setState(Inactive);
}

void SshRemoteProcessRunner::handleProcessStarted()
{
    QTC_ASSERT(d->m_state == Connected, return);

    setState(ProcessRunning);
    emit started();
}

void SshRemoteProcessRunner::handleProcessFinished()
{
    d->m_exitStatus = d->m_process->exitStatus();
    d->m_exitCode = d->m_process->exitCode();
    d->m_errorString = d->m_process->errorString();
    setState(Inactive);
    emit finished();
}

void SshRemoteProcessRunner::setState(int newState)
{
    if (d->m_state == newState)
        return;

    d->m_state = static_cast<State>(newState);
    if (d->m_state == Inactive) {
        if (d->m_process)
            d->m_process.release()->deleteLater();
        if (d->m_connection) {
            disconnect(d->m_connection, nullptr, this, nullptr);
            SshConnectionManager::releaseConnection(d->m_connection);
            d->m_connection = nullptr;
        }
    }
}

QString SshRemoteProcessRunner::lastConnectionErrorString() const {
    return d->m_lastConnectionErrorString;
}

bool SshRemoteProcessRunner::isRunning() const
{
    return d->m_process && d->m_process->isRunning();
}

QProcess::ExitStatus SshRemoteProcessRunner::exitStatus() const
{
    QTC_CHECK(!isRunning());
    return d->m_exitStatus;
}

int SshRemoteProcessRunner::exitCode() const
{
    QTC_CHECK(exitStatus() == QProcess::NormalExit);
    return d->m_exitCode;
}

QString SshRemoteProcessRunner::errorString() const
{
    return d->m_errorString;
}

QByteArray SshRemoteProcessRunner::readAllStandardOutput()
{
    return d->m_process.get() ? d->m_process->readAllStandardOutput() : QByteArray();
}

QByteArray SshRemoteProcessRunner::readAllStandardError()
{
    return d->m_process.get() ? d->m_process->readAllStandardError() : QByteArray();
}

void SshRemoteProcessRunner::cancel()
{
    setState(Inactive);
}

} // namespace QSsh
