/**
 * xiaozhi-esp32 WebSocket Client for OpenClaw MCP Server
 * 
 * 功能：
 * - WebSocket长连接
 * - Token认证
 * - 心跳保活
 * - 断线重连
 * - 接收异步任务结果
 */

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

// ==================== 配置 ====================
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* MCP_SERVER_HOST = "49.232.54.53";  // OpenClaw MCP Server IP
const int MCP_SERVER_PORT = 9000;
const char* MCP_TOKEN = "2564be3543c53b6e8033f5ed99e67711a0cb4436d5f8c4dc";

const int HEARTBEAT_INTERVAL = 30000;  // 心跳间隔 30秒
const int RECONNECT_INTERVAL = 5000;   // 重连间隔 5秒

// ==================== 全局变量 ====================
WebSocketsClient webSocket;
unsigned long lastHeartbeat = 0;
bool wsConnected = false;

// ==================== WebSocket事件处理 ====================
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("[WS] ❌ 断开连接");
            wsConnected = false;
            break;
            
        case WStype_CONNECTED:
            Serial.println("[WS] ✅ 连接成功");
            wsConnected = true;
            // 连接成功后可以发送订阅消息
            sendSubscribeMessage();
            break;
            
        case WStype_TEXT:
            Serial.printf("[WS] 📩 收到消息: %s\n", payload);
            handleWebSocketMessage((char*)payload);
            break;
            
        case WStype_ERROR:
            Serial.println("[WS] ⚠️ 连接错误");
            wsConnected = false;
            break;
            
        default:
            break;
    }
}

// ==================== 消息处理 ====================
void handleWebSocketMessage(const char* message) {
    StaticJsonDocument<4096> doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Serial.println("[WS] JSON解析失败");
        return;
    }
    
    const char* type = doc["type"];
    
    if (strcmp(type, "task_completed") == 0) {
        // 任务完成，获取结果
        const char* taskId = doc["task_id"];
        const char* result = doc["result"];
        
        Serial.printf("[WS] 🎉 任务完成: %s\n", taskId);
        Serial.printf("[WS] 📝 结果: %s\n", result);
        
        // TODO: 调用TTS播报结果
        // speakText(result);
    }
    else if (strcmp(type, "subscribed") == 0) {
        const char* clientId = doc["client_id"];
        Serial.printf("[WS] 📌 订阅成功, ClientID: %s\n", clientId);
    }
    else if (strcmp(type, "pong") == 0) {
        Serial.println("[WS] 💓 心跳响应");
    }
}

// ==================== 发送消息 ====================
void sendHeartbeat() {
    if (!wsConnected) return;
    
    StaticJsonDocument<128> doc;
    doc["type"] = "ping";
    
    char buffer[128];
    serializeJson(doc, buffer);
    webSocket.sendTXT(buffer);
    
    Serial.println("[WS] 💓 发送心跳");
}

void sendSubscribeMessage() {
    if (!wsConnected) return;
    
    // 可以订阅特定任务（可选）
    StaticJsonDocument<128> doc;
    doc["type"] = "subscribe_task";
    // doc["task_id"] = "xxx";  // 可选：订阅特定任务
    
    char buffer[128];
    serializeJson(doc, buffer);
    webSocket.sendTXT(buffer);
    
    Serial.println("[WS] 📤 发送订阅消息");
}

// ==================== MCP HTTP调用 ====================
// 如果需要调用MCP工具，可以用HTTP请求
#include <HTTPClient.h>

String callMCPTool(String toolName, String message, bool asyncMode = true) {
    HTTPClient http;
    String url = "http://" + String(MCP_SERVER_HOST) + ":" + String(MCP_SERVER_PORT) + "/mcp";
    
    http.begin(url);
    http.addHeader("Authorization", "Bearer " + String(MCP_TOKEN));
    http.addHeader("Content-Type", "application/json");
    
    // 构建JSON请求
    StaticJsonDocument<512> doc;
    doc["jsonrpc"] = "2.0";
    doc["id"] = 1;
    doc["method"] = "tools/call";
    
    JsonObject params = doc.createNestedObject("params");
    params["name"] = toolName;
    
    JsonObject arguments = params.createNestedObject("arguments");
    arguments["message"] = message;
    arguments["async"] = asyncMode;
    
    String requestBody;
    serializeJson(doc, requestBody);
    
    Serial.printf("[MCP] 📤 调用工具: %s\n", toolName.c_str());
    
    int httpCode = http.POST(requestBody);
    String response = "";
    
    if (httpCode > 0) {
        response = http.getString();
        Serial.printf("[MCP] 📥 响应: %s\n", response.c_str());
    } else {
        Serial.printf("[MCP] ❌ 请求失败: %d\n", httpCode);
    }
    
    http.end();
    return response;
}

// ==================== 初始化 ====================
void setup() {
    Serial.begin(115200);
    Serial.println("\n\n========================================");
    Serial.println("  xiaozhi-esp32 MCP WebSocket Client");
    Serial.println("========================================\n");
    
    // 连接WiFi
    Serial.print("[WiFi] 连接中...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n[WiFi] ✅ 已连接");
    Serial.printf("[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());
    
    // 初始化WebSocket
    setupWebSocket();
}

void setupWebSocket() {
    Serial.println("[WS] 初始化WebSocket连接...");
    
    // 设置WebSocket服务器
    String url = "/ws?token=" + String(MCP_TOKEN);
    webSocket.begin(MCP_SERVER_HOST, MCP_SERVER_PORT, url);
    
    // 设置事件回调
    webSocket.onEvent(webSocketEvent);
    
    // 启用心跳（可选，服务器端可能有自己的心跳）
    webSocket.enableHeartbeat(30000, 10000, 2);
    
    // 重连设置
    webSocket.setReconnectInterval(RECONNECT_INTERVAL);
    
    Serial.printf("[WS] 服务器: ws://%s:%d%s\n", MCP_SERVER_HOST, MCP_SERVER_PORT, url.c_str());
}

// ==================== 主循环 ====================
void loop() {
    // WebSocket循环
    webSocket.loop();
    
    // 定期发送心跳
    unsigned long now = millis();
    if (wsConnected && (now - lastHeartbeat > HEARTBEAT_INTERVAL)) {
        sendHeartbeat();
        lastHeartbeat = now;
    }
    
    // 这里可以添加语音识别逻辑
    // 当用户说话时，调用 callMCPTool("run_agent", "用户说的话", true);
}

// ==================== 示例：处理语音命令 ====================
void handleVoiceCommand(String command) {
    Serial.printf("[语音] 收到命令: %s\n", command.c_str());
    
    // 调用OpenClaw Agent
    String response = callMCPTool("run_agent", command, true);
    
    // 解析task_id
    StaticJsonDocument<1024> doc;
    deserializeJson(doc, response);
    
    // 检查是否是异步任务
    if (doc["result"]["content"][0]["text"]) {
        String text = doc["result"]["content"][0]["text"].as<String>();
        
        StaticJsonDocument<512> resultDoc;
        deserializeJson(resultDoc, text);
        
        if (resultDoc["async"] == true) {
            String taskId = resultDoc["task_id"].as<String>();
            Serial.printf("[语音] 任务已提交: %s\n", taskId.c_str());
            Serial.println("[语音] 等待WebSocket推送结果...");
            
            // TODO: TTS播报 "任务进行中，完成后通知您"
        }
    }
}