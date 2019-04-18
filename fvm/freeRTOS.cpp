#include <fvm.h>
extern FVM F;
void Task0(void *pvParameters) { 
  while (true) { int n=20; while(n--) F.update(); vTaskDelay(1); } // vTaskDelay(0) not working
}
void xxx0(){
  xTaskCreatePinnedToCore(
    Task0,            // pvTaskCode,Pointer to the task entry function
    "Task0",          // pcName, A descriptive name for the task.
    8192,             // usStackDepth, The size of the task stack specified as the number of bytes
    NULL,             // pvParameters, Pointer that will be used as the parameter for the task being created
    1,                // uxPriority, The priority at which the task should run
    NULL,             // pxCreatedTask, Used to pass back a handle by which the created task can be referenced
	0    // xCoreID,  Specify the number of the cpu core which the task should be pinned to
  );
}
/*
void xxx1(){
  xTaskCreatePinnedToCore(
    Task1,            // pvTaskCode,Pointer to the task entry function
    "Task0",          // pcName, A descriptive name for the task.
    1024,             // usStackDepth, The size of the task stack specified as the number of bytes
    NULL,             // pvParameters, Pointer that will be used as the parameter for the task being created
    1,                // uxPriority, The priority at which the task should run
    NULL,             // pxCreatedTask, Used to pass back a handle by which the created task can be referenced
	1    				// xCoreID,  Specify the number of the cpu core which the task should be pinned to
  );
}
*/