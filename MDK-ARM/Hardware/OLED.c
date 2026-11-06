#include "OLED.h"
#include "OLED_Data.h"
#include "FreeRTOS.h"
#include "semphr.h" 
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

/* ==================== 全局变量 ==================== */

/**
  * OLED 显存数组
  * 所有显示函数只写这个数组，调用 OLED_Update 才会真正发送到屏幕
  */
uint8_t OLED_DisplayBuf[8][128];

/* ==================== 底层通信（HAL 硬件 I2C） ==================== */

/**
  * 函    数：OLED写命令
  */
void OLED_WriteCommand(uint8_t Command)
{
    HAL_I2C_Mem_Write(&hi2c1,
                      OLED_I2C_ADDR,
                      0x00,                     // 控制字节：命令
                      I2C_MEMADD_SIZE_8BIT,
                      &Command,
                      1,
                      100);
}

/**
  * 函    数：OLED写数据
  * 参    数：Data  要写入数据的起始地址
  * 参    数：Count 要写入数据的数量
  */
void OLED_WriteData(uint8_t *Data, uint8_t Count)
{
    HAL_I2C_Mem_Write(&hi2c1,
                      OLED_I2C_ADDR,
                      0x40,                     // 控制字节：数据
                      I2C_MEMADD_SIZE_8BIT,
                      Data,
                      Count,
                      100);
}

/**
  * 函    数：引脚初始化（HAL 版）
  * 说    明：CubeMX 已经完成了 I2C 引脚配置，这里只需要给 OLED 上电延时
  */
static void OLED_GPIO_Init(void)
{
    HAL_Delay(50);
}

/* ==================== 硬件配置 ==================== */

/**
  * 函    数：OLED 初始化
  */
void OLED_Init(void)
{
    OLED_GPIO_Init();

    OLED_WriteCommand(0xAE);    // 关闭显示
    OLED_WriteCommand(0xD5);    // 时钟分频
    OLED_WriteCommand(0x80);
    OLED_WriteCommand(0xA8);    // 多路复用率
    OLED_WriteCommand(0x3F);
    OLED_WriteCommand(0xD3);    // 显示偏移
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x40);    // 起始行
    OLED_WriteCommand(0xA1);    // 左右方向
    OLED_WriteCommand(0xC8);    // 上下方向
    OLED_WriteCommand(0xDA);    // COM 引脚配置
    OLED_WriteCommand(0x12);
    OLED_WriteCommand(0x81);    // 对比度
    OLED_WriteCommand(0xCF);
    OLED_WriteCommand(0xD9);    // 预充电周期
    OLED_WriteCommand(0xF1);
    OLED_WriteCommand(0xDB);    // VCOMH
    OLED_WriteCommand(0x30);
    OLED_WriteCommand(0xA4);    // 整体显示
    OLED_WriteCommand(0xA6);    // 正常显示
    OLED_WriteCommand(0x8D);    // 充电泵
    OLED_WriteCommand(0x14);
    OLED_WriteCommand(0xAF);    // 开启显示

    OLED_Clear();
    OLED_Update();
}

/**
  * 函    数：设置光标位置
  * 参    数：Page 页地址 0~7
  * 参    数：X    列地址 0~127
  */
void OLED_SetCursor(uint8_t Page, uint8_t X)
{
    X += 2;   // 若使用 1.3 寸 SH1106，需解除此偏移

    OLED_WriteCommand(0xB0 | Page);
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));
    OLED_WriteCommand(0x00 | (X & 0x0F));
}

/* ==================== 工具函数 ==================== */

/**
  * 函    数：次方函数
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

/**
  * 函    数：自定义 round（ARMCC V5 无 round 函数）
  */
static double OLED_Round(double x)
{
    return (x >= 0.0) ? (double)(long)(x + 0.5) : (double)(long)(x - 0.5);
}

/**
  * 函    数：判断指定点是否在多边形内部
  */
uint8_t OLED_pnpoly(uint8_t nvert, int16_t *vertx, int16_t *verty, int16_t testx, int16_t testy)
{
    int16_t i, j, c = 0;
    for (i = 0, j = nvert - 1; i < nvert; j = i++)
    {
        if (((verty[i] > testy) != (verty[j] > testy)) &&
            (testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
        {
            c = !c;
        }
    }
    return c;
}

/**
  * 函    数：判断指定点是否在指定角度内
  */
uint8_t OLED_IsInAngle(int16_t X, int16_t Y, int16_t StartAngle, int16_t EndAngle)
{
    int16_t PointAngle;
    PointAngle = atan2(Y, X) / 3.14 * 180;

    if (StartAngle < EndAngle)
    {
        if (PointAngle >= StartAngle && PointAngle <= EndAngle)
        {
            return 1;
        }
    }
    else
    {
        if (PointAngle >= StartAngle || PointAngle <= EndAngle)
        {
            return 1;
        }
    }
    return 0;
}

/* ==================== 更新函数 ==================== */

/**
  * 函    数：将显存数组全部更新到屏幕
  */
void OLED_Update(void)
{
    uint8_t j;
    for (j = 0; j < 8; j++)
    {
        OLED_SetCursor(j, 0);
        OLED_WriteData(OLED_DisplayBuf[j], 128);
    }
}

/**
  * 函    数：将显存数组部分更新到屏幕
  */
void OLED_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t j;
    int16_t Page, Page1;

    Page = Y / 8;
    Page1 = (Y + Height - 1) / 8 + 1;
    if (Y < 0)
    {
        Page -= 1;
        Page1 -= 1;
    }

    for (j = Page; j < Page1; j++)
    {
        if (X >= 0 && X <= 127 && j >= 0 && j <= 7)
        {
            OLED_SetCursor(j, X);
            OLED_WriteData(&OLED_DisplayBuf[j][X], Width);
        }
    }
}

/**
  * 函    数：显存全部清零
  */
void OLED_Clear(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++)
    {
        for (i = 0; i < 128; i++)
        {
            OLED_DisplayBuf[j][i] = 0x00;
        }
    }
}

/**
  * 函    数：显存部分清零
  */
void OLED_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t i, j;

    for (j = Y; j < Y + Height; j++)
    {
        for (i = X; i < X + Width; i++)
        {
            if (i >= 0 && i <= 127 && j >= 0 && j <= 63)
            {
                OLED_DisplayBuf[j / 8][i] &= ~(0x01 << (j % 8));
            }
        }
    }
}

/**
  * 函    数：显存全部取反
  */
void OLED_Reverse(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++)
    {
        for (i = 0; i < 128; i++)
        {
            OLED_DisplayBuf[j][i] ^= 0xFF;
        }
    }
}

/**
  * 函    数：显存部分取反
  */
void OLED_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t i, j;

    for (j = Y; j < Y + Height; j++)
    {
        for (i = X; i < X + Width; i++)
        {
            if (i >= 0 && i <= 127 && j >= 0 && j <= 63)
            {
                OLED_DisplayBuf[j / 8][i] ^= 0x01 << (j % 8);
            }
        }
    }
}

/* ==================== 显示函数 ==================== */

/**
  * 函    数：显示一个字符
  */
void OLED_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize)
{
    if (FontSize == OLED_8X16)
    {
        OLED_ShowImage(X, Y, 8, 16, OLED_F8x16[Char - ' ']);
    }
    else if (FontSize == OLED_6X8)
    {
        OLED_ShowImage(X, Y, 6, 8, OLED_F6x8[Char - ' ']);
    }
    else if (FontSize == OLED_12X24)
    {
        OLED_ShowImage(X, Y, 12, 24, OLED_F12x24[Char - ' ']);
    }
}

/**
  * 函    数：显示字符串
  */
void OLED_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize)
{
    uint16_t i = 0;
    char SingleChar[5];
    uint8_t CharLength = 0;
    uint16_t XOffset = 0;
    uint16_t pIndex;

    while (String[i] != '\0')
    {
#ifdef OLED_CHARSET_GB2312
        if ((String[i] & 0x80) == 0x00)
        {
            CharLength = 1;
            SingleChar[0] = String[i++];
            SingleChar[1] = '\0';
        }
        else
        {
            CharLength = 2;
            SingleChar[0] = String[i++];
            if (String[i] == '\0') { break; }
            SingleChar[1] = String[i++];
            SingleChar[2] = '\0';
        }
#endif

        if (CharLength == 1)
        {
            OLED_ShowChar(X + XOffset, Y, SingleChar[0], FontSize);
            XOffset += FontSize;
        }
        else
        {
            for (pIndex = 0; strcmp(OLED_CF16x16[pIndex].Index, "") != 0; pIndex++)
            {
                if (strcmp(OLED_CF16x16[pIndex].Index, SingleChar) == 0)
                {
                    break;
                }
            }
            if (FontSize == OLED_8X16)
            {
                OLED_ShowImage(X + XOffset, Y, 16, 16, OLED_CF16x16[pIndex].Data);
                XOffset += 16;
            }
            else if (FontSize == OLED_6X8)
            {
                OLED_ShowChar(X + XOffset, Y, '?', OLED_6X8);
                XOffset += OLED_6X8;
            }
        }
    }
}

/**
  * 函    数：显示十进制正整数
  */
void OLED_ShowNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(X + i * FontSize, Y,
                      Number / OLED_Pow(10, Length - i - 1) % 10 + '0', FontSize);
    }
}

/**
  * 函    数：显示有符号十进制整数
  */
void OLED_ShowSignedNum(int16_t X, int16_t Y, int32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    uint32_t Number1;

    if (Number >= 0)
    {
        OLED_ShowChar(X, Y, '+', FontSize);
        Number1 = Number;
    }
    else
    {
        OLED_ShowChar(X, Y, '-', FontSize);
        Number1 = -Number;
    }

    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(X + (i + 1) * FontSize, Y,
                      Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0', FontSize);
    }
}

/**
  * 函    数：显示十六进制正整数
  */
void OLED_ShowHexNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i, SingleNumber;
    for (i = 0; i < Length; i++)
    {
        SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
        if (SingleNumber < 10)
        {
            OLED_ShowChar(X + i * FontSize, Y, SingleNumber + '0', FontSize);
        }
        else
        {
            OLED_ShowChar(X + i * FontSize, Y, SingleNumber - 10 + 'A', FontSize);
        }
    }
}

/**
  * 函    数：显示二进制正整数
  */
void OLED_ShowBinNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(X + i * FontSize, Y,
                      Number / OLED_Pow(2, Length - i - 1) % 2 + '0', FontSize);
    }
}

/**
  * 函    数：显示浮点数
  */
void OLED_ShowFloatNum(int16_t X, int16_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize)
{
    uint32_t PowNum, IntNum, FraNum;

    if (Number >= 0)
    {
        OLED_ShowChar(X, Y, '+', FontSize);
    }
    else
    {
        OLED_ShowChar(X, Y, '-', FontSize);
        Number = -Number;
    }

    IntNum = (uint32_t)Number;
    Number -= IntNum;
    PowNum = OLED_Pow(10, FraLength);
    FraNum = (uint32_t)OLED_Round(Number * PowNum);
    IntNum += FraNum / PowNum;

    OLED_ShowNum(X + FontSize, Y, IntNum, IntLength, FontSize);
    OLED_ShowChar(X + (IntLength + 1) * FontSize, Y, '.', FontSize);
    OLED_ShowNum(X + (IntLength + 2) * FontSize, Y, FraNum, FraLength, FontSize);
}

/**
  * 函    数：显示图像
  */
void OLED_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image)
{
    uint8_t i = 0, j = 0;
    int16_t Page, Shift;

    OLED_ClearArea(X, Y, Width, Height);

    for (j = 0; j < (Height - 1) / 8 + 1; j++)
    {
        for (i = 0; i < Width; i++)
        {
            if (X + i >= 0 && X + i <= 127)
            {
                Page = Y / 8;
                Shift = Y % 8;
                if (Y < 0)
                {
                    Page -= 1;
                    Shift += 8;
                }

                if (Page + j >= 0 && Page + j <= 7)
                {
                    OLED_DisplayBuf[Page + j][X + i] |= Image[j * Width + i] << (Shift);
                }

                if (Page + j + 1 >= 0 && Page + j + 1 <= 7)
                {
                    OLED_DisplayBuf[Page + j + 1][X + i] |= Image[j * Width + i] >> (8 - Shift);
                }
            }
        }
    }
}

/**
  * 函    数：printf 格式化显示
  */
void OLED_Printf(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...)
{
    char String[256];
    va_list arg;
    va_start(arg, format);
    vsprintf(String, format, arg);
    va_end(arg);
    OLED_ShowString(X, Y, String, FontSize);
}

/* ==================== 图形函数 ==================== */

/**
  * 函    数：画点
  */
void OLED_DrawPoint(int16_t X, int16_t Y)
{
    if (X >= 0 && X <= 127 && Y >= 0 && Y <= 63)
    {
        OLED_DisplayBuf[Y / 8][X] |= 0x01 << (Y % 8);
    }
}

/**
  * 函    数：读点
  */
uint8_t OLED_GetPoint(int16_t X, int16_t Y)
{
    if (X >= 0 && X <= 127 && Y >= 0 && Y <= 63)
    {
        if (OLED_DisplayBuf[Y / 8][X] & 0x01 << (Y % 8))
        {
            return 1;
        }
    }
    return 0;
}

/**
  * 函    数：画线（Bresenham）
  */
void OLED_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1)
{
    int16_t x, y, dx, dy, d, incrE, incrNE, temp;
    int16_t x0 = X0, y0 = Y0, x1 = X1, y1 = Y1;
    uint8_t yflag = 0, xyflag = 0;

    if (y0 == y1)
    {
        if (x0 > x1) { temp = x0; x0 = x1; x1 = temp; }
        for (x = x0; x <= x1; x++) { OLED_DrawPoint(x, y0); }
    }
    else if (x0 == x1)
    {
        if (y0 > y1) { temp = y0; y0 = y1; y1 = temp; }
        for (y = y0; y <= y1; y++) { OLED_DrawPoint(x0, y); }
    }
    else
    {
        if (x0 > x1)
        {
            temp = x0; x0 = x1; x1 = temp;
            temp = y0; y0 = y1; y1 = temp;
        }

        if (y0 > y1)
        {
            y0 = -y0;
            y1 = -y1;
            yflag = 1;
        }

        if (y1 - y0 > x1 - x0)
        {
            temp = x0; x0 = y0; y0 = temp;
            temp = x1; x1 = y1; y1 = temp;
            xyflag = 1;
        }

        dx = x1 - x0;
        dy = y1 - y0;
        incrE = 2 * dy;
        incrNE = 2 * (dy - dx);
        d = 2 * dy - dx;
        x = x0;
        y = y0;

        if (yflag && xyflag) { OLED_DrawPoint(y, -x); }
        else if (yflag)      { OLED_DrawPoint(x, -y); }
        else if (xyflag)     { OLED_DrawPoint(y, x); }
        else                 { OLED_DrawPoint(x, y); }

        while (x < x1)
        {
            x++;
            if (d < 0) { d += incrE; }
            else       { y++; d += incrNE; }

            if (yflag && xyflag) { OLED_DrawPoint(y, -x); }
            else if (yflag)      { OLED_DrawPoint(x, -y); }
            else if (xyflag)     { OLED_DrawPoint(y, x); }
            else                 { OLED_DrawPoint(x, y); }
        }
    }
}

/**
  * 函    数：画矩形
  */
void OLED_DrawRectangle(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled)
{
    int16_t i, j;
    if (!IsFilled)
    {
        for (i = X; i < X + Width; i++)
        {
            OLED_DrawPoint(i, Y);
            OLED_DrawPoint(i, Y + Height - 1);
        }
        for (i = Y; i < Y + Height; i++)
        {
            OLED_DrawPoint(X, i);
            OLED_DrawPoint(X + Width - 1, i);
        }
    }
    else
    {
        for (i = X; i < X + Width; i++)
        {
            for (j = Y; j < Y + Height; j++)
            {
                OLED_DrawPoint(i, j);
            }
        }
    }
}

/**
  * 函    数：画三角形
  */
void OLED_DrawTriangle(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint8_t IsFilled)
{
    int16_t minx = X0, miny = Y0, maxx = X0, maxy = Y0;
    int16_t i, j;
    int16_t vx[] = {X0, X1, X2};
    int16_t vy[] = {Y0, Y1, Y2};

    if (!IsFilled)
    {
        OLED_DrawLine(X0, Y0, X1, Y1);
        OLED_DrawLine(X0, Y0, X2, Y2);
        OLED_DrawLine(X1, Y1, X2, Y2);
    }
    else
    {
        if (X1 < minx) { minx = X1; }
        if (X2 < minx) { minx = X2; }
        if (Y1 < miny) { miny = Y1; }
        if (Y2 < miny) { miny = Y2; }
        if (X1 > maxx) { maxx = X1; }
        if (X2 > maxx) { maxx = X2; }
        if (Y1 > maxy) { maxy = Y1; }
        if (Y2 > maxy) { maxy = Y2; }

        for (i = minx; i <= maxx; i++)
        {
            for (j = miny; j <= maxy; j++)
            {
                if (OLED_pnpoly(3, vx, vy, i, j)) { OLED_DrawPoint(i, j); }
            }
        }
    }
}

/**
  * 函    数：画圆（Bresenham）
  */
void OLED_DrawCircle(int16_t X, int16_t Y, uint8_t Radius, uint8_t IsFilled)
{
    int16_t x, y, d, j;

    d = 1 - Radius;
    x = 0;
    y = Radius;

    OLED_DrawPoint(X + x, Y + y);
    OLED_DrawPoint(X - x, Y - y);
    OLED_DrawPoint(X + y, Y + x);
    OLED_DrawPoint(X - y, Y - x);

    if (IsFilled)
    {
        for (j = -y; j < y; j++)
        {
            OLED_DrawPoint(X, Y + j);
        }
    }

    while (x < y)
    {
        x++;
        if (d < 0) { d += 2 * x + 1; }
        else       { y--; d += 2 * (x - y) + 1; }

        OLED_DrawPoint(X + x, Y + y);
        OLED_DrawPoint(X + y, Y + x);
        OLED_DrawPoint(X - x, Y - y);
        OLED_DrawPoint(X - y, Y - x);
        OLED_DrawPoint(X + x, Y - y);
        OLED_DrawPoint(X + y, Y - x);
        OLED_DrawPoint(X - x, Y + y);
        OLED_DrawPoint(X - y, Y + x);

        if (IsFilled)
        {
            for (j = -y; j < y; j++)
            {
                OLED_DrawPoint(X + x, Y + j);
                OLED_DrawPoint(X - x, Y + j);
            }
            for (j = -x; j < x; j++)
            {
                OLED_DrawPoint(X - y, Y + j);
                OLED_DrawPoint(X + y, Y + j);
            }
        }
    }
}

/**
  * 函    数：画椭圆
  */
void OLED_DrawEllipse(int16_t X, int16_t Y, uint8_t A, uint8_t B, uint8_t IsFilled)
{
    int16_t x, y, j;
    int16_t a = A, b = B;
    float d1, d2;

    x = 0;
    y = b;
    d1 = b * b + a * a * (-b + 0.5);

    if (IsFilled)
    {
        for (j = -y; j < y; j++)
        {
            OLED_DrawPoint(X, Y + j);
        }
    }

    OLED_DrawPoint(X + x, Y + y);
    OLED_DrawPoint(X - x, Y - y);
    OLED_DrawPoint(X - x, Y + y);
    OLED_DrawPoint(X + x, Y - y);

    while (b * b * (x + 1) < a * a * (y - 0.5))
    {
        if (d1 <= 0) { d1 += b * b * (2 * x + 3); }
        else         { d1 += b * b * (2 * x + 3) + a * a * (-2 * y + 2); y--; }
        x++;

        if (IsFilled)
        {
            for (j = -y; j < y; j++)
            {
                OLED_DrawPoint(X + x, Y + j);
                OLED_DrawPoint(X - x, Y + j);
            }
        }

        OLED_DrawPoint(X + x, Y + y);
        OLED_DrawPoint(X - x, Y - y);
        OLED_DrawPoint(X - x, Y + y);
        OLED_DrawPoint(X + x, Y - y);
    }

    d2 = b * b * (x + 0.5) * (x + 0.5) + a * a * (y - 1) * (y - 1) - a * a * b * b;

    while (y > 0)
    {
        if (d2 <= 0) { d2 += b * b * (2 * x + 2) + a * a * (-2 * y + 3); x++; }
        else         { d2 += a * a * (-2 * y + 3); }
        y--;

        if (IsFilled)
        {
            for (j = -y; j < y; j++)
            {
                OLED_DrawPoint(X + x, Y + j);
                OLED_DrawPoint(X - x, Y + j);
            }
        }

        OLED_DrawPoint(X + x, Y + y);
        OLED_DrawPoint(X - x, Y - y);
        OLED_DrawPoint(X - x, Y + y);
        OLED_DrawPoint(X + x, Y - y);
    }
}

/**
  * 函    数：画圆弧
  */
void OLED_DrawArc(int16_t X, int16_t Y, uint8_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled)
{
    int16_t x, y, d, j;

    d = 1 - Radius;
    x = 0;
    y = Radius;

    if (OLED_IsInAngle(x, y, StartAngle, EndAngle))   { OLED_DrawPoint(X + x, Y + y); }
    if (OLED_IsInAngle(-x, -y, StartAngle, EndAngle)) { OLED_DrawPoint(X - x, Y - y); }
    if (OLED_IsInAngle(y, x, StartAngle, EndAngle))   { OLED_DrawPoint(X + y, Y + x); }
    if (OLED_IsInAngle(-y, -x, StartAngle, EndAngle)) { OLED_DrawPoint(X - y, Y - x); }

    if (IsFilled)
    {
        for (j = -y; j < y; j++)
        {
            if (OLED_IsInAngle(0, j, StartAngle, EndAngle)) { OLED_DrawPoint(X, Y + j); }
        }
    }

    while (x < y)
    {
        x++;
        if (d < 0) { d += 2 * x + 1; }
        else       { y--; d += 2 * (x - y) + 1; }

        if (OLED_IsInAngle(x, y, StartAngle, EndAngle))   { OLED_DrawPoint(X + x, Y + y); }
        if (OLED_IsInAngle(y, x, StartAngle, EndAngle))   { OLED_DrawPoint(X + y, Y + x); }
        if (OLED_IsInAngle(-x, -y, StartAngle, EndAngle)) { OLED_DrawPoint(X - x, Y - y); }
        if (OLED_IsInAngle(-y, -x, StartAngle, EndAngle)) { OLED_DrawPoint(X - y, Y - x); }
        if (OLED_IsInAngle(x, -y, StartAngle, EndAngle))  { OLED_DrawPoint(X + x, Y - y); }
        if (OLED_IsInAngle(y, -x, StartAngle, EndAngle))  { OLED_DrawPoint(X + y, Y - x); }
        if (OLED_IsInAngle(-x, y, StartAngle, EndAngle))  { OLED_DrawPoint(X - x, Y + y); }
        if (OLED_IsInAngle(-y, x, StartAngle, EndAngle))  { OLED_DrawPoint(X - y, Y + x); }

        if (IsFilled)
        {
            for (j = -y; j < y; j++)
            {
                if (OLED_IsInAngle(x, j, StartAngle, EndAngle))  { OLED_DrawPoint(X + x, Y + j); }
                if (OLED_IsInAngle(-x, j, StartAngle, EndAngle)) { OLED_DrawPoint(X - x, Y + j); }
            }
            for (j = -x; j < x; j++)
            {
                if (OLED_IsInAngle(-y, j, StartAngle, EndAngle)) { OLED_DrawPoint(X - y, Y + j); }
                if (OLED_IsInAngle(y, j, StartAngle, EndAngle))  { OLED_DrawPoint(X + y, Y + j); }
            }
        }
    }
}
