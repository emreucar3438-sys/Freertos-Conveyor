# Freertos-Conveyor
# ESP32 FreeRTOS Industrial Conveyor Controller

An industrial conveyor belt and safety system simulation developed on an ESP32 using **FreeRTOS**. This project demonstrates core real-time operating system concepts, including multitasking, thread-safe data queues, and resource locking (Mutex).

---

## ⏱️ Timing & Safety Architecture (Core Focus)

System stability and emergency management are handled via a hybrid approach combining FreeRTOS software timers and physical hardware interrupts:

### 1. Watchdog Timer (One-Shot Software Timer) - 5000ms
* **Purpose:** Monitors sensor health and prevents system freezes.
* **Mechanism:** The sensor task resets this timer (`xTimerReset`) every time it generates data. If the sensor drops offline or freezes for more than 5 seconds, the timer fires its callback and automatically locks the system into **Safe Mode**.

### 2. Heartbeat Monitor (Auto-Reload Software Timer) - 3000ms
* **Purpose:** Verifies that the RTOS scheduler is running smoothly without deadlocks.
* **Mechanism:** Automatically wakes up every 3 seconds to print a *"Core is alive"* status log to the Serial Monitor.

### 3. Emergency Stop (E-Stop - Hardware Interrupt / ISR)
* **Purpose:** Provides instantaneous physical override to halt the machine.
* **Mechanism:** Bypasses task priorities and RTOS ticks entirely. Pressing the physical button triggers a hardware-level interrupt (`IRAM_ATTR ISR`). It overrides all queues to bring the conveyor speed to `0` within microseconds.

---

## ⚙️ Task Concurrency & Data Flow

* **Task_InfraredSensor (Priority 1):** Simulates an infrared sensor to generate belt speed data. Sends data via a safe FreeRTOS **Queue** and continuously pets the Watchdog Timer.
* **Task_Controller (Priority 1):** Block-waits on the queue. Once data arrives, it securely acquires a **Mutex** lock to update the shared global structural data, preventing race conditions.

---

## 🚀 How to Run
1. Open the project directory using **PlatformIO**.
2. Connect your ESP32 development board.
3. (Optional) Connect a physical push button between **GPIO 12** and **GND** to test the hardware E-Stop.
4. Upload the firmware and open the Serial Monitor at `115200` baud.
