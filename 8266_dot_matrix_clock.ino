/**
 * ESP8266 点阵时钟
 * NTP授时
 * author: Louie.v (Check.vv@gmail.com)
 * 2018-3-4
 */


#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <user_interface.h>

#include <time.h> 
#include <sys/time.h> 
#include <coredecls.h>

#include "LedControl.h"
//https://github.com/PaulStoffregen/OneWire
#include "OneWire.h" 

#define DEBUG
#ifdef DEBUG
#define DOUT(w) Serial.println(w)
#else
#define DOUT(w) 
#endif

#define DS_PIN 5

//onbord led pin define
#define LED LED_BUILTIN

//MAX7219 pin define 
#define LED_DAT 13
#define	LED_CLKPIN 14
#define LED_CSPIN 15
#define LED_NUM_DEVICES 3
#define LED_NUM_COL LED_NUM_DEVICES*8
#define LED_NUM_CHAR LED_NUM_DEVICES * 2

#define SPEED 10
#define FONT_WIDTH 4
#define FONT_ASSIC_START 32
static const byte font[] PROGMEM=
{

	//special font 
	0x00, 0x00, 0x00, 0x00,		// Space	0
	0x00, 0x6C, 0x00, 0x00,		//dot up for time 1
	0x00, 0x36, 0x00, 0x00,		//dot down fot time 2
	0x38, 0x44, 0x44, 0x00, 	//℃		3	
	0x08, 0x08, 0x08, 0x00,		// -		4

	0x00, 0x00, 0x00, 0x00, 	// Space	5	
	0x00, 0x00, 0x00, 0x00, 	// Space	6	
	0x00, 0x00, 0x00, 0x00, 	// Space	7	
	0x00, 0x00, 0x00, 0x00, 	// Space	8	
	0x00, 0x00, 0x00, 0x00, 	// Space	9	
	0x00, 0x00, 0x00, 0x00, 	// Space	10	
	0x00, 0x00, 0x00, 0x00, 	// Space	11	
	0x00, 0x00, 0x00, 0x00, 	// Space	12	
	0x00, 0x00, 0x00, 0x00, 	// Space	13	
	0x00, 0x00, 0x00, 0x00, 	// Space	14	
	0x00, 0x00, 0x00, 0x00, 	// Space	15	
	0x00, 0x00, 0x00, 0x00, 	// Space	16	
	0x00, 0x00, 0x00, 0x00, 	// Space	17	
	0x00, 0x00, 0x00, 0x00, 	// Space	18	
	0x00, 0x00, 0x00, 0x00, 	// Space	19	
	0x00, 0x00, 0x00, 0x00, 	// Space	20	
	0x00, 0x00, 0x00, 0x00, 	// Space	21	
	0x00, 0x00, 0x00, 0x00, 	// Space	22	
	0x00, 0x00, 0x00, 0x00, 	// Space	23	
	0x00, 0x00, 0x00, 0x00, 	// Space	24	
	0x00, 0x00, 0x00, 0x00, 	// Space	25	
	0x00, 0x00, 0x00, 0x00, 	// Space	26	
	0x00, 0x00, 0x00, 0x00, 	// Space	27	
	0x00, 0x00, 0x00, 0x00, 	// Space	28	
	0x00, 0x00, 0x00, 0x00, 	// Space	29	
	0x00, 0x00, 0x00, 0x00, 	// Space	30	
	0x00, 0x00, 0x00, 0x00, 	// Space	31	

	//nomal font
	//字体来源 https://github.com/aguegu/dot-matrix
	0x00, 0x00, 0x00, 0x00, 	// Space	ASCII--32
	0x5f, 0x00, 0x00, 0x00, 	// !		ASCII--33
	0x03, 0x00, 0x03, 0x00, 	// "		ASCII--34
	0x28, 0x7c, 0x28, 0x00,		// #		ASCII--35
	0x2e, 0x6b, 0x3a, 0x00, 	// $		ASCII--36
	0x48, 0x20, 0x10, 0x48, 	// %		ASCII--37
	0x36, 0x49, 0x26, 0x50, 	// &		ASCII--38
	0x03, 0x00, 0x00, 0x00, 	// '		ASCII--39

	0x1c, 0x22, 0x41, 0x00, 	// (		ASCII--40
	0x41, 0x22, 0x1c, 0x00, 	// )		ASCII--41
	0x0a, 0x04, 0x0a, 0x00, 	// *		ASCII--42
	0x04, 0x0e, 0x04, 0x00, 	// +		ASCII--43
	0x60, 0x00, 0x00, 0x00, 	// ,		ASCII--44
	0x04, 0x04, 0x04, 0x00, 	// -		ASCII--45
	0x40, 0x00, 0x00, 0x00, 	// .		ASCII--46
	0x60, 0x18, 0x06, 0x00, 	// /		ASCII--47

	0x7f, 0x41, 0x7f, 0x00,  	// 0		ASCII--48
	0x42, 0x7f, 0x40, 0x00, 	// 1		ASCII--49
	0x7d, 0x45, 0x47, 0x00,		// 2		ASCII--50
	0x45, 0x45, 0x7f, 0x00,		// 3		ASCII--51
	0x3f, 0x20, 0x7f, 0x00, 	// 4		ASCII--52
	0x47, 0x45, 0x7d, 0x00,		// 5		ASCII--53
	0x7f, 0x45, 0x7d, 0x00,		// 6		ASCII--54
	0x01, 0x01, 0x7f, 0x00,		// 7		ASCII--55

	0x7f, 0x45, 0x7f, 0x00,		// 8		ASCII--56
	0x5f, 0x51, 0x7f, 0x00,		// 9		ASCII--57
	0x50, 0x00, 0x00, 0x00, 	// :		ASCII--58
	0x68, 0x00, 0x00, 0x00, 	// ;		ASCII--59
	0x08, 0x14, 0x22, 0x00, 	// <		ASCII--60
	0x14, 0x14, 0x14, 0x00, 	// =		ASCII--61
	0x22, 0x14, 0x08, 0x00, 	// >		ASCII--62
	0x59, 0x0f, 0x00, 0x00, 	// ?		ASCII--63

	0x7a, 0x5a, 0x42, 0x7e, 	// @		ASCII--64
	0x7f, 0x21, 0x7f, 0x00,		// A		ASCII--65
	0x7f, 0x45, 0x47, 0x7c, 	// B		ASCII--66
	0x7f, 0x41, 0x63, 0x00,		// C		ASCII--67
	0x7f, 0x41, 0x41, 0x3e, 	// D		ASCII--68
	0x7f, 0x45, 0x45, 0x00,  	// E		ASCII--69
	0x7f, 0x05, 0x05, 0x00, 	// F		ASCII--70
	0x7f, 0x41, 0x7d, 0x00,		// G		ASCII--71
	0x7f, 0x04, 0x7f, 0x00,		// H		ASCII--72
	0x41, 0x7f, 0x41, 0x00, 	// I		ASCII--73
	0x40, 0x41, 0x7f, 0x01, 	// J		ASCII--74
	0x7f, 0x04, 0x0a, 0x71, 	// K		ASCII--75
	0x7f, 0x40, 0x40, 0x00, 	// L		ASCII--76
	0x7f, 0x01, 0x07, 0x7c,		// M		ASCII--77
	0x7f, 0x01, 0x7f, 0x00,		// N		ASCII--78
	0x7f, 0x41, 0x7f, 0x00,		// O		ASCII--79

	0x7f, 0x21, 0x3f, 0x00, 	// P		ASCII--80
	0x7f, 0x41, 0x21, 0x5f, 	// Q		ASCII--81
	0x7f, 0x09, 0x79, 0x4f, 	// R		ASCII--82
	0x47, 0x45, 0x7d, 0x00,  	// S		ASCII--83
	0x01, 0x7f, 0x01, 0x00, 	// T		ASCII--84
	0x7F, 0x40, 0x7f, 0x00,		// U		ASCII--85
	0x7f, 0x20, 0x10, 0x0f, 	// V		ASCII--86
	0x7f, 0x40, 0x70, 0x1f, 	// W		ASCII--87
	0x7b, 0x0e, 0x7b, 0x00, 	// X		ASCII--88
	0x07, 0x7c, 0x07, 0x00, 	// Y		ASCII--89
	0x79, 0x45, 0x43, 0x00, 	// Z		ASCII--90
	0x7f, 0x41, 0x00, 0x00, 	// [		ASCII--91
	0x06, 0x18, 0x60, 0x00, 	// "\"		ASCII--92
	0x41, 0x7f, 0x00, 0x00, 	// ]		ASCII--93
	0x02, 0x01, 0x02, 0x00, 	// ^		ASCII--94
	0x40, 0x40, 0x40, 0x00, 	// _		ASCII--95

	0x01, 0x02, 0x00, 0x00, 	// `		ASCII--96
	0x74, 0x54, 0x7c, 0x00, 	// a		ASCII--97
	0x7f, 0x44, 0x7c, 0x00, 	// b		ASCII--98
	0x7c, 0x44, 0x44, 0x00, 	// c		ASCII--99
	0x7c, 0x44, 0x7f, 0x00, 	// d		ASCII--100
	0x7c, 0x54, 0x5c, 0x00, 	// e		ASCII--101
	0x04, 0x7f, 0x05, 0x00, 	// f		ASCII--102
	0x5c, 0x54, 0x7c, 0x00, 	// g		ASCII--103
	0x7f, 0x04, 0x7c, 0x00, 	// h		ASCII--104
	0x7d, 0x00, 0x00, 0x00,  	// i		ASCII--105
	0x40, 0x7d,	0x00, 0x00, 	// j		ASCII--106
	0x7f, 0x10, 0x6c, 0x00, 	// k		ASCII--107
	0x7f, 0x40, 0x00, 0x00, 	// l		ASCII--108

	0x7c, 0x04, 0x1c, 0x70,		// m		ASCII--109
	0x7c, 0x04, 0x7c, 0x00, 	// n		ASCII--110
	0x7c, 0x44, 0x7c, 0x00, 	// o		ASCII--111
	0x7c, 0x24, 0x3c, 0x00, 	// p		ASCII--112
	0x3c, 0x24, 0x7c, 0x00, 	// q		ASCII--113
	0x7c, 0x04, 0x04, 0x00, 	// r		ASCII--114
	0x5c, 0x54, 0x74, 0x00, 	// s		ASCII--115
	0x04, 0x7e, 0x44, 0x00, 	// t		ASCII--116
	0x7c, 0x40, 0x7c, 0x00, 	// u		ASCII--117
	0x7c, 0x20, 0x1c, 0x00, 	// v		ASCII--118
	0x7c, 0x40, 0x70, 0x1c,		// w		ASCII--119
	0x6c, 0x10, 0x6c, 0x00, 	// x		ASCII--120
	0x5c, 0x50, 0x7c, 0x00, 	// y		ASCII--121
	0x64, 0x54, 0x4c, 0x00, 	// z		ASCII--122

	0x08, 0x7f, 0x41, 0x00, 	// {		ASCII--123
	0x7f, 0x00, 0x00, 0x00, 	// |		ASCII--124
	0x41, 0x7f, 0x08, 0x00, 	// }		ASCII--125
	0x01, 0x03, 0x02, 0x00, 	// ~		ASCII--126
	0x7f, 0x7f, 0x7f, 0x00, 	// Full Filled		ASCII--127

	0x22, 0x3E, 0x01, 0x00,		//℃		~ASCII-128
	0x00, 0x6C, 0x00, 0x00,		//dot up for time ~ASCII-129
	0x00, 0x36, 0x00, 0x00,		//dot down fot time ~ASCII-130

};
//RTC define
#define TZ 8       // (utc+) TZ in hours
#define DST_MN	0      // use 60mn for summer time in some countries
//
#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)

//unix timestamp
//time_t timestamp; //typedef long time_t. this define  in sdk file.
bool cbtime_set = false; //set time callback flag

OneWire  ds(DS_PIN);
byte ds_addr[8];

unsigned int localPort = 2390;      // local port to listen for UDP packets
IPAddress timeServerIP; 
const char* ntpServerName = "ntp1.aliyun.com";  //ali ntp server
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
WiFiUDP udp;// A UDP instance to let us send and receive packets over UDP

/**
	+---------+                +---------------------------------------------------+
	|         |GPIO13------DATA|                                                   |
	|   ESP   |GPIO14-------CLK|                                                   |
	|   8266  |GPIO15--------CS|                MAX7219   MODULE                   |
	|         |             GND|                                                   |
	|         |           +3.3V|                                                   |
	+---------+                +---------------------------------------------------+
*/

LedControl max7219=LedControl(LED_DAT,LED_CLKPIN,LED_CSPIN,LED_NUM_DEVICES);

byte display_buff[LED_NUM_COL] = {};
//time buff
char display_time_string_buff[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
//date buff
char display_date_string_buff[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
//temperature buff
char display_temp_string_buff[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

/******************************************************************
 * 显示数据动态数据部分
 ******************************************************************/
//Display data, variable array
typedef struct display_arr
{
	byte *data;
	byte size;
}*DISPLAY_ARR;

/**
 * 创建动态数组
 * @param  size 创建的数组长度
 * @return arr 返回显示结构体 
 */
DISPLAY_ARR dis_create_display_arr(byte size)
{
	DISPLAY_ARR arr = (DISPLAY_ARR)malloc(sizeof(struct display_arr));
	arr->data = (byte*)malloc(sizeof(byte)*size);
	arr->size = size;
	return arr;
}

/**
 * 动态数组内存释放
 * @param parr 结构体指针
 */
void dis_free_display_arr(DISPLAY_ARR *parr)
{
	if(parr == NULL)
		return;
	if(*parr == NULL)
		return;
	free((*parr)->data);
	free(*parr);
	*parr = NULL;
} 

/**
 * 返回动态数组中某一位的值
 * @param  arr 结构体
 * @param  x   位置
 * @return     byte数据
 */
byte dis_get_display_array_value(DISPLAY_ARR arr ,byte x)
{
	if(arr == NULL)
		return 0;
	return arr->data[x];
}

/**
 * 设置动态数组中的某一位的值
 * @param arr 结构体
 * @param x   位置
 * @param e   设置的byte值
 */
void dis_set_display_array_value(DISPLAY_ARR arr,byte x,byte e)
{
	if(arr == NULL)
		return;
	arr->data[x] = e;
}


/******************************************************************
 * 显示程序部分
 ******************************************************************/
 /**
  * MAX7219初始化
  * 设置为最小亮度
  */
void dis_max7219_init()
{
	for(byte address = 0; address < LED_NUM_DEVICES; address++)
	 {
		max7219.shutdown(address,false);
		max7219.setIntensity(address,1);
		max7219.clearDisplay(address);
	}
}

/**
 * 清空点阵屏
 */
void dis_clear_all()
{
	for (int i = 0; i < LED_NUM_DEVICES; ++i)
	{
		max7219.clearDisplay(i);
	}
}

/**
 * 自动亮度
 * 根据ADC获取的值进行自动设置
 */
void dis_auto_set_intensity()
{
	byte i = adc_get_light_intensity();
	i = i > 16 ? 15:i;//亮度范围0~15
	//todo:逐级调整
	DOUT(i);
	for(byte address = 0; address < LED_NUM_DEVICES; address++)
	 {
		max7219.setIntensity(address,i);
	}
}

/**
 * 显示函数
 * 逐列扫描
 */
void dis_display()
{
	byte column = 0;
	for (int j = 0; j < 4; ++j)
	{
		for (int i = 7; i >= 0; --i)
		{
			max7219.setColumn(j, i, display_buff[column++]);
		}
	}
}

/**
 * 【时间、日期、温度】单屏显示数据加载到显示缓存
 * @param s 显示字符数组
 */
void dis_load_string_in_disbuff(char s[])
{
	char * _p = s;
	int len = strlen(s);
	if (len > LED_NUM_CHAR)
	{
		DOUT("IN dis_load_string_in_disbuff: the s[] is to long.");
		return ;
	}
	int i = (LED_NUM_COL  - (len * (FONT_WIDTH))) / 2;//居中显示
	if (i > 0)
	{
		memset(display_buff, 0x00, i); //预置前面数据
	}
	while (len)
	{
		//一次读取4个字节，点阵字库的一个显示字符
		int _ft_index = (*_p) * (FONT_WIDTH);
		display_buff[i++] = pgm_read_byte(font + _ft_index);
		display_buff[i++] = pgm_read_byte(font + _ft_index + 1);
		display_buff[i++] = pgm_read_byte(font + _ft_index + 2);
		display_buff[i++] = pgm_read_byte(font + _ft_index + 3);
		_p++;len--;
	}
	if(i < LED_NUM_COL)//补足后面数据
	{
		memset(display_buff + i, 0x00, LED_NUM_COL - i);
	}
}

/**
 * 【滚动】所有显示数据加载到动态显示数组上
 * @param char s[] 显示内容
 * @param DISPLAY_ARR * d_arr 动态显示数组指印
 */
void dis_load_display_string_in_disarray(char s[], DISPLAY_ARR * d_arr)
{
	char * _p = s;
	//todo:显示字符大于申请内存,不处理多于内容 
	int i = 0;
	int len = ((*d_arr)->size) / FONT_WIDTH;
	while (len)
	{
		//一次读取4个字节，点阵字库的一个显示字符
		int _ft_index = (*_p) * FONT_WIDTH;
		(*d_arr)->data[i++] = pgm_read_byte(font + _ft_index);
		(*d_arr)->data[i++] = pgm_read_byte(font + _ft_index + 1);
		(*d_arr)->data[i++] = pgm_read_byte(font + _ft_index + 2);
		(*d_arr)->data[i++] = pgm_read_byte(font + _ft_index + 3);
		_p++;len--;
	}
	if(i < LED_NUM_COL)//补足后面数据，避免未清空buff使用时的数据错乱
	{
		memset(display_buff + i, 0x00 , LED_NUM_COL - i);
	}
}

/**
 * 【滚动】左移动动画
 * @param d_buff 显示buff
 * @param d_arr  显示动态数组
 */
void dis_move_left(byte * d_buff, DISPLAY_ARR * d_arr)
{
	//todo:直接传入显示数组byte *
	byte _size = (*d_arr)->size;
	if (_size >= 32)
	{
		/* 超过最大列，左移 */
		for (int i = 1; i < (LED_NUM_COL + 1); ++i)
		{
			memcpy(d_buff + LED_NUM_COL - i,(*d_arr)->data,i);
			dis_display();
			delay(SPEED);
		}
		for (int i = 1; i <= (_size - LED_NUM_COL); ++i)
		{
			memcpy(d_buff,(*d_arr)->data + i,LED_NUM_COL);
			dis_display();
			delay(SPEED);
		}
	}
	else
	{
		/* 小于最宽列，左移 */
		for (int i = 1; i < (LED_NUM_COL - _size); ++i)
		{
			memcpy(d_buff + LED_NUM_COL - _size - i,&((*d_arr)->data),i);
			dis_display();
			delay(SPEED);
		}
	}
}

/**
 * 【滚动】左移显示字符串，阻塞
 * @param s[] 待显示的字符串数组
 */
void dis_display_string(char s[])
{
	byte s_len = strlen(s);
	DISPLAY_ARR _d_arr = dis_create_display_arr(s_len * FONT_WIDTH);
	memset(display_buff, 0x00 , 32);

	dis_load_display_string_in_disarray(s, &_d_arr);
	dis_move_left(display_buff, &_d_arr);
	dis_free_display_arr(&_d_arr);
}

/**
 * 【时间、日期、温度】左移出显示
 */
void dis_datetime_move_out()
{
	byte _t_buff[LED_NUM_COL] = {};
	memcpy(_t_buff, display_buff, LED_NUM_COL);

	for (int i = 1; i < LED_NUM_COL; ++i)
	{
		memset(display_buff, 0x00, LED_NUM_COL);
		memcpy(display_buff, _t_buff + i, LED_NUM_COL - i);
		dis_display();
		delay(1);
	}
	//最后一位
	memset(display_buff, 0x00, 1);
	dis_display();
	//delay(5);
}

/**
 * 【时间、日期、温度】上移出显示
 */
void dis_datetime_move_out_2()
{
	int len = strlen(display_time_string_buff);//todo:临时解决长度问题
	int index_last_font = (LED_NUM_COL  + (len * (FONT_WIDTH)))/ 2 - 1;//最后一位
	while(len)
	{
		for (int i = 0; i < 8; ++i)
		{
			display_buff[index_last_font]      >>= 1;
			display_buff[index_last_font - 1]  >>= 1;
			display_buff[index_last_font - 2]  >>= 1;
			display_buff[index_last_font - 3]  >>= 1;
			
			dis_display();
			delay(1);
		}
		index_last_font -= 4;
		len--;
	}
}

/**
 * 【时间、日期、温度】左移入显示
 */
void dis_datetime_move_in()
{
	//todo:动态内存
	byte _t_buff[LED_NUM_COL] = {};
	memcpy(_t_buff, display_buff, LED_NUM_COL);
	memset(display_buff, 0x00, LED_NUM_COL);

	for (int i = 1; i < (LED_NUM_COL + 1); ++i)
	{
		memcpy(display_buff + LED_NUM_COL - i, _t_buff, i);
		dis_display();
		delay(1);
	}
	//缓冲动画
	memcpy(display_buff, _t_buff + 1, LED_NUM_COL - 1);
	memset(display_buff + (LED_NUM_COL - 1), 0x00, 1);
	dis_display();
	delay(20);
	memcpy(display_buff, _t_buff, LED_NUM_COL);
	dis_display();
	delay(5);
}

/**
 * 【时间、日期、温度】上移入显示
 */
void dis_datetime_move_in_2()
{
		//todo:动态内存
		//todo:最后一位小动画
	byte _t_buff[LED_NUM_COL] = {};
	memcpy(_t_buff, display_buff, LED_NUM_COL);
	memset(display_buff, 0x00, LED_NUM_COL);
	int len = strlen(display_time_string_buff);//临时解决长度问题
	int index_last_font = (LED_NUM_COL  + (len * (FONT_WIDTH)))/ 2 - 1;//最后一位
	while(len)
	{
		for (int i = 7; i >= 0; --i)
		{
			display_buff[index_last_font]      = _t_buff[index_last_font]       << i;
			display_buff[index_last_font - 1]  = _t_buff[index_last_font - 1]   << i;
			display_buff[index_last_font - 2]  = _t_buff[index_last_font - 2]   << i;
			display_buff[index_last_font - 3]  = _t_buff[index_last_font - 3]  << i;
			
			dis_display();
			delay(1);
		}
		index_last_font -= 4;
		len--;
	}
}

/**
 * 【时间、日期、温度】显示数据获取
 */
void dis_set_datetimetemp_dis_buff()
{
	struct tm _tm_time;
	rtc_get_time(&_tm_time);
	byte temp[2] = {0x00, 0x00};
	if(ds_addr[0])//判断18B20是否正确初始化
	{
		ds18b20_get_temp(temp, ds_addr);
	}
	else
	{
		ds18b20_init(ds_addr);
		if(ds_addr[0])
		{
			ds18b20_get_temp(temp, ds_addr);
		}
	}

	//+48 is num2char
	display_time_string_buff[0] = (char)(_tm_time.tm_hour / 10 + 48);
	display_time_string_buff[1] = (char)(_tm_time.tm_hour % 10 + 48);
	display_time_string_buff[2] = 0x01; //special font
	display_time_string_buff[3] = (char)(_tm_time.tm_min / 10 + 48);
	display_time_string_buff[4] = (char)(_tm_time.tm_min % 10 + 48);

	display_date_string_buff[0] = (char)(_tm_time.tm_mon / 10 + 48);
	display_date_string_buff[1] = (char)(_tm_time.tm_mon % 10 + 48);
	display_date_string_buff[2] = 0x04;//special font
	display_date_string_buff[3] = (char)(_tm_time.tm_mday / 10 + 48);
	display_date_string_buff[4] = (char)(_tm_time.tm_mday % 10 + 48);

	display_temp_string_buff[0] = (char)(temp[0] / 10 + 48);
	display_temp_string_buff[1] = (char)(temp[0] % 10 + 48);
	display_temp_string_buff[2] = '.';
	display_temp_string_buff[3] = (char)(temp[1] + 48);
	display_temp_string_buff[4] = 0x03;//special font
}

/**
 * 【时间、日期、温度】更新时间显示缓存
 */
void dis_relese_time()
{
	dis_load_string_in_disbuff(display_time_string_buff);
}

/**
 * 【时间、日期、温度】更新日期显示缓存
 */
void dis_relese_date()
{
	dis_load_string_in_disbuff(display_date_string_buff);
}

/**
 * 【时间、日期、温度】更新温度显示缓存
 */
void dis_relese_temp()
{
	dis_load_string_in_disbuff(display_temp_string_buff);
}

/**
 * 【时间、日期、温度】切换显示，动画随机
 */
void dis_display_date2time_trans(byte t)
{
	int r = random(1, 3);
	dis_set_datetimetemp_dis_buff();
	switch(r)
	{
		case 1:
			dis_datetime_move_out();
			break;
		case 2:
			dis_datetime_move_out_2();
			break;
		default:
			dis_datetime_move_out();
	}

	switch(t)
	{
		case 1:
			dis_relese_time();
			break;
		case 2:
			dis_relese_date();
			break;
		case 3:
			dis_relese_temp();
			break;
		default:
			dis_relese_time();
	}
	r = random(1, 3);
	switch(r)
	{
		case 1:
			dis_datetime_move_in();
			break;
		case 2:
			dis_datetime_move_in_2();
			break;
		default:
			dis_datetime_move_in();
	}
}

/******************************************************************
 * Wifi部分
 ******************************************************************/

struct station_config wifi_conf; //wifi 配置结构体
STATUS scan_done_state;  //wifi 扫描状态

// typedef enum {
//     OK = 0,
//     FAIL,
//     PENDING,
//     BUSY,
//     CANCEL,
// } STATUS;

/**
 * wifi 初始化，先smartconfig,再尝试连接已存储wifi
 */
void wifi_init()
{
	WiFi.mode(WIFI_STA);
	if(!wifi_smart_config())
	{
		wifi_auto_connect();
	}
}

/**
 * 自动连接已存储的wifi 
 */
void wifi_auto_connect()
{
	wifi_station_get_config_default(&wifi_conf);//获取连接配置

	if(os_strlen((char *)wifi_conf.ssid) != 0)//保存有wifi信息
	{
		DOUT("get the wifi config");
		char ss[] = "Scan The Last Used AP ";
		dis_display_string(ss);
		wifi_scan();//扫描是在已连接的wifi是否在线
		byte i = 100;//等10秒扫描结果 
		while(i--)
		{
			if(scan_done_state == OK)
			{
				DOUT("use the config connect the ap");
				char ss[] = "Connect The Last Used AP ";
				dis_display_string(ss);
				wifi_station_connect();//如果存在，直接连接
				wifi_station_dhcpc_start();//开户DHCP
				return ;
			}
			delay(100);
		}
		DOUT("Scan failed,start smartconfig");
		char ss1[] = "Not Find The AP ";
		dis_display_string(ss1);
		wifi_smart_config();//未扫描到，进行smartconfig
	}
	else
	{
		DOUT("can not find the wifi config,start smartconfig");
		wifi_smart_config();//未找到存储的wifi,进行smartconfig
	}
	
}

/**
 * wifi信号扫描，回调方式
 */
void wifi_scan()
{
   struct scan_config config;

   os_memset(&config, 0, sizeof(config));

   config.ssid = wifi_conf.ssid;

   wifi_station_scan(&config, wifi_scan_done);

}

/**
 * wifi信号扫描完成回调函数
 * @param arg    扫描到的wifi配置参数
 * @param status 扫描结果状态
 */
void wifi_scan_done(void *arg, STATUS status)
{
	DOUT("status:");
	DOUT(status);
	if (status == OK)
	{
		scan_done_state = status;
	}
}

 /**
  * SmartConfig配置连网
  */
byte wifi_smart_config()
{
	WiFi.beginSmartConfig();
	DOUT("begin smartconfig");
	char ss[] = "Begin Smartconfig ";
	dis_display_string(ss);
	char ss1[] = "Wait Smartconfig ";
	byte i = 10;
	while (i--)
	{
		dis_display_string(ss1);
		digitalWrite(LED_BUILTIN, 0);
		delay(100);
		digitalWrite(LED_BUILTIN, 1);
		delay(900);
		if (WiFi.smartConfigDone())
		{
			DOUT("SmartConfig Success");
			char ss[] = "SmartConfig Success ";
			dis_display_string(ss);
			DOUT("SSID:");
			DOUT(WiFi.SSID().c_str());
			DOUT("PSW:");
			DOUT(WiFi.psk().c_str());
			delay(900);//wait for see the message.
			return 1;
		}
	}
	WiFi.stopSmartConfig();
	return 0;
}

/******************************************************************
 * RTC时间存取部分
 ******************************************************************/
/**
 * RTC时间设置回调函数
 */
void rtc_time_is_set ()
{
	timeval cbtime;
	gettimeofday(&cbtime, NULL);
	cbtime_set = true;
}

/**
 * 设置RTC时间
 * @param t UNIX时间戳
 */
void rtc_set_time(time_t t)
{
	// ESP.eraseConfig();
	timeval tv = { t, 0 };
	timezone tz = { TZ_MN + DST_MN, 0 };
	settimeofday(&tv, &tz);
}

/**
 * 获取RTC时间
 * @param tm_time tm时间结构体
 * 	// struct tm
 *	// {
 *	//   int	tm_sec;
 *	//   int	tm_min;
 *	//   int	tm_hour;
 *	//   int	tm_mday;
 *	//   int	tm_mon;
 *	//   int	tm_year;
 *	//   int	tm_wday;
 *	//   int	tm_yday;
 *	//   int	tm_isdst;
 *	// };
 */
void rtc_get_time(struct tm * tm_time)
{
	timeval tv;
	gettimeofday(&tv, nullptr);
	DOUT("rtc_get_time   &tv.tv_sec:");
	DOUT(tv.tv_sec);
	tv.tv_sec += TZ_SEC;
	*tm_time = *localtime(&tv.tv_sec);
	tm_time->tm_mon += 1;
	tm_time->tm_year += 1900;
}

/**
 * RTC初始化
 */
void rtc_init()
{
	settimeofday_cb(rtc_time_is_set);
	rtc_set_time(1520168766); //预置时间，项目开始时间
}

/******************************************************************
 * NTP授时部部分
 ******************************************************************/
void ntp_udp_init()
{
	//nothing here
}

/**
 * 发送NTP包
 * @param address NTP授时服务器地址
 */
void ntp_sendNTPpacket(IPAddress& address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

/**
 * 获取NTP授时服务器时间
 * @return UNIX时间戳
 */
time_t ntp_get_time()
{
	int i = 0;
	while (WiFi.status() != WL_CONNECTED) 
	{
	    delay(500);
	    DOUT(".");
	    if(20 == i++)
	    	{
	    		DOUT("In ntp_get_time() : get WIFI.status time out.");
	    		return 0;
	    	}
	}
	udp.begin(localPort);
	char ss[] = "Start Get Ntp Time ";
	dis_display_string(ss);
	
	WiFi.hostByName(ntpServerName, timeServerIP); 
	ntp_sendNTPpacket(timeServerIP); // send an NTP packet to a time server
	// wait to see if a reply is available
	delay(1000);
	int cb = udp.parsePacket();
	if (!cb)
	{
		DOUT("no packet yet");
		udp.stop();
		return 0;
	}
	else
	{
		DOUT("get the packet,begin to calculate the unix timestamp");
		udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
		unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
		unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
		// combine the four bytes (two words) into a long integer
		// this is NTP time (seconds since Jan 1 1900):
		unsigned long secsSince1900 = highWord << 16 | lowWord;
		unsigned long secsSince1970 = secsSince1900 -2208988800UL;
		DOUT("secsSince1970");
		DOUT(secsSince1970);
		char ss[] = "Get Ntp Time Success ";
		dis_display_string(ss);
		udp.stop();
		return secsSince1970;
	}
}

/**
 * 同步时间，更新rtc时间
 */
void ntp_sync_time()
{
	byte i = 10;
	while(i--)//尝试10次
	{
		time_t _t = ntp_get_time();
		if(_t)//获取到时间
		{
			rtc_set_time(_t);
			return;
		} 
		delay(1000);
	}
}
/******************************************************************
 * ADC程序部分
 ******************************************************************/

/**
 * 初始化亮度，需要在max7219_init后
 */
void adc_init()
{
	dis_auto_set_intensity();
}

/**
 * 读取光敏电阻值，并返回亮度比例
 * @return 0~15 整数值
 */
byte adc_get_light_intensity()
{
	return (byte)(analogRead(A0)/64);
}

/******************************************************************
 * DS18B20程序部分
 ******************************************************************/
/**
 * 18b20初始化，获得器件地址
 * @param  addr 地址指针
 * @return      结果状态
 */
byte ds18b20_init(byte * addr)
{
	if ( !ds.search(addr)) 
	{
		Serial.println("No more addresses.");
		Serial.println();
		ds.reset_search();
		delay(250);
		Serial.print("ROM =");
		return 0;
	}
	else 
	{
			for(int i = 0; i < 8; i++)
		{
			Serial.write(' ');
			Serial.print(addr[i], HEX);
		}
		if (OneWire::crc8(addr, 7) != addr[7])
		{
			Serial.println("CRC is not valid!");
			return 0 ;
		}
		return 1;
	}

}

/**
 * 获取温度值，摄氏度
 * @param  temp byte温度数组，共2byte，分别为整数位和小数位
 * @param  addr 器件地址
 * @return      获取状态
 */
byte ds18b20_get_temp(byte * temp, byte * addr )
{
	//todo:debug
	byte present = 0;
	byte type_s;
	byte data[12];
	float celsius;
	// the first ROM byte indicates which chip
	switch (addr[0]) 
	{
		case 0x10:
		Serial.println("  Chip = DS18S20");  // or old DS1820
		type_s = 1;
		break;
		case 0x28:
		Serial.println("  Chip = DS18B20");
		type_s = 0;
		break;
		case 0x22:
		Serial.println("  Chip = DS1822");
		type_s = 0;
		break;
		default:
		Serial.println("Device is not a DS18x20 family device.");
		addr[0] = 0;
		return 0;
	} 

	ds.reset();
	ds.select(addr);
	ds.write(0x44, 1);        // start conversion, with parasite power on at the end

	delay(1000);     // maybe 750ms is enough, maybe not
	// we might do a ds.depower() here, but the reset will take care of it.

	present = ds.reset();
	ds.select(addr);    
	ds.write(0xBE);         // Read Scratchpad

	Serial.print("  Data = ");
	Serial.print(present, HEX);
	Serial.print(" ");
	for ( int i = 0; i < 9; i++) 
	{           // we need 9 bytes
		data[i] = ds.read();
		Serial.print(data[i], HEX);
		Serial.print(" ");
	}
	Serial.print(" CRC=");
	Serial.print(OneWire::crc8(data, 8), HEX);
	Serial.println();

	// Convert the data to actual temperature
	// because the result is a 16 bit signed integer, it should
	// be stored to an "int16_t" type, which is always 16 bits
	// even when compiled on a 32 bit processor.
	int16_t raw = (data[1] << 8) | data[0];
	if (type_s) 
	{
		raw = raw << 3; // 9 bit resolution default
		if (data[7] == 0x10) 
		{
			// "count remain" gives full 12 bit resolution
			raw = (raw & 0xFFF0) + 12 - data[6];
		}
	} 
	else 
	{
		byte cfg = (data[4] & 0x60);
		// at lower res, the low bits are undefined, so let's zero them
		if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
		else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
		else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
		//// default is 12 bit resolution, 750 ms conversion time
	}
		celsius = (float)raw / 16.0;
		*temp = (byte)celsius;
		*(temp + 1) = (byte)((celsius - ((byte)celsius)) * 10);//只保留一位小数
		return 1;
}

/******************************************************************
 * 主程序部分
 ******************************************************************/
void setup() 
{
	#ifdef DEBUG
	Serial.begin(115200);
	#endif
	
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, 1);

	dis_max7219_init();
	wifi_init();
	while (WiFi.status() != WL_CONNECTED) 
	{
	    delay(500);
	    DOUT(".");
	}
	DOUT("Wifi Connected!");
	char ss[] = "Wifi Connected ";
	dis_display_string(ss);
	delay(1000);

	ds18b20_init(ds_addr);
	rtc_init();
	adc_init();
	ntp_udp_init();
	
	ntp_sync_time();
	delay(1000);
}

int count = 0;//显示计数器
byte t_flag = 1;//显示内容标识

void loop() 
{
	if(count % 10 == 0)//每10秒转换显示
	{
		dis_display_date2time_trans(t_flag);
		++t_flag = t_flag == 4 ?1:t_flag;
	}

	if(28800 == count)//每7小时尝试NTP授时
	{
		ntp_sync_time();
	}

	dis_auto_set_intensity();//自动亮度

	count = count>=30000?0:(count+1);//计数器重置
	delay(990);
}
