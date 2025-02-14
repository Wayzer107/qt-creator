/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#include <QList>
#include <QSet>
#include <QSharedPointer>
#include <QVersionNumber>
#include <QtGlobal>

#if defined(MCUSUPPORT_LIBRARY)
#define MCUSUPPORTSHARED_EXPORT Q_DECL_EXPORT
#else
#define MCUSUPPORTSHARED_EXPORT Q_DECL_IMPORT
#endif

namespace McuSupport::Internal {

class McuTarget;
class McuAbstractPackage;
class McuToolChainPackage;

using McuPackagePtr = QSharedPointer<McuAbstractPackage>;
using McuToolChainPackagePtr = QSharedPointer<McuToolChainPackage>;
using McuTargetPtr = QSharedPointer<McuTarget>;

static const QVersionNumber minimalVersion{2, 0, 0};
static const QVersionNumber newVersion{2, 2};
using Targets = QList<McuTargetPtr>;
using Packages = QSet<McuPackagePtr>;

} // namespace McuSupport::Internal
