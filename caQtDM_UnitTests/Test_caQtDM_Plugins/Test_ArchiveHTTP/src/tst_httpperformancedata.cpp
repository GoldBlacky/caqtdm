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

#include "gtest/gtest.h"

#define CAQTDM_UNITTEST_INCLUDE
#include "httpperformancedata.h"
#include "urlhandlerhttp.h"
#include <QDateTime>
#include <QString>
#include <QDebug>

class tst_HttpPerformanceData : public ::testing::Test {
protected:
    HttpPerformanceData *m_perfData;
    UrlHandlerHttp *m_urlHandler;

    std::mutex m_testSerializerMutex;

    void SetUp() override
    {
        m_testSerializerMutex.lock();
        m_perfData = new HttpPerformanceData();
        m_urlHandler = new UrlHandlerHttp();
        m_urlHandler->setBeginTime(QDateTime::fromSecsSinceEpoch(1710922940));
        m_urlHandler->setEndTime(QDateTime::fromSecsSinceEpoch(1710937940));
        m_urlHandler->setBinned(true);
        m_urlHandler->setBinCount(50);
    }

    void TearDown() override
    {
        delete m_perfData;
        m_urlHandler->deleteLater();
        m_testSerializerMutex.unlock();
    }
};

TEST_F(tst_HttpPerformanceData, TestAddNewRequest)
{
    m_perfData->addNewRequest(m_urlHandler);
    EXPECT_EQ(m_urlHandler->beginTime(), QDateTime::fromSecsSinceEpoch(1710922940));
    EXPECT_EQ(m_urlHandler->endTime(), QDateTime::fromSecsSinceEpoch(1710937940));
    EXPECT_TRUE(m_urlHandler->binned());
    EXPECT_EQ(m_urlHandler->binCount(), 50);
}

TEST_F(tst_HttpPerformanceData, TestAddNewResponse_Successful)
{
    m_perfData->addNewRequest(m_urlHandler);
    QDateTime continueAtTime = QDateTime::currentDateTime();
    m_perfData->addNewResponse(5000, 200, false, continueAtTime);

    QString report = m_perfData->generateReport();
    EXPECT_NE(report.indexOf("Request has finished successfully."), -1);
    EXPECT_NE(report.indexOf("Response had size:                       5kb"), -1);
    EXPECT_NE(report.indexOf("Http response code was:             200"), -1);
}

TEST_F(tst_HttpPerformanceData, TestAddNewResponse_ContinueAt)
{
    m_perfData->addNewRequest(m_urlHandler);
    QDateTime continueAtTime = QDateTime::currentDateTime();
    m_perfData->addNewResponse(10000, 100, true, continueAtTime);

    QString report = m_perfData->generateReport();
    EXPECT_NE(report.indexOf("Request is currently in Progress."), -1);
    EXPECT_NE(report.indexOf("Response had size:                       10kb"), -1);
    EXPECT_NE(report.indexOf("Http response code was:             100"), -1);
    EXPECT_NE(report.indexOf("Request received continueAt:     true"), -1);
}

TEST_F(tst_HttpPerformanceData, TestAddNewResponse_Error)
{
    m_perfData->addNewRequest(m_urlHandler);
    QDateTime continueAtTime = QDateTime::currentDateTime();
    m_perfData->addNewResponse(2000, 500, false, continueAtTime);

    QString report = m_perfData->generateReport();
    EXPECT_NE(report.indexOf("Request has finished with errors."), -1);
    EXPECT_NE(report.indexOf("Response had size:                       2kb"), -1);
    EXPECT_NE(report.indexOf("Http response code was:             500"), -1);
}

TEST_F(tst_HttpPerformanceData, TestGenerateReport_InProgress)
{
    m_perfData->addNewRequest(m_urlHandler);

    QString report = m_perfData->generateReport();
    EXPECT_NE(report.indexOf("Request is currently in Progress."), -1);
}

TEST_F(tst_HttpPerformanceData, TestGenerateReport_NoRequest)
{
    QString report = m_perfData->generateReport();
    EXPECT_NE(report.indexOf("Request has finished with errors."), -1);
}

TEST_F(tst_HttpPerformanceData, TestEdgeCases_EmptyResponse)
{
    m_perfData->addNewRequest(m_urlHandler);
    QDateTime continueAtTime = QDateTime();
    m_perfData->addNewResponse(0, 0, false, continueAtTime);

    QString report = m_perfData->generateReport();
    EXPECT_NE(report.indexOf("Response had size:                       0kb"), -1);
    EXPECT_NE(report.indexOf("Http response code was:             0"), -1);
    EXPECT_NE(report.indexOf("Request received continueAt:     false"), -1);
}

TEST_F(tst_HttpPerformanceData, TestEdgeCases_LongResponseTime)
{
    QDateTime continueAtTime = QDateTime();
    m_perfData->addNewRequest(m_urlHandler);
    // Simulating long response time
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    m_perfData->addNewResponse(15000, 200, false, continueAtTime);

    QString report = m_perfData->generateReport();
    // Optimally, this would end with 2000ms but we don't have that accuracy so we accept 1/10 of a seconds precision.
    EXPECT_NE(report.indexOf("API server response took:            20"), -1);
}

TEST_F(tst_HttpPerformanceData, TestEdgeCases_HugeResponseSize)
{
    m_perfData->addNewRequest(m_urlHandler);
    QDateTime continueAtTime = QDateTime::currentDateTime();
    m_perfData->addNewResponse(10000000, 200, false, continueAtTime);

    QString report = m_perfData->generateReport();
    EXPECT_NE(report.indexOf("Response had size:                       10000kb"), -1);
}

TEST_F(tst_HttpPerformanceData, TestMultipleRequests)
{
    for (int i = 0; i < 5; ++i) {
        m_perfData->addNewRequest(m_urlHandler);
        QDateTime continueAtTime = QDateTime::currentDateTime();
        m_perfData->addNewResponse(1000 * (i + 1), 200, false, continueAtTime);
    }

    QString report = m_perfData->generateReport();
    EXPECT_NE(report.indexOf("Response had size:                       5kb"), -1);
    EXPECT_NE(report.indexOf("Http response code was:             200"), -1);
    EXPECT_NE(report.indexOf("Request received continueAt:     false"), -1);
}

