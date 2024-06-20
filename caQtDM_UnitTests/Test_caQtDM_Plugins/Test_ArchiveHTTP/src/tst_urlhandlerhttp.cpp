/*
 *  This file is part of the caQtDM Framework, developed at the Paul Scherrer Institut,
 *  Villigen, Switzerland
 *
 *  The caQtDM Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The caQtDM Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the caQtDM Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2010 - 2024
 *
 *  Author:
 *    Erik Schwarz
 *  Contact details:
 *    erik.schwarz@psi.ch
 */

/// This is an example file for unit tests how they can be implemented for all caQtDM helper classes and functions.

#include "gtest/gtest.h"

#define CAQTDM_UNITTEST_INCLUDE
#include "urlhandlerhttp.h"
#include <QUrl>
#include <QString>
#include <QDebug>

class tst_UrlHandlerHttp : public ::testing::Test {
protected:
    UrlHandlerHttp *m_urlHandler;

    // The easiest way to serialize the tests so I can reuse the m_urlHandler pointer.
    std::mutex m_testSerializerMutex;

    void SetUp() override
    {
        m_testSerializerMutex.lock();
        m_urlHandler = new UrlHandlerHttp();
    }

    void TearDown() override
    {
        m_urlHandler->deleteLater();
        m_testSerializerMutex.unlock();
    }
};

TEST_F(tst_UrlHandlerHttp, TestAssembleUrl_NonBinned)
{
    m_urlHandler->setUrl(QUrl("https://localhost"));
    m_urlHandler->setBackend("myBackend");
    m_urlHandler->setChannelName("myChannel");
    m_urlHandler->setBeginTime(QDateTime::fromSecsSinceEpoch(1710922940));
    m_urlHandler->setEndTime(QDateTime::fromSecsSinceEpoch(1710937940));
    m_urlHandler->setBinned(false);

    EXPECT_EQ(QUrl("https://localhost/api/4/events?backend=myBackend&channelName=myChannel&begDate=2024-03-20T08:22:20Z&endDate=2024-03-20T12:32:20Z&allowLargeResult=true"), m_urlHandler->assembleUrl());
}
TEST_F(tst_UrlHandlerHttp, TestAssembleUrl_Binned)
{
    m_urlHandler->setUrl(QUrl("https://12.4.22.1"));
    m_urlHandler->setBackend("myBackend");
    m_urlHandler->setChannelName("myChannel");
    m_urlHandler->setBeginTime(QDateTime::fromSecsSinceEpoch(1710822940));
    m_urlHandler->setEndTime(QDateTime::fromSecsSinceEpoch(1710937940));
    m_urlHandler->setBinned(true);
    m_urlHandler->setBinCount(2334);
    m_urlHandler->setUsesHttps(false);

    EXPECT_EQ(QUrl("http://12.4.22.1/api/4/binned?backend=myBackend&channelName=myChannel&begDate=2024-03-19T04:35:40Z&endDate=2024-03-20T12:32:20Z&binCount=2334&allowLargeResult=true"), m_urlHandler->assembleUrl());
}

TEST_F(tst_UrlHandlerHttp, SetUrl_FullUrlWithAllParameters) {
    QUrl url("http://example.com/api/4/binned?backend=test&channelName=test&begDate=2024-01-01T12:00:00Z&endDate=2024-01-02T12:00:00Z&binCount=100");
    m_urlHandler->setUrl(url);

    EXPECT_EQ(m_urlHandler->apiPathBinned(), "/api/4/binned");
    EXPECT_EQ(m_urlHandler->apiPathRaw(), "/api/4/events");
    EXPECT_FALSE(m_urlHandler->usesHttps());
    EXPECT_EQ(m_urlHandler->hostName().toString(), "example.com");
    EXPECT_TRUE(m_urlHandler->binned());
    EXPECT_EQ(m_urlHandler->binCount(), 100);
    EXPECT_EQ(m_urlHandler->backend(), "test");
    EXPECT_EQ(m_urlHandler->channelName(), "test");

    QDateTime expectedBeginTime = QDateTime::fromString("2024-01-01T12:00:00Z", Qt::ISODate);
    QDateTime expectedEndTime = QDateTime::fromString("2024-01-02T12:00:00Z", Qt::ISODate);
    EXPECT_EQ(m_urlHandler->beginTime(), expectedBeginTime);
    EXPECT_EQ(m_urlHandler->endTime(), expectedEndTime);
}

TEST_F(tst_UrlHandlerHttp, SetUrl_UrlWithOnlyDomainName) {
    QUrl url("http://example.com");
    m_urlHandler->setUrl(url);

    EXPECT_EQ(m_urlHandler->apiPathBinned(), "/api/4/binned");
    EXPECT_EQ(m_urlHandler->apiPathRaw(), "/api/4/events");
    EXPECT_FALSE(m_urlHandler->usesHttps());
    EXPECT_EQ(m_urlHandler->hostName().toString(), "example.com");
}

TEST_F(tst_UrlHandlerHttp, SetUrl_UrlWithOnlyPath) {
    QUrl url("http://example.com/api/4/raw");
    m_urlHandler->setUrl(url);

    EXPECT_EQ(m_urlHandler->apiPathBinned(), "/api/4/binned");
    EXPECT_EQ(m_urlHandler->apiPathRaw(), "/api/4/raw");
    EXPECT_FALSE(m_urlHandler->usesHttps());
    EXPECT_EQ(m_urlHandler->hostName().toString(), "example.com");
}

TEST_F(tst_UrlHandlerHttp, SetUrl_EmptyUrl) {
    QUrl url("");
    m_urlHandler->setUrl(url);

    EXPECT_EQ(m_urlHandler->apiPathBinned(), "/api/4/binned");
    EXPECT_EQ(m_urlHandler->apiPathRaw(), "/api/4/events");
    EXPECT_EQ(m_urlHandler->hostName().toString(), "");
}

TEST_F(tst_UrlHandlerHttp, SetUrl_UrlWithHttpsAndNoQueryParameters) {
    QUrl url("https://badurl.pdf");
    m_urlHandler->setUrl(url);

    EXPECT_EQ(m_urlHandler->apiPathBinned(), "/api/4/binned");
    EXPECT_EQ(m_urlHandler->apiPathRaw(), "/api/4/events");
    EXPECT_TRUE(m_urlHandler->usesHttps());
    EXPECT_EQ(m_urlHandler->hostName().toString(), "badurl.pdf");
}

TEST_F(tst_UrlHandlerHttp, TestApiPathBinned) {
    // Normal case: Set and get a valid binned path
    QString expectedBinnedPath = "/api/4/binned";
    m_urlHandler->setApiPathBinned(expectedBinnedPath);
    EXPECT_EQ(m_urlHandler->apiPathBinned(), expectedBinnedPath);

    // Edge case: Set and get an empty binned path
    QString emptyBinnedPath = "";
    m_urlHandler->setApiPathBinned(emptyBinnedPath);
    EXPECT_EQ(m_urlHandler->apiPathBinned(), emptyBinnedPath);

    // Edge case: Set and get a very long binned path
    QString longBinnedPath = QString(10000, 'a'); // 10000 'a' characters
    m_urlHandler->setApiPathBinned(longBinnedPath);
    EXPECT_EQ(m_urlHandler->apiPathBinned(), longBinnedPath);
}

TEST_F(tst_UrlHandlerHttp, TestApiPathRaw) {
    // Normal case: Set and get a valid raw path
    QString expectedRawPath = "/api/4/raw";
    m_urlHandler->setApiPathRaw(expectedRawPath);
    EXPECT_EQ(m_urlHandler->apiPathRaw(), expectedRawPath);

    // Edge case: Set and get an empty raw path
    QString emptyRawPath = "";
    m_urlHandler->setApiPathRaw(emptyRawPath);
    EXPECT_EQ(m_urlHandler->apiPathRaw(), emptyRawPath);

    // Edge case: Set and get a very long raw path
    QString longRawPath = QString(10000, 'b'); // 10000 'b' characters
    m_urlHandler->setApiPathRaw(longRawPath);
    EXPECT_EQ(m_urlHandler->apiPathRaw(), longRawPath);
}

TEST_F(tst_UrlHandlerHttp, TestAllowLargeResult) {
    // Normal case: Set and get allowLargeResult
    m_urlHandler->setAllowLargeResult(true);
    EXPECT_TRUE(m_urlHandler->allowLargeResult());

    m_urlHandler->setAllowLargeResult(false);
    EXPECT_FALSE(m_urlHandler->allowLargeResult());
}

TEST_F(tst_UrlHandlerHttp, TestHostName) {
    // Normal case: Set and get hostName
    QUrl expectedHostName("http://example.com");
    m_urlHandler->setHostName(expectedHostName);
    EXPECT_EQ(m_urlHandler->hostName(), expectedHostName);

    // Edge case: Set and get hostName with no scheme
    QUrl hostNameWithoutScheme("example.org");
    m_urlHandler->setHostName(hostNameWithoutScheme);
    EXPECT_EQ(m_urlHandler->hostName(), hostNameWithoutScheme);
}

TEST_F(tst_UrlHandlerHttp, TestUsesHttps) {
    // Normal case: Set and get usesHttps
    m_urlHandler->setUsesHttps(true);
    EXPECT_TRUE(m_urlHandler->usesHttps());

    m_urlHandler->setUsesHttps(false);
    EXPECT_FALSE(m_urlHandler->usesHttps());
}

TEST_F(tst_UrlHandlerHttp, TestBinned) {
    // Normal case: Set and get binned
    m_urlHandler->setBinned(true);
    EXPECT_TRUE(m_urlHandler->binned());

    m_urlHandler->setBinned(false);
    EXPECT_FALSE(m_urlHandler->binned());
}

TEST_F(tst_UrlHandlerHttp, TestBinCount) {
    // Normal case: Set and get binCount
    int expectedBinCount = 100;
    m_urlHandler->setBinCount(expectedBinCount);
    EXPECT_EQ(m_urlHandler->binCount(), expectedBinCount);

    // Edge case: Set and get binCount with zero
    m_urlHandler->setBinCount(0);
    EXPECT_EQ(m_urlHandler->binCount(), 0);

    // Edge case: Set and get a large binCount
    int largeBinCount = 100000;
    m_urlHandler->setBinCount(largeBinCount);
    EXPECT_EQ(m_urlHandler->binCount(), largeBinCount);
}

TEST_F(tst_UrlHandlerHttp, TestBackend) {
    // Normal case: Set and get backend
    QString expectedBackend = "myBackend";
    m_urlHandler->setBackend(expectedBackend);
    EXPECT_EQ(m_urlHandler->backend(), expectedBackend);

    // Edge case: Set and get empty backend
    QString emptyBackend = "";
    m_urlHandler->setBackend(emptyBackend);
    EXPECT_EQ(m_urlHandler->backend(), emptyBackend);
}

TEST_F(tst_UrlHandlerHttp, TestChannelName) {
    // Normal case: Set and get channelName
    QString expectedChannelName = "myChannel";
    m_urlHandler->setChannelName(expectedChannelName);
    EXPECT_EQ(m_urlHandler->channelName(), expectedChannelName);

    // Edge case: Set channel with suffix but get without
    QStringList suffixList = {".X",".Y",".minY",".maxY"};
    for (const QString &suffix : suffixList) {
        m_urlHandler->setChannelName(expectedChannelName + suffix);
        EXPECT_EQ(m_urlHandler->channelName(), expectedChannelName);
    }

    // Edge case: Set and get empty channelName
    QString emptyChannelName = "";
    m_urlHandler->setChannelName(emptyChannelName);
    EXPECT_EQ(m_urlHandler->channelName(), emptyChannelName);
}

TEST_F(tst_UrlHandlerHttp, TestBeginTime) {
    // Normal case: Set and get beginTime
    QDateTime expectedBeginTime = QDateTime::currentDateTime();
    m_urlHandler->setBeginTime(expectedBeginTime);
    EXPECT_EQ(m_urlHandler->beginTime(), expectedBeginTime);

    // Edge case: Set and get invalid (null) beginTime
    QDateTime invalidBeginTime;
    m_urlHandler->setBeginTime(invalidBeginTime);
    EXPECT_EQ(m_urlHandler->beginTime(), invalidBeginTime);
}

TEST_F(tst_UrlHandlerHttp, TestEndTime) {
    // Normal case: Set and get endTime
    QDateTime expectedEndTime = QDateTime::currentDateTime().addDays(1);
    m_urlHandler->setEndTime(expectedEndTime);
    EXPECT_EQ(m_urlHandler->endTime(), expectedEndTime);

    // Edge case: Set and get invalid (null) endTime
    QDateTime invalidEndTime;
    m_urlHandler->setEndTime(invalidEndTime);
    EXPECT_EQ(m_urlHandler->endTime(), invalidEndTime);
}
