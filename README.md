# 简单的操作系统实现

## 环境

Ubuntu 18.04/16.04

QEMU 模拟器

as 汇编器

## 编程规范

1. 开始编程前请确保已经与远程库同步, 编写, 测试之后及时同步到远程库
2. 除了 `Makefile` 外, 全部使用空格控制缩进 (即软tab)
3. 汇编代码使用 8 个空格缩进, C代码使用 4 个空格缩进

## 文件列表

- **boot**:
  - `bootsect.S`: 启动区, 编译后被放在软盘的第一个扇区 (512 字节), 加载初始化程序并跳转
  - `setup.S`: 初始化程序, 编译后被放在软盘的第二到第五扇区, 完成一些初始化工作然后跳转到操作系统
- **system**:
  - `sys_head.S`: 调用 `main()`, 确保 C 代码从 `main()` 开始执行, 并为 C 语言提供一些函数
  - **headers**:
    - `macro_def.h`: 宏定义, 涉及到一些参数
    - `struct_def.h`: 结构体的定义
    - `func_def.h`: 各种函数的声明
  - **C**:
    - `os_main.c`: `main()` 函数的定义, 操作系统的起点
    - `console_io.c`: 包含控制台输出的相关函数的实现
    - `hardware_init.c`: 硬件初始化函数的实现
    - `men_manage.c`: 内存管理相关函数的实现
    - `text_video.c`: VGA 0x03 号字符模式下画面控制相关函数的实现
- 其他:
  - `ld_script.ld` 链接脚本, 用于去掉 elf 文件头

## `bootsect.S` 启动区

1. 启动时, 被加载到 `[0x07c00, 0x07e00)`
2. 把 `setup.S` 加载到 `[0x80000, 0x80800)`
3. 跳转到 `setup.S` 进行操作系统的初始化

## `setup.S` 操作系统初始化程序

1. 把 C0-H0-S6 ~ C19-H1-S18 这 715 个扇区的内容加载到 `[0x10000, 0x69600)` 的位置. 这也是目前限定的操作系统的大小 (357kb应该足够使用)
2. 打开 A20 地址线
3. 切换图形模式 `VGA 0x03 80x25x16 text`, 显存从 `0xb8000` 开始, 每 2 个 byte 表示一个位置的字符, 前者是 ASCII 值, 后者是颜色
4. 把操作系统移动到 `[0x00000, 0x59600)`
5. 设置 GDT, GDTR 的值为 `0x59700|0x0f00`
5. 进入保护模式
6. 跳转到 `0x0000` 继续执行

## `system`

生成 `system` 就是操作系统, 会被加载到 `0x0000`.

目前 `system` 由 `sys_head.S` 和 C 语言代码拼接而成.

## 内存分布

(进入 system 之后, 这个图是上闭下开的区间)

```

+----0x00000000----+
|                  |
|                  |
|                  |
|      system      |
|                  |
|                  |
|                  |
+----0x00059600----+
|      empty       |
+----0x00059700----+
|                  |
|       GDT        |
|   at most 3072   |
|   Descriptors    |
|                  |
+----0x0005f700----+
|      empty       |
+----0x00060000----+
|                  |
|       IDT        |
|   at most 256    |
|    Interrupts    |
|                  |
+----0x00060200----+
|                  |
|      system      |
|      stack       |
|                  |
+----0x000a0000----+
|                  |
|      empty       |
|                  |
+----0x000b8000----+
|                  |
|  VGA text VRAM   |
|                  |
+----0x000b8fa0----+
|                  |
|                  |

```

## 备注

1. 目前从软盘启动, 改成从硬盘驱动, 需要修改 bootsect.S 和 setup.S 中的 INT 0x13 调用的参数
2. 打印字符串的一些调用可以风格统一/优化
3. 目前不太清楚 BIOS 中设置的显示模式和进入操作系统之后设置的显示模式有什么关系, 当然, 我们的操作系统不会涉及这一点