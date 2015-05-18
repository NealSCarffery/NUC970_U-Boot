/*
 * Copyright (c) 2014 Nuvoton Technology Corp.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Description:   NUC970 Video driver source file
 */
#include <common.h>
#include <asm/io.h>
#include <lcd.h>
#include "nuc970_fb.h" 

#define REG_AHBIPRST	0xB0000060
#define REG_HCLKEN      0xB0000210
#define REG_CLKDIVCTL0  0xB0000220
#define REG_CLKDIVCTL1  0xB0000224
#define REG_CLKUPLLCON  0xB0000264
#define REG_MFP_GPA_L   0xB0000070
#define REG_MFP_GPA_H   0xB0000074
#define REG_MFP_GPD_H   0xB000008C
#define REG_MFP_GPG_L   0xB00000A0
#define REG_MFP_GPG_H   0xB00000A4


vpost_cfg_t vpost_cfg = {
        .clk            = 3000000,
        .hight          = 480,
        .width          = 800,
        .left_margin    = 4,
        .right_margin   = 12,
        .hsync_len      = 2,
        .upper_margin   = 2,
        .lower_margin   = 2,
        .vsync_len      = 1,
        .dccs           = 0x0e00040a,//0x0e00041a,
        .devctl         = 0x070000C0,
        .fbctrl         = 0x01900190,//0x03200320,
        .scale          = 0x04000400,
};



vidinfo_t panel_info = {
        .vl_col         = 800,
        .vl_row         = 480,
        .vl_bpix        = 5,  // 2^5 = 32bpp
};

//int lcd_line_length;  
int lcd_color_fg;
int lcd_color_bg;

void *lcd_base;                 /* Start of framebuffer memory  */
void *lcd_console_address;              /* Start of console buffer      */

short console_col;
short console_row;


void lcd_enable(void)
{
        // Turn on back light
        writel(readl(REG_GPIOG_DOUT) | (1 << 2), REG_GPIOG_DOUT);
        return;
}

void lcd_disable(void)
{
        // Turn off back light
        writel(readl(REG_GPIOG_DOUT) & ~(1 << 2), REG_GPIOG_DOUT);
        return;
}

void lcd_ctrl_init(void *lcdbase)
{

        writel((readl(REG_AHBIPRST) | (1 << 9)), REG_AHBIPRST);
        writel((readl(REG_AHBIPRST) & ~(1 << 9)), REG_AHBIPRST);

        // VPOST clk
        writel(readl(REG_HCLKEN) | 0x02000000, REG_HCLKEN); // LCD

        writel((readl(REG_CLKDIVCTL1) & ~0xffff) | 0xe18, REG_CLKDIVCTL1); // Set VPOST clock source from UCLKOUT

	//GPG6 (CLK), GPG7 (HSYNC)
        writel((readl(REG_MFP_GPG_L) & ~0xFF000000) | 0x22000000, REG_MFP_GPG_L); // LCD_CLK LCD_HSYNC
	//GPG8 (VSYNC), GPG9 (DEN)
        writel((readl(REG_MFP_GPG_H) & ~0xFF) | 0x22, REG_MFP_GPG_H); // LCD_VSYNC LCD_DEN

        // GPIO
        writel(0x22222222, REG_MFP_GPA_L); // LCD_DATA0~7
        writel(0x22222222, REG_MFP_GPA_H); // LCD_DATA8~15
        writel(0x22222222, REG_MFP_GPD_H); // LCD_DATA16~23

        writel(readl(REG_GPIOG_DIR) | (1 << 2), REG_GPIOG_DIR);
        writel(readl(REG_GPIOG_DOUT) | (1 << 2), REG_GPIOG_DOUT);

	//LCD register
        writel(vpost_cfg.devctl, REG_LCM_DEV_CTRL);  //1677721 color, 24bit
        writel(0x00000000, REG_LCM_MPU_CMD);
        writel(0x80000001, REG_LCM_INT_CS);

        writel(0x020d03a0, REG_LCM_CRTC_SIZE);  //800*480
        writel(0x01e00320, REG_LCM_CRTC_DEND);
        writel(0x03250321, REG_LCM_CRTC_HR);
        writel(0x03780348, REG_LCM_CRTC_HSYNC);

        writel(0x01f001ed, REG_LCM_CRTC_VR);
        writel((unsigned int)lcdbase, REG_LCM_VA_BADDR0);
        writel(vpost_cfg.fbctrl, REG_LCM_VA_FBCTRL);

        writel(vpost_cfg.scale, REG_LCM_VA_SCALE);
        writel(0x000107FF, REG_LCM_VA_WIN);

        writel(vpost_cfg.dccs, REG_LCM_DCCS);  //enable vpost, rgb565

        return;
}

void lcd_getcolreg (ushort regno, ushort *red, ushort *green, ushort *blue)
{
        return;
}
#ifdef CONFIG_LCD_INFO
#include <version.h>
void lcd_show_board_info(void)
{
        lcd_printf ("%s\n", U_BOOT_VERSION);
        lcd_printf ("(C) 2014 Nuvoton Technology Corp.\n");
        lcd_printf ("NUC970 Evaluation Board\n");

}
#endif /* CONFIG_LCD_INFO */

