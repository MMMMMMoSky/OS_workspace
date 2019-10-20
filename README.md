# 简单的操作系统实现

## 环境

Ubuntu 18.04/16.04

QEMU 模拟器

as 汇编器

## 编程规范

1. 除了 `Makefile` 外, 全部使用空格控制缩进 (即软tab)
2. 汇编代码使用 8 个空格缩进, C代码使用 4 个空格缩进

## 文件列表

- `bootsect.S` 启动区, 编译后被放在软盘的第一个扇区 (512 字节)
- `ld_script.ld` 链接脚本, 用于去掉 elf 文件头
- `setup.S` 初始化程序, 编译后被放在软盘的第二到第五扇区

## `bootsect.S` 启动区

1. 启动时, 被加载到 `[0x07c00, 0x07e00)`
2. 把 `setup.S` 加载到 `[0x08000, 0x08800)`
3. 跳转到 `setup.S` 进行操作系统的初始化