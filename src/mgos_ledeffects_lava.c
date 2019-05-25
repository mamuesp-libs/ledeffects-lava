#include "mgos.h"
#include "led_master.h"

static int color_start = 0;
static int color_end = 0;
static int color_range = 0;
static double color_divisor = 1.0;

void mgos_intern_lava_init(mgos_rgbleds* leds)
{
    leds->dim_all = mgos_sys_config_get_ledeffects_lava_dim_all();
    leds->timeout = mgos_sys_config_get_ledeffects_lava_timeout();

    color_start = mgos_sys_config_get_ledeffects_lava_color_start() % 256;
    color_end = mgos_sys_config_get_ledeffects_lava_color_end() % 256;
    color_range = color_end - color_start;
    color_divisor = mgos_sys_config_get_ledeffects_lava_color_divisor();
    color_divisor = color_divisor <= 0.0 ? 1.0 : color_divisor;
}

static void mgos_intern_lava_loop(mgos_rgbleds* leds)
{
    uint32_t num_rows = leds->panel_height;
    uint32_t num_cols = leds->panel_width;
    tools_rgb_data out_pix;
    leds->pix_pos = leds->pix_pos < 0 ? 255 : leds->pix_pos;

    int run = leds->internal_loops;
    run = run <= 0 ? 1 : run;

    while (run--) {
        for (int col = 0; col < num_cols; col++) {
            for (int row = 0; row < num_rows; row++) {
                int step = num_cols ? (256 / num_cols) : 0;
                int pix_pos = (leds->pix_pos + col * step) % 256;
                out_pix = tools_color_wheel(((row * 256 / num_rows) + pix_pos) & 255, 255.0);
                LOG(LL_VERBOSE_DEBUG, ("mgos_lava:\tR: 0x%.02X\tG: 0x%.02X\tB: 0x%.02X", out_pix.r, out_pix.g, out_pix.b));
                mgos_universal_led_plot_pixel(leds, col, row, out_pix, true);
            }
        }
        mgos_universal_led_show(leds);
        leds->pix_pos -= leds->internal_loops;
        leds->pix_pos = leds->pix_pos < 0 ? 255 : leds->pix_pos;
    }
}

void mgos_ledeffects_lava(void* param, mgos_rgbleds_action action)
{
    static bool do_time = false;
    static uint32_t max_time = 0;
    uint32_t time = (mgos_uptime_micros() / 1000);
    mgos_rgbleds* leds = (mgos_rgbleds*)param;

    switch (action) {
    case MGOS_RGBLEDS_ACT_INIT:
        LOG(LL_INFO, ("mgos_lava: called (init)"));
        mgos_intern_lava_init(leds);
        break;
    case MGOS_RGBLEDS_ACT_EXIT:
        LOG(LL_INFO, ("mgos_lava: called (exit)"));
        break;
    case MGOS_RGBLEDS_ACT_LOOP:
        mgos_intern_lava_loop(leds);
        if (do_time) {
            time = (mgos_uptime_micros() /1000) - time;
            max_time = (time > max_time) ? time : max_time;
            LOG(LL_VERBOSE_DEBUG, ("Lava loop duration: %d milliseconds, max: %d ...", time / 1000, max_time / 1000));
        }
        break;
    }
}

bool mgos_ledeffects_lava_init(void) {
  LOG(LL_INFO, ("mgos_lava_init ..."));
  ledmaster_add_effect("ANIM_LAVA", mgos_ledeffects_lava);
  return true;
}
