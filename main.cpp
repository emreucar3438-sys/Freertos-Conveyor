#include <Arduino.h>
SemaphoreHandle_t Mutex;
TimerHandle_t oneShotTimer;
TimerHandle_t autoReloadTimer;
QueueHandle_t xQueue;
const int ESTOP_BUTTON_PIN = 12;
struct ConveyorState {
    int total_products;
    int speed;
    bool is_running;
};
ConveyorState g_conveyor = {0, 0, true}; 
void Task_InfraredSensor(void *pvParameter){
    for(;;){
        if (!g_conveyor.is_running) {
            vTaskDelay(200 / portTICK_PERIOD_MS);
            continue;
        }
        int localSpeed = rand() % 10 + 1; 
        
        xQueueSend(xQueue, &localSpeed, portMAX_DELAY);
        
        xTimerReset(oneShotTimer, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
void Task_Controller(void *pvParameter){
    for(;;){
        int receivedSpeed = 0;
        
        xQueueReceive(xQueue, &receivedSpeed, portMAX_DELAY); 
        
        
        xSemaphoreTake(Mutex, portMAX_DELAY);
        
        if (g_conveyor.is_running) {
        g_conveyor.speed = receivedSpeed; 
        g_conveyor.total_products++;
        Serial.print("Hiz: "); Serial.println(g_conveyor.speed);
        Serial.print("Adet: "); Serial.println(g_conveyor.total_products);
        }
        xSemaphoreGive(Mutex);
    }
}
 void vHeartbeatCallback(TimerHandle_t xTimer){
  Serial.println("[TIMER] Heartbeat: Core is alive and running smoothly.");
}
void vWatchdogCallback(TimerHandle_t xTimer){
    xSemaphoreTake(Mutex, portMAX_DELAY);
    g_conveyor.is_running = false;
    xSemaphoreGive(Mutex);
    Serial.println("🚨 [CRITICAL] Watchdog Timeout! Sensor lost. Safe mode active.");}
    void IRAM_ATTR estopISR(){
        g_conveyor.is_running = false;
        g_conveyor.speed = 0;
    }
    void setup(){
        Serial.begin(115200);
        Mutex = xSemaphoreCreateMutex();
        xQueue = xQueueCreate(5, sizeof(int));
       autoReloadTimer =  xTimerCreate("HeartbeatTimer", pdMS_TO_TICKS(3000), pdTRUE, NULL, vHeartbeatCallback);
        oneShotTimer = xTimerCreate("WatchdogTimer", pdMS_TO_TICKS(5000), pdFALSE, NULL, vWatchdogCallback);
        xTimerStart(oneShotTimer, 0);
        xTimerStart(autoReloadTimer, 0);
        xTaskCreate(Task_InfraredSensor, "InfraredSensorTask", 2048, NULL, 1, NULL);
        xTaskCreate(Task_Controller, "ControllerTask", 2048, NULL, 1, NULL);
        pinMode(ESTOP_BUTTON_PIN, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(ESTOP_BUTTON_PIN), estopISR, FALLING);
    }
        void loop(){

        }
