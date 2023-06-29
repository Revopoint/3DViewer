/*******************************************************************************
* This file is part of the 3DViewer                                            *
*                                                                              *
* Copyright (C) 2022 Revopoint3D Company Ltd.                                  *
* All rights reserved.                                                         *
*                                                                              *
* This program is free software: you can redistribute it and/or modify         *
* it under the terms of the GNU General Public License as published by         *
* the Free Software Foundation, either version 3 of the License, or            *
* (at your option) any later version.                                          *
*                                                                              *
* This program is distributed in the hope that it will be useful,              *
* but WITHOUT ANY WARRANTY; without even the implied warranty of               *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)             *
* for more details.                                                            *
*                                                                              *
********************************************************************************/

#include "cscamera.h"
#include "cameraparaid.h"

#include <QDebug>
#include <QSize>
#include <QVariant>
#include <QThread>
#include <QRectF>
#include <QTimer>
#include <QMetaEnum>
#include <algorithm>
#include <3DCamera.hpp>

static CSRange DEPTH_RANGE_LIMIT = { 0, 65535 };
static CSRange CAMERA_HDR_LEVEL_RANGE = { 2,8 };
#define HDR_SCALE_DEFAULT  5

#ifdef _DEBUG
    #define _CS_DEBUG_
#endif

using namespace cs;
using namespace cs::parameter;

QMetaEnum CSCamera::metaEnum = QMetaEnum::fromType<CAMERA_PARA_ID>();

static const QMap<int, const char*> STREAM_FORMAT_MAP =
{ 
    { STREAM_FORMAT_MJPG,    "MJPG" },
    { STREAM_FORMAT_RGB8,    "RGB8" },
    { STREAM_FORMAT_Z16,     "Z16" },
    { STREAM_FORMAT_Z16Y8Y8, "Z16Y8Y8" },
    { STREAM_FORMAT_PAIR,    "PAIR" },
    { STREAM_FORMAT_H264,    "H264" },
    { STREAM_FORMAT_I8DS,    "I8DS" },
    { STREAM_FORMAT_XZ32,    "XZ32" },
    { STREAM_FORMAT_GRAY,    "GRAY" },
};

const QMap<int, const char*> CSCamera::AUTO_EXPOSURE_MODE_MAP =
{
    { AUTO_EXPOSURE_MODE_CLOSE,             QT_TR_NOOP("Close")},
    { AUTO_EXPOSURE_MODE_FIX_FRAMETIME,     QT_TR_NOOP("Speed First")},
    { AUTO_EXPOSURE_MODE_HIGH_QUALITY,      QT_TR_NOOP("Quality First")},
    { AUTO_EXPOSURE_MODE_FORE_GROUND,       QT_TR_NOOP("Foreground")},
};

const QMap<int, const char*> CSCamera::FILTER_TYPE_MAP =
{
    { FILTER_CLOSE,      QT_TR_NOOP("Close")},
    { FILTER_SMOOTH,     QT_TR_NOOP("Smooth")},
    { FILTER_MEDIAN,     QT_TR_NOOP("Median")},
    { FILTER_TDSMOOTH,   QT_TR_NOOP("TDSmooth")},
};

static const QMap<int, CSRange> FILTER_RANGE_MAP =
{
    { FILTER_CLOSE,      { 0, 0 }},
    { FILTER_SMOOTH,     { 3, 9 }},
    { FILTER_MEDIAN,     { 3, 5 }},
    { FILTER_TDSMOOTH,   { 3, 7 }},
};

struct ParaInfo 
{
    int type;   //  0 : STREAM_TYPE_DEPTH, 1 : STREAM_TYPE_RGB
    int id;     //  PROPERTY_TYPE
};

static const QMap<CAMERA_PARA_ID, ParaInfo> CAMERA_PROPERTY_MAP =
{
    { PARA_DEPTH_GAIN,              { STREAM_TYPE_DEPTH , PROPERTY_GAIN} },
    { PARA_DEPTH_EXPOSURE,          { STREAM_TYPE_DEPTH , PROPERTY_EXPOSURE} },
    { PARA_DEPTH_FRAMETIME,         { STREAM_TYPE_DEPTH , PROPERTY_FRAMETIME} },
    

    { PARA_RGB_GAIN,                { STREAM_TYPE_RGB ,   PROPERTY_GAIN} },
    { PARA_RGB_AUTO_EXPOSURE,       { STREAM_TYPE_RGB ,   PROPERTY_ENABLE_AUTO_EXPOSURE} },
    { PARA_RGB_AUTO_WHITE_BALANCE,  { STREAM_TYPE_RGB ,   PROPERTY_ENABLE_AUTO_WHITEBALANCE} },
    { PARA_RGB_WHITE_BALANCE,       { STREAM_TYPE_RGB ,   PROPERTY_WHITEBALANCE} }
};

static const QMap<CAMERA_PARA_ID, int> CAMERA_EXTENSION_PROPERTY_MAP =
{
    { PARA_DEPTH_AUTO_EXPOSURE,    PROPERTY_EXT_AUTO_EXPOSURE_MODE },
    { PARA_DEPTH_THRESHOLD,        PROPERTY_EXT_CONTRAST_MIN },
    { PARA_DEPTH_HDR_MODE,         PROPERTY_EXT_HDR_MODE },
    { PARA_DEPTH_HDR_SETTINGS,     PROPERTY_EXT_HDR_EXPOSURE },
    { PARA_DEPTH_HDR_LEVEL,        PROPERTY_EXT_HDR_SCALE_SETTING },
    { PARA_DEPTH_SCALE,            PROPERTY_EXT_DEPTH_SCALE },  
    { PARA_DEPTH_ROI,              PROPERTY_EXT_DEPTH_ROI },
    { PARA_DEPTH_RANGE,            PROPERTY_EXT_DEPTH_RANGE },
    { PARA_TRIGGER_MODE,           PROPERTY_EXT_TRIGGER_MODE },
    { PARA_CAMERA_IP,              PROPERTY_EXT_CAMERA_IP },
    { PARA_RGB_EXPOSURE,           PROPERTY_EXT_EXPOSURE_TIME_RGB }
};

static const QList<CAMERA_PARA_ID> CAMERA_OPTION_PARA_LIST =
{
    PARA_DEPTH_STREAM_FORMAT,
    PARA_DEPTH_RESOLUTION,
    PARA_DEPTH_AUTO_EXPOSURE,
    PARA_DEPTH_FILTER_TYPE,
    PARA_RGB_STREAM_FORMAT,
    PARA_RGB_RESOLUTION,
};

const QMap<CAMERA_HDR_MODE, const char*> CSCamera::CAMERA_HDR_MAP =
{
    { HDR_MODE_CLOSE,       QT_TR_NOOP("Close") },
    { HDR_MODE_SHINE,       QT_TR_NOOP("Shiny") },
    { HDR_MODE_DARK,        QT_TR_NOOP("Dark") },
    { HDR_MODE_BOTH,        QT_TR_NOOP("Both") },
    { HDR_MODE_MANUAL,      QT_TR_NOOP("Manual") },
};

static const QMap<CAMERA_PARA_ID, QVariant> CAMERA_DEFAULT_STREAM_TYPE =
{
    { PARA_DEPTH_STREAM_FORMAT,   (int)STREAM_FORMAT_Z16 },
    { PARA_DEPTH_RESOLUTION,      QSize(640, 400) },
    { PARA_RGB_STREAM_FORMAT,     (int)STREAM_FORMAT_RGB8 },
    { PARA_RGB_RESOLUTION,        QSize(1280, 800) },
};

static const QMap<CAMERA_PARA_ID, QVariant> CAMERA_DEFAULT_PARA_VALUE =
{
    { PARA_DEPTH_RANGE,          QVariant::fromValue(QPair<float, float>({ 50.0f, 2000.0f })) },
    { PARA_DEPTH_GAIN,           1.0f },
    { PARA_DEPTH_EXPOSURE,       7000.0f },
    { PARA_TRIGGER_MODE,         (int)TRIGGER_MODE_OFF}
};

int CSCamera::setCameraChangeCallback(CameraChangeCallback callback, void* userData)
{
    return cs::getSystemPtr()->setCameraChangeCallback(callback, userData);
}

int CSCamera::setCameraAlarmCallback(CameraAlarmCallback callback, void* userData)
{
    return cs::getSystemPtr()->setCameraAlarmCallback(callback, userData);
}

void CSCamera::setSdkLogPath(QString path)
{
    setLogSavePath(path.toStdString().c_str());
    enableLoging(true);
}

void CSCamera::queryCameras(std::vector<CameraInfo>& cameras)
{
    cs::getSystemPtr()->queryCameras(cameras);
}

static void depthCallback(cs::IFramePtr frame, void* usrData)
{
    CSCamera* camera = (CSCamera*)usrData;

    FrameData frameData;
    camera->onProcessFrame(TYPE_DEPTH, frame, frameData.data);
   
    emit camera->framedDataUpdated(frameData);
}

static void rgbCallback(cs::IFramePtr frame, void* usrData)
{
    CSCamera* camera = (CSCamera*)usrData;

    FrameData frameData;
    camera->onProcessFrame(TYPE_RGB, frame, frameData.data);
    emit camera->framedDataUpdated(frameData);
}

CSCamera::StreamThread::StreamThread(CSCamera& camera)
    : m_camera(camera)
{
    setObjectName("StreamThread");
}

void CSCamera::StreamThread::run()
{
    while (!isInterruptionRequested())
    {
        FrameData frameData;
        auto ret = m_camera.onGetFrame(frameData);

        if (frameData.data.size() > 0)
        {
            frameData.rgbIntrinsics = m_camera.m_rgbIntrinsics;
            frameData.depthIntrinsics = m_camera.m_depthIntrinsics;
            frameData.extrinsics = m_camera.m_extrinsics;
            frameData.depthScale = m_camera.m_depthScale;

            emit m_camera.framedDataUpdated(frameData);
        }

        if (ret == ERROR_DEVICE_NOT_CONNECT)
        {
            // If the return value indicates that the camera is not connected, add a delay.
            QThread::msleep(200);
        }
    }
}

CSCamera::StreamThread::~StreamThread()
{
    requestInterruption();
    wait();
    qDebug() << "~StreamThread";
}

CSCamera::CSCamera()
    : m_cameraState(CAMERA_DISCONNECTED)
    , m_cameraPtr(cs::getCameraPtr())
    , m_filterValue(0)
    , m_filterType(0)
    , m_fillHole(false)
    , m_hasIrStream(false)
    , m_hasDepthStream(false)
    , m_isRgbStreamSup(false)
    , m_isDepthStreamSup(false)
    , m_hdrMode(HDR_MODE_CLOSE)
    , m_hdrTimes(2)
    , m_streamThread(new StreamThread(*this))
    , m_cameraThread(nullptr)
    , m_cachedDepthExposure(0)
    , m_cachedDepthGain(0)
    , m_triggerMode(TRIGGER_MODE_OFF)
    , m_depthScale(0.1)
{
    m_manualHdrSetting.count = 0;
}

CSCamera::~CSCamera()
{
    doDisconnectCamera();
    delete m_streamThread;
    qDebug() << "~CSCamera";
}

bool CSCamera::restartStream()
{
    qInfo() << "begin restart stream.";
    
    // start stream after stop stream
    bool result = stopStream();
    QThread::msleep(200);
    result &= startStream();

    qDebug() << "restartStream end!";
    return result;
}

bool CSCamera::startStream()
{
    bool suc = true;
    suc &= (bool)connect(this, &CSCamera::updateParaSignal, this, &CSCamera::onParaUpdated);
    Q_ASSERT(suc);

    setCameraState(CAMERA_STARTING_STREAM);
    bool result = true;
    if (m_isDepthStreamSup)
    {
        qInfo() << "begin start depth stream";
        result &= startDepthStream();
        if (!result)
        {
            Q_ASSERT(false);
            qWarning("Error : start depth stream failed.");
        }
        qInfo() << "start depth stream end";
    }

    if (m_isRgbStreamSup)
    {
        qInfo() << "begin start rgb stream";
        result &= startRgbStream();
        if (!result)
        {
            Q_ASSERT(false);
            qWarning("Error : start rgb stream failed.");
        }
        qInfo() << "start rgb stream end";
    }

    if (!result)
    {
        qWarning("Error : start stream failed.");
        setCameraState(CAMERA_START_STREAM_FAILED);
        return false;
    }

    // start get frame thread
    startStreamThread();

    onStreamStarted();

    // notify camera stream started
    setCameraState(CAMERA_STARTED_STREAM);

    return result;
}

void CSCamera::onStreamStarted()
{
    // set default para
    qInfo() << "set camera default para";
    for (auto key : CAMERA_DEFAULT_PARA_VALUE.keys())
    {
        setCameraPara(key, CAMERA_DEFAULT_PARA_VALUE.value(key));
    }

    // get depth scale
    PropertyExtension propExt;
    ERROR_CODE ret = m_cameraPtr->getPropertyExtension(PROPERTY_EXT_DEPTH_SCALE, propExt);
    m_depthScale = propExt.depthScale;

    DEPTH_RANGE_LIMIT.max = 65535 * m_depthScale;
}

bool CSCamera::stopStream()
{
    bool result = true;

    disconnect(this, &CSCamera::updateParaSignal, this, &CSCamera::onParaUpdated);

    // stop get frame thread
    stopStreamThread();

    if (m_isDepthStreamSup)
    {
        result &= stopDepthStream();
        if (!result)
        {
            Q_ASSERT(false);
            qWarning("Error : stop depth stream failed.");
        }
    }

    if (m_isRgbStreamSup)
    {
        result &= stopRgbStream();
        if (!result)
        {
            Q_ASSERT(false);
            qWarning("Error : stop rgb stream failed.");
        }

    }

    m_hasIrStream = false;
    m_hasDepthStream = false;

    setCameraState(CAMERA_STOPPED_STREAM);
    return true;
}

bool CSCamera::startDepthStream()
{
    ERROR_CODE ret;

    // get stream info by depthFormat and depthResolution
    StreamInfo info = getDepthStreamInfo();

    // you can also use the callback method
    //ret = cameraPtr->startStream(STREAM_TYPE_DEPTH, info, depthCallback, this);
    ret = m_cameraPtr->startStream(STREAM_TYPE_DEPTH, info, nullptr, this);
    if (ret != SUCCESS)
    {
        qWarning("camera start depth stream failed(%d)!", ret);
        Q_ASSERT(false);
        return false;
    }
    else
    {
        qWarning("start depth format:%2d, width:%4d, height:%4d, fps:%2.1f", info.format, info.width, info.height, info.fps);
    }

    m_hasIrStream = m_isDepthStreamSup && ((info.format == STREAM_FORMAT_Z16Y8Y8) || (info.format == STREAM_FORMAT_PAIR));
    m_hasDepthStream = m_isDepthStreamSup && (info.format != STREAM_FORMAT_PAIR);

    return ret == SUCCESS;
}

bool CSCamera::startRgbStream()
{
    ERROR_CODE ret;

    // get stream info by rgbFormat and rgbResolution
    StreamInfo info = getRgbStreamInfo();

    // you can also use the callback method
    //ret = cameraPtr->startStream(STREAM_TYPE_RGB, info, rgbCallback, this);
    ret = m_cameraPtr->startStream(STREAM_TYPE_RGB, info, nullptr, this);
    if (ret != SUCCESS)
    {
        qDebug("camera start rgb stream failed(%d)!", ret);
        Q_ASSERT(false);
        return false;
    }

    return ret == SUCCESS;
}

bool CSCamera::stopDepthStream()
{
    ERROR_CODE ret = m_cameraPtr->stopStream(STREAM_TYPE_DEPTH);
    if (ret != SUCCESS)
    {
        qDebug("camera stop depth stream failed(%d)!", ret);
        return false;
    }

    return true;
}

bool CSCamera::stopRgbStream()
{
    ERROR_CODE ret = m_cameraPtr->stopStream(STREAM_TYPE_RGB);
    if (ret != SUCCESS)
    {
        qDebug("camera stop rgb stream failed(%d)!", ret);
        return false;
    }

    return true;
}

bool CSCamera::restartCamera()
{
    qInfo() << "CSCamera: restart camera";
    setCameraState(CAMERA_RESTARTING_CAMERA);

    disconnect(this, &CSCamera::updateParaSignal, this, &CSCamera::onParaUpdated);
    stopStreamThread();

    ERROR_CODE ret = m_cameraPtr->restart();
    if (ret != SUCCESS)
    {
        qInfo() << "camera restart failed, ret = " << ret;
        Q_ASSERT(false);
        setCameraState(CAMERA_CONNECTFAILED);
        return false;
    }

    qInfo() << "CSCamera: restart end, wait connect...";
    
    return true;
}

bool CSCamera::connectCamera(CameraInfo info)
{
    qInfo() << "connectCamera, camera seiral : "<< info.serial;

    m_cameraInfo.cameraInfo = info;

    setCameraState(CAMERA_CONNECTING);

    ERROR_CODE ret = m_cameraPtr->connect(info);
    if (ret != SUCCESS)
    {
        qWarning() << "camera connect failed, error code : " << ret;
        setCameraState(CAMERA_CONNECTFAILED);
        return false;
    }

    // judge if depth stream is supported or not
    ret = m_cameraPtr->isStreamSupport(STREAM_TYPE_DEPTH, m_isDepthStreamSup);
    if (ret != SUCCESS)
    {
        qWarning() << "camera call is support stream failed, error code : " << ret;
        Q_ASSERT(false);
    }

    // judge if RGB stream is supported or not
    ret = m_cameraPtr->isStreamSupport(STREAM_TYPE_RGB, m_isRgbStreamSup);
    if (ret != SUCCESS)
    {
        qWarning() << "camera call is support stream failed, error code : " << ret;
        Q_ASSERT(false);
    }

    initDefaultStreamInfo();

    initCameraInfo();

    setCameraState(CAMERA_CONNECTED);

    return  ret == SUCCESS;
}

bool CSCamera::reconnectCamera()
{
    qInfo() << "CSCamera, begin reconnect camera";
    bool result = connectCamera(m_cameraInfo.cameraInfo);
    qInfo() << "CSCamera, reconnect camera end";
    
    return result;
}

bool CSCamera::disconnectCamera()
{
    qInfo() << "CSCamera, begin disconenct camera";
    disconnect(this, &CSCamera::updateParaSignal, this, &CSCamera::onParaUpdated);

    doDisconnectCamera();
    qInfo() << "CSCamera, disconenct camera end";

    return true;
}

void CSCamera::doDisconnectCamera()
{
    if (getCameraState() == CAMERA_STARTED_STREAM || getCameraState() == CAMERA_PAUSED_STREAM)
    {
        qInfo() << "CSCamera, begin  stop stream";
        stopStream();
    }

    setCameraState(CAMERA_DISCONNECTING);
    
    // disconnect camera
    ERROR_CODE ret = m_cameraPtr->disconnect();
    if (ret != SUCCESS)
    {
        qWarning() << "disconnect camera failed, error code : " << ret;
        Q_ASSERT(false);
        setCameraState(CAMERA_DISCONNECTFAILED);
    }
    else
    {
        setCameraState(CAMERA_DISCONNECTED);
    }

    m_isRgbStreamSup = false;
    m_isDepthStreamSup = false;
}

bool CSCamera::pauseStream()
{
    if (getCameraState() != CAMERA_STARTED_STREAM) {
        qInfo() << "please pause stream latter, state = " << getCameraState();
        return false;
    }

    stopStreamThread();

    ERROR_CODE ret = SUCCESS;
    //depth 
    if (m_isDepthStreamSup)
    {
        ret = m_cameraPtr->pauseStream(STREAM_TYPE_DEPTH);
        if (ret != SUCCESS)
        {
            Q_ASSERT(false);
            qWarning() << "pause depth stream failed, error code : " << ret;
        }
    }
     
    //rgb
    if (m_isRgbStreamSup)
    {
        ret = m_cameraPtr->pauseStream(STREAM_TYPE_RGB);
        if (ret != SUCCESS)
        {
            //Q_ASSERT(false);
            qWarning() << "pause rgb stream failed, error code : " << ret;
        }
    }

    setCameraState(CAMERA_PAUSED_STREAM);

    return ret == SUCCESS;
}

bool CSCamera::resumeStream()
{
    if (getCameraState() != CAMERA_PAUSED_STREAM) {
        qInfo() << "please resume stream latter, state = " << getCameraState();
        return false;
    }

    //depth 
    ERROR_CODE ret = SUCCESS;
    if (m_isDepthStreamSup)
    {
        ret = m_cameraPtr->resumeStream(STREAM_TYPE_DEPTH);
        if (ret != SUCCESS)
        {
            Q_ASSERT(false);
            qWarning() << "resume depth stream failed, error code : " << ret;
        }
    }

    //rgb
    if (m_isRgbStreamSup)
    {
        ret = m_cameraPtr->resumeStream(STREAM_TYPE_RGB);
        if (ret != SUCCESS)
        {
            //Q_ASSERT(false);
            qWarning() << "resume rgb stream failed, error code : " << ret;
        }
    }

    startStreamThread();
    setCameraState(CAMERA_STARTED_STREAM);

    return ret = SUCCESS;
}

bool CSCamera::softTrigger()
{
    if (getCameraState() != CAMERA_STARTED_STREAM)
    {
        qWarning() << "";
        return false;
    }

    int frameCount = (m_filterType == FILTER_TDSMOOTH) ? m_filterValue : 1;
  
    bool result = true;

    FrameData frameData;

    for(int i = 0; i < frameCount; i++)
    {
        ERROR_CODE ret = m_cameraPtr->softTrigger();
        if (ret != SUCCESS)
        {
            qWarning("camera soft trigger failed(%d)!", ret);
            result = false;
        }

        doGetFrame(frameData.data, 3000);
    }

    // notify
    if (frameData.data.size() > 0)
    {
        frameData.rgbIntrinsics = m_rgbIntrinsics;
        frameData.depthIntrinsics = m_depthIntrinsics;
        frameData.extrinsics = m_extrinsics;
        frameData.depthScale = m_depthScale;

        emit framedDataUpdated(frameData);
    }

    return result;
}

void CSCamera::initCameraInfo()
{
    CameraType cameraType = getCameraTypeBySN(m_cameraInfo.cameraInfo.serial);
    m_cameraInfo.model = getCameraTypeName(cameraType);

    CS_SDK_VERSION* sdkVersion = nullptr;
    getSdkVersion(&sdkVersion);

    if (sdkVersion && sdkVersion->version)
    {
        m_cameraInfo.sdkVersion = sdkVersion->version;
    }

    //connect type
    m_cameraInfo.connectType = isNetworkConnect(m_cameraInfo.cameraInfo.uniqueId) ? CONNECT_TYPE_NET : CONNECT_TYPE_USB;

    m_hasIrStream = m_isDepthStreamSup && ((m_depthFormat == STREAM_FORMAT_Z16Y8Y8) || (m_depthFormat == STREAM_FORMAT_PAIR));
    m_hasDepthStream = m_isDepthStreamSup && (m_depthFormat != STREAM_FORMAT_PAIR);

    ERROR_CODE ret = SUCCESS;
    if (m_isDepthStreamSup)
    {
        ret = m_cameraPtr->getIntrinsics(STREAM_TYPE_DEPTH, m_depthIntrinsics);
        if (ret != SUCCESS)
        {
            qWarning() << "camera get depth intrinsics failed, error code : " << ret;
            Q_ASSERT(false);
        }
    }

    if (m_isRgbStreamSup)
    {
        ret = m_cameraPtr->getIntrinsics(STREAM_TYPE_RGB, m_rgbIntrinsics);
        if (ret != SUCCESS)
        {
            qWarning() << "camera get rgb intrinsics failed, error code : " << ret;
            Q_ASSERT(false);
        }

        PropertyExtension proExt;
        proExt.depthRgbMatchParam.iRgbOffset = 0;
        proExt.depthRgbMatchParam.iDifThreshold = 0;
        ret = m_cameraPtr->setPropertyExtension(PROPERTY_EXT_DEPTH_RGB_MATCH_PARAM, proExt);
        if (ret != SUCCESS)
        {
            qWarning() << "camera set depth rgb match para failed, error code : " << ret;
            Q_ASSERT(false);
        }
    }

    ret = m_cameraPtr->getExtrinsics(m_extrinsics);
    if (ret != SUCCESS)
    {
        qWarning() << "camera get extrinsics failed, error code : " << ret;
        Q_ASSERT(false);
    }
}

void CSCamera::initDefaultStreamInfo()
{
    ERROR_CODE ret;
    std::vector<StreamInfo> streamInfos;

    // default depth stream
    if (m_isDepthStreamSup)
    {
        ret = m_cameraPtr->getStreamInfos(STREAM_TYPE_DEPTH, streamInfos);
        if (ret != SUCCESS || streamInfos.empty())
        {
            qDebug("camera get stream info failed(%d)!\n", ret);
        }
        else
        {
            // format
            auto defaultFromat = (STREAM_FORMAT)CAMERA_DEFAULT_STREAM_TYPE[PARA_DEPTH_STREAM_FORMAT].toInt();
            auto it = std::find_if(streamInfos.begin(), streamInfos.end()
                , [=](const StreamInfo& info)
                {
                    return defaultFromat == info.format;
                });

            it = (it == streamInfos.end()) ? streamInfos.begin() : it;
            m_depthFormat = it->format;

            // resolution
            auto defaultRes = CAMERA_DEFAULT_STREAM_TYPE[PARA_DEPTH_RESOLUTION].toSize();
            auto itRes = std::find_if(streamInfos.begin(), streamInfos.end()
                , [=](const StreamInfo& info)
                {
                    return (defaultRes.width() == info.width && defaultRes.height() == info.height)
                        && (m_depthFormat == info.format);
                });

            // use *it if not find the resolution
            itRes = (itRes == streamInfos.end()) ? it : itRes;
            m_depthResolution = QSize(itRes->width, itRes->height);
        }
    }

    if (m_isRgbStreamSup)
    {
        // default rgb stream
        streamInfos.clear();
        ret = m_cameraPtr->getStreamInfos(STREAM_TYPE_RGB, streamInfos);
        if (ret != SUCCESS || streamInfos.empty())
        {
            qWarning("camera get stream info failed(%d)!\n", ret);
        }
        else
        {
            // format
            auto defaultFromat = (STREAM_FORMAT)CAMERA_DEFAULT_STREAM_TYPE[PARA_RGB_STREAM_FORMAT].toInt();
            auto it = std::find_if(streamInfos.begin(), streamInfos.end()
                , [=](const StreamInfo& info)
                {
                    return defaultFromat == info.format;
                });

            it = (it == streamInfos.end()) ? streamInfos.begin() : it;
            m_rgbFormat = it->format;

            // resolution
            auto defaultRes = CAMERA_DEFAULT_STREAM_TYPE[PARA_RGB_RESOLUTION].toSize();
            auto itRes = std::find_if(streamInfos.begin(), streamInfos.end()
                , [=](const StreamInfo& info)
                {
                    return (defaultRes.width() == info.width && defaultRes.height() == info.height)
                        && (m_rgbFormat == info.format);
                });

            // use *it if not find the resolution
            itRes = (itRes == streamInfos.end()) ? it : itRes;
            m_rgbResolution = QSize(itRes->width, itRes->height);
        }
    }
}

int CSCamera::doGetFrame(QVector<StreamData>& streamDatas, int timeout)
{
    auto ret = SUCCESS;

    if (m_isRgbStreamSup)
    {
        IFramePtr depthFrame, rgbFrame;

        ret = m_cameraPtr->getPairedFrame(depthFrame, rgbFrame, timeout);
        if (ret != SUCCESS)
        {
            return ret;
        }

        onProcessFrame(TYPE_DEPTH, depthFrame, streamDatas);
        onProcessFrame(TYPE_RGB, rgbFrame, streamDatas);
    }
    else
    {
        IFramePtr depthFrame;
        ret = m_cameraPtr->getFrame(STREAM_TYPE_DEPTH, depthFrame, timeout);
        if (ret != SUCCESS)
        {
            return ret;
        }

        onProcessFrame(TYPE_DEPTH, depthFrame, streamDatas);
    }

    return SUCCESS;
}

// use getPairedFrame if has a RGB camera, or use getFrame
int CSCamera::onGetFrame(FrameData& frameData, int timeout)
{
    return doGetFrame(frameData.data, timeout);
}

bool CSCamera::onProcessFrame(STREAM_DATA_TYPE streamDataType, const IFramePtr& frame, QVector<StreamData>& streamDatas)
{
    if (!frame || frame->empty())
    {
        return false;
    }

    StreamDataInfo streamDataInfo = { streamDataType, frame->getFormat(),frame->getWidth(), frame->getHeight(), frame->getTimeStamp()};
    const int dataSize = frame->getSize();

    //copy data
    QByteArray data;
    data.resize(dataSize);
    memcpy(data.data(), frame->getData(), dataSize);

    StreamData streamData = { streamDataInfo, data };

    streamDatas.push_back(streamData);

    return true;
}

void CSCamera::setCameraThread(QThread* thread)
{
    m_cameraThread = thread;
    Q_ASSERT(m_cameraThread);

    moveToThread(m_cameraThread);
}

StreamInfo CSCamera::getDepthStreamInfo()
{
    StreamInfo info = { STREAM_FORMAT_COUNT, 0, 0, 0.0f};

    std::vector<StreamInfo> streamInfos;
    ERROR_CODE ret = m_cameraPtr->getStreamInfos(STREAM_TYPE_DEPTH, streamInfos);
    if (ret != SUCCESS || streamInfos.empty())
    {
        qWarning("camera get depth stream info failed(%d)!\n", ret);
        return info;
    }

    info.fps = -1;
    for (const auto& sInfo : streamInfos)
    {
        if (sInfo.format == m_depthFormat && sInfo.width == m_depthResolution.width() 
            && sInfo.height == m_depthResolution.height() && sInfo.fps > info.fps)
        {
            info = sInfo;
        }
    }

    return info;
}

StreamInfo CSCamera::getRgbStreamInfo()
{
    StreamInfo info = { STREAM_FORMAT_COUNT, 0, 0, 0.0f };

    std::vector<StreamInfo> streamInfos;
    ERROR_CODE ret = m_cameraPtr->getStreamInfos(STREAM_TYPE_RGB, streamInfos);
    if (ret != SUCCESS || streamInfos.empty())
    {
        qWarning("camera get rgb stream info failed(%d)!\n", ret);
        return info;
    }

    info.fps = -1;
    for (const auto& sInfo : streamInfos)
    {
        if (sInfo.format == m_rgbFormat && sInfo.width == m_rgbResolution.width()
            && sInfo.height == m_rgbResolution.height() && sInfo.fps > info.fps)
        {
            info = sInfo;
        }
    }

    return info;
}

void CSCamera::getUserParaPrivate(CAMERA_PARA_ID paraId, QVariant& value)
{
    switch (paraId)
    {
    case PARA_DEPTH_STREAM_FORMAT:
        value = (int)m_depthFormat;
        break;
    case PARA_DEPTH_RESOLUTION:
        value = m_depthResolution;
        break;
    case PARA_DEPTH_FILTER:
        value = m_filterValue;
        break;
    case PARA_DEPTH_FILTER_TYPE:
        value = m_filterType;
        break;
    case PARA_DEPTH_FILL_HOLE:
        value = m_fillHole;
        break;
    case PARA_RGB_STREAM_FORMAT:
        value = (int)m_rgbFormat;
        break;
    case PARA_RGB_RESOLUTION:
        value = m_rgbResolution;
        break;
    case PARA_HAS_RGB:
        value = m_isRgbStreamSup;
        break;
    case PARA_DEPTH_HAS_IR:
        value = m_hasIrStream;
        break;
    case PARA_HAS_DEPTH:
        value = m_hasDepthStream;
        break;
    case PARA_DEPTH_INTRINSICS:
        value = QVariant::fromValue(m_depthIntrinsics);
        break;
    case PARA_RGB_INTRINSICS:
        value = QVariant::fromValue(m_rgbIntrinsics);
        break;
    case PARA_EXTRINSICS:
        value = QVariant::fromValue(m_extrinsics);
        break;
    default:
        qDebug() << "unknow camera para : " << paraId;
        break;
    }
}

void CSCamera::setUserParaPrivate(CAMERA_PARA_ID paraId, QVariant value)
{
    switch (paraId)
    {
    case PARA_DEPTH_STREAM_FORMAT:
        setDepthFormat((STREAM_FORMAT)value.toInt());
        return;
    case PARA_DEPTH_RESOLUTION:
        setDepthResolution(value.toSize());
        return;
    case PARA_DEPTH_FILTER:
        setDepthFilterValue(value.toInt());
        return;
    case PARA_DEPTH_FILTER_TYPE:
        if (m_filterType == value.toInt())
        {
            return;
        }
        m_filterType = value.toInt();
        break;
    case PARA_DEPTH_FILL_HOLE:
        if (m_fillHole == value.toBool())
        {
            return;
        }

        m_fillHole = value.toBool();
        break;
    case PARA_RGB_STREAM_FORMAT:
        setRgbFormat((STREAM_FORMAT)value.toInt());
        return;
    case PARA_RGB_RESOLUTION:
        setRgbResolution(value.toSize());
        return; 
    default:
        qDebug() << "unknow camera para : " << paraId;
        break;
    }

    emit cameraParaUpdated(paraId, value);
}

void CSCamera::getUserParaRangePrivate(CAMERA_PARA_ID paraId, QVariant& min, QVariant& max, QVariant& step)
{
    switch (paraId)
    {
    case PARA_DEPTH_RANGE:
        min = DEPTH_RANGE_LIMIT.min;
        max = DEPTH_RANGE_LIMIT.max;
        step = 1;
        break;
    case PARA_DEPTH_FILTER:
        min = FILTER_RANGE_MAP[m_filterType].min;
        max = FILTER_RANGE_MAP[m_filterType].max;
        step = 1;
        break;
    default:
        //qDebug() << "range does not exist, para : " << paraId;
        break;
    }
}

void CSCamera::getCameraPara(CAMERA_PARA_ID paraId, QVariant& value)
{
    if (CAMERA_PROPERTY_MAP.contains(paraId))
    {
        getPropertyPrivate(paraId, value);
        return;
    }

    if (CAMERA_EXTENSION_PROPERTY_MAP.contains(paraId))
    {
        getExtensionPropertyPrivate(paraId, value);
        return;
    }

    getUserParaPrivate(paraId, value);
}

void CSCamera::setCameraPara(CAMERA_PARA_ID paraId, QVariant value)
{
    if (CAMERA_PROPERTY_MAP.contains(paraId))
    {
        setPropertyPrivate(paraId, value);

        onParaUpdatedDelay(paraId, 200);
    }
    else if (CAMERA_EXTENSION_PROPERTY_MAP.contains(paraId))
    {
        setExtensionPropertyPrivate(paraId, value);

        onParaUpdatedDelay(paraId, 200);
    }
    else
    {
        setUserParaPrivate(paraId, value);
    }
    
    // parameter linkage response
    onParaLinkResponse(paraId, value);
}

void CSCamera::onParaLinkResponse(CAMERA_PARA_ID paraId, const QVariant& value)
{
    switch (paraId)
    {
    case PARA_DEPTH_FILTER_TYPE:
        emit cameraParaRangeUpdated(PARA_DEPTH_FILTER);
        setDepthFilterValue(FILTER_RANGE_MAP[m_filterType].min);
        break;
    case PARA_DEPTH_HDR_MODE: 
        {
            // close HDR
            if (value.toInt() == HDR_MODE_CLOSE)
            {
                restoreExposureGain();
            }
            else 
            {
                setCameraPara(PARA_DEPTH_HDR_LEVEL, m_hdrTimes);
            }
            break;
        }
    case PARA_DEPTH_HDR_LEVEL:
        if (m_hdrMode == HDR_MODE_MANUAL)
        {
            QVariant value;
            getCameraPara(PARA_DEPTH_HDR_SETTINGS, value);
            setCameraPara(PARA_DEPTH_HDR_SETTINGS, value);
        }
        else 
        {
            onParaUpdatedDelay(PARA_DEPTH_HDR_SETTINGS, 7000);
        } 
        break;
    case PARA_TRIGGER_MODE:
        onTriggerModeChanged(value.toInt() == TRIGGER_MODE_SOFTWAER);
        break;
    case PARA_CAMERA_IP:
    {
        // update cameraInfo when ip changed
        QVariant ip;
        getCameraPara(PARA_CAMERA_IP, ip);

        CameraIpSetting cameraIp = ip.value<CameraIpSetting>();
        QString ipStr = QString("%1.%2.%3.%4").arg(cameraIp.ipBytes[0]).arg(cameraIp.ipBytes[1]).arg(cameraIp.ipBytes[2]).arg(cameraIp.ipBytes[3]);
        strncpy(m_cameraInfo.cameraInfo.uniqueId, ipStr.toStdString().c_str(), sizeof(m_cameraInfo.cameraInfo.uniqueId));
        break;
    }
    default:
        break;
    }
}

void CSCamera::onParaUpdated(int paraId)
{
    CAMERA_PARA_ID cameraParaId = (CAMERA_PARA_ID)paraId;
    QVariant value;
    getCameraPara(cameraParaId, value);
    emit cameraParaUpdated(cameraParaId, value);
}

// get para after delayMS millisecond
void CSCamera::onParaUpdatedDelay(CAMERA_PARA_ID paraId, int delayMS)
{
    QTimer::singleShot(delayMS, this, [=]() {
            emit updateParaSignal(paraId);
        });
}

void CSCamera::restoreExposureGain()
{
    QTimer::singleShot(3000, this, 
        [=]() {
            qInfo() << "close HDR, then restore exposure : " << m_cachedDepthExposure << ", gain : " << m_cachedDepthGain;
            setPropertyPrivate(PARA_DEPTH_EXPOSURE, m_cachedDepthExposure);
            setPropertyPrivate(PARA_DEPTH_GAIN, m_cachedDepthGain);
        });
}

void CSCamera::getCameraParaRange(CAMERA_PARA_ID paraId, QVariant& min, QVariant& max, QVariant& step)
{
    if (CAMERA_PROPERTY_MAP.contains(paraId))
    {
        //skip some property
        if (paraId == PARA_RGB_AUTO_EXPOSURE || paraId == PARA_RGB_AUTO_WHITE_BALANCE)
        {
            return;
        }

        getPropertyRangePrivate(paraId, min, max, step);
        return;
    }

    if (CAMERA_EXTENSION_PROPERTY_MAP.contains(paraId))
    {
        getExtensionPropertyRangePrivate(paraId, min, max, step);
        return;
    }

    getUserParaRangePrivate(paraId, min, max, step);
}

void CSCamera::getCameraParaItems(CAMERA_PARA_ID paraId, QList<QPair<QString, QVariant>>& list)
{
    switch (paraId)
    {
    case PARA_DEPTH_STREAM_FORMAT:
        getFromats(STREAM_TYPE_DEPTH, list);
        break;
    case PARA_DEPTH_RESOLUTION:
        getResolutions(STREAM_TYPE_DEPTH, list);
        break;
    case PARA_DEPTH_AUTO_EXPOSURE:
        getAutoExposureModes(list);
        break;
    case PARA_DEPTH_FILTER_TYPE:
        getFilterTypes(list);
        break;
    case PARA_RGB_STREAM_FORMAT:
        getFromats(STREAM_TYPE_RGB, list);
        break;
    case PARA_RGB_RESOLUTION:
        getResolutions(STREAM_TYPE_RGB, list);
        break;
    case PARA_DEPTH_HDR_MODE:
        getHdrModes(list);
        break;
    case PARA_DEPTH_HDR_LEVEL:
        getHdrLevels(list);
        break;
    case PARA_DEPTH_GAIN:
    case PARA_RGB_GAIN:
        getGains(paraId, list);
        break;
    default:
        break;
    }
}

void CSCamera::getPropertyPrivate(CAMERA_PARA_ID paraId, QVariant& value)
{
    if (getCameraState() != CAMERA_STARTED_STREAM)
    {
        qWarning() << "get property failed, stream not started.";
        return;
    }

    float v = 0;

    STREAM_TYPE  streamType = (STREAM_TYPE)CAMERA_PROPERTY_MAP[paraId].type;
    PROPERTY_TYPE propertyType = (PROPERTY_TYPE)CAMERA_PROPERTY_MAP[paraId].id;

    ERROR_CODE ret = m_cameraPtr->getProperty(streamType, propertyType, v);
    if (ret != SUCCESS)
    {
        qWarning() << "get camera property failed, paraId:" << paraId << ", ret:" << ret;
        Q_ASSERT(false);
    }
    value = v;

#ifdef  _CS_DEBUG_
    qDebug() << "getPropertyPrivate, paraId : " << metaEnum.valueToKey(paraId) << ", v  = " << v;
#endif
}

void CSCamera::setPropertyPrivate(CAMERA_PARA_ID paraId, QVariant value)
{
#ifdef  _CS_DEBUG_
    qDebug() << "setPropertyPrivate, paraId : " << metaEnum.valueToKey(paraId) << ", value  = " << value.toFloat();
#endif

    STREAM_TYPE  streamType = (STREAM_TYPE)CAMERA_PROPERTY_MAP[paraId].type;
    PROPERTY_TYPE propertyType = (PROPERTY_TYPE)CAMERA_PROPERTY_MAP[paraId].id;

    float valueF = value.toFloat();

    // set frame time before set depth exposure
    if (paraId == PARA_DEPTH_EXPOSURE)
    {
        updateFrametime(valueF);
    }

    ERROR_CODE ret = m_cameraPtr->setProperty(streamType, propertyType, valueF);

    if (ret != SUCCESS)
    {
        qWarning() << "set the camera property failed, paraId:" << paraId << ",ret:" << ret << ",value:" << valueF;
        Q_ASSERT(false);
    }
}

void CSCamera::getPropertyRangePrivate(CAMERA_PARA_ID paraId, QVariant& min, QVariant& max, QVariant& step)
{
    float minf = 0, maxf = 0, stepf = 1;

    STREAM_TYPE  streamType = (STREAM_TYPE)CAMERA_PROPERTY_MAP[paraId].type;
    PROPERTY_TYPE propertyType = (PROPERTY_TYPE)CAMERA_PROPERTY_MAP[paraId].id;

    ERROR_CODE ret = m_cameraPtr->getPropertyRange(streamType, propertyType, minf, maxf, stepf);
    if (ret != SUCCESS)
    {
        qWarning() << "get camera property range failed, paraId:" << paraId << ",ret:" << ret;
        Q_ASSERT(false);
        return;
    }

    min = minf;
    max = maxf;
    step = stepf;
}

void CSCamera::getExtensionPropertyPrivate(CAMERA_PARA_ID paraId, QVariant& value)
{
    PROPERTY_TYPE_EXTENSION type = (PROPERTY_TYPE_EXTENSION)CAMERA_EXTENSION_PROPERTY_MAP[paraId];

    PropertyExtension propExt;
    ERROR_CODE ret = m_cameraPtr->getPropertyExtension(type, propExt);
    if (ret != SUCCESS)
    {
        qWarning() << "get the camera extension property failed, paraId:" << paraId << ",ret:" << ret;
        Q_ASSERT(false);
        return;
    }

    switch (type)
    {
    case PROPERTY_EXT_AUTO_EXPOSURE_MODE:
        value = (int)propExt.autoExposureMode;
        break;
    case PROPERTY_EXT_CONTRAST_MIN:
        value = (int)propExt.algorithmContrast;
        break;
    case PROPERTY_EXT_HDR_MODE: 
        {
            getHdrMode((int)propExt.hdrMode);
            value = m_hdrMode;
        }
        break;
    case PROPERTY_EXT_HDR_EXPOSURE:
        if (m_hdrMode != HDR_MODE_CLOSE)
        {
            if (m_hdrMode == HDR_MODE_MANUAL)
            {
                //add new settings
                for (int i = propExt.hdrExposureSetting.count; i < m_hdrTimes; i++)
                {
                    HdrExposureParam param;
                    param.gain = 1;
                    param.exposure = 7000;

                    propExt.hdrExposureSetting.param[i] = param;
                }

                propExt.hdrExposureSetting.count = m_hdrTimes;
            }

            value = QVariant::fromValue(propExt.hdrExposureSetting);  
        }   
        break;
    case PROPERTY_EXT_HDR_SCALE_SETTING: 
        {
            HdrScaleSetting scaleSetting = propExt.hdrScaleSetting;
            getHdrTimes(scaleSetting);
            value = m_hdrTimes;
            break;
        }
    case PROPERTY_EXT_DEPTH_SCALE:
        value = propExt.depthScale;
        break;  
    case PROPERTY_EXT_DEPTH_ROI: 
    {
        QRectF roiRect;
        roiRect.setLeft(propExt.depthRoi.left * 1.0 / 100);
        roiRect.setTop(propExt.depthRoi.top * 1.0 / 100);
        roiRect.setRight(propExt.depthRoi.right * 1.0 / 100);
        roiRect.setBottom(propExt.depthRoi.bottom * 1.0 / 100);
        value = roiRect;
        break;
    }
    case PROPERTY_EXT_DEPTH_RANGE:
        value = QVariant::fromValue(QPair<float, float>{ (float)propExt.depthRange.min, (float)propExt.depthRange.max});
        break;
    case PROPERTY_EXT_TRIGGER_MODE:
        value = (int)propExt.triggerMode;
        break;
    case PROPERTY_EXT_CAMERA_IP:
    {
        value = QVariant::fromValue(propExt.cameraIp);
        break;
    }
    case PROPERTY_EXT_EXPOSURE_TIME_RGB:
        value = propExt.uiExposureTime;
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}

void CSCamera::setExtensionPropertyPrivate(CAMERA_PARA_ID paraId, QVariant value)
{
    PROPERTY_TYPE_EXTENSION type = (PROPERTY_TYPE_EXTENSION)CAMERA_EXTENSION_PROPERTY_MAP[paraId];

    PropertyExtension propExt;
    switch (type)
    {
    case PROPERTY_EXT_AUTO_EXPOSURE_MODE:
        propExt.autoExposureMode = (AUTO_EXPOSURE_MODE)value.toInt();
        break;
    case PROPERTY_EXT_CONTRAST_MIN:
        propExt.algorithmContrast = value.toInt();
        break;
    case PROPERTY_EXT_HDR_MODE:
    {
        // cached exposure and gain
        if (m_hdrMode == HDR_MODE_CLOSE && value.toInt() != HDR_MODE_CLOSE)
        {
            QVariant tmp;
            getCameraPara(PARA_DEPTH_EXPOSURE, tmp);
            m_cachedDepthExposure = qRound(tmp.toFloat());

            getCameraPara(PARA_DEPTH_GAIN, tmp);
            m_cachedDepthGain = qRound(tmp.toFloat());

            qDebug() << "HDR is close, cached depth exposure : " << m_cachedDepthExposure << ", gain : " << m_cachedDepthGain;
        }
        // update HDR mode
        m_hdrMode = (CAMERA_HDR_MODE)value.toInt();
        int autoHdrMode = getAutoHdrMode(m_hdrMode);
        propExt.hdrMode = (HDR_MODE)autoHdrMode;
        break;
    }
    case PROPERTY_EXT_HDR_EXPOSURE:
        propExt.hdrExposureSetting = value.value<HdrExposureSetting>();
        break;
    case PROPERTY_EXT_HDR_SCALE_SETTING:
    {
        HdrScaleSetting settings;
        setHdrTimes(settings, value.toInt());
        propExt.hdrScaleSetting = settings;
        break;
    }
    case PROPERTY_EXT_DEPTH_ROI:
    {
        QRectF roiRect = value.toRectF();
        propExt.depthRoi.top = roiRect.top() * 100;
        propExt.depthRoi.left = roiRect.left() * 100;
        propExt.depthRoi.right = roiRect.right() * 100;
        propExt.depthRoi.bottom = roiRect.bottom() * 100;
        break;
    }
    case PROPERTY_EXT_DEPTH_RANGE:
    {
        auto range = value.value<QPair<float, float>>();
        propExt.depthRange.min = qRound(range.first);
        propExt.depthRange.max = qRound(range.second);
        break;
    }
    case PROPERTY_EXT_TRIGGER_MODE:
        propExt.triggerMode = (TRIGGER_MODE)value.toInt();
        break;
    case PROPERTY_EXT_CAMERA_IP:
        propExt.cameraIp = value.value<CameraIpSetting>();
        break;
    case PROPERTY_EXT_EXPOSURE_TIME_RGB:
        propExt.uiExposureTime = value.toFloat();
        break;
    default:
        Q_ASSERT(false);
        return;
    }

    ERROR_CODE ret = m_cameraPtr->setPropertyExtension(type, propExt);
    if (ret != SUCCESS)
    {
        qWarning() << "set the camera extension property failed," << "paraId:" << paraId << ",ret:" << ret;
        Q_ASSERT(false);
    }
}

void CSCamera::getExtensionPropertyRangePrivate(CAMERA_PARA_ID paraId, QVariant& min, QVariant& max, QVariant& step)
{
    switch (paraId)
    {
    case PARA_DEPTH_THRESHOLD:
        min = 0;
        max = 40;
        step = 1;
        break;
    case PARA_DEPTH_RANGE:
        min = DEPTH_RANGE_LIMIT.min;
        max = DEPTH_RANGE_LIMIT.max;
        step = 1;
        break;
    case PARA_DEPTH_HDR_LEVEL:
        min = CAMERA_HDR_LEVEL_RANGE.min;
        max = CAMERA_HDR_LEVEL_RANGE.max;
        step = 1;
        break;
    case PARA_RGB_EXPOSURE:
    {
        PropertyExtension propExt;
        ERROR_CODE ret = m_cameraPtr->getPropertyExtension(PROPERTY_EXT_EXPOSURE_TIME_RANGE_RGB, propExt);
        if (ret != SUCCESS)
        {
            qWarning() << "get the camera extension property failed, type:" << PROPERTY_EXT_EXPOSURE_TIME_RANGE_RGB << ",ret:" << ret;
            Q_ASSERT(false);
        }
        else 
        {
            min = propExt.objVRange_.fMin_;
            max = propExt.objVRange_.fMax_;
            step = propExt.objVRange_.fStep_;
        }
        break;
    }
    default:
        break;
    }
}

// get supported formats currently
void CSCamera::getFromats(STREAM_TYPE sType, QList<QPair<QString, QVariant>>& formats) const
{
    ERROR_CODE ret;
    std::vector<StreamInfo> streamInfos;

    ret = m_cameraPtr->getStreamInfos(sType, streamInfos);
    if (ret != SUCCESS)
    {
        qWarning("camera get stream info failed(%d)!\n", ret);
        return;
    }

    for (const auto& info : streamInfos)
    {
        QPair<QString, QVariant> pair(STREAM_FORMAT_MAP[info.format], info.format);
        if (!formats.contains(pair))
        {
            formats.push_back(pair);
        }
    }
}

// get supported resolutions currently
void CSCamera::getResolutions(STREAM_TYPE sType, QList<QPair<QString, QVariant>>& resolutions) const
{
    ERROR_CODE ret;
    std::vector<StreamInfo> streamInfos;

    STREAM_FORMAT dstFormat = (sType == STREAM_TYPE_DEPTH) ? m_depthFormat : m_rgbFormat;

    ret = m_cameraPtr->getStreamInfos(sType, streamInfos);
    if (ret != SUCCESS)
    {
        qInfo("camera get stream info failed(%d)!\n", ret);
        return;
    }

    for (const auto& info : streamInfos)
    {
        if (info.format != dstFormat)
        {
            continue;
        }

        QString resStr = QString("%1x%2").arg(info.width).arg(info.height);
        QSize size(info.width, info.height);

        QPair<QString, QVariant> pair(resStr, size);

        if (!resolutions.contains(pair))
        {
            resolutions.push_back(pair);
        }
    }
}

void CSCamera::getAutoExposureModes(QList<QPair<QString, QVariant>>& list) const
{
    list.push_back({ tr(AUTO_EXPOSURE_MODE_MAP[AUTO_EXPOSURE_MODE_CLOSE]),          AUTO_EXPOSURE_MODE_CLOSE } );
    list.push_back({ tr(AUTO_EXPOSURE_MODE_MAP[AUTO_EXPOSURE_MODE_FIX_FRAMETIME]),  AUTO_EXPOSURE_MODE_FIX_FRAMETIME });
    list.push_back({ tr(AUTO_EXPOSURE_MODE_MAP[AUTO_EXPOSURE_MODE_HIGH_QUALITY]),   AUTO_EXPOSURE_MODE_HIGH_QUALITY });
    list.push_back({ tr(AUTO_EXPOSURE_MODE_MAP[AUTO_EXPOSURE_MODE_FORE_GROUND]),    AUTO_EXPOSURE_MODE_FORE_GROUND });
}

void CSCamera::getFilterTypes(QList<QPair<QString, QVariant>>& list) const
{
    list.push_back({ tr(FILTER_TYPE_MAP[FILTER_CLOSE]),    FILTER_CLOSE });
    list.push_back({ tr(FILTER_TYPE_MAP[FILTER_SMOOTH]),   FILTER_SMOOTH });
    list.push_back({ tr(FILTER_TYPE_MAP[FILTER_MEDIAN]),   FILTER_MEDIAN });
    list.push_back({ tr(FILTER_TYPE_MAP[FILTER_TDSMOOTH]), FILTER_TDSMOOTH });
}

void CSCamera::getHdrModes(QList<QPair<QString, QVariant>>& list) const
{
    list.push_back({ tr(CAMERA_HDR_MAP[HDR_MODE_CLOSE]),    HDR_MODE_CLOSE });
    list.push_back({ tr(CAMERA_HDR_MAP[HDR_MODE_SHINE]),    HDR_MODE_SHINE });
    list.push_back({ tr(CAMERA_HDR_MAP[HDR_MODE_DARK]),     HDR_MODE_DARK });
    list.push_back({ tr(CAMERA_HDR_MAP[HDR_MODE_BOTH]),     HDR_MODE_BOTH });
    list.push_back({ tr(CAMERA_HDR_MAP[HDR_MODE_MANUAL]),   HDR_MODE_MANUAL });
}

void CSCamera::getHdrLevels(QList<QPair<QString, QVariant>>& list) const
{
    for (int i = CAMERA_HDR_LEVEL_RANGE.min; i <= CAMERA_HDR_LEVEL_RANGE.max; i++)
    {
        list.push_back({ QString::number(i), i });
    }
}

void CSCamera::getGains(CAMERA_PARA_ID paraId, QList<QPair<QString, QVariant>>& list)
{
    QVariant min, max, step;
    getCameraParaRange(paraId, min, max, step);

    int minV = min.toInt();
    int maxV = max.toInt();

    for (int i = minV; i <= maxV; i++)
    {
        list.push_back({ QString::number(i), i });
    }
}

void CSCamera::setDepthFormat(STREAM_FORMAT format)
{
    if (format == m_depthFormat)
    {
        return;
    }

    updateStreamType();

    // 1. update format
    m_depthFormat = format;
    emit cameraParaUpdated(PARA_DEPTH_STREAM_FORMAT, (int)m_depthFormat);

    // 2. update resolution items
    QList<QPair<QString, QVariant>> resolutions;
    getResolutions(STREAM_TYPE_DEPTH, resolutions);
    
    if(resolutions.size() <= 0)
    {
        qWarning() << "Error, the depth resolutions is empty";
        return;
    }
    
    emit cameraParaItemsUpdated(PARA_DEPTH_RESOLUTION);

    // 3. update resolution
    bool findRes = false;
    for (auto pair : resolutions)
    {
        if (pair.second.toSize() == m_depthResolution)
        {
            findRes = true;
            break;
        }
    }
    
    if (!findRes)
    {
        m_depthResolution = resolutions.first().second.toSize();
    }

    emit cameraParaUpdated(PARA_DEPTH_RESOLUTION, m_depthResolution);
}

void CSCamera::setDepthResolution(QSize res)
{
    if (res == m_depthResolution)
    {
        return;
    }
    
    //  notify
    updateStreamType();

    // update resolution
    m_depthResolution = res;
    emit cameraParaUpdated(PARA_DEPTH_RESOLUTION, m_depthResolution);
}

void CSCamera::setRgbFormat(STREAM_FORMAT format)
{
    if (format == m_rgbFormat)
    {
        return;
    }

    // notify
    updateStreamType();

    // 1. update format
    m_rgbFormat = format;
    emit cameraParaUpdated(PARA_RGB_STREAM_FORMAT, (int)m_rgbFormat);

    // 2. update resolution items
    QList<QPair<QString, QVariant>> resolutions;
    getResolutions(STREAM_TYPE_RGB, resolutions);
    if(resolutions.size() <= 0)
    {
        qWarning() << "Error, the RGB resolutions is empty";
        return;
    }

    emit cameraParaItemsUpdated(PARA_RGB_RESOLUTION);

    // 3. update resolution
    bool findRes = false;
    for (auto pair : resolutions)
    {
        if (pair.second.toSize() == m_rgbResolution)
        {
            findRes = true;
        }
    }

    if (!findRes)
    {
        m_rgbResolution = resolutions.first().second.toSize();
    }

    emit cameraParaUpdated(PARA_RGB_RESOLUTION, m_rgbResolution);
}

void CSCamera::setRgbResolution(QSize res)
{
    if (res == m_rgbResolution)
    {
        return;
    }

    //  notify
    updateStreamType();

    // update resolution
    m_rgbResolution = res;
    emit cameraParaUpdated(PARA_RGB_RESOLUTION, m_rgbResolution);
}

void CSCamera::setDepthFilterValue(int value)
{
    const int min = FILTER_RANGE_MAP[m_filterType].min;
    const int max = FILTER_RANGE_MAP[m_filterType].max;

    value = (value < min) ? min : value;
    value = (value > max) ? max : value;
    
    m_filterValue = value;

    emit cameraParaUpdated(PARA_DEPTH_FILTER, m_filterValue);
}

// update frame time before set exposure
void CSCamera::updateFrametime(float exposure)
{
    const float moreTime = 2000; // us
    const float frameTime = exposure + moreTime;

    setCameraPara(PARA_DEPTH_FRAMETIME, frameTime);
}

void CSCamera::getHdrMode(int value)
{
    if (m_hdrMode == HDR_MODE_MANUAL)
    {
        return;
    }

    HDR_MODE hdrM = (HDR_MODE)value;
    switch (hdrM)
    {
    case HDR_MODE_OFF:
        m_hdrMode = HDR_MODE_CLOSE;
        break;
    case HDR_MODE_HIGH_RELECT:
        m_hdrMode = HDR_MODE_SHINE;
        break;
    case HDR_MODE_LOW_RELECT:
        m_hdrMode = HDR_MODE_DARK;
        break;
    case HDR_MODE_ALL_RELECT:
        m_hdrMode = HDR_MODE_BOTH;
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}

int CSCamera::getAutoHdrMode(int mode)
{
    CAMERA_HDR_MODE hdrM = (CAMERA_HDR_MODE)mode;

    int autoHdrMode = HDR_MODE_OFF;
    switch (hdrM)
    {
    case HDR_MODE_CLOSE:
        autoHdrMode = HDR_MODE_OFF;
        break;
    case HDR_MODE_SHINE:
        autoHdrMode = HDR_MODE_HIGH_RELECT;
        break;
    case HDR_MODE_DARK:
        autoHdrMode = HDR_MODE_LOW_RELECT;
        break;
    case HDR_MODE_BOTH:
    case HDR_MODE_MANUAL:
        autoHdrMode = HDR_MODE_ALL_RELECT;
        break; 
    default:
        Q_ASSERT(false);
        break;
    }

    return autoHdrMode;
}

void CSCamera::getHdrTimes(const HdrScaleSetting& settings)
{
    switch (m_hdrMode)
    {
    case HDR_MODE_SHINE:
        m_hdrTimes = settings.highReflectModeCount + 1;
        break;
    case HDR_MODE_DARK:
        m_hdrTimes = settings.lowReflectModeCount + 1;
        break;
    case HDR_MODE_CLOSE:
    case HDR_MODE_BOTH:
    case HDR_MODE_MANUAL:
        m_hdrTimes = (settings.lowReflectModeCount + settings.highReflectModeCount + 1);
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}

void CSCamera::setHdrTimes(HdrScaleSetting& settings, int times)
{
    times = (times < 2) ? 2 : times;
    times = (times > 8) ? 8 : times;

    const int scale = HDR_SCALE_DEFAULT;
    switch (m_hdrMode)
    {
    case HDR_MODE_SHINE:
        settings = { (unsigned int)(times - 1), scale, 0, scale };
        break;
    case HDR_MODE_DARK:
        settings = { 0, scale, (unsigned int)(times - 1), scale };
        break;
    case HDR_MODE_CLOSE:
    case HDR_MODE_BOTH:
    case HDR_MODE_MANUAL:
    {
        unsigned int time_high = (times - 1) / 2;
        unsigned int time_low = times - 1 - time_high;
        settings = { time_high, scale, time_low, scale };
        break;
    }
    default:
        Q_ASSERT(false);
        break;
    }
}

CSCameraInfo CSCamera::getCameraInfo() const
{
    return m_cameraInfo;
}

int CSCamera::getCameraState() const
{
    m_lock.lockForRead();
    int result = m_cameraState;
    m_lock.unlock();

    return result;
}

void CSCamera::setCameraState(CAMERA_STATE state)
{
    m_lock.lockForWrite();
    m_cameraState = state;
    m_lock.unlock();

    emit cameraStateChanged(m_cameraState);
}

void CSCamera::updateStreamType()
{
    if (getCameraState() == CAMERA_PAUSED_STREAM)
    {
        setCameraState(CAMERA_STOPPING_STREAM);
        stopStream();
    }
}

void CSCamera::onTriggerModeChanged(bool isSoftTrigger)
{
    if (isSoftTrigger)
    {
        stopStreamThread();
    }
    else 
    {
        startStreamThread();
    }
}

void CSCamera::stopStreamThread()
{
    if (m_streamThread->isRunning())
    {
        qInfo() << "stop stream thread";
        m_streamThread->requestInterruption();
        m_streamThread->wait();
    }
}

void CSCamera::startStreamThread()
{
    qInfo() << "start stream thread";

    if (!m_streamThread->isRunning())
    {
        qInfo() << "stream thread is not running, start stream thread";
        m_streamThread->start();
    }
}

//judge camera is network connect or not
bool CSCamera::isNetworkConnect(QString uuid)
{
    return uuid.contains(".");
}