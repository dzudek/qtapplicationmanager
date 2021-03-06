/****************************************************************************
**
** Copyright (C) 2016 Pelagicore AG
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Pelagicore Application Manager.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT-QTAS$
** Commercial License Usage
** Licensees holding valid commercial Qt Automotive Suite licenses may use
** this file in accordance with the commercial license agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and The Qt Company.  For
** licensing terms and conditions see https://www.qt.io/terms-conditions.
** For further information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/QtCore>
#include <QtTest/QtTest>

#include "global.h"
#include "signature.h"

QT_USE_NAMESPACE_AM

class tst_Signature : public QObject
{
    Q_OBJECT

public:
    tst_Signature();

private slots:
    void initTestCase();
    void check();

private:
    QByteArray m_signingP12;
    QByteArray m_signingNoKeyP12;
    QByteArray m_signingPassword;
    QList<QByteArray> m_verifyingPEM;
};

tst_Signature::tst_Signature()
{ }

void tst_Signature::initTestCase()
{
    QFile s(qSL(":/signing.p12"));
    QVERIFY(s.open(QIODevice::ReadOnly));
    m_signingP12 = s.readAll();
    QVERIFY(!m_signingP12.isEmpty());

    QFile snk(qSL(":/signing-no-key.p12"));
    QVERIFY(snk.open(QIODevice::ReadOnly));
    m_signingNoKeyP12 = snk.readAll();
    QVERIFY(!m_signingNoKeyP12.isEmpty());

    QFile v(qSL(":/verifying.crt"));
    QVERIFY(v.open(QIODevice::ReadOnly));
    m_verifyingPEM << v.readAll();
    QVERIFY(!m_verifyingPEM.first().isEmpty());

    m_signingPassword = "password";
}

void tst_Signature::check()
{
    QByteArray hash("foo");
    Signature s(hash);
    QVERIFY(s.errorString().isEmpty());
    QByteArray signature = s.create(m_signingP12, m_signingPassword);
    QVERIFY2(!signature.isEmpty(), qPrintable(s.errorString()));

    Signature s2(hash + "bar");
    QByteArray signature2 = s2.create(m_signingP12, m_signingPassword);
    QVERIFY2(!signature2.isEmpty(), qPrintable(s2.errorString()));
    QVERIFY(signature != signature2);

    QVERIFY2(s.verify(signature, m_verifyingPEM), qPrintable(s.errorString()));
    QVERIFY2(s2.verify(signature2, m_verifyingPEM), qPrintable(s2.errorString()));
    QVERIFY(!s.verify(signature2, m_verifyingPEM));
    QVERIFY(!s2.verify(signature, m_verifyingPEM));

    QVERIFY(s.create(m_signingP12, m_signingPassword + "not").isEmpty());
    QVERIFY2(s.errorString().contains(qSL("not parse")), qPrintable(s.errorString()));

    QVERIFY(s.create(QByteArray(), m_signingPassword).isEmpty());
    QVERIFY2(s.errorString().contains(qSL("not read")), qPrintable(s.errorString()));

    Signature s3(QByteArray(4096, 'x'));
    QVERIFY(!s3.create(m_signingP12, m_signingPassword).isEmpty());

    QVERIFY(!s.verify(signature, QList<QByteArray>()));
    QVERIFY2(s.errorString().contains(qSL("Failed to verify")), qPrintable(s.errorString()));
    QVERIFY(!s.verify(signature, QList<QByteArray>() << m_signingP12));
    QVERIFY2(s.errorString().contains(qSL("not load")), qPrintable(s.errorString()));
    QVERIFY(!s.verify(hash, QList<QByteArray>() << m_signingP12));
    QVERIFY2(s.errorString().contains(qSL("not read")), qPrintable(s.errorString()));

    Signature s4 { QByteArray() };
    QVERIFY(s4.create(m_signingP12, m_signingPassword).isEmpty());

    QVERIFY(s.create(m_signingNoKeyP12, m_signingPassword).isEmpty());
    QVERIFY2(s.errorString().contains(qSL("private key")), qPrintable(s.errorString()));
}

QTEST_APPLESS_MAIN(tst_Signature)

#include "tst_signature.moc"

