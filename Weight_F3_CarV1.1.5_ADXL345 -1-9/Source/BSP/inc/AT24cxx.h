#ifndef __AT24CXX_H__
#define __AT24CXX_H__

//---------------------------------------------------
#define RUN_ADDR_BASE				0x00                                      //参数初始化标志位，避免每次上电都写EEPROMP
#define SLAVE_ADDR				(RUN_ADDR_BASE                  + 0x01)
#define BAUDRATE				(SLAVE_ADDR                     + 0x01)
#define PARITY					(BAUDRATE                       + 0x01)
#define OFFSET_ENABLE                           (PARITY                         + 0x01)
#define FILTER_LEVEL				(OFFSET_ENABLE                  + 0x01)   //滤波时间
#define AUTO_UPLOAD_TIME			(FILTER_LEVEL                   + 0x01)   //自动上传时间
#define MBT_WORK_TYPE				(AUTO_UPLOAD_TIME               + 0x01)   //工作模式
#define LOAD_SHOVE_DTIME			(MBT_WORK_TYPE                  + 0x01)   //铲与铲间隔时间
#define LOAD_EMPTY_FIFO				(LOAD_SHOVE_DTIME               + 0x01)   //空载值10组数据缓存 20 bytes
#define LOAD_FULL_FIFO				(LOAD_EMPTY_FIFO                + 0x14)   //满载值10组数据缓存 20 bytes
#define USER_FLOW_K                             (LOAD_FULL_FIFO                 + 0x14)   //用户输出系数K      
#define USER_FLOW_B                             (USER_FLOW_K                    + 0x04)   //用户输出系数B
#define LOAD_RATED_VALUE 		        (USER_FLOW_B                    + 0x04)   //额定载重值 2 bytes
#define LOAD_SHOVEL_NUM			        (LOAD_RATED_VALUE               + 0x02)   //记录铲数 1 byte
#define LOAD_WORK_STATE                         (LOAD_SHOVEL_NUM                + 0x01)   //工作状态 1 byte
#define LOAD_OVER_K				(LOAD_WORK_STATE                + 0x01)   //超载系数 2 bytes
#define LOAD_VARIANCE_VALUE                     (LOAD_OVER_K                    + 0x02)   //方差值 2 bytes
#define TEST_MIN_HIGH			        (LOAD_VARIANCE_VALUE            + 0x02)   //最小高度值 2 bytes
#define TEST_MIN_ADC			        (TEST_MIN_HIGH                  + 0x02)   //最小AD值 2 bytes
#define TEST_MID_HIGH			        (TEST_MIN_ADC                   + 0x02)   //中间高度值 2 bytes
#define TEST_MID_ADC			        (TEST_MID_HIGH                  + 0x02)   //中间AD值 2 bytes
#define TEST_MAX_HIGH			        (TEST_MID_ADC                   + 0x02)   //最大高度值 2 bytes
#define TEST_MAX_ADC			        (TEST_MAX_HIGH                  + 0x02)   //最大AD值 2 bytes
#define MAX_DLY_TIME                            (TEST_MAX_ADC                   + 0x02)   //装满后最大延迟时间 2 bytes 单位为0.1S
         
#define VEHICLE_WEIGHT                          (MAX_DLY_TIME                   + 0x02)   //整车重量/载荷重量
#define LOAD_MEASURE_SCHEME                     (VEHICLE_WEIGHT                 + 0x02)   //载重测量方案
#define OVERLOAD_LIMIT                          (LOAD_MEASURE_SCHEME            + 0x02)   //超载阀值
#define OVERLOAD_LIMIT_DEVIATION                (OVERLOAD_LIMIT                 + 0x02)   //超载阀值偏差
#define LOAD_LIMIT                              (OVERLOAD_LIMIT_DEVIATION       + 0x02)   //重载阀值 
#define LOAD_LIMIT_DEVIATION                    (LOAD_LIMIT                     + 0x02)   //重载阀值偏差
#define EMPTYLOAD_LIMIT                         (LOAD_LIMIT_DEVIATION           + 0x02)   //空载阀值
#define EMPTYLOAD_LIMIT_DEVIATION               (EMPTYLOAD_LIMIT                + 0x02)   //空载阀值偏差
#define LIGHTLOAD_LIMIT                         (EMPTYLOAD_LIMIT_DEVIATION      + 0x02)   //轻载阀值
#define LIGHTLOAD_LIMIT_DEVIATION               (LIGHTLOAD_LIMIT                + 0x02)   //轻载阀值偏差 
#define WEIGHT_UNIT                             (LIGHTLOAD_LIMIT_DEVIATION      + 0x02)
#define PRO_DEFAULT_LEN                         (WEIGHT_UNIT                    + 0x01)   //默认参数长度 91个字节     

#define  WEIGHTPARAADDR                         140                                        //WeightParaAddr

#define USER_DEFAULT_PARA_BAK1		        (2 * 0x80)//256
#define USER_DEFAULT_PARA_BAK2		        (4 * 0x80)//512


//-------------------------------------------------------------------------------

#define COMPANY						(USER_DEFAULT_PARA_BAK2 		+ 0x80)//512+128=640
#define DEV_ENCODING				        (COMPANY				+ 0x40)//640+64
#define HWVERSION                                       (DEV_ENCODING				+ 0x40)//640+128
#define SFVERSION                                       (HWVERSION                              + 0x40)//640+192
#define DEV_ID					        (SFVERSION				+ 0x40)//640+256
#define CUSTOMERCODE					(DEV_ID				        + 0x40)//640+320=960

#define AD_WEIGHT_PACKS                                 0x043E                                         //1086
#define AD_WEIGHT_ADDR(x)                               (0x0440 + 0x50 * x)                            //1088+ 80*x

extern ProductParaTypeDef ProductPara;
extern RunVarTypeDef RunVar;
extern UartCommTypeDef UartComm;
extern UserParamTypeDef UserParam;

extern u8 Cur_Param[PRO_DEFAULT_LEN];
extern u8 CALIB_Data[2];
extern u8 HighFilterbufNum;
extern u8 FilterStartFlag;

void EEProm_Init(void);
void Switch_Fiter(u8 value);
void Rd_Dev_Param(void);


#endif
