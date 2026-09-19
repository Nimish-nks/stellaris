Press SW1 and green led will glow only. Sometimes red and green glow together.
---

### 1. Physical Hardware: It is 1 Single Tri-Color LED

On the TM4C123 LaunchPad, **PF1 (Red), PF2 (Blue), and PF3 (Green)** are not three separate bulbs spaced apart on the board. They are three color chips inside **one single RGB LED package**.

When two color chips turn on at the exact same time, their light mixes:

* **Red + Green = Yellow**
* **Blue + Green = Cyan**
* **Red + Blue + Green = White**

Because `vHeartbeatTask` toggles the **Red LED outside of the Mutex**, the Red LED blinks on and off continuously every second. If you press SW1 while the Red LED happens to be ON, the physical LED turns **Yellow** (Red + Green) for 300 ms!

---

### 2. What the Mutex IS Doing (Blue vs. Green)

If you watch closely, **Blue and Green will NEVER turn on together.**

1. When you press SW1, `vProcessingTask` grabs `xSharedMutex` and turns on the Green LED for 300 ms.
2. If `vHeartbeatTask` tries to turn on the Blue LED during those 300 ms, it calls `xSemaphoreTake(xSharedMutex, 50)`.
3. Because the Green task holds the lock, the Blue task is rejected (times out after 50 ms) and skips turning on the Blue LED entirely.

The Mutex successfully prevented the Blue and Green tasks from running at the same time.

---

### 3. What Would Happen WITHOUT a Mutex? (Race Condition)

If we removed `xSharedMutex` completely:

1. **Register Overwriting (Corruption):**
In microcontrollers, setting GPIO pins requires reading the register, modifying the bit, and writing it back (**Read-Modify-Write**).
* Task A reads GPIO Port F (`0x02` - Red is ON).
* Right before Task A can set the Green bit, Task B interrupts it, reads `0x02`, sets the Blue bit (`0x06`), and writes `0x06` to hardware.
* Task A resumes, completes its old calculation, and writes `0x0A` (Red + Green), **accidentally wiping out the Blue LED write that Task B just performed!**


2. **Unpredictable Visual Glitches:**
Without the Mutex, rapid button presses would cause the LEDs to flicker, stay stuck ON, or miss state changes randomly because both tasks would constantly overwrite each other's register settings.

---

### Summary

* **Multiple colors at once?** Normal! That's the physical RGB LED mixing colors because Red blinks independently in the background.
* **The Mutex's job?** It prevents the tasks from fighting over hardware registers and ensures the Blue pulse and Green pulse never collide or corrupt each other.