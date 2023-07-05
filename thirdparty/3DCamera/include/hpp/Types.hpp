 /*****************************************************************************
*  3DCamera SDK header
*
*
*  @file     Types.hpp
*  @brief    3DCamera sdk header
*
*  @version  1.0
*  @date     2019 / 08 / 17
*
*****************************************************************************/
#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct CS_SDK_VERSION_TAG
	{			
		const char*	version;		//版本号
		const char*	name;			//模块名称
		const char*	author;			//创建者
		const char*	date;			//创建日期
		const char*	desc;			//描述，每次修改都需要描述
	}CS_SDK_VERSION, PCS_SDK_VERSION;

	//ProbeType
	typedef enum
	{
		PT_START,
		PT_STOP
	}ProbeType;

	/**
	 * @~chinese
	 * @brief	IMU标定平面
	 * @~english
	 * @brief	IMU CalibrationPlane
	 */
	typedef enum {
		IMU_XY = 0,			//XY平面
		IMU_XZ = 1,			//XZ平面
		IMU_YZ = 2,			//YZ平面
	}IMU_CALIBRATTION_PLANE;

	/**
	 * @~chinese
	 * @brief	IMU标定状态
	 * @~english
	 * @brief	IMU Calibration state
	 */
	typedef enum {
		CALIBRATION_STATE_ING,			//未完成
		CALIBRATION_STATE_FIN			//已完成
	}IMU_CALIBRATTION_STATE;

	/**
	 * @~chinese
	 * @brief	LED标识
	 * @~english
	 * @brief	LED ID
	 */
	typedef enum {
		IR_LED,				//IR LED
		RGB_LED,			//RGB LED
		LASER_LED,			//激光器 LED
	}LED_ID;

	/**
	 * @~chinese
	 * @brief	LED控制类型
	 * @~english
	 * @brief	LED contrl type
	 */
	typedef enum {
		OFTEN_BRIGHT_LED,	//常亮
		TWINKLE_LED,		//闪烁
		ENABLE_LED,			//使能LED
		DISABLE_LED,		//失能LED
	}LED_CTRL_TYPE;

	//CameraType
	typedef enum
	{
		CAMERA_UNKNOW,				//unknow camera type
		CAMERA_HD,					//HD
		CAMERA_SURFACE,				//surface
		CAMERA_SENSE_PRO,			//sense pro
		CAMERA_POP_1,			    //pop1
		CAMERA_POP_2 = 10,		    //pop2
		CAMERA_MINI_NO_RGB,			//mini camera without rgb sensor
		CAMERA_MINI_NORMAL,			//
        CAMERA_TRACER_P1,           //tracer p1
		CAMERA_RANGE,				//Range
		CAMERA_TRACER_P2,           //tracer p2
		CAMERA_POP_3,               //pop3
	}CameraType;

#define FRAMERATE_ANY 0
#define CAMERA_SERIAL_MAX		32
#define FIRMWARE_VERSION_MAX	32
#define ALGORITHM_VERSION_MAX	32

#ifndef OUT
#define OUT
#endif

#ifndef IN
#define IN
#endif

typedef enum CAMERA_STATUS
{
	CS_NONE,			/**< @~chinese 无效的相机             @~english invalid camera*/
	CS_IDLE,			/**< @~chinese 空闲中             @~english in idle status*/
	CS_CONNECTED_BY_SDK,		/**< @~chinese 本SDK连接中             @~english current sdk connect to the camera.*/
	CS_CONNECTED_BY_OTHER,		/**< @~chinese 其他实例(进程)连接中             @~english other instance connect to the camera*/
}CAMERA_STATUS;

/** 
* @~chinese
* 枚举: 返回的错误码
* @~english
* enumeration: returned error code
**/
typedef enum ERROR_CODE
{
	SUCCESS = 0,					/**< @~chinese 成功             @~english success*/ 
	ERROR_PARAM,					/**< @~chinese 参数输入错误     @~english param input error*/
	ERROR_DEVICE_NOT_FOUND,			/**< @~chinese 未找到设备       @~english device not found*/
	ERROR_DEVICE_NOT_CONNECT,		/**< @~chinese 设备未连接       @~english device not connected*/
	ERROR_DEVICE_BUSY,				/**< @~chinese 设备忙           @~english device busy*/
	ERROR_STREAM_NOT_START = 5,			/**< @~chinese 流尚未打开       @~english stream not start*/
	ERROR_STREAM_BUSY,				/**< @~chinese 流已打开         @~english stream had started*/
	ERROR_FRAME_TIMEOUT,			/**< @~chinese 获取帧数据失败   @~english get frame failed*/
	ERROR_NOT_SUPPORT,				/**< @~chinese 尚不支持         @~english not support*/
	ERROR_PROPERTY_GET_FAILED,		/**< @~chinese 获取属性失败     @~english get property failed*/
	ERROR_PROPERTY_SET_FAILED=10,		/**< @~chinese 设置属性失败     @~english set property failed*/
	ERROR_HID_CHANNEL_ERROR,		/**< @~chinese hid 通道异常	     @~english hid channel error*/
	ERROR_HID_WRITE_ERROR,
	ERROR_HID_READ_ERROR,
	ERROR_UNKNOW
}ERROR_CODE;

/**
* @~chinese
* 枚举: 相机流类型
* @~english
* enumeration: stream type
**/
typedef enum STREAM_TYPE
{
	STREAM_TYPE_DEPTH	= 0, /**<@~chinese 深度流  @~english Depth camera stream */
    STREAM_TYPE_RGB		= 1, /**<@~chinese RGB流   @~english RGB camera stream */
	STREAM_TYPE_COUNT
}STREAM_TYPE;


/// \~chinese
/// \defgroup StreamFormat 数据流格式
/// \brief 深度流和RGB流所支持的所有格式
/// @{
/// \~english
/// \defgroup StreamFormat Stream format
/// \brief Format of depth stream and RGB stream
/// @{
/**
* @~chinese
* 枚举: 流数据格式
* @~english
* enumeration: stream format
**/
typedef enum STREAM_FORMAT
{
	STREAM_FORMAT_MJPG		= 0x00,		 /**< @~chinese RGB流的MJPG压缩的数据			
											  @~english MJPG compressed data*/ 
	STREAM_FORMAT_RGB8		= 0x01,		 /**< @~chinese RGB流的8位红,绿,蓝3通道数据			
											  @~english 8-bit red, green and blue channels*/ 
	STREAM_FORMAT_Z16		= 0x02,		 /**< @~chinese 深度流的深度图格式, 每一个深度值以unsigned short表示
											  @~english 16-bit unsigned short depth values. The depth in millimeters is equal to depth scale * pixel value. */ 
	STREAM_FORMAT_Z16Y8Y8	= 0x03,		 /**< @~chinese 深度流的深度图+红外图组合格式,	
														通过FRAME_DATA_FORMAT_Z16获得深度数据，
														通过FRAME_DATA_FORMAT_IR_LEFT获得左红外图, 
														通过FRAME_DATA_FORMAT_IR_RIGHT获得右红外图
											  @~english output depth map and infrared, 
														get depth map by FRAME_DATA_FORMAT_Z16
														get left infrared by FRAME_DATA_FORMAT_IR_LEFT,
														get right infrared by FRAME_DATA_FORMAT_IR_RIGHT*/ 
	STREAM_FORMAT_PAIR		= 0x04,		 /**< @~chinese 深度流的红外图格式，适用于双目相机
														通过FRAME_DATA_FORMAT_IR_LEFT获得左红外图, 
														通过FRAME_DATA_FORMAT_IR_RIGHT获得右红外图	
											  @~english output infrared，suitable for binocular camera
														get left infrared by FRAME_DATA_FORMAT_IR_LEFT,
														get right infrared by FRAME_DATA_FORMAT_IR_RIGHT*/ 
	STREAM_FORMAT_H264		= 0x05,		 /**< @~chinese RGB流的H264压缩的数据			
											  @~english H264 compressed data*/ 
	STREAM_FORMAT_I8DS      = 0x100,	 /**< @~chinese 降采样红外预览格式
														通过FRAME_DATA_FORMAT_IR_LEFT获得左红外图,
														通过FRAME_DATA_FORMAT_IR_RIGHT获得右红外图，
														通过FRAME_DATA_FORMAT_VCENTER_LEFT获得左红外图中光刀中心的y坐标数组,
														通过FRAME_DATA_FORMAT_VCENTER_RIGHT获得右红外图中光刀中心的y坐标数组，
														通过FRAME_DATA_FORMAT_LASER_WIDTH_LEFT 获得左红外图光刀宽度数组,
														通过FRAME_DATA_FORMAT_LASER_WIDTH_RIGHT获得右红外图光刀宽度数组
									    	  @~english output down sampled infrared for preview
														get left infrared by FRAME_DATA_FORMAT_IR_LEFT, 
														get right infrared by FRAME_DATA_FORMAT_IR_RIGHT,
														get left laser center by FRAME_DATA_FORMAT_VCENTER_LEFT,
														get right laser center by FRAME_DATA_FORMAT_VCENTER_RIGHT,
														get left laser width by FRAME_DATA_FORMAT_LASER_WIDTH_LEFT,
														get right laser width by FRAME_DATA_FORMAT_LASER_WIDTH_RIGHT*/
	STREAM_FORMAT_XZ32		= 0x101,	 /**< @~chinese 点云输出格式, 通过cs::Pointcloud::generatePointsFromXZ计算成点云		
											  @~english output point cloud, call cs::Pointcloud::generatePointsFromXZ to compute a point cloud*/
	STREAM_FORMAT_GRAY		= 0x102,	 /**< @~chinese 深度流的红外图格式，适用于单目相机
											  @~english output infrared，suitable for monocular cameras */ 
	STREAM_FORMAT_COUNT
}STREAM_FORMAT;

/**
* @~chinese
* 枚举: 帧数据格式，用于获取复合流数据中的指定格式数据起始地址
* @~english
* enumeration: format of frame data, used for get specified data in a composite frame
**/
typedef enum FRAME_DATA_FORMAT
{
	FRAME_DATA_FORMAT_Z16				= 0x00,		/**< @~chinese 深度流的深度图格式, 每一个深度值以unsigned short表示
														 @~english 16-bit unsigned short depth values. The depth in millimeters is equal to depth scale * pixel value. */ 
	FRAME_DATA_FORMAT_IR_LEFT			= 0x01,		/**< @~chinese 左红外图数据， 8-bit unsigned char表示一个灰度值				
														 @~english 8-bit unsigned char gray level of left infrared*/
	FRAME_DATA_FORMAT_IR_RIGHT			= 0x02,		/**< @~chinese 右红外图数据， unsigned char表示一个灰度值				
														 @~english 8-bit unsigned char gray level of right infrared*/
	FRAME_DATA_FORMAT_VCENTER_LEFT		= 0x03,		/**< @~chinese 左红外图光刀中心， unsigned char表示一个y坐标				
														 @~english 8-bit unsigned char y coordinate of left laser center*/
	FRAME_DATA_FORMAT_VCENTER_RIGHT		= 0x04,		/**< @~chinese 右红外图光刀中心， unsigned char表示一个y坐标				
														 @~english 8-bit unsigned char y coordinate of right laser center*/
	FRAME_DATA_FORMAT_LASER_WIDTH_LEFT	= 0x05,		/**< @~chinese 左红外图光刀宽度， unsigned char表示一个宽度值, 光刀太宽时影响点的精确度				
														 @~english 8-bit unsigned char left laser width array，The accuracy of the point is affected when the laser is too wide*/
	FRAME_DATA_FORMAT_LASER_WIDTH_RIGHT = 0x06,		/**< @~chinese 右红外图光刀宽度， unsigned char表示一个宽度值, 光刀太宽时影响点的精确度					
														 @~english 8-bit unsigned char right laser width array，The accuracy of the point is affected when the laser is too wide*/
}FRAME_DATA_FORMAT;
/// @}


/// \~chinese
/// \defgroup PropertyType 基础属性
/// \brief 列举所有可设置的基础属性
/// @{
/// \~english
/// \defgroup PropertyType Basic property
/// \brief List basic properties
/// @{

/**
* @~chinese
* 枚举: 相机的基本属性
* @~english
* enumeration: basic property of camera
**/
typedef enum PROPERTY_TYPE
{
    PROPERTY_GAIN                       = 0x00,	/**<@~chinese 增益
                                                              调节相机亮度,提高增益会引入噪声,导致深度精度下降。调节范围1~16, 高精度时,建议增益≤3
                                                    @~english gain of depth camera or RGB camera*/
    PROPERTY_EXPOSURE                   = 0x01,	/**<@~chinese 曝光值
                                                              用于调节相机亮度,数值越大亮度越高,不会引入噪声,但是帧率会随着曝光时间增大而降低,调节范围3000~60000微秒
                                                    @~english Controls exposure time of depth camera or RGB camera*/
	PROPERTY_FRAMETIME					= 0x02,	/**<@~chinese 帧时间            @~english Frame time of depth camera */
	PROPERTY_FOCUS						= 0x03,	/**<@~chinese 焦距              @~english Focus of RGB camera*/
	PROPERTY_ENABLE_AUTO_FOCUS			= 0x04,	/**<@~chinese 是否自动对焦      @~english Enable / disable auto-focus of RGB camera*/
	PROPERTY_ENABLE_AUTO_EXPOSURE		= 0x05, /**<@~chinese 是否自动曝光      @~english Enable / disable auto-exposure of RGB camera*/
	PROPERTY_ENABLE_AUTO_WHITEBALANCE	= 0x06, /**<@~chinese 是否自动白平衡    @~english White balance of RGB camera*/
	PROPERTY_WHITEBALANCE				= 0x07,	/**<@~chinese 白平衡值          @~english adjust white balance of RGB camera*/
	PROPERTY_WHITEBALANCE_R				= 0x08,	/**<@~chinese 白平衡R通道       @~english Channel r of RGB camera, adjust white balance*/
	PROPERTY_WHITEBALANCE_B				= 0x09,	/**<@~chinese 白平衡B通道       @~english Channel b of RGB camera, adjust white balance*/
	PROPERTY_WHITEBALANCE_G				= 0x10,	/**<@~chinese 白平衡G通道       @~english Channel g of RGB camera, adjust white balance*/
	
} PROPERTY_TYPE;
/// @}

/**
* @~chinese
* 枚举: 相机触发模式 
* @~english
* enumeration: trigger mode
**/
typedef enum TRIGGER_MODE
{
	TRIGGER_MODE_OFF		= 0, /**< @~chinese 关闭触发模式，持续输出深度流	
									  @~english output depth map continuously*/ 
	TRIGGER_MODE_HARDWAER	= 1, /**< @~chinese 外触发模式，需要在触发口输入硬件信号才能出图
									  @~english external trigger mode,you should input hardware pulse to get depth frame*/
	TRIGGER_MODE_SOFTWAER	= 2, /**< @~chinese 软触发模式，需要调用cs::ICamera::softTrigger才能出深度图
									  @~english software trigger mode,you should call cs::ICamera::softTrigger to get depth frame*/
}TRIGGER_MODE;

/**
* @~chinese
* 枚举: 高动态的模式
* @~english
* enumeration: mode of HDR
**/
typedef enum HDR_MODE
{
	HDR_MODE_OFF			= 0,	/**< @~chinese 关闭				
                                         @~english HDR off*/ 
	HDR_MODE_HIGH_RELECT	= 1,	/**< @~chinese 高反模式适用于测高反物体,	
                                                   会按照设定的曝光级数,增加较低的曝光参数进行多次曝光后进行融合输出。曝光级数由用户设定
                                         @~english suitable for shiny object*/
	HDR_MODE_LOW_RELECT		= 2,	/**< @~chinese 暗色模式适用于测深色物体,	
                                                   会按照设定的曝光级数,增加较高曝光参数进行几次曝光后进行融合输出。曝光级数由用户设定
                                         @~english suitable for dark object*/
	HDR_MODE_ALL_RELECT		= 3		/**< @~chinese 复合模式适用于测复合表面	
                                                   会平均分配曝光级数,分别增加较低曝光参数和较高曝光参数来进行多次曝光并融合输出。曝光级数由用户设定
                                         @~english suitable for composite object*/
}HDR_MODE;

/**
* @~chinese
* @brief 枚举: 自动曝光模式
* @~english
* @brief enumeration: mode of auto exposure
**/
typedef enum AUTO_EXPOSURE_MODE
{
	AUTO_EXPOSURE_MODE_CLOSE = 0,			/**< @~chinese 关闭				
                                                 @~english off*/
	AUTO_EXPOSURE_MODE_FIX_FRAMETIME = 1,	/**< @~chinese 固定帧率模式：在当前帧率允许范围下自动实时调节相机的曝光时间和增益, 不调节相机的输出帧率
											     @~english adjust exposure automatically and keep frame time unchanged*/
	AUTO_EXPOSURE_MODE_HIGH_QUALITY = 2,	/**< @~chinese 高质量模式：以获得最高质量模型为目标, 自动实时调节相机的曝光时间和增益, 同时会按需调节帧率
											     @~english adjust exposure and frame time automatically*/
    AUTO_EXPOSURE_MODE_FORE_GROUND = 3		/**< @~chinese 近景优先模式：以获得最高质量模型为目标, 自动实时调节相机的曝光时间和增益, 同时会按需调节帧率
                                                 @~english adjust exposure and frame time automatically*/
}AUTO_EXPOSURE_MODE;

/**
* @~chinese
* @brief 枚举: 网络传输压缩方式
* @~english
* @brief enumeration: mode of compress
**/
typedef enum NETWORK_COMPRESS_MODE
{
	NETWORK_COMPRESS_MODE_CLOSE = 0,		/**< @~chinese 关闭               @~english off*/
	NETWORK_COMPRESS_MODE_ZIP	= 1,		/**< @~chinese ZIP(默认设置)      @~english ZIP(Default)*/
}NETWORK_COMPRESS_MODE;


/**
* @~chinese
* @brief 测量深度范围，超出范围的值将被置零
* @~english
* @brief range of depth, value out of range will be set to zero
**/
typedef struct DepthRange
{
	int min;		/**< @~chinese 深度最小值        @~english minimum of depth*/ 
	int max;		/**< @~chinese 深度最大值        @~english maximum of depth*/ 
}DepthRange;

/**
* @~chinese
* @brief 网络连接时设备的IP设置，当autoEnable设置为true时，无需设置ipFourthByte
* @~english
* @brief IP setting, when autoEnable is true, there is no need to set ipFourthByte
**/
typedef struct IpSetting
{
	unsigned int autoEnable;	/**< @~chinese 是否开启DHCP             @~english enable/disable DHCP*/ 
	unsigned char ipFourthByte;	/**< @~chinese IP地址的第四位           @~english the fourth byte of ip*/ 
}IpSetting;

typedef struct CameraIpSetting
{
    unsigned char autoEnable;	/**< @~chinese 是否开启DHCP             @~english enable/disable DHCP*/
    unsigned char ipBytes[4];	/**< @~chinese IP地址 {192,168,3,99}    @~english the first byte of ip*/
}CameraIpSetting;

/**
 * @~chinese
 * @brief	联网模式
 * @~english
 * @brief	net working mode
 */
typedef enum {
	NET_WORKING_TERMINAL,			//终端模式
	NET_WORKING_WIFI_HOST_AP,		//5G AP 模式，默认是5G模式
	NET_WORKING_WIFI_HOST_2_4G,		//2.4G AP模式，2.4G模式不开放，限内部使用
	NET_WORKING_TERMINAL_ONLY,		//终端模式并关闭AP
	NET_WORKING_WIFI_HOST_5GONLY,	//AP模式并关闭终端
}NET_WORKING_MODE;

#pragma pack(push, 4)
typedef enum WifiChannel
{
	WIFI_CHANNEL_DEFAULT = 0,	//默认信道，内部转换为WIFI_CHANNEL_44
	WIFI_CHANNEL_44 = 44,
	WIFI_CHANNEL_149 = 149
}WifiChannel;

/**
 * @~chinese
 * @brief		联网参数信息,对应命令PROPERTY_EXT_NET_WORKING_INFO
 * @~english
 * @brief		net working info,corresponding PROPERTY_EXT_NET_WORKING_INFO
 */
typedef struct
{
	NET_WORKING_MODE	netMode;
	unsigned char		cEnable;		//0:disable,1:enable
	char 				reserved[3];	//reserved
	char 				ssid[100];		//ap name or wifi host name
	char 				psk[100];		//password 
	WifiChannel			channel;		//WIFI通道
}NetworkingInfo;

#pragma pack(pop)

/**
* @~chinese
* @brief HDR自动模式时曝光级数及两级曝光之间的倍数设置
* @~english
* @brief exposure times and interstage scale of HDR
**/
typedef struct HdrScaleSetting
{
	unsigned int highReflectModeCount;	/**< @~chinese 高反模式曝光级数         @~english exposure times of high-reflective mode*/ 
	unsigned int highReflectModeScale;	/**< @~chinese 高反模式两级间倍数       @~english interstage scale of high-reflective mode*/ 
	unsigned int lowReflectModeCount;	/**< @~chinese 深色模式曝光级数         @~english exposure times of low-reflective mode*/ 
	unsigned int lowReflectModeScale;	/**< @~chinese 深色模式两级间倍数       @~english interstage scale of low-reflective mode*/ 
}HdrScaleSetting;

#pragma pack(push, 1)

/**
* @~chinese
* @brief HDR某一级曝光的参数
* @~english
* @brief exposure param of HDR
**/
typedef struct HdrExposureParam
{
	unsigned int  exposure;	/**< @~chinese 曝光时间         @~english exposure time*/ 
	unsigned char gain;		/**< @~chinese 增益             @~english gain*/ 
}HdrExposureParam;

/**
* @~chinese
* @brief HDR曝光参数
* @~english
* @brief all exposure params of HDR
**/
typedef struct HdrExposureSetting
{
	unsigned char count;			/**< @~chinese 总曝光级数        @~english total exposure times of HDR*/ 
	HdrExposureParam param[11];		/**< @~chinese 各级曝光参数      @~english all params of HDR*/ 
}HdrExposureSetting;

/**
* @~chinese
* @brief HDR某一级的曝光时间及光机亮度设置
* @~english
* @brief exposure and laser level of HDR
**/
typedef struct HdrBrightnessParam
{
	unsigned char laserLevel;		/**< @~chinese 激光器等级        @~english brightness of laser*/ 
	float exposure;					/**< @~chinese 曝光时间          @~english exposure time*/ 
}HdrBrightnessParam;

/**
* @~chinese
* @brief HDR所有级别的曝光时间及光机亮度设置
* @~english
* @brief all settings of exposure and laser level
**/
typedef struct HdrBrightnessSetting
{
	unsigned char count;			/**< @~chinese 总曝光级数        @~english total exposure times of HDR*/
	HdrBrightnessParam param[11];	/**< @~chinese 各级曝光参数      @~english all params of HDR*/
}HdrBrightnessSetting;

typedef struct DepthRgbMatchParam
{
	int		iRgbOffset;				/**< @~chinese 匹配时RGB时间戳的偏移值     @~english the offset of rgb frame's timestamp when matching*/
	int		iDifThreshold;			/**< @~chinese 深度与rgb时间戳的误差阈值   @~english the threshold of depth and rgb frame's timestamp*/
	bool	bMakeSureRgbIsAfterDepth;	/**< @~chinese 确保RGB时间戳在depth之后   @~english make sure rgb's timestamp is after depth.*/
}DepthRgbMatchParam;

//条纹pattern切换前后需要stop_stream[0x483]和start_stream[0x481],sdk中已经处理好了，外部不需要重复调用483和481
//POP2的算法在1和3切换，当标志点模式时使用1，当其它模式时使用3--彭磊
//EN_FRINGE_PATTERN_TYPE_3FREQ4STEP_MULTI是18帧高帧率
const int id_msg_set_fringe_pattern_t = 0xa05;
typedef enum EN_FRINGE_PATTERN_TYPE_T 
{
	EN_FRINGE_PATTERN_TYPE_STANDARD = 0x00,
	EN_FRINGE_PATTERN_TYPE_STANDARD_WHITE_ADDED = 0x01,
	EN_FRINGE_PATTERN_TYPE_MULTI_DEPTH = 0x02,
	EN_FRINGE_PATTERN_TYPE_MULTI_DEPTH_WHITE_ADDED = 0x03,
	EN_FRINGE_PATTERN_TYPE_MULTI_DEPTH_DOUBLE_WHITE_ADDED = 0x04,
	EN_FRINGE_PATTERN_TYPE_3FREQ4STEP = 0x05,
	EN_FRINGE_PATTERN_TYPE_3FREQ4STEP_MULTI_DEPTH = 0x06,

}FRINGE_PATTERN_TYPE;

typedef struct MaxFrameTimeGain_tag
{
	int		iMaxFrame;	//the recommended value is 15000
	int		iMaxGain;	//the recommended value is 48
}MaxFrameTimeGain;

typedef struct StreamResolution_tag
{
	int iWidth;
	int iHeight;
}StreamResolution;

typedef struct DepthRoi_tag
{
	int left;		//[0,100],coordinates take the value 0-100,which represents the percentage of width or height.
	int top;		//[0,100]
	int right;		//[0,100]
	int bottom;		//[0,100]
}DepthRoi;

typedef struct HidHeader_tag
{
	int		magic;
	int		type;
	char	checksum[32]; // 0 not check, other check
	int		iDataLen;//指示acData中的有效数据大小
}HidHeader;

typedef struct WriteReadHidData_tag
{
	int iDataMax;		//指示acData最大可用空间
	int iTimeoutMs;		//指示读取数据的超时时间，单位毫秒

	unsigned char	reportId;//特别+1(ReportId)
	HidHeader		hidHeader;
	char	acData[1];		//指示具体数据

}WriteReadHidData;

//透视变换矩阵行数,列数
#define PER_TRANSF_MATRIX_ROW	4
#define PER_TRANSF_MATRIX_COL	4
#define PER_TRANSF_MATRIX_SIZE	16

/**
* @~chinese
* @brief 透视变换矩阵
* @~english
* @brief Perspective transformation matrix
**/
typedef struct PerspectiveTransformationMatrix 
{
	short width;
	short height;
	union 
	{
		float matrix[PER_TRANSF_MATRIX_ROW][PER_TRANSF_MATRIX_COL];
		float mat_[PER_TRANSF_MATRIX_SIZE];
	};
	
} PerspectiveTransformationMatrix;

/**
* @~chinese
* @brief 值范围
* @~english
* @brief Value range
**/
typedef struct ValueRange {
	float fMin_;
	float fMax_;
	float fStep_;
}*ValueRange_PTR;

/**
* @~chinese
* @brief LED控制参数
* @~english
* @brief LED Contrl param
**/
typedef struct LedCtrlParam {
	LED_ID emLedId;
	LED_CTRL_TYPE emCtrlType;
}*LedCtrlParam_ptr;

#pragma pack(pop)




/// \~chinese
/// \defgroup PropertyExtensionType 扩展属性
/// \brief 列举所有可设置的扩展属性
/// @{
/// \~english
/// \defgroup PropertyExtensionType Extensional property
/// \brief List extensional properties
/// @{

/**
* @~chinese
* @brief 枚举: 扩展属性
* @~english
* @brief enumeration: extensional of property
**/
typedef enum PROPERTY_TYPE_EXTENSION
{
	PROPERTY_EXT_DEPTH_SCALE			= 0x0,	 /**< @~chinese 深度值缩放系数                @~english depth unit for real distance */
	PROPERTY_EXT_TRIGGER_MODE			= 0x1,	 /**< @~chinese 触发模式                      @~english/PROPERTY_EXT_TRIGGER_OUT_MODE set trigger mode ,normal or trigge mode, value 1 stands for software trigger mode, value 2 stands for hardware trigger mode, other stands for trigger off(default)*/
	PROPERTY_EXT_TRIGGER_OUT_MODE		= 0x2,	 /**< @~chinese 是否开启脉冲输出              @~english enable/disable trigger out*/
	PROPERTY_EXT_HDR_BRIGHTNESS			= 0x3,	 /**< @~chinese HDR各级参数                   @~english all params of HDR*/
	PROPERTY_EXT_IP_SETTING				= 0x4,	 /**< @~chinese deprecated IP设置             @~english IP setting*/
	PROPERTY_EXT_NETWORK_COMPRESS		= 0x5,	 /**< @~chinese 网络传输时流是否压缩
													  @~english whether the stream compresses when transmited by network*/
    PROPERTY_EXT_SERIAL_NUMBER          = 0x6,   /**< @~chinese 修改序列号*/
    PROPERTY_EXT_MAC_ADDRESS            = 0x7,   /**< @~chinese 修改mac地址*/
    PROPERTY_EXT_CAMERA_IP              = 0x8,	 /**< @~chinese IP设置                        @~english IP setting*/
	PROPERTY_EXT_AP_NET_WORKING_INFO	= 0x9,   /**<@~chinese ap模式联网 属性,参考NetworkingInfo	  @~english property of net working,reference NetworkingInfo*/
	PROPERTY_EXT_FAST_SCAN_MODE			= 0x0A,   /**<@~chinese  快速扫描模式,参考bFastScanMode	  @~english property of fast scan mode ,reference bFastScanMode*/
	PROPERTY_EXT_HARDSYNCSWITCH			= 0x0B,		/**<@~chinese  硬同步开关,参考bHardSyncSwitch	  @~english property of hard synchronization ,reference bHardSyncSwitch*/
	PROPERTY_EXT_DEPTH_RGB_MATCH_PARAM	= 0x0C,		/**<@~chinese  深度和RGB帧匹配参数,参考DepthRgbMatchParam	  @~english property of depth and rgb match param ,reference DepthRgbMatchParam*/
	PROPERTY_EXT_PAUSE_DEPTH_STREAM		= 0x0D,		/**<@~chinese  暂停深度流	  @~english pause the depth stream*/
	PROPERTY_EXT_RESUME_DEPTH_STREAM	= 0x0E,		/**<@~chinese  恢复深度流	  @~english resume the depth stream*/
	PROPERTY_EXT_TERMINAL_NET_WORKING_INFO = 0x10,   /**<@~chinese 终端模式联网 属性,参考NetworkingInfo	  @~english property of net working,reference NetworkingInfo*/
	PROPERTY_EXT_IS_WIFI_HOST_MODE		= 0x11,   /**<@~chinese 判断是否在wifi-host模式 属性,bIsWifiHostMode	  @~english property of bool ,which is in wifi host mode,reference bIsWifiHostMode*/
	PROPERTY_EXT_CLEAR_FRAME_BUFFER		= 0x13,   /**<@~chinese 清除SDK内部的Frame队列	  @~english empty sdk frame buffer*/
	PROPERTY_EXT_GET_RECONSTRUCTIONMAT	= 0x14,	 /**<@~chinese 获取视差重建点云的矩阵	  @~english get parallax reconstruction point cloud.*/

	PROPERTY_EXT_EXPOSURE_TIME_RGB      = 0x15,	 /**<@~chinese 曝光时间,单位微秒,仅限RGB流	  @~english exposure time,unit us.*/
	PROPERTY_EXT_EXPOSURE_TIME_RANGE_RGB = 0x16, /**<@~chinese 曝光时间范围,单位微秒,仅限RGB流	  @~english exposure time,unit us.*/
	PROPERTY_EXT_CPU_TEMPRATRUE			= 0x17,  /**<@~chinese CPU温度,单位摄氏度,	  @~english CPU tempratrue,unit degree Celsius */
	//PROPERTY_EXT_GET_CAMERA_STATE       = 0x18, /**< @~chinese 获取相机当前状态,	  @~english Get camera current state */
	PROPERTY_EXT_IS_SUPPORT_GYRO        = 0x19,   /**< @~chinese 判断相机是否支持陀螺仪, 参考bSupportGyro	  @~english the camera supports gyroscope or not, reference bSupportGyro*/
	PROPERTY_EXT_GET_GYRO_VERSION       = 0x20,   /**< @~chinese 获取相机陀螺仪版本信息, 参考gyroVersion	  @~english get camera gyroscope version information,reference gyroVersion */
 
	PROPERTY_EXT_SET_FAKE_MODE			= 0x904, /**< @~chinese 设置真假分辨率                @~english set fake mode */
	PROPERTY_EXT_AUTO_EXPOSURE_MODE		= 0x912, /**< @~chinese 深度相机自动曝光模式,参考AUTO_EXPOSURE_MODE         @~english auto exposure mode of depth camera，reference AUTO_EXPOSURE_MODE*/
	PROPERTY_EXT_DEPTH_ROI				= 0x913, /**< @~chinese 深度数据的ROI,参考DepthRoi            @~english roi of depth data,reference DepthRoi*/
	PROPERTY_EXT_HDR_MODE				= 0x914, /**< @~chinese HDR模式                       @~english HDR mode*/
	PROPERTY_EXT_HDR_SCALE_SETTING		= 0x915, /**< @~chinese HDR自动模式的配置             @~english setting of auto-HDR*/
	PROPERTY_EXT_HDR_EXPOSURE			= 0x916, /**< @~chinese HDR各级参数                   @~english all params of HDR*/
	PRPOERTY_EXT_MAX_FRAMETIME_GAIN		= 0x917, /**< @~chinese 最大帧时间和增益,参考MaxFrameTimeGain              @~english set the max frame and gran,reference MaxFrameTimeGain*/
	PROPERTY_EXT_LASER_BRIGHTNESS		= 0x920, /**< @~chinese 激光器亮度                    @~english brightness level of laser*/
	PROPERTY_EXT_LASER_ON_OFF			= 0x922, /**< @~chinese 激光器开/关                   @~english turn on/off laser*/
	PROPERTY_EXT_SPEED					= 0x924, /**< @~chinese 拍图速度                      @~english speed of shot*/
	
	PROPERTY_EXT_SET_STREAM_RESOLUTION	= 0x702, /**< @~chinese 设置分辨率,参考StreamResolution                    @~english set resolution of depth stream，reference StreamResolution*/
	PROPERTY_EXT_CONTRAST_MIN			= 0x705, /**< @~chinese 对比度阈值，此阈值用于删除原始图像中低曝光区域,将删除灰度低于该阈值的区域,阈值越大低曝光区域删除越多.调节范围0~40灰阶,建议设置为5
                                                      @~english remove where fringe contrast below this value*/
	PROPERTY_EXT_ALGO_SET_BACKGROUND	= 0x705, /**< @~chinese 背景阈值,范围[0-40],参考algoSetBackground @~english background threshold,[0,40]，reference algoSetBackground*/
	PROPERTY_EXT_DEPTH_RANGE			= 0x707, /**< @~chinese 深度范围                      @~english depth range of camera*/
	
	PROPERTY_EXT_UNKNOWA00				= 0xa00, /**< @~chinese                 @~english */
	PROPERTY_EXT_MULTIFRAME_FUSION		= 0xa04, /**< @~chinese 多帧融合(双曝光)，取值0/1,参考multiframeFusion                @~english multiframe fusion,reference multiframeFusion*/
	PROPERTY_EXT_SET_FRINGE_PATTERN		= 0xa05, /**< @~chinese 条纹pattern参数，参考FRINGE_PATTERN_TYPE  @~english fringe pattern,reference FRINGE_PATTERN_TYPE*/
	PROPERTY_EXT_LED_ON_OFF				= 0xb00, /**< @~chinese 是否打开LED灯                 @~english turn on/off led*/
	PROPERTY_EXT_LED_CTRL				= 0xb01, /**< @~chinese LED灯控制					  @~english led contrl*/
	
	
	PROPERTY_EXT_TRIGGER_IN_MODE		= 0x106, /**< @~chinese 触发模式                      @~english trigger mode*/
	
} PROPERTY_TYPE_EXTENSION;
/// @}

/**
* @~chinese
* @brief 扩展属性值，联合体表示，设置和获取时只取指定属性对应的字段即可
* @~english
* @brief union of extensional property
**/
typedef union PropertyExtension
{
	float depthScale;							/**< @~chinese 对应PROPERTY_EXT_DEPTH_SCALE			    @~english corresponding PROPERTY_EXT_DEPTH_SCALE			*/
	TRIGGER_MODE triggerMode;					/**< @~chinese 对应PROPERTY_EXT_TRIGGER_MODE			@~english corresponding PROPERTY_EXT_TRIGGER_MODE			*/
	int algorithmContrast;						/**< @~chinese 对应PROPERTY_EXT_CONTRAST_MIN			@~english corresponding PROPERTY_EXT_CONTRAST_MIN			*/
	AUTO_EXPOSURE_MODE autoExposureMode;		/**< @~chinese 对应PROPERTY_EXT_AUTO_EXPOSURE_MODE	    @~english corresponding PROPERTY_EXT_AUTO_EXPOSURE_MODE	*/
	HdrScaleSetting hdrScaleSetting;			/**< @~chinese 对应PROPERTY_EXT_HDR_SCALE_SETTING	    @~english corresponding PROPERTY_EXT_HDR_SCALE_SETTING	*/
	HdrExposureSetting hdrExposureSetting;		/**< @~chinese 对应PROPERTY_EXT_HDR_EXPOSURE			@~english corresponding PROPERTY_EXT_HDR_EXPOSURE			*/
	int ledOnOff;								/**< @~chinese 对应PROPERTY_EXT_LED_ON_OFF			@~english corresponding PROPERTY_EXT_LED_ON_OFF			*/
	
	TRIGGER_MODE triggerInMode;					/**< @~chinese 对应PROPERTY_EXT_TRIGGER_OUT_MODE		@~english corresponding PROPERTY_EXT_TRIGGER_OUT_MODE	 */
	int triggerOutEnable;						/**< @~chinese 对应PROPERTY_EXT_TRIGGER_IN_MODE		    @~english corresponding PROPERTY_EXT_TRIGGER_IN_MODE	 */
	int laserBrightness;						/**< @~chinese 对应PROPERTY_EXT_LASER_BRIGHTNESS		@~english corresponding PROPERTY_EXT_LASER_BRIGHTNESS	 */
	int laserOnOff;								/**< @~chinese 对应PROPERTY_EXT_LASER_ON_OFF			@~english corresponding PROPERTY_EXT_LASER_ON_OFF		 */
	int speed;									/**< @~chinese 对应PROPERTY_EXT_SPEED				    @~english corresponding PROPERTY_EXT_SPEED				 */
	HdrBrightnessSetting hdrBrightnessSetting;	/**< @~chinese 对应PROPERTY_EXT_HDR_BRIGHTNESS		    @~english corresponding PROPERTY_EXT_HDR_BRIGHTNESS		 */

	HDR_MODE hdrMode;							/**< @~chinese 对应PROPERTY_EXT_HDR_MODE				@~english corresponding PROPERTY_EXT_HDR_MODE				*/
	DepthRange depthRange;						/**< @~chinese 对应PROPERTY_EXT_DEPTH_RANGE			    @~english corresponding PROPERTY_EXT_DEPTH_RANGE			*/
	IpSetting ipSetting;						/**< @~chinese deprecated 对应PROPERTY_EXT_IP_SETTING	@~english deprecated corresponding PROPERTY_EXT_IP_SETTING*/
    CameraIpSetting cameraIp;                         /**< @~chinese 对应PROPERTY_EXT_CAMERA_IP		    @~english corresponding PROPERTY_EXT_CAMERA_IP*/
    NETWORK_COMPRESS_MODE networkCompressMode;	/**< @~chinese 对应PROPERTY_EXT_NETWORK_COMPRESS		@~english corresponding PROPERTY_EXT_NETWORK_COMPRESS	*/
	NetworkingInfo		networkingInfo;			/**< @~chinese 对应PROPERTY_EXT_AP_NET_WORKING_INFO/PROPERTY_EXT_TERMINAL_NET_WORKING_INFO		@~english corresponding PROPERTY_EXT_AP_NET_WORKING_INFO/PROPERTY_EXT_TERMINAL_NET_WORKING_INFO	*/
	bool				bFastScanMode;			/**< @~chinese 对应PROPERTY_EXT_FAST_SCAN_MODE			@~english corresponding PROPERTY_EXT_FAST_SCAN_MODE	*/
	bool				bHardSyncSwitch;		/**< @~chinese 对应PROPERTY_EXT_HARDSYNCSWITCH			@~english corresponding PROPERTY_EXT_HARDSYNCSWITCH	*/
	DepthRgbMatchParam	depthRgbMatchParam;		/**< @~chinese 对应PROPERTY_EXT_DEPTH_RGB_MATCH_PARAM	@~english corresponding PROPERTY_EXT_DEPTH_RGB_MATCH_PARAM	*/
	FRINGE_PATTERN_TYPE	fringePatternType;		/**< @~chinese 对应PROPERTY_EXT_SET_FRINGE_PATTERN	@~english corresponding PROPERTY_EXT_SET_FRINGE_PATTERN	*/
	int					algoSetBackground;		/**< @~chinese 对应PROPERTY_EXT_ALGO_SET_BACKGROUND	@~english corresponding PROPERTY_EXT_ALGO_SET_BACKGROUND	*/
	bool				multiframeFusion;		/**< @~chinese 对应PROPERTY_EXT_MULTIFRAME_FUSION	@~english corresponding PROPERTY_EXT_MULTIFRAME_FUSION	*/
	bool				bIsWifiHostMode;		/**< @~chinese 对应PROPERTY_EXT_IS_WIFI_HOST_MODE	@~english corresponding PROPERTY_EXT_IS_WIFI_HOST_MODE	*/
	MaxFrameTimeGain	maxFrameTimeGain;		/**< @~chinese 对应PRPOERTY_EXT_MAX_FRAMETIME_GAIN	@~english corresponding PRPOERTY_EXT_MAX_FRAMETIME_GAIN	*/
	StreamResolution	streamResolution;		/**< @~chinese 对应PROPERTY_EXT_SET_STREAM_RESOLUTION	@~english corresponding PROPERTY_EXT_SET_STREAM_RESOLUTION	*/
	char acSerialNumber[60];					/**< @~chinese 对应PROPERTY_EXT_SERIAL_NUMBER	@~english corresponding PROPERTY_EXT_SERIAL_NUMBER	*/
	char acMac[60];								/**< @~chinese 对应PROPERTY_EXT_MAC_ADDRESS	@~english corresponding PROPERTY_EXT_MAC_ADDRESS	*/
	DepthRoi	depthRoi;						/**< @~chinese 对应PROPERTY_EXT_DEPTH_ROI	@~english corresponding PROPERTY_EXT_DEPTH_ROI	*/
	WriteReadHidData*	writeReadHidData;		/**< @~chinese 对应PROPERTY_EXT_WRITE_READ_HID	@~english corresponding PROPERTY_EXT_WRITE_READ_HID	*/
	PerspectiveTransformationMatrix	reconstructionMat; /**< @~chinese 对应PROPERTY_EXT_GET_RECONSTRUCTIONMAT	@~english corresponding PROPERTY_EXT_GET_RECONSTRUCTIONMAT	*/

	unsigned int  uiExposureTime;	            /**< @~chinese 对应PROPERTY_EXT_EXPOSURE_TIME_RGB 曝光时间,单位微秒  @~english corresponding PROPERTY_EXT_EXPOSURE_TIME exposure time,unit us*/
	ValueRange objVRange_;						/**< @~chinese 对应PROPERTY_EXT_EXPOSURE_TIME_RANGE_RGB 曝光时间范围,单位微秒  @~english corresponding PROPERTY_EXT_EXPOSURE_TIME_RANGE exposure time range,unit us*/
	unsigned int uiTempratrue_;					/**< @~chinese 对应PROPERTY_EXT_CPU_TEMPRATRUE 温度,单位摄氏度  @~english corresponding PROPERTY_EXT_CPU_TEMPRATRUE exposure tempratrue,unit degree Celsius*/

	bool boolCameraState;						/**< @~chinese 对应PROPERTY_EXT_GET_CAMERA_STATE 相机当前连接状态  @~english Camera current connect state*/

	bool bSupportGyro;                          /**< @~chinese 对应PROPERTY_EXT_IS_SUPPORT_GYRO   @~english corresponding PROPERTY_EXT_IS_SUPPORT_GYRO */
	unsigned short gyroVersion;                 /**< @~chinese 对应PROPERTY_EXT_GET_GYRO_VERSION  @~english corresponding PROPERTY_EXT_GET_GYRO_VERSION */
	
	LedCtrlParam ledCtrlParam;					/**< @~chinese 对应PROPERTY_EXT_LED_CTRL  @~english corresponding PROPERTY_EXT_LED_CTRL */


	char reservedStr[60];                       /**< @~chinese 预留									    @~english reserved */
    int reserved[15];							/**< @~chinese 预留									    @~english reserved */
}PropertyExtension;		

#define KEY_INFO_RESERVED_LEN	2

#pragma pack(push, 1)
/**
* @~chinese
* @brief 按键信息
* @~english
* @brief key information
**/
typedef struct KeyInfo {
    char   key_num;
	char   key_level;	//KEY_EVENT_DOWN = 0,KEY_EVENT_UP = 1
	char   reserved[KEY_INFO_RESERVED_LEN];	//reserved fot 2 bytes
    int   key_time;		//按键时间，特别说明：设备上电到第一次按键值一直为0
}KeyInfo;

/**
* @~chinese
* @brief	陀螺仪位置和位姿
* @~english
* @brief position and pose of gyro
*/
/*
typedef struct GyroPositionAndPose_tag
{
	float	x;
	float	y;
	float	z;
	float	roll;
	float	pitch;
	float	yaw;
}GyroPositionAndPose;
*/
typedef struct GyroPositionAndPose_tag
{	
	float q0; // w					/**< (q0,q1,q2,q3)->(w,x,y,z): @~chinese 陀螺仪姿态，使用四元素表示  @~english pose of gyro，Use four elements to represent */
	float q1; // x
	float q2; // y
	float q3; // z
	float gx;						/**< @~chinese 陀螺仪绕X轴转动角速度，单位为：radian/second   @~english  Gyroscope rotates angular velocity around the X axis, unit: radian/second */
	float gy;						/**< @~chinese  陀螺仪绕Y轴转动角速度，单位为：radian/second   @~english  Gyroscope rotates angular velocity around the Y axis, unit: radian/second */
	float gz;						/**< @~chinese  陀螺仪绕Z轴转动角速度，单位为：radian/second  @~english  Gyroscope rotates angular velocity around the Z axis, unit: radian/second */
	float ax;						/**< @~chinese  陀螺仪X方向加速度，单位为：g  @~english  Acceleration in the X direction of the gyroscope, unit: g */
	float ay;						/**< @~chinese  陀螺仪Y方向加速度，单位为：g  @~english  Acceleration in the Y direction of the gyroscope, unit: g */
	float az;						/**< @~chinese  陀螺仪Z方向加速度，单位为：g  @~english  Acceleration in the Z direction of the gyroscope, unit: g */
	float mx;						/**< @~chinese  陀螺仪X方向磁场，单位为：uT  @~english  The magnetic field in the X direction of the gyroscope, unit: uT */
	float my;						/**< @~chinese  陀螺仪Y方向磁场，单位为：uT  @~english  The magnetic field in the Y direction of the gyroscope, unit: uT */
	float mz;						/**< @~chinese  陀螺仪Z方向磁场，单位为：uT  @~english  The magnetic field in the Z direction of the gyroscope, unit: uT */
	float score;					/**< @~chinese  运动分数，范围为：[0-1], 数值越大越稳定  @~english  Sports score, the range is: [0-1], the larger the value, the more stable */
}GyroPositionAndPose;

#define ADDITION_RESERVED_LEN	8
#define ADDITION_RESERVED_LEN2	4

typedef struct AdditionData
{
    char    reserved[ADDITION_RESERVED_LEN];
    KeyInfo keyInfo;
    char    reserved2[ADDITION_RESERVED_LEN2];
    int     timestamp;
	GyroPositionAndPose	gyroPosAndPos;
}AdditionData;

#define EXTRA_INFO_RESERVED_LEN	24

/**
* @~chinese
* @brief 帧的附加数据
* @~english
* @brief extra information of frame
**/
typedef union ExtraInfo
{
    AdditionData addition;
    char reserved[EXTRA_INFO_RESERVED_LEN];
}ExtraInfo;
#pragma pack(pop)

/**
* @~chinese
* @brief 流信息组合，用于打开流时使用，可通过ICamera::getStreamInfos获得
* @~english
* @brief stream information, returned by ICamera::getStreamInfos
**/
typedef struct StreamInfo
{
	STREAM_FORMAT format;	/**< @~chinese 流信息         @~english stream format*/ 
	int width;				/**< @~chinese 宽度           @~english stream width*/
	int height;				/**< @~chinese 高度           @~english stream height*/
	float fps;				/**< @~chinese 帧率           @~english stream framerate*/
}StreamInfo;

/**
* @~chinese
* @brief 相机信息，可通过ICamera::getInfo或ISystem::queryCameras获得
* @~english
* @brief camera informations, returned by ICamera::getStreamInfos or ISystem::queryCameras
**/
typedef struct CameraInfo
{
	char name[32];					/**< @~chinese 相机类型         @~english type of camera*/ 
	char serial[CAMERA_SERIAL_MAX];				/**< @~chinese 序列号           @~english serial number of camera*/
	char uniqueId[32];				/**< @~chinese 相机标识         @~english unique Id of camera*/
	char firmwareVersion[FIRMWARE_VERSION_MAX];		/**< @~chinese 固件版本         @~english version of firmware*/
	char algorithmVersion[ALGORITHM_VERSION_MAX];		/**< @~chinese 算法版本         @~english version of algorithm*/
}CameraInfo;

/**
* @~chinese
* @brief 相机内参
* @~english
* @brief Intrinsics of depth camera or RGB camera
**/
typedef struct Intrinsics
{
	short width;	/**< @~chinese 标定分辨率-宽度		@~english calibration resolution-width*/ 
	short height;	/**< @~chinese 标定分辨率-高度		@~english calibration resolution-height*/ 
	float fx;
	float zero01;
	float cx;
	float zeor10;
	float fy;
	float cy;
	float zeor20;
	float zero21;
	float one22;
}Intrinsics;

/**
* @~chinese
* @brief 深度相机到RGB相机间的旋转平移信息
* @~english
* @brief Rotation and translation offrom depth camera to RGB camera
**/
typedef struct Extrinsics
{
	float rotation[9];                           /**<@~chinese 3x3旋转矩阵      @~english column-major 3x3 rotation matrix */
	float translation[3];                        /**<@~chinese 3元素的平移矩阵  @~english three-element translation vector */
}Extrinsics;

//畸变参数个数
#define DISTORT_PARAM_CNT	5

/**
* @~chinese
* @brief 深度相机或RGB相机畸变参数
* @~english
* @brief Distort of depth camera or RGB camera
**/
typedef struct Distort
{
	float k1;
	float k2;
	float k3;
	float k4;
	float k5;
}Distort;




#ifdef __cplusplus
}
#endif

namespace cs {
	/**
	* @~chinese
	* @brief 二维点坐标
	* @~english
	* @brief 2D point coordinates
	**/
#ifndef POINT2F
#define POINT2F
	typedef struct Point2f
	{
		float x;
		float y;
	} Point2f;
#endif
	/**
	* @~chinese
	* @brief 三维点坐标
	* @~english
	* @brief 3D point coordinates
	**/
#ifndef POINT3F
#define POINT3F
	typedef struct Point3f
	{
		float x;
		float y;
		float z;
	} Point3f;
#endif
}

#endif
