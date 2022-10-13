/*******************************************************************************
* This file is part of the 3DViewer
*
* Copyright 2022-2026 (C) Revopoint3D AS
* All rights reserved.
*
* Revopoint3D Software License, v1.0
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistribution of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistribution in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of Revopoint3D AS nor the names of its contributors may be used
* to endorse or promote products derived from this software without specific
* prior written permission.
*
* 4. This software, with or without modification, must not be used with any
* other 3D camera than from Revopoint3D AS.
*
* 5. Any software provided in binary form under this license must not be
* reverse engineered, decompiled, modified and/or disassembled.
*
* THIS SOFTWARE IS PROVIDED BY REVOPOINT3D AS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL REVOPOINT3D AS OR CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* Info:  https://www.revopoint3d.com
******************************************************************************/

#ifndef _CS_CSCAMERA_H
#define _CS_CSCAMERA_H

#include <QObject>
#include <QVector>
#include <QRectF>
#include <QThread>
#include <QMetaEnum>
#include <QReadWriteLock>

#include <hpp/System.hpp>
#include <hpp/Camera.hpp>

#include "icscamera.h"
#include "cstypes.h"

#define GET_FRAME_TIME_OUT 10         //ms

namespace cs {

using namespace parameter;
class CSCamera : public ICSCamera
{
    Q_OBJECT
public:
    CSCamera();
    ~CSCamera();
    static int setCameraChangeCallback(CameraChangeCallback callback, void* userData);
    static int setCameraAlarmCallback(CameraAlarmCallback callback, void* userData);
    static void setSdkLogPath(QString path);
    static void queryCameras(std::vector<CameraInfo>& cameras);

    void getCameraPara(CAMERA_PARA_ID paraId, QVariant& value) override;
    void setCameraPara(CAMERA_PARA_ID paraId, QVariant value) override;
    void getCameraParaRange(CAMERA_PARA_ID paraId, QVariant& min, QVariant& max, QVariant& step) override;
    void getCameraParaItems(CAMERA_PARA_ID paraId, QList<QPair<QString, QVariant>>& list) override;

    bool connectCamera(CameraInfo info);
    bool disconnectCamera() override;
    bool reconnectCamera() override;
    bool restartCamera() override;

    bool startStream() override;
    bool stopStream() override;
    bool restartStream() override;
    bool pauseStream() override;
    bool resumeStream() override;
    bool softTrigger() override;

    void onGetFrame(int timeout = GET_FRAME_TIME_OUT);
    bool onProcessFrame(STREAM_DATA_TYPE streamDataType, const IFramePtr& frame, FrameData& frameData);
    void setCameraThread(QThread*  thread);

    CSCameraInfo getCameraInfo() const override;
    int getCameraState() const override;
private:
    bool startRgbStream();
    bool startDepthStream();
    bool stopDepthStream();
    bool stopRgbStream();
    void doDisconnectCamera();
private:
    void setCameraState(CAMERA_STATE state);
    void initDefaultStreamInfo();
    void initCameraInfo();
    StreamInfo getDepthStreamInfo();
    StreamInfo getRgbStreamInfo();

    void getFromats(STREAM_TYPE sType, QList<QPair<QString, QVariant>>&) const;
    void getResolutions(STREAM_TYPE sType, QList<QPair<QString, QVariant>>&) const;
    void getAutoExposureModes(QList<QPair<QString, QVariant>>&) const;
    void getFilterTypes(QList<QPair<QString, QVariant>>&) const;
    void getHdrModes(QList<QPair<QString, QVariant>>&) const;
    void getHdrLevels(QList<QPair<QString, QVariant>>&) const;

    void setDepthFormat(STREAM_FORMAT format);
    void setDepthResolution(QSize res);
    void setRgbFormat(STREAM_FORMAT format);
    void setRgbResolution(QSize res);
    void getHdrMode(int value);
    int getAutoHdrMode(int mode);
    void getHdrTimes(const HdrScaleSetting& settings);
    void setHdrTimes(HdrScaleSetting& settings, int times);
    void updateFrametime(float exposure);

    void setDepthFilterValue(int value);
    void updateStreamType();
    void onTriggerModeChanged(bool isSoftTrigger);

    void stopStreamThread();
    void startStreamThread();

    bool isNetworkConnect(QString uuid);
signals:
    void updateParaSignal(int paraId);
private slots:
    void onParaLinkResponse(CAMERA_PARA_ID paraId, const QVariant& value);
    void onParaUpdated(int paraId);
    void onParaUpdatedDelay(CAMERA_PARA_ID paraId, int delayMS);
    void onStreamStarted();
private:
    void getUserParaPrivate(CAMERA_PARA_ID paraId, QVariant& value);
    void setUserParaPrivate(CAMERA_PARA_ID paraId, QVariant value);
    void getUserParaRangePrivate(CAMERA_PARA_ID paraId, QVariant& min, QVariant& max, QVariant& step);

    void getPropertyPrivate(CAMERA_PARA_ID paraId, QVariant& value);
    void setPropertyPrivate(CAMERA_PARA_ID paraId, QVariant value);
    void getPropertyRangePrivate(CAMERA_PARA_ID paraId, QVariant& min, QVariant& max, QVariant& step);

    void getExtensionPropertyPrivate(CAMERA_PARA_ID paraId, QVariant& value);
    void setExtensionPropertyPrivate(CAMERA_PARA_ID paraId, QVariant value);
    void getExtensionPropertyRangePrivate(CAMERA_PARA_ID paraId, QVariant& min, QVariant& max, QVariant& step);

private:
    class StreamThread : public QThread
    {
    public:
        StreamThread(CSCamera& camera);
        ~StreamThread();
        void run() override;
    private:
        CSCamera& camera;
    };

private:
    static const QMap<int, const char*> AUTO_EXPOSURE_MODE_MAP;
    static const QMap<int, const char*> FILTER_TYPE_MAP;
    static const QMap<CAMERA_HDR_MODE, const char*> CAMERA_HDR_MAP;
    static QMetaEnum metaEnum;
private:
    CAMERA_STATE cameraState;
    ICameraPtr cameraPtr;
    CSCameraInfo cameraInfo;

    STREAM_FORMAT depthFormat;
    STREAM_FORMAT rgbFormat;
    QSize depthResolution;
    QSize rgbResolution;
    TRIGGER_MODE triggerMode;

    int filterValue;
    int filterType;
    bool fillHole;

    bool hasIrStream;
    bool hasDepthStream;

    bool isRgbStreamSup;
    bool isDepthStreamSup;

    //hdr
    CAMERA_HDR_MODE hdrMode;
    int hdrTimes;
    HdrExposureSetting manualHdrSetting;
    //for restoring exposure and gain when cloing HDR
    int cachedDepthExposure;
    int cachedDepthGain;

    //roi
    QRectF roiRectF;

    Intrinsics depthIntrinsics;
    Intrinsics rgbIntrinsics;
    Extrinsics extrinsics;

    StreamThread* streamThread;
    friend StreamThread;

    QThread* cameraThread;
    mutable QReadWriteLock lock;
};
}

#endif // _CS_CSCAMERA_H
