idf_component_register(
    SRCS "hello_world_main.c"
    INCLUDE_DIRS "."
    REQUIRES "driver" "led_strip"  # ← Agregar led_strip como requerimiento
)