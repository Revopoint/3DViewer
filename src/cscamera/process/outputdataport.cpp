#include "process/outputdataport.h"

OutputDataPort::OutputDataPort()
{

}

OutputDataPort::OutputDataPort(const FrameData& frameData)
    : frameData(frameData)
{

}

OutputDataPort::OutputDataPort(const OutputDataPort& other)
{
    outputData2DMap = other.outputData2DMap;
    pointCloud = other.pointCloud;
    frameData = other.frameData;
}

OutputDataPort::~OutputDataPort()
{
    outputData2DMap.clear();
}

bool OutputDataPort::isEmpty() const
{
    return !hasData(CAMERA_DTA_POINT_CLOUD) && outputData2DMap.empty();
}

bool OutputDataPort::hasData(CS_CAMERA_DATA_TYPE dataType) const
{
    if (dataType == CAMERA_DTA_POINT_CLOUD)
    {
        return pointCloud.size() > 0;
    }
    else 
    {
        return outputData2DMap.contains(dataType);
    }
}

cs::Pointcloud OutputDataPort::getPointCloud()
{
    return pointCloud;
}

OutputData2D OutputDataPort::getOutputData2D(CS_CAMERA_DATA_TYPE dataType)
{
    if (!hasData(dataType))
    {
        return OutputData2D();
    }

    return outputData2DMap[dataType];
}

QMap<CS_CAMERA_DATA_TYPE, OutputData2D> OutputDataPort::getOutputData2Ds()
{
    return outputData2DMap;
}

FrameData OutputDataPort::getFrameData()
{
    return frameData;
}

void OutputDataPort::setPointCloud(const cs::Pointcloud& pointCloud)
{
    this->pointCloud = pointCloud;
}

void OutputDataPort::addOutputData2D(const OutputData2D& outputData2D)
{
    CS_CAMERA_DATA_TYPE dataType = (CS_CAMERA_DATA_TYPE)outputData2D.info.cameraDataType;
    outputData2DMap[dataType] = outputData2D;
}

void OutputDataPort::addOutputData2D(const QVector<OutputData2D>& outputData2Ds)
{
    for (const auto& data : outputData2Ds)
    {
        addOutputData2D(data);
    }
}

void OutputDataPort::setFrameData(const FrameData& frameData)
{
    this->frameData = frameData;
}
