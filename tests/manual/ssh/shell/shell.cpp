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

#include "shell.h"

#include <ssh/sshremoteprocess.h>

#include <QCoreApplication>
#include <QFile>
#include <QSocketNotifier>

#include <cstdlib>
#include <iostream>

using namespace QSsh;
using namespace Utils;

Shell::Shell(const SshConnectionParameters &parameters, QObject *parent)
    : QObject(parent),
      m_connection(new SshConnection(parameters)),
      m_stdin(new QFile(this))
{
    connect(m_connection, &SshConnection::connected, this, &Shell::handleConnected);
    connect(m_connection, &SshConnection::errorOccurred, this, &Shell::handleConnectionError);
}

Shell::~Shell()
{
    delete m_connection;
}

void Shell::run()
{
    if (!m_stdin->open(stdin, QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        std::cerr << "Error: Cannot read from standard input." << std::endl;
        QCoreApplication::exit(EXIT_FAILURE);
        return;
    }

    m_connection->connectToHost();
}

void Shell::handleConnectionError()
{
    std::cerr << "SSH connection error: " << qPrintable(m_connection->errorString()) << std::endl;
    QCoreApplication::exit(EXIT_FAILURE);
}

void Shell::handleConnected()
{
    m_shell = m_connection->createRemoteShell();
    connect(m_shell.get(), &QtcProcess::started, this, &Shell::handleShellStarted);
    connect(m_shell.get(), &QtcProcess::readyReadStandardOutput,
            this, &Shell::handleRemoteStdout);
    connect(m_shell.get(), &QtcProcess::readyReadStandardError,
            this, &Shell::handleRemoteStderr);
    connect(m_shell.get(), &QtcProcess::done, this, &Shell::handleChannelClosed);
    m_shell->start();
}

void Shell::handleShellStarted()
{
    QSocketNotifier * const notifier = new QSocketNotifier(0, QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &Shell::handleStdin);
}

void Shell::handleRemoteStdout()
{
    std::cout << m_shell->readAllStandardOutput().data() << std::flush;
}

void Shell::handleRemoteStderr()
{
    std::cerr << m_shell->readAllStandardError().data() << std::flush;
}

void Shell::handleChannelClosed()
{
    std::cerr << "Shell closed. Exit code was " << m_shell->exitCode() << "." << std::endl;
    QCoreApplication::exit(m_shell->errorString().isEmpty() && m_shell->exitCode() == 0
        ? EXIT_SUCCESS : EXIT_FAILURE);
}

void Shell::handleStdin()
{
    m_shell->write(m_stdin->readLine());
}
