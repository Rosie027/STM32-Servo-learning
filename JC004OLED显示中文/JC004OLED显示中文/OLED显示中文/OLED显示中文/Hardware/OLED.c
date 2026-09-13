#include "stm32f10x.h"
#include "OLED_Font.h"
#include "OLED.h"

/* 引脚配置宏 */
#define OLED_W_SCL(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(x))
#define OLED_W_SDA(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)(x))

/* 问题：一般都会使用这两个函数名的修改名字后的宏常量吗？
   答：是的，这种宏定义是标准做法，便于移植和阅读，把硬件操作抽象为函数名。
   补充：宏定义中的 GPIO_WriteBit 函数第三个参数需要强制类型转换 (BitAction)(x)，确保传入0/1。 */

/* 缓冲区：8页 × 128列，每字节代表一列上的8个垂直像素（1 bit=1像素） */
static uint8_t OLED_Buffer[8][128];
/* 问题：缓冲区是什么？
   答：缓冲区是内存中的一块区域，保存了当前屏幕所有像素点数据。操作时先在缓冲区修改，再一次性或按需刷到屏幕，避免频繁I2C通信，提高效率并支持画点等操作。
   补充：缓冲区的内容与屏幕显存一一对应，任何修改屏幕的函数都应同步更新缓冲区，否则画点等操作会失效。 */

static uint8_t current_page = 0;    // 当前页（0~7）
static uint8_t current_col = 0;     // 当前列（0~127）

/* 引脚初始化 */
void OLED_I2C_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    /* 问题：OLED必须在APB2吗？必须在GPIOB吗？
       答：所有GPIO时钟都在APB2，不是OLED特殊。GPIOB是开发板硬件连接决定的，可以换成其他GPIO，只要修改引脚宏定义。 */

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    /* 问题：哪些外设和OLED是一样的模式？这个模式的原理、选择原因？
       答：I2C总线设备（如EEPROM、温湿度传感器）都用开漏输出。原理：输出高电平靠外部上拉电阻，低电平由NMOS拉低。
       选择原因：I2C是多主机总线，开漏可实现“线与”功能，避免总线冲突。OLED的I2C接口要求SDA/SCL为开漏。
       补充：若使用硬件I2C外设，GPIO模式也必须配置为复用开漏（AF_OD）。这里使用软件模拟I2C，所以普通开漏即可。 */

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    /* 问题：OLED选择50的原因？修改成其他会有什么变化？
       答：I2C速率通常400kHz以下，2MHz都够，但50MHz是通用配置，不影响功能，只是功耗略高。改低不影响功能，可降低EMI。 */

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_WriteBit(GPIOB, GPIO_Pin_8, Bit_SET);
    GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_SET);
    /*
    问题：GPIO_WriteBit的参数：无论是BSRR还是BRR，它们都是“写1有效，写0无效”，所以只需要传入0或1，而不需要读取现在状态？
    答：正确。传入Bit_SET（1）操作BSRR置位，传入Bit_RESET（0）操作BRR复位，硬件自动处理，无需读当前状态。
    补充：初始化时拉高SDA和SCL，为后续I2C通信准备空闲状态。
    */
}

/**
  * @brief  I2C开始
  */
void OLED_I2C_Start(void)
{
    OLED_W_SDA(1);
    OLED_W_SCL(1);
    OLED_W_SDA(0);
    /*
    I2C协议：起始信号 = SCL高时，SDA从高到低跳变。
    停止信号 = SCL高时，SDA从低到高跳变。
    */
    OLED_W_SCL(0); // 拉低SCL，便于后续传输数据时不误判起始/停止。
}

/**
  * @brief  I2C停止
  */
void OLED_I2C_Stop(void)
{
    OLED_W_SDA(0);
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

/**
  * @brief  I2C发送一个字节
  * @param  Byte 要发送的一个字节
  */
void OLED_I2C_SendByte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        /* 问题：详细解释括号内，看不明白
           答：Byte & (0x80 >> i) 用于逐位取出Byte的最高位到最低位。
           i=0时 0x80>>0=0x80（1000 0000），Byte & 0x80 取出最高位；
           i=1时 0x80>>1=0x40（0100 0000），取出次高位……依此类推。
           结果非0则写1，否则写0。这样依次发送8个数据位。 */
        OLED_W_SDA(Byte & (0x80 >> i));
        OLED_W_SCL(1); // 上升沿，从机采样
        OLED_W_SCL(0);
    }
    OLED_W_SCL(1);    // 额外的一个时钟，不处理应答信号（忽略从机ACK）
    OLED_W_SCL(0);
    /* 问题：可以得到的结果，一般什么场景使用？
       答：此函数是I2C底层发送字节的通用函数，可用于任何I2C通信（写寄存器、写数据等）。
       补充：实际产品中最好检测应答位以确保通信可靠，这里简化处理。 */
}

/**
  * @brief  OLED写命令
  */
void OLED_WriteCommand(uint8_t Command)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);		// 从机地址
    /* 问题：这个是如何确定的？需要修改吗？
       答：0x78是OLED SSD1306的默认I2C从机地址（7位地址0x3C，左移一位加写位=0x78）。
       一般不需要改，除非硬件上SA0引脚接法不同（地址会变），参考OLED手册。 */
    OLED_I2C_SendByte(0x00);		// 控制字节：后续为命令
    /* 问题：有什么作用？下一步呢？
       答：0x00表示后续发送的是命令字节（控制字节），OLED内部据此区分命令和数据。
       下一步发送真正的命令值（Command）。 */
    OLED_I2C_SendByte(Command);
    OLED_I2C_Stop();
}

/**
  * @brief  OLED写数据，并更新缓冲区及当前列
  */
void OLED_WriteData(uint8_t Data)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78); // 从机地址
    OLED_I2C_SendByte(0x40); // 控制字节：后续为数据
    OLED_I2C_SendByte(Data);
    OLED_I2C_Stop();

    // 更新缓冲区并递增列
    OLED_Buffer[current_page][current_col] = Data;
    current_col++;
    if (current_col >= 128) {
        current_col = 0; // 列地址回绕（实际上需根据OLED寻址模式，此处简化）
    }
    /* 问题：上一个函数和这个函数最大的区别就是有么有缓冲区，所以区别和意义是什么？使用场景呢？
       答：WriteData更新缓冲区，WriteCommand不更新。缓冲区用于维护屏幕内容的副本，便于后续画点、读回等。
       使用场景：需要单独修改某个像素或实现图形操作时，必须依赖缓冲区；纯文本显示可以不使用缓冲区，但画点功能就必须有。
       补充：此函数还自动维护当前列，但实际OLED内部列地址由硬件管理，这里是为了保持软件缓冲区与硬件同步。 */
}

/**
  * @brief  OLED设置光标位置，并更新当前页和列
  */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
    current_page = Y;
    current_col = X;
    OLED_WriteCommand(0xB0 | Y);                // 设置页地址（0xB0~0xB7）
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4)); // 设置列地址高4位（0x10~0x1F）
    OLED_WriteCommand(0x00 | (X & 0x0F));         // 设置列地址低4位（0x00~0x0F）
    /* 问题：这几行干了什么？写入了什么命令？
       答：OLED采用页寻址模式，先设置页（0~7），再设置列（0~127）。
       命令格式：0xB0+页号；列高四位通过0x10+高四位，低四位通过0x00+低四位。
       补充：列地址低4位命令通常写作0x00 | (X & 0x0F)，但实际SSD1306允许直接写0x00~0x0F，也可只写0x0F，标准做法是写0x00~0x0F。 */
}

/**
  * @brief  OLED清屏，并清空缓冲区
  */
void OLED_Clear(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++) {
        OLED_SetCursor(j, 0);
        for (i = 0; i < 128; i++) {
            OLED_WriteData(0x00); // 清空屏幕并更新缓冲区
        }
    }
}

/**
  * @brief  OLED显示一个字符（8x16字体）
  */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
    uint8_t i;
    OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);		// 上半部分
    /* 问题：这一步的意义是什么？如果不是不是就随机位置显示了？
       答：字符显示需要精确位置，否则会覆盖其他内容。这里将逻辑行（1~4）映射到实际页（0~7），列（1~16）映射到列（0~127）。
       如果不设光标，默认会在上次操作的位置继续，导致乱序。 */
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i]);			// 上半部分数据
        /* 问题：Char - ' '是什么意思？
           答：字库数组下标从0开始，而ASCII空格' '是32，所以' '- ' '=0，'A'-' '=33，对应字库中的索引。 */
    }
    OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	// 下半部分
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);		// 下半部分数据
    }
    /* 问题：所以如果一个字符是8位，显示8位的16进制的数字，显示出一个字符，一个for循环中显示一个字符？那两个for循环的意义是什么？
       答：8x16字体，垂直方向16像素，分上下两页（每页8像素）。上半部分8字节对应上半8行，下半部分8字节对应下半8行。
       两个for循环分别写上半和下半，共同组成一个完整字符。 */
}

/**
  * @brief  OLED显示字符串
  */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
    {
        OLED_ShowChar(Line, Column + i, String[i]);
    }
}

/**
  * @brief  OLED次方函数（内部使用）
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
    /* 问题：本质上就是一个数学运算，和OLED的实际应用有什么关联？
       答：用于显示数字时，计算各位的权重（10^(len-1-i)等），从而提取数字的每一位。 */
}

/**
  * @brief  OLED显示数字（十进制，无符号）
  */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
        /* 问题：解释 Number / OLED_Pow(10, Length - i - 1) % 10 + '0' 是什么意思？
           答：先除以10^(剩余位数-1)得到最高位，再%10取个位，最后+'0'转为数字字符。
           例如 Number=123, Length=3, i=0: 123/100=1, 1%10=1, +'0'='1' */
    }
}

/**
  * @brief  OLED显示数字（十进制，带符号）
  */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
    uint8_t i;
    uint32_t Number1;
    if (Number >= 0)
    {
        OLED_ShowChar(Line, Column, '+');
        Number1 = Number;
    }
    else
    {
        OLED_ShowChar(Line, Column, '-');
        Number1 = -Number;
    }
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

/**
  * @brief  OLED显示数字（十六进制）
  */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i, SingleNumber;
    for (i = 0; i < Length; i++)
    {
        SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
        /* 问题：显示16进制和10进制和2进制，OLED一般可以显示什么进制？几个不一样的进制，是不是只有这行的数字不一样就可以了？
           答：OLED本身只显示字符，这些函数本质是把数值转换成对应的ASCII字符。对于16进制，需要把0~15转为'0'-'9'和'A'-'F'。
           不同进制只需改变基数（10/16/2）和转换逻辑，其他都一样。 */
        if (SingleNumber < 10)
        {
            OLED_ShowChar(Line, Column + i, SingleNumber + '0');
            /* 问题：为什么加0？
               答：将数字0~9转为字符'0'~'9'，ASCII码差值为'0'的值。 */
        }
        else
        {
            OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
            /* 问题：为什么-10后加A？
               答：将10~15转为'A'~'F'，ASCII码'A'对应65，10-10=0+'A'='A'。 */
        }
    }
}

/**
  * @brief  OLED显示数字（二进制）
  */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
    }
}

/**
  * @brief  OLED初始化
  */
void OLED_Init(void)
{
    uint32_t i, j;
    for (i = 0; i < 1000; i++)			// 上电延时
    {
        for (j = 0; j < 1000; j++);
    }
    /* 问题：这一步为什么需要有？
       答：OLED上电后需要等待内部复位稳定，否则后续命令可能无效。这是简单的延时，也可用精确的定时器。 */

    OLED_I2C_Init();			// 端口初始化

    /* 以下为SSD1306标准初始化命令序列 */
    OLED_WriteCommand(0xAE);	// 关闭显示

    OLED_WriteCommand(0xD5);	// 设置显示时钟分频比/振荡器频率
    OLED_WriteCommand(0x80);

    OLED_WriteCommand(0xA8);	// 设置多路复用率（64行）
    OLED_WriteCommand(0x3F);

    OLED_WriteCommand(0xD3);	// 设置显示偏移
    OLED_WriteCommand(0x00);

    OLED_WriteCommand(0x40);	// 设置显示开始行（0行）

    OLED_WriteCommand(0xA1);	// 设置段重映射（0xA1正常，0xA0左右反置）

    OLED_WriteCommand(0xC8);	// 设置COM扫描方向（0xC8正常，0xC0上下反置）

    OLED_WriteCommand(0xDA);	// 设置COM引脚硬件配置
    OLED_WriteCommand(0x12);

    OLED_WriteCommand(0x81);	// 设置对比度控制
    OLED_WriteCommand(0xCF);

    OLED_WriteCommand(0xD9);	// 设置预充电周期
    OLED_WriteCommand(0xF1);

    OLED_WriteCommand(0xDB);	// 设置VCOMH取消选择级别
    OLED_WriteCommand(0x30);

    OLED_WriteCommand(0xA4);	// 设置整个显示打开/关闭（0xA4恢复显示，0xA5全亮）

    OLED_WriteCommand(0xA6);	// 设置正常/倒转显示（0xA6正常，0xA7反显）

    OLED_WriteCommand(0x8D);	// 设置充电泵（必须开启，否则屏幕不亮）
    OLED_WriteCommand(0x14);

    OLED_WriteCommand(0xAF);	// 开启显示

    OLED_Clear();				// OLED清屏

    /* 问题：这些里面什么需要掌握和未来嵌入式研发迁移需要修改的？重要的？
       答：这些初始化序列基本固定，不同驱动IC略有差异。迁移时需根据新屏幕的数据手册修改命令值。
       关键是理解每一组命令的含义（如对比度、复用率、电荷泵等），但大多无需改动，直接复制即可。
       补充：0x8D 0x14 是电荷泵使能，非常重要，忘记开启屏幕无显示。 */
}

/**
  * @brief  OLED显示汉字（支持正反显示）
  */
void OLED_ShowCN(uint8_t Line, uint8_t Column, uint8_t Num, uint8_t mode)
{
    uint8_t i;
    uint8_t page_start = (Line - 1) * 2;   // 计算起始页（汉字占2页）
    uint8_t col_start = (Column - 1) * 16; // 计算起始列（汉字占16列）

    // 显示上半部分（前16字节）
    OLED_SetCursor(page_start, col_start);
    for (i = 0; i < 16; i++)
    {
        uint8_t data = OLED_F10x16[Num][i];
        OLED_WriteData(mode ? data : ~data); // 根据mode取反（反显）
    }

    // 显示下半部分（后16字节）
    OLED_SetCursor(page_start + 1, col_start);
    for (i = 16; i < 32; i++)
    {
        uint8_t data = OLED_F10x16[Num][i];
        OLED_WriteData(mode ? data : ~data);
    }
}

/**
  * @brief  OLED画点函数
  */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t mode)
{
    if (x >= 128 || y >= 64) return; // 坐标越界处理

    uint8_t page = y / 8;      // 计算页地址
    uint8_t bit_pos = y % 8;   // 计算页内位位置（0~7）

    // 修改缓冲区对应位
    if (mode) {
        OLED_Buffer[page][x] |= (1 << bit_pos);  // 点亮像素
    } else {
        OLED_Buffer[page][x] &= ~(1 << bit_pos); // 熄灭像素
    }

    // 更新到OLED屏幕
    OLED_SetCursor(page, x);
    OLED_WriteData(OLED_Buffer[page][x]);
    /* 补充：画点函数是图形操作的基础，必须依赖缓冲区，因为需要读取原有数据再修改某一位。
       如果不使用缓冲区，则无法实现“读-改-写”操作，只能整页覆盖。 */
}

/**
  * @brief  OLED显示位图（支持任意位置和尺寸）
  */
void OLED_ShowBMP(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bmp, uint8_t mode)
{
    uint8_t page_end = y + height / 8;
    uint8_t col_end = x + width;
    uint16_t index = 0;

    for (uint8_t page = y; page < page_end; page++)
    {
        OLED_SetCursor(page, x);
        for (uint8_t col = x; col < col_end; col++)
        {
            uint8_t data = bmp[index++];
            OLED_WriteData(mode ? data : ~data); // 根据mode取反
        }
    }
    /* 补充：位图数据要求宽度和高度必须是8的倍数，因为页寻址最小单位是8行。此函数可显示任意图片。 */
}

/* 总结补充：
   1. 缓冲区是绘图的核心，所有修改屏幕的函数（画点、清屏等）都应同步缓冲区。
   2. 初始化命令序列中电荷泵（0x8D 0x14）最易被忽略，导致屏幕不亮。
   3. I2C通信中忽略应答位虽简化，但在噪声大的环境下可能出错。
   4. OLED显示字符时，实际使用字模取模方式，需与取模软件格式匹配（通常为列行式、逆向、阴码）。
   5. 此代码使用软件模拟I2C，移植到其他MCU时只需修改GPIO操作宏即可。
*/