#ifndef ABL_APP_H
#define ABL_APP_H

/**
 * @brief Точка входа приложения (реализуется проектом).
 *
 * D7: переносимый код проекта определяет abl_main(), а платформенные
 * трaмполины только вызывают её:
 *   - STM32/AVR: int main(void)  — в soc/ (startup вызывает main);
 *   - ESP32:     void app_main() — в target/esp32 (обёртка IDF, Шаг 9).
 *
 * Реализация обязана вызвать abl_target_init() до бизнес-логики.
 */
void abl_main(void);

#endif /* ABL_APP_H */
