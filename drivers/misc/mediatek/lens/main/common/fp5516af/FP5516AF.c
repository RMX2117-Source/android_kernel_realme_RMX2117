/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

/*
 * FP5516AF voice coil motor driver
 *
 *
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

#include "lens_info.h"


#define AF_DRVNAME "FP5516AF_DRV"
#define AF_I2C_SLAVE_ADDR        0x18

#define AF_DEBUG
#ifdef AF_DEBUG
#define LOG_INF(format, args...) printk(AF_DRVNAME " [%s] " format, __func__, ##args)
#else
#define LOG_INF(format, args...)
#endif
//wanjianfeng@camera, add for actuator reset to mediumPos in powerdown
#define AGOLD_FP5516_AF_OPTIMIZE

static struct i2c_client *g_pstAF_I2Cclient;
static int *g_pAF_Opened;
static spinlock_t *g_pAF_SpinLock;


static unsigned long g_u4AF_INF;
static unsigned long g_u4AF_MACRO = 1023;
static unsigned long g_u4TargetPosition;
/* Tian.Tian@Camera.Driver, 2019/11/19, add for af init end */
static unsigned long g_u4CurrPosition = 0;

static int g_SR = 3;

static int i2c_read(u8 a_u2Addr , u8 * a_puBuff)
{
    int  i4RetValue = 0;
    char puReadCmd[1] = {(char)(a_u2Addr)};
    i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puReadCmd, 1);
    LOG_INF("[FP5516AF]i2c_master_send i4RetValue - %x\n\n",i4RetValue);
    if (i4RetValue != 1) {
        LOG_INF("[FP5516AF] I2C write failed!! \n");
        return -1;
    }
    i4RetValue = i2c_master_recv(g_pstAF_I2Cclient, (char *)a_puBuff, 1);
    LOG_INF("[FP5516AF]i2c_master_recv i4RetValue - %x\n\n",i4RetValue);
    if (i4RetValue != 1) {
        LOG_INF("[FP5516AF] I2C read failed!! \n");
        return -1;
    }
    return 0;
}

static u8 read_data(u8 addr)
{
    u8 get_byte=0;
    i2c_read( addr ,&get_byte);
    LOG_INF("[FP5516AF]  get_byte %d \n",  get_byte);
    return get_byte;
}

static int s4AF_ReadReg(unsigned short *a_pu2Result)
{
/*
    int i4RetValue = 0;
    char pBuff[2];

    g_pstAF_I2Cclient->addr = AF_I2C_SLAVE_ADDR;

    g_pstAF_I2Cclient->addr = g_pstAF_I2Cclient->addr >> 1;

    i4RetValue = i2c_master_recv(g_pstAF_I2Cclient, pBuff, 2);

    if (i4RetValue < 0) {
        LOG_INF("I2C read failed!!\n");
        return -1;
    }

    *a_pu2Result = (((u16) pBuff[0]) << 4) + (pBuff[1] >> 4);
*/
    g_pstAF_I2Cclient->addr = AF_I2C_SLAVE_ADDR;
    g_pstAF_I2Cclient->addr = g_pstAF_I2Cclient->addr >> 1;

    *a_pu2Result = ((unsigned short)(read_data(0x03)&0x03) << 8) + (read_data(0x04)&0xff);

    return 0;
}

static int s4AF_WriteReg(u16 a_u2Data)
{
    int i4RetValue = 0;

    //char puSendCmd[2] = { (char)(a_u2Data >> 4), (char)(((a_u2Data & 0xF) << 4) + g_SR) };
    char puSendCmd[3] = {0x03,(char)(a_u2Data >> 8), (char)(a_u2Data & 0xff)}; //FP5516 AF mode

    g_pstAF_I2Cclient->addr = AF_I2C_SLAVE_ADDR;

    g_pstAF_I2Cclient->addr = g_pstAF_I2Cclient->addr >> 1;

    //i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 2);
     //g_pstAF_I2Cclient->ext_flag |= I2C_A_FILTER_MSG;
    i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 3);

    if (i4RetValue < 0) {
        LOG_INF("I2C send failed!!\n");
        return -1;
    }

    return 0;
}

static inline int getAFInfo(__user struct stAF_MotorInfo *pstMotorInfo)
{
    struct stAF_MotorInfo stMotorInfo;

    stMotorInfo.u4MacroPosition = g_u4AF_MACRO;
    stMotorInfo.u4InfPosition = g_u4AF_INF;
    stMotorInfo.u4CurrentPosition = g_u4CurrPosition;
    stMotorInfo.bIsSupportSR = 1;

    stMotorInfo.bIsMotorMoving = 1;

    if (*g_pAF_Opened >= 1)
        stMotorInfo.bIsMotorOpen = 1;
    else
        stMotorInfo.bIsMotorOpen = 0;

    if (copy_to_user(pstMotorInfo, &stMotorInfo,
             sizeof(struct stAF_MotorInfo)))
        LOG_INF("copy to user failed when getting motor information\n");

    return 0;
}

static inline int moveAF(unsigned long a_u4Position)
{
    int ret = 0;
    unsigned short InitPos;
    int stepnum = 0;
    int size = 100;
    int step = 0;
    //int Position = 900;
    if ((a_u4Position > g_u4AF_MACRO) || (a_u4Position < g_u4AF_INF)) {
        LOG_INF("out of range\n");
        return -EINVAL;
    }
    ret = s4AF_ReadReg(&InitPos);
    if (*g_pAF_Opened == 1) {
        if (ret == 0) {
            //wanjianfeng@camera, add for smooth initPos
            stepnum = abs(InitPos - a_u4Position) / size;
            LOG_INF("stepnum:%d, InitPos:%d, a_u4Position:%d", stepnum, InitPos, a_u4Position);
            if(a_u4Position >= InitPos) {
                for(step = 0; step <= stepnum; step++)
                {
                    if((InitPos + size) <= a_u4Position) {
                        InitPos = InitPos + size;
                        s4AF_WriteReg(InitPos);
                        LOG_INF("InitPos:%d", InitPos);
                        msleep(20);
                    } else {
                        s4AF_WriteReg(a_u4Position);
                        LOG_INF("a_u4Position:%d", a_u4Position);
                    }
                }
            }
            s4AF_ReadReg(&InitPos);
            spin_lock(g_pAF_SpinLock);
            g_u4CurrPosition = (unsigned long)InitPos;
            spin_unlock(g_pAF_SpinLock);
        } else {
            spin_lock(g_pAF_SpinLock);
            g_u4CurrPosition = 0;  //medium
            spin_unlock(g_pAF_SpinLock);
        }

        spin_lock(g_pAF_SpinLock);
        *g_pAF_Opened = 2;
        spin_unlock(g_pAF_SpinLock);
    }

    if (g_u4CurrPosition == a_u4Position)
        return 0;

    spin_lock(g_pAF_SpinLock);
    g_u4TargetPosition = a_u4Position;
    g_SR = 3;
    spin_unlock(g_pAF_SpinLock);

    /* LOG_INF("move [curr] %d [target] %d\n", g_u4CurrPosition, g_u4TargetPosition); */
    if (s4AF_WriteReg((unsigned short)g_u4TargetPosition) == 0) {
        spin_lock(g_pAF_SpinLock);
        g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
        spin_unlock(g_pAF_SpinLock);
        printk("[zp],FP5516af moveaf TargetPosition = %ld\n",g_u4TargetPosition);
    } else {
        LOG_INF("set I2C failed when moving the motor\n");
    }

    return 0;
}

static inline int setAFInf(unsigned long a_u4Position)
{
    spin_lock(g_pAF_SpinLock);
    g_u4AF_INF = a_u4Position;
    spin_unlock(g_pAF_SpinLock);
    return 0;
}

static inline int setAFMacro(unsigned long a_u4Position)
{
    spin_lock(g_pAF_SpinLock);
    g_u4AF_MACRO = a_u4Position;
    spin_unlock(g_pAF_SpinLock);
    return 0;
}

/* ////////////////////////////////////////////////////////////// */
long FP5516AF_Ioctl(struct file *a_pstFile, unsigned int a_u4Command, unsigned long a_u4Param)
{
    long i4RetValue = 0;

    switch (a_u4Command) {
    case AFIOC_G_MOTORINFO:
        i4RetValue =
            getAFInfo((__user struct stAF_MotorInfo *)(a_u4Param));
        break;

    case AFIOC_T_MOVETO:
        i4RetValue = moveAF(a_u4Param);
        break;

    case AFIOC_T_SETINFPOS:
        i4RetValue = setAFInf(a_u4Param);
        break;

    case AFIOC_T_SETMACROPOS:
        i4RetValue = setAFMacro(a_u4Param);
        break;

    default:
        LOG_INF("No CMD\n");
        i4RetValue = -EPERM;
        break;
    }

    return i4RetValue;
}

/* Main jobs: */
/* 1.Deallocate anything that "open" allocated in private_data. */
/* 2.Shut down the device on last close. */
/* 3.Only called once on last time. */
/* Q1 : Try release multiple times. */
int FP5516AF_Release(struct inode *a_pstInode, struct file *a_pstFile)
{

    #ifdef AGOLD_FP5516_AF_OPTIMIZE
    int pos = g_u4CurrPosition;
    int pos_size = 200;
    int step_num =0;
    int step = 0;
    if(pos > 512)
        step_num = (pos-512)/pos_size;
    else {
        if (*g_pAF_Opened) {
            LOG_INF("directly Free\n");
            spin_lock(g_pAF_SpinLock);
            *g_pAF_Opened = 0;
            spin_unlock(g_pAF_SpinLock);
        }
        return 0;
    }
    #endif

    LOG_INF("Start\n");

    if (*g_pAF_Opened == 2) {
        LOG_INF("Wait\n");
        #ifdef AGOLD_FP5516_AF_OPTIMIZE
        for(step = 0;step <step_num;step ++)
        {
            if(pos > 512)
            {
                pos = pos - pos_size;
                  if(pos > 512)
                  {
                    s4AF_WriteReg(pos);
                     msleep(20);
                }
            }
            else if(pos < 512)
            {
                 pos = pos + pos_size;
                  if(pos < 512)
                  {
                      s4AF_WriteReg(pos);
                     msleep(20);
                }
            }
        }
        s4AF_WriteReg(512);
        LOG_INF("[FP5516AF]Move to ini pos\n");
    #else
        g_SR = 5;
        s4AF_WriteReg(g_u4CurrPosition);
        msleep(20);
    #endif
    }

    if (*g_pAF_Opened) {
        LOG_INF("Free\n");

        spin_lock(g_pAF_SpinLock);
        *g_pAF_Opened = 0;
        spin_unlock(g_pAF_SpinLock);
    }

    LOG_INF("End\n");

    return 0;
}

/* Tian.Tian@Camera.Driver, 2019/11/19, add for af init start */
/* initAF include driver initialization and standby mode */
static int initAF(void)
{
    LOG_INF("+\n");
    if (*g_pAF_Opened == 1) {

        int i4RetValue = 0;
        char puSendCmd[2] = {0x02, 0x01}; /* soft power on */
        char puSendCmd2[2] = {0x02, 0x00};
        char puSendCmd3[2] = {0x02, 0x02};
        char puSendCmd4[2] = {0x06, 0x80}; /* soft power on */
        char puSendCmd5[2] = {0x07, 0x00};
        char puSendCmd6[2] = {0x08, 0x01};

        i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 2);

        if (i4RetValue < 0) {
            LOG_INF("I2C send 0x01 failed!!\n");
            return -1;
        }

        i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd2, 2);

        if (i4RetValue < 0) {
            LOG_INF("I2C send 0x02 failed!!\n");
            return -1;
        }
        udelay(100);
        i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd3, 2);

        if (i4RetValue < 0) {
            LOG_INF("I2C send 0x03 failed!!\n");
            return -1;
        }

        i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd4, 2);

        if (i4RetValue < 0) {
            LOG_INF("I2C send 0x04 failed!!\n");
            return -1;
        }

        i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd5, 2);

        if (i4RetValue < 0) {
            LOG_INF("I2C send 0x05 failed!!\n");
            return -1;
        }

        i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd6, 2);

        if (i4RetValue < 0) {
            LOG_INF("I2C send 0x06 failed!!\n");
            return -1;
        }

        LOG_INF("driver init success!!\n");

        spin_lock(g_pAF_SpinLock);
        *g_pAF_Opened = 1;
        spin_unlock(g_pAF_SpinLock);
    }

    LOG_INF("-\n");

    return 0;
}
/* Tian.Tian@Camera.Driver, 2019/11/19, add for af init end */
int FP5516AF_SetI2Cclient(struct i2c_client *pstAF_I2Cclient, spinlock_t *pAF_SpinLock, int *pAF_Opened)
{
    g_pstAF_I2Cclient = pstAF_I2Cclient;
    g_pAF_SpinLock = pAF_SpinLock;
    g_pAF_Opened = pAF_Opened;
    /* Tian.Tian@Camera.Driver, 2019/11/19, add for af init */
    initAF();
    return 1;
}

int FP5516AF_GetFileName(unsigned char *pFileName)
{
    #if SUPPORT_GETTING_LENS_FOLDER_NAME
    char FilePath[256];
    char *FileString;

    sprintf(FilePath, "%s", __FILE__);
    FileString = strrchr(FilePath, '/');
    *FileString = '\0';
    FileString = (strrchr(FilePath, '/') + 1);
    strncpy(pFileName, FileString, AF_MOTOR_NAME);
    LOG_INF("FileName : %s\n", pFileName);
    #else
    pFileName[0] = '\0';
    #endif
    return 1;
}
