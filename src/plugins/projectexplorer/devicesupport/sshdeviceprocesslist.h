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

#pragma once

#include "deviceprocesslist.h"

#include <memory>

namespace ProjectExplorer {

class PROJECTEXPLORER_EXPORT SshDeviceProcessList : public DeviceProcessList
{
    Q_OBJECT
public:
    explicit SshDeviceProcessList(const IDeviceConstPtr &device, QObject *parent = nullptr);
    ~SshDeviceProcessList() override;

private:
    void handleConnectionError();
    void handleListProcessFinished();
    void handleKillProcessFinished(const QString &errorString);

    virtual QString listProcessesCommandLine() const = 0;
    virtual QList<Utils::ProcessInfo> buildProcessList(const QString &listProcessesReply) const = 0;

    void doUpdate() override;
    void doKillProcess(const Utils::ProcessInfo &process) override;

    void handleProcessError(const QString &errorMessage);
    void setFinished();

    class SshDeviceProcessListPrivate;
    const std::unique_ptr<SshDeviceProcessListPrivate> d;
};

} // namespace ProjectExplorer
