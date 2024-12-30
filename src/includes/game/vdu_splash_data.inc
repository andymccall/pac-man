BITRIOTDEV_SMALL_LOGO_ID:         EQU 500
BITRIOTDEV_SMALL_LOGO_WIDTH:      EQU 130
BITRIOTDEV_SMALL_LOGO_HEIGHT:     EQU 10

vdu_splash_data:

    ; Splash logo
    macro_bitmap_make_buffered BITRIOTDEV_SMALL_LOGO_ID, BITRIOTDEV_SMALL_LOGO_WIDTH, BITRIOTDEV_SMALL_LOGO_HEIGHT, "src/assets/splash/small_bitriotdev.rgba2"
vdu_splash_data_end: