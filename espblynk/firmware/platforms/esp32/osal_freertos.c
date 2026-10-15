/* FreeRTOS delay, monotonic-time, and task adapters for the OSAL contract. */
#include "osal.h"

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Preserve the caller's portable callback and argument in task-owned storage. */
typedef struct
{
    osal_task_fn_t task_function;
    void *argument;
} osal_task_context_t;

/* Release the startup context before entering the long-lived portable task. */
static void osal_task_entry(void *argument)
{
    osal_task_context_t task_context = *((osal_task_context_t *)argument);
    vPortFree(argument);
    task_context.task_function(task_context.argument);
    vTaskDelete(NULL);
}

/**
 * @brief Suspend the current FreeRTOS task for at least the requested duration.
 * @param[in] milliseconds Delay in milliseconds; zero yields for one tick.
 */
void osal_delay_ms(uint32_t milliseconds)
{
    TickType_t ticks = pdMS_TO_TICKS(milliseconds);
    vTaskDelay((ticks == 0U) ? 1U : ticks);
}

/**
 * @brief Return monotonic elapsed time since boot in wrapping milliseconds.
 * @return Milliseconds from the ESP timer converted to an unsigned 32-bit counter.
 */
uint32_t osal_uptime_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000LL);
}

/**
 * @brief Create a FreeRTOS task with an OSAL callback and caller argument.
 * @param[in] task_function Portable task function; must not be NULL.
 * @param[in] task_name Null-terminated task name.
 * @param[in] stack_bytes Requested stack size in bytes.
 * @param[in] argument Value passed unchanged to task_function.
 * @param[in] priority FreeRTOS task priority.
 * @return Non-null task handle on success, or NULL when validation/allocation fails.
 */
osal_task_t osal_task_create(osal_task_fn_t task_function,
                             const char *task_name,
                             size_t stack_bytes,
                             void *argument,
                             uint8_t priority)
{
    osal_task_context_t *task_context = NULL;
    TaskHandle_t task_handle = NULL;
    BaseType_t result = pdFAIL;

    if ((task_function != NULL) && (task_name != NULL) &&
        (stack_bytes >= sizeof(StackType_t)))
    {
        task_context = (osal_task_context_t *)pvPortMalloc(sizeof(*task_context));
        if (task_context != NULL)
        {
            task_context->task_function = task_function;
            task_context->argument = argument;
            result = xTaskCreate(osal_task_entry,
                                 task_name,
                                 (configSTACK_DEPTH_TYPE)(stack_bytes / sizeof(StackType_t)),
                                 task_context,
                                 (UBaseType_t)priority,
                                 &task_handle);

            if (result != pdPASS)
            {
                vPortFree(task_context);
                task_context = NULL;
            }
        }
    }

    return (result == pdPASS) ? (osal_task_t)task_handle : NULL;
}
