## 选择语言 / Choose Language
- [English Document](./README.md)
- [简体中文文档](./README.zh-CN.md)
## 项目简介
本项目基于 ESP32-S3-WROOM-1-N16R8 芯片，搭建一套支持本地触控、远程实时同步的智能家居 IoT 触控终端，完整实现"SPI 触控屏驱动、LVGL 图形界面、WiFi 配网、Home Assistant 双向通信、NVS 持久化存储、OTA 远程升级"全套方案。
硬件搭载 3.2 寸 320×240 电阻触摸屏（ILI9341 显示驱动 + XPT2046 触控驱动），软件基于 ESP-IDF 5.5.2 开发。
## 核心功能

1. **硬件底层驱动**
   - 分层 BSP 架构封装 SPI 总线，统一管理ILI9341显示屏、XPT2046触控芯片；
   - 8MB PSRAM 大内存分配，缓存LVGL帧缓冲区、JSON大数据
   - FreeRTOS 多核任务隔离：核1运行LVGL图形任务，核0处理网络任务
2. **本地图形界面（LVGLv9.3 + GUI Guider）**
   - GUI Guider生成界面代码，共5大交互页面：主页、WiFi 配置、HA设备绑定、OTA升级、系统设置(未开发)
   - 多设备控制、气象数据实时展示，自动切换天气图标（晴天 / 多云 / 雨天）
3. **WiFi 配网模块**
   - STA/AP双模式自动切换：无保存配网信息时开启AP热点，内置Web配网页面
   - Web端表单一键填写 WiFi 账号、Home Assistant IP/Token，参数自动存入NVS
4. **Home Assistant通信**
   - 采用**HTTP + WebSocket**
   - WebSocket 长连接：低延迟双向实时控制设备，订阅设备状态变更，无轮询CPU占用
   - HTTP短连接：设备初始化批量拉取全部实体信息(其实也可以用ws直接拉取，懒得重写方法就没改)
   - cJSON内存重定向至 PSRAM
5. **NVS**
   - 独立命名空间存储UI绑定设备ID、HA 连接参数，断电保持，上电读取
6. **OTA**
   - 两套升级方案：本地HTTP和华为云平台MQTT检测固件版本
7. **低延迟同步**
本地触控事件上传HA和HA中设备状态更新下发通过WebSocket实时同步，端到端延迟最低60ms。
