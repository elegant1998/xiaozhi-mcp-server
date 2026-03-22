# xiaozhi-mcp-server

让小智智能盒子连接OpenClaw，成为你的智能语音助手。

## 一句话说明

安装后，小智说的话会传给OpenClaw处理，小智再把结果念给你听。

## 功能

- 小智语音 → OpenClaw处理 → 语音播报结果
- 支持各种任务：查天气、写文档、搜信息...
- 异步处理，长任务不超时

## 安装

```bash
clawhub install xiaozhi-mcp-server
```

## 使用

### 1. 启动服务

```bash
./scripts/start.sh
```

### 2. 获取连接码

```bash
cat ~/.config/openclaw-mcp/token
```

### 3. 小智配置

1. 瞄小智启动后，跟瞄小智说：微信绑定  ，这时候会出现绑定二维码，微信扫码，绑定 瞄小智服务号

2. 在瞄小智服务号的配置界面填入：

- 服务器地址：你的服务器IP
- 连接码：刚才获取的token
- 端口：28765

3. 完成！现在对小智说话，欧克劳（OpenClaw）会帮你处理。

### 4. 完成

## 示例对话

| 你对小智说                       | OpenClaw(欧克劳)回复     |
| -------------------------------- | ------------------------ |
| "问下欧克劳，那边今天天气怎么样" | "北京今天晴，气温11度"   |
| "告诉欧克劳帮我写个会议纪要"     | "会议纪要已写好，请查看" |
| "欧克劳，查一下快递"             | "您的快递正在派送中"     |

## 配置

编辑 `config.yaml`：
- `port`：端口号（默认28765）
- `target_session`：OpenClaw会话地址

## 文件

```
xiaozhi-mcp-server/
├── SKILL.md
├── README.md
├── config.yaml
├── package.json
└── scripts/
    ├── server.py
    ├── start.sh
    └── stop.sh
```

## 作者

无敌哥@瞄小智

## 许可证

MIT

## 已经支持的瞄小智固件



<img src="img/image-20260302090718564.png" alt="image-20260202125738101" style="zoom:30%;" />

<img src="img/20260308133243_177_21.jpg" alt="20260308122132_186_69" style="zoom:20%;"/>

见固件目录

没有对应硬件板子的，需要定制的进QQ群提需求。

## 交流群

扫码加入瞄小智QQ群，一起探索：

<img src="img/qrcode_miaoxiaozhi_qq.jpg" alt="qrcode_miaoxiaozhi_qq" style="zoom:33%;" />

